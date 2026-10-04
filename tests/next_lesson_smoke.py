"""在虚构档案与真实 HTTP/SSE 分片上验收下一课任务，不读取私人档案。"""
import io
import asyncio
import sqlite3
import json
import sys
import tempfile
import threading
import time
import urllib.error
import urllib.parse
from concurrent.futures import ThreadPoolExecutor
from contextlib import closing

import learning_flow_smoke as flow
import websockets
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")


class NextModel(flow.FlowModel):
    next_calls = []
    action = "advance"
    selected_course = "chem"
    gate_stage = ""
    gate_entered = threading.Event()
    gate_release = threading.Event()
    fail_stage = ""

    def do_POST(self):
        raw = self.rfile.read(int(self.headers["Content-Length"]))
        request = json.loads(raw)
        system = request["messages"][0]["content"]
        stage = "decision" if "用户主动要求准备下一课" in system else ""
        data = json.loads(request["messages"][-1]["content"]) if stage else None
        if "备课教师" in system and "当前板块的完整严格JSON" in system:
            data = json.loads(request["messages"][-1]["content"])
            stage = data["block"]
        if not stage:
            self.rfile = io.BytesIO(raw)
            return super().do_POST()
        NextModel.next_calls.append((stage, request, data))
        assert request["stream"] is True
        assert request["max_tokens"] == 8192
        if NextModel.gate_stage == stage:
            NextModel.gate_entered.set()
            NextModel.gate_release.wait(10)
        if NextModel.fail_stage == stage:
            self.send_response(503)
            self.end_headers()
            return
        if stage == "decision":
            candidates = [item for item in data["candidates"] if item["courseId"] == NextModel.selected_course]
            if NextModel.action == "advance":
                selected = next(item for item in candidates if item["kind"] == "lesson" and not item.get("completed") and item.get("canAdvance", True) and not item.get("requires"))
            elif NextModel.action == "review":
                selected = next(item for item in candidates if item["kind"] == "review")
            else:
                selected = next(item for item in candidates if item["kind"] == "lesson" and item.get("canConsolidate"))
            content = {"reason": "根据最新实际反馈准备下一课；未作答部分信息不足。", "taskId": selected["taskId"],
                       "action": NextModel.action, "minutes": 15, "instruction": "根据原文补充具体方法与练习", "knowledgePoints": [selected["title"]]}
        else:
            content = flow.block_data(stage, data["goal"] + " " + data["phase"] + " " + data["topic"])
            if stage == "examples":
                for item in content["examples"]: item["solution"] = "绝不能公开的标准答案"
            if stage == "practice":
                for item in content["practice"]: item["check"] = "绝不能公开的评分依据"
            if stage == "quiz":
                for item in content["quiz"]: item["explanation"] = "绝不能公开的测验解析"
        text = json.dumps(content, ensure_ascii=False)
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.end_headers()
        try:
            for offset in range(0, len(text), 13):
                chunk = {"model": "next-fixture-ai", "choices": [{"delta": {"content": text[offset:offset + 13]}}]}
                self.wfile.write(("data: " + json.dumps(chunk, ensure_ascii=False) + "\n\n").encode())
                self.wfile.flush()
                time.sleep(.008)
            self.wfile.write(("data: " + json.dumps({"model": "next-fixture-ai", "choices": [{"delta": {}, "finish_reason": "stop"}]}) + "\n\n").encode())
            self.wfile.write(b"data: [DONE]\n\n")
            self.wfile.flush()
        except OSError:
            pass


def wait_task(harness, task_id, wanted=None):
    def done():
        value = harness.http("/api/learn/next/" + task_id)
        if wanted:
            return value if value["status"] == wanted else None
        return value if value["status"] in ("ready", "failed", "cancelled", "stale") else None
    return flow.wait_for(done, 80)


def status_detail(harness, task):
    if task["status"] == "ready":
        return "ready"
    private = json.loads(harness.sql("SELECT payload FROM ClassroomActivity WHERE id=?", ("next-preparation:" + task["id"],))[0][0])
    versions = private.get("versions", {})
    changed = []
    current_revision = harness.sql("SELECT value FROM ProfileMeta WHERE key='learning-revision'")
    if current_revision and int(current_revision[0][0]) != versions.get("learningVersion"):
        changed.append(("learningVersion", versions.get("learningVersion"), current_revision[0][0]))
    for key, content in versions.get("sessions", {}).items():
        current = harness.sql("SELECT content FROM LearningSession WHERE id=?", (key,))
        if not current or current[0][0] != content:
            changed.append(("session", key))
    current_plan = json.loads(harness.sql("SELECT value FROM ProfileMeta WHERE key='learning-flow'")[0][0]).get("plan", {})
    if current_plan != versions.get("studyPlan"):
        changed.append(("plan", current_plan.get("version"), versions.get("studyPlan", {}).get("version")))
    return dict(status=task["status"], stage=task["stage"], message=task.get("message"), changed=changed)


async def replay_events(harness, task_id, after_seq):
    received = []
    async with websockets.connect(f"ws://127.0.0.1:{harness.port}/ws/learn/next") as socket:
        await socket.send(json.dumps(dict(type="subscribe", id=task_id, afterSeq=after_seq)))
        while True:
            event = json.loads(await asyncio.wait_for(socket.recv(), 10))
            received.append(event)
            if event["type"] in ("ready", "failed", "cancelled"):
                return received


def seed(h):
    # 固定无可靠画像的初始版本，避免无关的启动画像刷新改变备课版本。
    h.sql("UPDATE ProfileMeta SET value='1' WHERE key='assessed'")
    h.sql("INSERT INTO ProfileMeta VALUES('ability-profile',?) ON CONFLICT(key) DO UPDATE SET value=excluded.value",
          (json.dumps({"dimensions": [], "version": 1, "attemptVersion": 1, "error": "", "model": "fixture"}),))
    for course, goal, topics in [("chem", "高中化学氧化还原", ["化合价", "反应判断", "电子转移"]), ("math", "高中数学函数", ["函数定义", "单调性"])]:
        outline = {"title": goal, "summary": goal, "generation": {"source": "ai", "promptVersion": "ai-plan-v1"},
                   "courseStructure": [{"stage": "基础", "topics": topics}], "roadmap": [{"name": "基础", "topics": topics}]}
        h.sql("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)", (course, "fixture", None, goal, "deep", goal, goal, "ai", "active", "2026-10-04", "2026-10-04"))
        h.sql("INSERT INTO CourseSnapshot VALUES(?,?,?,?,?)", ("snapshot-" + course, course, 1, json.dumps(outline, ensure_ascii=False), "2026-10-04"))
    h.sql("INSERT INTO LearningCardProgress VALUES(?,?,?,?,?,?,?,?,?,?)", ("completed-chem", "chem", "fixture", "高中化学氧化还原", "deep", 1, "基础", 1, "化合价", "in_progress"))
    # 原课堂与旧来源保留，不允许巩固课覆盖它。
    blocks = {name: flow.block_data(name, "高中化学氧化还原 基础 化合价") for name in ("overview", "steps", "examples", "practice", "quiz", "assessment")}
    content = {"contentVersion": 1, "promptVersion": "ai-block-v1", "blocks": blocks, "references": [],
               "generations": {name: {"source": "ai", "model": "old-fixture", "promptVersion": "ai-block-v1"} for name in blocks}}
    h.sql("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", ("fixture-session", "chem", "fixture", "高中化学氧化还原", "deep", 1, "基础", 1,
          "化合价", "化合价", "", "", json.dumps(content, ensure_ascii=False), "[]", 0, "ai"))
    plan = {"availability": [{"weekday": 1, "minutes": 30}, {"weekday": 3, "minutes": 30}, {"weekday": 5, "minutes": 30}], "entries": [], "version": 1, "manual": True}
    h.sql("INSERT INTO ProfileMeta VALUES(?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value",
          ("learning-flow", json.dumps({"plan": plan, "status": "ready", "attemptedRevision": 1, "preparationAttemptedRevision": 1})))


def main(executable):
    flow.FlowModel = NextModel
    with tempfile.TemporaryDirectory(prefix="gangyi-next-") as directory:
        h = flow.Harness(executable, directory)
        try:
            time.sleep(2.5)
            seed(h)
            source_content = h.sql("SELECT content FROM LearningSession WHERE id='fixture-session'")[0][0]
            source = dict(courseId="chem", phaseIndex=1, topicIndex=1)
            body = dict(source, requestId="first-next", drafts=[dict(questionId="fixture-q", kind="practice", index=0, text="尚未发送的原文")])
            NextModel.gate_stage = "decision"
            with ThreadPoolExecutor(max_workers=4) as pool:
                created = list(pool.map(lambda _: h.http("/api/learn/next", body), range(4)))
            ids = {item["id"] for item in created}
            assert len(ids) == 1, created
            task_id = ids.pop()
            assert NextModel.gate_entered.wait(8)
            alias = dict(body, requestId="duplicate-click")
            assert h.http("/api/learn/next", alias)["id"] == task_id
            try:
                h.http("/api/learn/next", dict(body, topicIndex=2, requestId="wrong-source-click"))
                raise AssertionError("不同课堂来源不能复用在途任务")
            except urllib.error.HTTPError as error:
                assert error.code == 409
            assert h.http("/api/learn/next", alias)["id"] == task_id
            assert len(NextModel.next_calls) == 1, NextModel.next_calls
            NextModel.gate_stage = ""
            NextModel.gate_release.set()
            ready = wait_task(h, task_id)
            assert ready["status"] == "ready", ready
            calls = NextModel.next_calls[:]
            assert [item[0] for item in calls] == ["decision", "overview", "steps", "examples", "practice", "quiz", "assessment"], calls
            assert "绝不能公开" not in json.dumps(ready, ensure_ascii=False), ready
            assert "answerIndex" not in json.dumps(ready, ensure_ascii=False), ready
            seqs = [event["seq"] for event in ready["events"]]
            assert seqs == sorted(set(seqs)), seqs
            assert ready["events"][-1]["type"] == "ready"
            after_seq = ready["events"][len(ready["events"]) // 2]["seq"]
            replay = asyncio.run(replay_events(h, task_id, after_seq))
            assert all(event["seq"] > after_seq for event in replay), replay
            assert replay[-1]["type"] == "ready" and replay[-1]["seq"] == ready["latestSeq"]
            assert all(any(event["type"] == "delta" and event["stage"] == stage for event in ready["events"]) for stage in ("decision", "overview", "examples", "quiz"))
            assert h.http("/api/learn/next", body)["id"] == task_id
            assert len(NextModel.next_calls) == 7, "刷新或重复请求不得再次调用模型"
            params = urllib.parse.parse_qs(urllib.parse.urlsplit(ready["classroomUrl"]).query)
            lesson_id = params["lessonTaskId"][0]
            learned = h.http("/api/learn?" + urllib.parse.urlencode({key: value[0] for key, value in params.items()}))
            assert learned["blocks"]["overview"], learned
            assert len(NextModel.next_calls) == 7, "进入已准备课堂不能再次生成"
            private = json.loads(h.sql("SELECT payload FROM ClassroomActivity WHERE id=?", ("prepared-lesson:" + lesson_id,))[0][0])
            assert private["kind"] == "lesson"
            assert all(value["model"] == "next-fixture-ai" for value in private["content"]["generations"].values())
            assert h.sql("SELECT content FROM LearningSession WHERE courseId='chem' AND topicIndex=2"), "普通下一课应保存LearningSession"
            assert h.sql("SELECT status FROM LearningCardProgress WHERE id='completed-chem'")[0][0] == "in_progress", "允许正常推进但不强制完成来源课"
            assert "尚未发送的原文" in h.sql("SELECT value FROM ProfileMeta WHERE key='question-drafts:chem:1:1'")[0][0]

            # 不强制完成前两节，连续进入第三节仍由这次真实 AI 决策推进。
            third = h.http("/api/learn/next", dict(courseId="chem", phaseIndex=1, topicIndex=2, lessonTaskId=lesson_id,
                           contentVersion=private["content"]["contentVersion"], requestId="continuous-third"))
            third_ready = wait_task(h, third["id"])
            assert third_ready["status"] == "ready", status_detail(h, third_ready)
            assert "topicIndex=3" in third_ready["classroomUrl"]
            assert h.sql("SELECT content FROM LearningSession WHERE id='fixture-session'")[0][0] == source_content, "已抵达的原课堂不能重新推进覆盖"
            assert h.sql("SELECT status FROM LearningCardProgress WHERE id='completed-chem'")[0][0] == "in_progress"
            assert h.sql("SELECT COUNT(*) FROM LearningCardProgress")[0][0] == 1, "抵达标记不能增加完成计数"

            NextModel.action = "consolidate"
            original = h.sql("SELECT content FROM LearningSession WHERE id='fixture-session'")[0][0]
            consolidation = h.http("/api/learn/next", dict(source, requestId="consolidate-next"))
            consolidated = wait_task(h, consolidation["id"])
            assert consolidated["status"] == "ready", status_detail(h, consolidated)
            isolated = json.loads(h.sql("SELECT payload FROM ClassroomActivity WHERE id=?", ("prepared-lesson:" + consolidation["id"],))[0][0])
            assert isolated["kind"] == "consolidation"
            assert h.sql("SELECT content FROM LearningSession WHERE id='fixture-session'")[0][0] == original
            finish = dict(source, lessonTaskId=consolidation["id"], contentVersion=isolated["content"]["contentVersion"], status="completed")
            before = h.sql("SELECT COUNT(*) FROM LearningCardProgress")[0][0]
            h.http("/api/learn/progress", finish)
            revision = h.sql("SELECT value FROM ProfileMeta WHERE key='learning-revision'")[0][0]
            h.http("/api/learn/progress", finish)
            assert h.sql("SELECT value FROM ProfileMeta WHERE key='learning-revision'")[0][0] == revision
            assert h.sql("SELECT COUNT(*) FROM LearningCardProgress")[0][0] == before

            NextModel.action = "advance"
            NextModel.selected_course = "math"
            cross = h.http("/api/learn/next", dict(source, requestId="cross-next"))
            cross_ready = wait_task(h, cross["id"])
            assert cross_ready["status"] == "ready", status_detail(h, cross_ready)
            assert "courseId=math" in cross_ready["classroomUrl"]

            # 未完成评价先等待；失败后把原文和明确失败状态送给真实模型。
            pending_text = "已提交但评价失败的虚构原文：没有写解题过程"
            dialogue_key = "classroom:chem:1:1:practice:99:dialogue"
            pending = {"version": 1, "turns": [{"user": pending_text, "requestId": "fixture-pending", "status": "pending"}]}
            h.sql("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)", (dialogue_key, "chem", 1, 1, "dialogue", json.dumps(pending, ensure_ascii=False), "2026-10-04"))
            before_wait = len(NextModel.next_calls)
            waiting = h.http("/api/learn/next", dict(source, requestId="wait-evaluation"))
            wait_task(h, waiting["id"], "waiting_evaluation")
            time.sleep(.2)
            assert len(NextModel.next_calls) == before_wait, "评价在途不得提前决定下一课"
            pending["turns"][0].update(status="failed", error="评价失败，本次未形成可靠评价")
            h.sql("UPDATE ClassroomActivity SET payload=? WHERE id=?", (json.dumps(pending, ensure_ascii=False), dialogue_key))
            evaluated_next = wait_task(h, waiting["id"])
            assert evaluated_next["status"] == "ready", status_detail(h, evaluated_next)
            learning_input = NextModel.next_calls[before_wait][2]["latestLearning"]
            assert pending_text in json.dumps(learning_input, ensure_ascii=False)
            assert "failed" in json.dumps(learning_input)
            h.sql("DELETE FROM ClassroomActivity WHERE id=?", (dialogue_key,))

            # 近期可靠反馈触发画像时，等待终态与学习版本原子发布后才发七阶段请求。
            current_revision = int(h.sql("SELECT value FROM ProfileMeta WHERE key='revision'")[0][0])
            pending_revision = current_revision + 1
            with closing(sqlite3.connect(h.database, timeout=8)) as database, database:
                for key, value in [("revision", str(pending_revision)), ("assessed", str(current_revision)),
                                   ("profile-failed-revision", ""), ("profile-worker-active-revision", str(pending_revision)),
                                   ("ability-worker-active-revision", str(pending_revision))]:
                    database.execute("INSERT INTO ProfileMeta VALUES(?,?) ON CONFLICT(key) DO UPDATE SET value=excluded.value", (key, value))
                for index in range(3):
                    payload = {"phaseIndex": 1, "topicIndex": 1, "results": [{"answered": True, "credible": True,
                               "questionId": "recent-fixture-" + str(index), "givenAnswer": "近期可靠反馈" + str(index),
                               "correct": True, "evidenceSource": "ai-evaluation", "model": "fixture-ai"}]}
                    database.execute("INSERT INTO LearningInteraction VALUES(?,?,?,?,?,?,?)",
                                     ("recent-result-" + str(index), "chem", "recent-fiction", None, "question-evaluation", json.dumps(payload, ensure_ascii=False), "2026-10-04"))
            profile_start = len(NextModel.next_calls)
            profile_wait = h.http("/api/learn/next", dict(source, requestId="wait-current-profile"))
            wait_task(h, profile_wait["id"], "waiting_evaluation")
            time.sleep(.2)
            assert len(NextModel.next_calls) == profile_start, "画像在途不能先开始生成"
            with closing(sqlite3.connect(h.database, timeout=8)) as database, database:
                database.execute("UPDATE ProfileMeta SET value=? WHERE key='assessed'", (str(pending_revision),))
                database.execute("UPDATE ProfileMeta SET value=? WHERE key='ability-profile'",
                                 (json.dumps({"dimensions": [], "version": pending_revision, "attemptVersion": pending_revision, "error": "", "model": "fixture-ai"}),))
                database.execute("UPDATE ProfileMeta SET value=CAST(value AS INTEGER)+1 WHERE key='learning-revision'")
                database.execute("UPDATE ProfileMeta SET value='' WHERE key IN ('profile-worker-active-revision','ability-worker-active-revision')")
            profile_ready = wait_task(h, profile_wait["id"])
            assert profile_ready["status"] == "ready", status_detail(h, profile_ready)
            assert "近期可靠反馈2" in json.dumps(NextModel.next_calls[profile_start][2], ensure_ascii=False)

            NextModel.action = "review"; NextModel.selected_course = "chem"
            review = {"due": "2026-10-04", "day": 1, "status": "pending", "quiz": json.loads(original)["blocks"]["quiz"]["quiz"]}
            h.sql("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)", ("fixture-review", "chem", 1, 1, "review", json.dumps(review, ensure_ascii=False), "2026-10-04"))
            review_task = h.http("/api/learn/next", dict(source, requestId="review-next"))
            review_ready = wait_task(h, review_task["id"])
            assert review_ready["status"] == "ready", status_detail(h, review_ready)
            review_query = urllib.parse.parse_qs(urllib.parse.urlsplit(review_ready["classroomUrl"]).query)
            assert review_query["reviewId"] == ["fixture-review"] and review_query["review"] == ["1"]
            review_private = json.loads(h.sql("SELECT payload FROM ClassroomActivity WHERE id=?", ("prepared-lesson:" + review_task["id"],))[0][0])
            assert review_private["kind"] == "review" and review_private["reviewId"] == "fixture-review"
            NextModel.action = "advance"; NextModel.selected_course = "math"

            NextModel.gate_entered.clear(); NextModel.gate_release.clear(); NextModel.gate_stage = "overview"
            stale = h.http("/api/learn/next", dict(source, requestId="stale-next"))
            assert NextModel.gate_entered.wait(8)
            h.sql("UPDATE ProfileMeta SET value=CAST(value AS INTEGER)+1 WHERE key='learning-revision'")
            NextModel.gate_stage = ""; NextModel.gate_release.set()
            stale_result = wait_task(h, stale["id"])
            assert stale_result["status"] == "stale", stale_result
            assert not h.sql("SELECT payload FROM ClassroomActivity WHERE id=?", ("prepared-lesson:" + stale["id"],))

            NextModel.fail_stage = "practice"
            start_count = len(NextModel.next_calls)
            failed = h.http("/api/learn/next", dict(source, requestId="fail-next"))
            failed_result = wait_task(h, failed["id"])
            assert failed_result["status"] == "failed", failed_result
            assert [call[0] for call in NextModel.next_calls[start_count:]].count("practice") == 1, "失败不得循环重试"
            assert "classroomUrl" not in failed_result
            NextModel.fail_stage = ""
            flow.wait_for(lambda: h.http("/api/learn/next/" + failed["id"] + "/retry", {}), 5)
            retried = wait_task(h, failed["id"])
            assert retried["status"] == "ready", retried
            assert retried["attempt"] == 2
            assert not any(event["type"] == "failed" for event in retried["events"])

            NextModel.gate_entered.clear(); NextModel.gate_release.clear(); NextModel.gate_stage = "overview"
            cancelling = h.http("/api/learn/next", dict(source, requestId="cancel-next"))
            assert NextModel.gate_entered.wait(8)
            cancelled = h.http("/api/learn/next/" + cancelling["id"] + "/cancel", {})
            assert cancelled["status"] == "cancelled", cancelled
            NextModel.gate_stage = ""; NextModel.gate_release.set()
            time.sleep(.2)
            assert not h.sql("SELECT payload FROM ClassroomActivity WHERE id=?", ("prepared-lesson:" + cancelling["id"],))
            print("NEXT_LESSON_SMOKE_OK: 七阶段真实分片、公开隐藏、幂等、刷新重连、巩固隔离、跨课程、评价在途/失败原文、版本失效、失败重试与取消", flush=True)
        finally:
            NextModel.gate_release.set()
            try:
                h.close()
            finally:
                if h.process.poll() is None:
                    h.process.kill()
                    h.process.wait(timeout=10)
                h.log.close()


if __name__ == "__main__":
    main(sys.argv[1])
