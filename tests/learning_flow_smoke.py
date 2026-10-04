"""在隔离虚构档案中验证连续对话、动态备课、共享预算、预览和版本保护。"""
import asyncio
import io
import json
import os
import sqlite3
import subprocess
import sys
import tempfile
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
from contextlib import closing
from http.server import ThreadingHTTPServer
from pathlib import Path
import websockets
if hasattr(sys.stdout, 'reconfigure'): sys.stdout.reconfigure(encoding='utf-8')
from classroom_api_smoke import MockAI, free_port

def wait_for(predicate, seconds=30):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        try:
            value = predicate()
            if value:
                return value
        except (OSError, ValueError):
            pass
        time.sleep(.1)
    raise AssertionError("后台任务未在期限内完成")

def block_data(name, base):
    if name == "overview":
        return {"title": base, "summary": base + " 从基础判断开始", "inferredDomain": "化学",
                "keyConcepts": ["化合价", "升降", "电子"]}
    if name == "steps":
        return {"lessonSteps": [{"title": "分步" + str(i), "explanation": base + " 根据新反馈先标化合价",
                "example": "Zn + 2HCl → ZnCl₂ + H₂↑", "action": "先判断变化", "check": "说明依据"} for i in range(4)]}
    if name == "examples":
        return {"examples": [{"title": "例题" + str(i), "content": base + " 判断化合价变化" + str(i), "solution": "先标化合价再判断升降"} for i in range(2)]}
    if name == "practice":
        return {"practice": [{"title": "练习" + str(i), "task": base + " 逐项标出化合价" + str(i), "check": "对比反应前后"} for i in range(3)]}
    if name == "quiz":
        return {"quiz": [{"question": base + " 新练习" + str(i), "options": ["升高", "降低", "不变", "无法判断"],
                "answerIndex": 0, "explanation": "对比前后化合价的变化"} for i in range(3)]}
    return {"checkpoint": [base + " 给出判断依据", "说清电子得失"], "commonMistakes": ["只看氧气", "忽略变化"], "resourceSummary": "复习化合价"}

class FlowModel(MockAI):
    calls = []
    failure = ""
    blocked = threading.Event()
    release = threading.Event()
    block_task = ""
    def do_POST(self):
        raw = self.rfile.read(int(self.headers["Content-Length"]))
        body = json.loads(raw)
        system = body["messages"][0]["content"]
        FlowModel.calls.append(body)
        if body.get("stream"):
            if FlowModel.failure == "stream":
                self.send_response(503); self.end_headers(); return
            chunks = ["先看化合价有没有变化。", "铁从零价变成二价，是升高还是降低？"]
            self.send_response(200); self.send_header("Content-Type", "text/event-stream"); self.end_headers()
            for chunk in chunks:
                try:
                    self.wfile.write(("data: " + json.dumps({"model": "fixture-ai", "choices": [{"delta": {"content": chunk}}]}, ensure_ascii=False) + "\n\n").encode())
                    self.wfile.flush(); time.sleep(.15)
                except OSError: return
            try: self.wfile.write(b"data: [DONE]\n\n"); self.wfile.flush()
            except OSError: pass
            return
        kind = ("evaluate" if "课堂作答评价教师" in system else "plan" if "学习统筹教师" in system else
                "preview" if "课程路线预览教师" in system else "prepare" if "动态备课教师" in system else "")
        if not kind:
            self.rfile = io.BytesIO(raw); return super().do_POST()
        data = json.loads(body["messages"][-1]["content"])
        if FlowModel.block_task == kind:
            FlowModel.blocked.set(); FlowModel.release.wait(10)
        if FlowModel.failure == kind:
            self.send_response(503); self.end_headers(); return
        if kind == "evaluate":
            unknown = "不会" in data["studentAnswer"] or "不知道" in data["studentAnswer"]
            result = {"isAnswer": data.get("intent") != "ask", "correct": not unknown,
                      "unknown": unknown, "confidence": .95,
                      "feedback": "先从化合价是否变化开始，说明判断依据。", "misconception": "化合价的升降还不熟悉" if unknown else ""}
        elif kind == "preview":
            stages = data["outline"]["roadmap"]
            result = {"slides": [{"phaseIndex": i, "title": "课程总览" if i == 0 else stage["name"],
                "content": "先学化合价变化，再用真实反应判断氧化还原与电子转移。", "bullets": ["标出化合价", "比较升降并解释"]}
                for i, stage in enumerate([{}] + stages)]}
        elif kind == "prepare":
            base = data["goal"] + " " + data["phase"] + " " + data["topic"] + " 新补弱"
            result = {name: block_data(name, base) for name in data["schemas"]}
        else:
            candidates = data["candidates"]
            reviews = [x for x in candidates if x["kind"] == "review"]
            lessons = [x for x in candidates if x["kind"] != "review"]
            # 测试模型明确优先化学课程，程序仍独立验证候选、预算和前置关系。
            lessons.sort(key=lambda x: (x["courseId"] != "chem", x["phaseIndex"], x["topicIndex"]))
            ordered = reviews + lessons
            result = {"order": [{"taskId": x["taskId"], "minutes": min(10 if x["kind"] == "review" else 15, max(slot["minutes"] for slot in data["availability"]))} for x in ordered],
                      "teaching": [{"courseId": "chem", "phaseIndex": 1, "topicIndex": 1,
                          "instruction": "先补讲化合价的升降，再用更简单的反应判断。", "supplement": "铁由零价变为二价，化合价升高。"}],
                      "nextTaskId": ordered[0]["taskId"] if ordered else "",
                      "reason": "根据真实反馈先补弱与到期复习，再推进未完成课时。"}
        response = json.dumps({"model": "fixture-ai", "choices": [{"message": {"content": json.dumps(result, ensure_ascii=False)}, "finish_reason": "stop"}]}, ensure_ascii=False).encode()
        self.send_response(200); self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(response))); self.end_headers()
        try: self.wfile.write(response)
        except OSError: pass

class Harness:
    def __init__(self, executable, root):
        self.executable = Path(executable).resolve()
        self.database = Path(root) / "fixture.db"
        self.port = free_port()
        self.base = f"http://127.0.0.1:{self.port}"
        self.model = ThreadingHTTPServer(("127.0.0.1", 0), FlowModel)
        self.thread = threading.Thread(target=self.model.serve_forever, daemon=True); self.thread.start()
        self.env = dict(os.environ, HOST="127.0.0.1", PORT=str(self.port), DATABASE_PATH=str(self.database),
            AI_API_KEY="fixture-key", AI_BASE_URL=f"http://127.0.0.1:{self.model.server_port}/v1",
            GANGYI_FLOW_TODAY="2026-10-04", LOCAL_CONTROL_TOKEN="flow-test")
        self.start()
    def start(self):
        self.log = (self.database.parent / "service.log").open("ab")
        self.process = subprocess.Popen([str(self.executable)], cwd=self.executable.parent, env=self.env,
            stdout=self.log, stderr=subprocess.STDOUT,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        wait_for(lambda: self.http("/health"), 10)
    def http(self, path, data=None, method=None):
        raw = None if data is None else json.dumps(data, ensure_ascii=False).encode()
        req = urllib.request.Request(self.base + path, data=raw, method=method,
            headers={"Content-Type": "application/json", "X-Gangyi-Control-Token": "flow-test"})
        try:
            with urllib.request.urlopen(req, timeout=15) as response: return json.loads(response.read())
        except urllib.error.HTTPError as error:
            print("FLOW_HTTP", path, error.code, error.read().decode(), flush=True)
            raise
    def stop(self):
        try: self.http("/internal/shutdown", {}, "POST")
        except OSError: pass
        self.process.wait(timeout=15)
        self.log.close()
    def close(self):
        if self.process.poll() is None: self.stop()
        self.model.shutdown(); self.model.server_close(); self.thread.join(timeout=2)
    def sql(self, statement, values=()):
        with closing(sqlite3.connect(self.database, timeout=8)) as db, db:
            return db.execute(statement, values).fetchall()
    def revision(self):
        self.sql("INSERT INTO ProfileMeta VALUES('learning-revision','2') ON CONFLICT(key) DO UPDATE SET value=CAST(value AS INTEGER)+1")
    def plan(self):
        return wait_for(lambda: (value if (value := self.http("/api/study-plan"))["status"] != "pending" else None))
    def view(self, kind="diagnostic", index=0):
        return self.http("/api/classroom/dialogue?" + urllib.parse.urlencode(dict(courseId="chem", phaseIndex=1, topicIndex=1, kind=kind, index=index)))
    async def dialogue(self, text, request_id, intent="answer", stop=False, version=None):
        view = self.view()
        message = dict(type="ask", courseId="chem", phaseIndex=1, topicIndex=1, kind="diagnostic", index=0,
            questionId=view["questionId"], version=view["version"] if version is None else version,
            requestId=request_id, question=text, answer=text, intent=intent)
        async with websockets.connect(f"ws://127.0.0.1:{self.port}/ws/classroom") as socket:
            await socket.send(json.dumps(message, ensure_ascii=False))
            events = []
            while True:
                event = json.loads(await asyncio.wait_for(socket.recv(), 15)); events.append(event)
                if stop and event["type"] == "delta": await socket.send(json.dumps({"type": "stop"}))
                if event["type"] in ("done", "error"): return events

def main(executable):
    with tempfile.TemporaryDirectory(prefix="gangyi-flow-") as directory:
        h = Harness(executable, directory)
        try:
            for course, goal in [("chem", "高中化学氧化还原"), ("math", "高中数学函数")]:
                outline = {"title": goal, "summary": goal, "generation": {"source": "ai", "promptVersion": "ai-plan-v1"},
                    "courseStructure": [{"stage": "基础", "topics": ["化合价", "反应判断"]}, {"stage": "迁移", "topics": ["电子转移"]}],
                    "roadmap": [{"name": "基础", "topics": ["化合价", "反应判断"]}, {"name": "迁移", "topics": ["电子转移"]}]}
                h.sql("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)", (course, "fixture", None, goal, "deep", goal, goal, "ai", "active", "2026-10-04", "2026-10-04"))
                h.sql("INSERT INTO CourseSnapshot VALUES(?,?,?,?,?)", ("snapshot-" + course, course, 1, json.dumps(outline, ensure_ascii=False), "2026-10-04"))
            blocks = {name: block_data(name, "高中化学氧化还原 基础 化合价") for name in ("overview", "steps", "examples", "practice", "quiz", "assessment")}
            content = {"contentVersion": 1, "promptVersion": "ai-block-v1", "blocks": blocks, "references": [],
                "generations": {name: {"source": "ai", "model": "fixture-ai", "promptVersion": "ai-block-v1"} for name in blocks}}
            h.sql("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", ("fixture-session", "chem", "fixture",
                "高中化学氧化还原", "deep", 1, "基础", 1, "化合价", "化合价", "", "", json.dumps(content, ensure_ascii=False), "[]", 0, "ai"))
            # 测试装入课堂后立即登记展示，避免后面的压力验收期间后台先完成重备。
            h.http("/api/learn/exposure", dict(courseId="chem", phaseIndex=1, topicIndex=1, contentVersion=1, blocks=["steps"]))
            for index in range(3):
                question = {"question": f"解释化合价升降 {index}", "type": "open", "rubric": "说明反应前后元素化合价变化", "status": "pending"}
                h.sql("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)", (f"classroom:chem:1:1:diagnostic:{index}", "chem", 1, 1, "diagnostic", json.dumps(question, ensure_ascii=False), "2026-10-04"))
            # 并发发布评价时，读取必须是同一数据库快照，不能出现已完成却缺少反馈。
            atomic_key = "classroom:chem:1:1:interaction:2"
            atomic_question = {"question": "虚构并发评价题", "type": "open", "status": "pending"}
            h.sql("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)", (atomic_key, "chem", 1, 1, "interaction", json.dumps(atomic_question), "2026-10-04"))
            h.sql("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)", (atomic_key + ":evaluation", "chem", 1, 1, "evaluation", '{"status":"waiting","dialogueVersion":0}', "2026-10-04"))
            atomic_stop = threading.Event(); atomic_errors = []
            def publish_atomic_evaluations():
                try:
                    with closing(sqlite3.connect(h.database, timeout=8)) as connection:
                        while not atomic_stop.is_set():
                            for ready in (False, True):
                                question = dict(atomic_question)
                                if ready: question.update(status="answered", correct=False, feedback="这是本轮真实评价的反馈。")
                                with connection:
                                    connection.execute("UPDATE ClassroomActivity SET payload=? WHERE id=?", (json.dumps(question), atomic_key))
                                    connection.execute("UPDATE ClassroomActivity SET payload=? WHERE id=?", (json.dumps({"status": "ready" if ready else "waiting", "dialogueVersion": 0}), atomic_key + ":evaluation"))
                                time.sleep(.002)
                except Exception as error: atomic_errors.append(error)
            atomic_writer = threading.Thread(target=publish_atomic_evaluations); atomic_writer.start()
            try:
                for _ in range(250):
                    value = h.view("interaction", 2)
                    if value["evaluationStatus"] == "ready":
                        assert value.get("correct") is False and value.get("feedback"), "已评价状态不能读取旧题目反馈"
            finally:
                atomic_stop.set(); atomic_writer.join(timeout=10)
            assert not atomic_writer.is_alive() and not atomic_errors, atomic_errors
            h.sql("DELETE FROM ClassroomActivity WHERE id IN (?,?)", (atomic_key, atomic_key + ":evaluation"))
            h.revision()
            # 显示过的讲解不得被新的后台模型结果覆盖。
            plan = h.plan(); assert plan["status"] == "ready", plan
            assert set(x["courseId"] for x in plan["entries"]) == {"chem", "math"}
            assert all(x["date"] >= "2026-10-05" for x in plan["entries"])
            assert plan["weekStart"] == "2026-10-05"
            totals = {}
            for entry in plan["entries"]: totals[entry["date"]] = totals.get(entry["date"], 0) + entry["minutes"]
            assert all(x <= 30 for x in totals.values()), totals
            wait_for(lambda: json.loads(h.sql("SELECT content FROM LearningSession WHERE id='fixture-session'")[0][0]).get("contentVersion", 1) > 1)
            changed = json.loads(h.sql("SELECT content FROM LearningSession WHERE id='fixture-session'")[0][0])
            assert changed["blocks"]["steps"] == blocks["steps"]
            assert changed["blocks"]["quiz"] != blocks["quiz"]
            # 即使页面版本已刷新，旧题快照仍必须被拒绝；只改其他板块不阻止原题提交。
            stale_quiz = dict(courseId="chem", phaseIndex=1, topicIndex=1, contentVersion=changed["contentVersion"],
                              questions=blocks["quiz"]["quiz"], answers=[None] * len(blocks["quiz"]["quiz"]))
            try:
                h.http("/api/quiz-attempts", stale_quiz)
                raise AssertionError("旧题不应按新题判分")
            except urllib.error.HTTPError as error:
                assert error.code == 409
            assert h.sql("PRAGMA user_version")[0][0] == 6
            # 真实预览缓存：全部阶段，刷新读取不额外请求。
            h.http("/api/courses/chem/preview", {})
            preview = wait_for(lambda: (x if (x := h.http("/api/courses/chem/preview"))["status"] == "ready" else None))
            assert [x["phaseIndex"] for x in preview["slides"]] == [0, 1, 2] and preview["source"] == "ai"
            count = len(FlowModel.calls)
            for _ in range(4): h.http("/api/courses/chem/preview")
            assert len(FlowModel.calls) == count
            # 连续对话、可靠评价和同题去重。
            events = asyncio.run(h.dialogue("我不知道", "turn-1")); assert events[-1]["type"] == "done", events
            view = wait_for(lambda: (x if (x := h.view())["evaluationStatus"] == "ready" else None))
            assert len(view["turns"]) == 1 and "升高" in view["turns"][0]["assistant"]
            item = json.loads(h.sql("SELECT payload FROM ClassroomActivity WHERE id='classroom:chem:1:1:diagnostic:0'")[0][0])
            assert item["unknown"] and item["credible"] and item["evidenceSource"] == "ai-evaluation"
            count = len(FlowModel.calls)
            asyncio.run(h.dialogue("我不知道", "turn-1")); assert len(h.view()["turns"]) == 1
            # 连续创建和关闭缓存对话连接，验证后台发送与关闭交错时的生命周期。
            for _ in range(20):
                cached = asyncio.run(h.dialogue("我不知道", "turn-1"))
                assert cached[-1].get("cached") and h.process.poll() is None
            assert len([x for x in FlowModel.calls[count:] if x.get("stream")]) == 0
            events = asyncio.run(h.dialogue("升高", "turn-2")); assert events[-1]["type"] == "done"
            wait_for(lambda: h.view()["evaluationStatus"] == "ready")
            item = json.loads(h.sql("SELECT payload FROM ClassroomActivity WHERE id='classroom:chem:1:1:diagnostic:0'")[0][0])
            assert item["assisted"]
            assert h.sql("SELECT COUNT(*) FROM ClassroomActivity WHERE courseId='chem' AND kind='diagnostic' AND payload LIKE '%\"status\":\"answered\"%'")[0][0] == 1
            stale = asyncio.run(h.dialogue("其他窗口", "stale", version=0)); assert stale[-1]["type"] == "error"
            # 旧评价请求返回前出现新作答，旧分数不能覆盖新一轮的明确不会。
            FlowModel.block_task = "evaluate"; FlowModel.blocked.clear(); FlowModel.release.clear()
            asyncio.run(h.dialogue("旧请求回答", "obsolete-evaluation"))
            assert FlowModel.blocked.wait(5), "旧评价请求应进入网络等待"
            asyncio.run(h.dialogue("我不知道", "current-evaluation"))
            FlowModel.block_task = ""; FlowModel.release.set()
            wait_for(lambda: h.view()["evaluationStatus"] == "ready")
            latest = json.loads(h.sql("SELECT payload FROM ClassroomActivity WHERE id='classroom:chem:1:1:diagnostic:0'")[0][0])
            assert latest["unknown"] and not latest["correct"] and latest["answer"] == "我不知道"
            # 停止不计评价；同请求重试只保存一轮。
            events = asyncio.run(h.dialogue("停止测试", "stop", stop=True)); assert events[-1].get("cancelled")
            assert h.view()["turns"][-1]["status"] == "failed"
            events = asyncio.run(h.dialogue("停止测试", "stop")); assert events[-1]["type"] == "done"
            assert len([x for x in h.view()["turns"] if x["requestId"] == "stop"]) == 1
            wait_for(lambda: h.view()["evaluationStatus"] == "ready")
            # 失败保留成绩，同一失败版本不循环请求；提供显式重试。
            FlowModel.failure = "evaluate"
            asyncio.run(h.dialogue("再次解释", "failure"))
            wait_for(lambda: h.view()["evaluationStatus"] == "waiting")
            count = len([x for x in FlowModel.calls if "课堂作答评价教师" in x["messages"][0]["content"]])
            time.sleep(1.5); assert len([x for x in FlowModel.calls if "课堂作答评价教师" in x["messages"][0]["content"]]) == count
            FlowModel.failure = ""
            h.http("/api/classroom/evaluation/retry", dict(courseId="chem", phaseIndex=1, topicIndex=1, kind="diagnostic", index=0))
            wait_for(lambda: h.view()["evaluationStatus"] == "ready")
            # 等待本轮画像任务结束，避免测试把正确的证据版本冲突当成取消失败。
            settled = {"version": None, "since": time.monotonic()}
            def profile_settled():
                meta = dict(h.sql("SELECT key,value FROM ProfileMeta"))
                revision = int(meta.get("revision", 0)); learning_revision = int(meta.get("learning-revision", 1))
                state = json.loads(meta.get("learning-flow", "{}"))
                profile_done = int(meta.get("assessed", 0)) >= revision or meta.get("profile-failed-revision") == str(revision)
                abilities_done = json.loads(meta.get("ability-profile", "{}")).get("attemptVersion", 0) >= revision
                preparation_done = state.get("status") == "ready" and state.get("preparationAttemptedRevision") == learning_revision
                version = (revision, learning_revision, state.get("plan", {}).get("version"))
                if settled["version"] != version:
                    settled.update(version=version, since=time.monotonic())
                return profile_done and abilities_done and preparation_done and time.monotonic() - settled["since"] >= 3
            wait_for(profile_settled)
            # 未保存的时间也用于预览；取消保留，确认同一结果。
            plan = h.plan()
            availability = [{"weekday": 1, "minutes": 45}, {"weekday": 3, "minutes": 45}, {"weekday": 5, "minutes": 45}]
            h.http("/api/study-plan/draft", dict(clientId="editor", active=True, version=plan["version"], availability=availability, entries=plan["entries"]))
            h.http("/api/study-plan/replan", dict(version=plan["version"], availability=availability, entries=plan["entries"], preview=True))
            proposal = wait_for(lambda: h.http("/api/study-plan").get("proposal"))
            assert proposal["plan"]["availability"] == availability
            h.http("/api/study-plan/cancel", dict(version=plan["version"], proposalId=proposal["id"]))
            assert h.http("/api/study-plan")["entries"] == plan["entries"]
            h.http("/api/study-plan/replan", dict(version=plan["version"], availability=availability, entries=plan["entries"], preview=True))
            proposal = wait_for(lambda: h.http("/api/study-plan").get("proposal"))
            accepted = h.http("/api/study-plan/confirm", dict(version=plan["version"], proposalId=proposal["id"]))["plan"]
            assert accepted["entries"] == proposal["plan"]["entries"] and accepted["version"] == plan["version"] + 1
            try:
                h.http("/api/study-plan/confirm", dict(version=plan["version"], proposalId=proposal["id"]))
                raise AssertionError("旧版本不应覆盖已确认计划")
            except urllib.error.HTTPError as error: assert error.code == 409
            h.http("/api/study-plan/draft", dict(clientId="editor", active=False))
            # 已在学课时仍计入预算；缩减到无法容纳时不得保存超预算新计划。
            h.http("/api/study-plan/replan", dict(version=accepted["version"], availability=[{"weekday":1,"minutes":10},{"weekday":3,"minutes":10},{"weekday":5,"minutes":10}]))
            insufficient = h.plan()
            assert insufficient["status"] == "waiting" and insufficient["entries"] == accepted["entries"]
            FlowModel.failure = "plan"
            h.http("/api/study-plan/replan", dict(version=accepted["version"]))
            failed = h.plan(); assert failed["status"] == "waiting" and failed["entries"] == accepted["entries"]
            count = len([x for x in FlowModel.calls if "学习统筹教师" in x["messages"][0]["content"]])
            for _ in range(4): h.http("/api/study-plan"); h.http("/api/home/next-step")
            time.sleep(1)
            assert len([x for x in FlowModel.calls if "学习统筹教师" in x["messages"][0]["content"]]) == count
            FlowModel.failure = ""
            # 重启恢复对话与计划；普通读取不生成新结果。
            h.stop(); h.start()
            assert len(h.view()["turns"]) == 6 and h.http("/api/study-plan")["entries"] == accepted["entries"]
            print("LEARNING_FLOW PASS: dialogue/retry/stop/idempotency/versions, AI preparation/exposure, shared budget/rollover, drafts/preview/CAS, cache/restart/failure")
        except Exception:
            print("FLOW_STATE", h.http("/api/study-plan"), flush=True)
            print("FLOW_META", h.sql("SELECT key,value FROM ProfileMeta WHERE key IN ('learning-flow','learning-revision','revision','assessed')"), flush=True)
            print("FLOW_TASKS", [call["messages"][0]["content"][:30] for call in FlowModel.calls], flush=True)
            print("FLOW_LOG", (h.database.parent / "service.log").read_text(encoding="utf8", errors="replace")[-6000:], flush=True)
            raise
        finally:
            FlowModel.failure = ""; FlowModel.release.set(); h.close()

if __name__ == "__main__":
    main(sys.argv[1])
