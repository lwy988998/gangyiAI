"""在隔离数据库和模拟模型上验证课堂流程及流式问答。"""

import asyncio
import datetime
import json
import os
import socket
import sqlite3
import subprocess
import tempfile
import threading
import time
import urllib.request
import urllib.parse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from contextlib import closing
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import websockets


def free_port():
    with socket.socket() as connection:
        connection.bind(("127.0.0.1", 0))
        return connection.getsockname()[1]


def request(base, path, data=None):
    body = None if data is None else json.dumps(data, ensure_ascii=False).encode()
    req = urllib.request.Request(base + path, data=body,
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=25) as response:
        result = json.loads(response.read())
    if path == "/api/classroom/submit":
        query = urllib.parse.urlencode({key: data[key] for key in ("courseId", "phaseIndex", "topicIndex", "kind", "index")})
        for _ in range(180):
            result = request(base, "/api/classroom/dialogue?" + query)
            if result["evaluationStatus"] != "pending":
                return result
            time.sleep(.1)
        raise AssertionError("后台评价未完成")
    return result


class MockAI(BaseHTTPRequestHandler):
    remedial_calls = 0
    stream_messages = []
    learn_block_calls = 0
    requests = []
    def log_message(self, *_):
        pass

    def do_POST(self):
        body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
        MockAI.requests.append(body)
        if body.get("stream"):
            MockAI.stream_messages = body.get("messages", [])
            stream_prompt = body["messages"][-1]["content"]
            if ("思考测试" in stream_prompt or
                    "停止界面测试" in stream_prompt):
                time.sleep(3.0)
                chunks = ["**重点**：把式子整理成 ", "$x^2+y^2=r^2$",
                          " 的形式。", "\n\n- 第一步\n- 第二步"]
            else:
                chunks = ["片段一", "片段二"]
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.end_headers()
            for chunk in chunks:
                event = {"model": "mock", "choices": [{"delta": {"content": chunk}}]}
                try:
                    self.wfile.write(("data: " + json.dumps(event, ensure_ascii=False) + "\n\n").encode())
                    self.wfile.flush()
                except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                    break
                time.sleep(.15)
            try:
                if body["messages"][-1]["content"] != "中断测试":
                    self.wfile.write(b"data: [DONE]\n\n"); self.wfile.flush()
            except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
                pass
            return
        prompt = body["messages"][-1]["content"]
        system = body['messages'][0]['content']
        if '首页学习目标推荐助手' in system:
            answer = {mode: {'continue': [f'{mode} 继续学习{i}' for i in range(3)],
                             'explore': [f'{mode} 探索方向{i}' for i in range(2)]} for mode in ('lite', 'deep')}
            answer['deep']['continue'][0] = '<img src=x onerror=window.homeInjected=1>'
        elif '"suggestions"' in prompt:
            answer = {"suggestions": ["先讲这一步的由来", "换个数再算一遍", "这类题的通用解法是什么"]}
        if '首页学习目标推荐助手' in system or '"suggestions"' in prompt:
            payload = json.dumps({"model": "mock", "choices": [{"message": {"content": json.dumps(answer, ensure_ascii=False)}}]}, ensure_ascii=False).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
            return
        parsed_prompt = json.loads(prompt) if prompt.startswith("{") else {}
        if '课程路线预览教师' in system:
            stages = parsed_prompt['outline']['roadmap']
            answer = {'slides': [{'phaseIndex': index, 'title': '总览' if index == 0 else stages[index-1]['name'],
                'content': '本阶段通过具体定义、例题和判断练习建立知识关系。', 'bullets': ['先理解定义，再做诊断练习']} for index in range(len(stages)+1)]}
        elif '学习统筹教师' in system:
            candidates = parsed_prompt['candidates']
            maximum = max(slot['minutes'] for slot in parsed_prompt['availability'])
            answer = {'order': [{'taskId': item['taskId'], 'minutes': min(15, maximum)} for item in candidates[:20]],
                      'teaching': [], 'nextTaskId': candidates[0]['taskId'] if candidates else '',
                      'reason': '结合真实反馈，先复习薄弱知识，再推进后续学习。'}
        elif '课堂作答评价教师' in system:
            given = parsed_prompt['givenAnswer']; question = parsed_prompt['question']
            answer = {'isAnswer': parsed_prompt.get('intent', 'answer') != 'ask',
                      'correct': isinstance(given, int) and given == question.get('answerIndex', 0),
                      'unknown': isinstance(given, str) and '不会' in given,
                      'confidence': .4 if question.get('type') == 'open' else .95,
                      'feedback': '先判断区间和定义，再给出具体依据。', 'misconception': '定义不够清楚'}
        elif '课堂教师，正在一道真实题目旁' in system:
            answer = '先理解区间上的变化，再回答一个小问题：函数值在增大还是减小？'
        elif '高中课堂出题教师' in system:
            answer = {'questions': [{'question': 'diagnostic' + str(i), 'options': ['正确', '错误', '其他', '不确定'],
                                    'answerIndex': 0, 'explanation': '检查定义与条件。'} for i in range(3)]}
        elif isinstance(parsed_prompt, dict) and parsed_prompt.get("block"):
            MockAI.learn_block_calls += 1
            block = parsed_prompt["block"]
            base = f'{parsed_prompt["goal"]} {parsed_prompt["phase"]} {parsed_prompt["topic"]}'
            if block == "overview":
                answer = {"title": base, "summary": base + " 概览", "inferredDomain": "数学",
                          "keyConcepts": ["定义", "区间", "变化"]}
            elif block == "steps":
                answer = {"lessonSteps": [{"title": f"步骤{i}", "explanation": base + f" 解释{i}",
                          "example": f"示例{i}", "action": f"完成练习{i}", "check": f"检查{i}"} for i in range(4)]}
            elif block == "examples":
                answer = {"examples": [{"title": f"例题{i}", "content": base + f" 题目{i}",
                          "solution": f"解析{i}"} for i in range(2)]}
            elif block == "practice":
                answer = {"practice": [{"title": f"练习{i}", "task": base + f" 任务{i}",
                          "check": f"核对{i}"} for i in range(3)]}
            elif block == "quiz":
                answer = {"quiz": [{"question": base + f" 测验{i}", "options": ["对", "错", "其他", "不确定"],
                          "answerIndex": 0, "explanation": f"解释{i}"} for i in range(3)]}
            else:
                answer = {"checkpoint": [base + " 检查1", "检查2"],
                          "commonMistakes": ["忽略区间", "混淆方向"], "resourceSummary": "复习定义"}
        elif '"answer"' in prompt and '"rubric"' in prompt:
            answer = {"correct": True, "confidence": .4, "feedback": "解释仍然缺少关键条件", "followUp": "为什么要满足这个条件？"}
        elif '"direction"' in prompt:
            context = json.loads(prompt)
            answer = ({"action": "skip", "topic": context["nextTopic"]} if context["direction"] == "strong"
                      else {"action": "insert", "topic": "函数单调性短补弱"})
        elif '"topics"' in prompt:
            answer = {"topics": ["函数单调性", "函数图像"]}
        else:
            MockAI.remedial_calls += 1
            answer = {"title": "补讲", "content": "先回到函数单调性的定义。" * 12,
                      "check": "如何判断一个区间内的单调性？"}
        payload = json.dumps({"model": "mock", "choices": [{"message": {"content": json.dumps(answer, ensure_ascii=False)}}]}, ensure_ascii=False).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)


async def check_stream(port, course_id):
    async with websockets.connect(f"ws://127.0.0.1:{port}/ws/classroom") as connection:
        await connection.send(json.dumps({"type": "ask", "courseId": course_id, "phaseIndex": 1,
                                          "topicIndex": 1, "question": "为何单调？", "selection": "单调性"}, ensure_ascii=False))
        events = []
        while True:
            event = json.loads(await asyncio.wait_for(connection.recv(), 10))
            events.append(event)
            if event["type"] in ("done", "error"):
                break
        assert [event["text"] for event in events if event["type"] == "delta"] == ["片段一", "片段二"], events
        assert events[-1]["type"] == "done", events
        await asyncio.sleep(.05)
        await connection.send(json.dumps({"type": "ask", "courseId": course_id, "phaseIndex": 1,
                                          "topicIndex": 1, "question": "停止测试"}, ensure_ascii=False))
        first = json.loads(await asyncio.wait_for(connection.recv(), 10))
        assert first["type"] == "delta"
        await connection.send(json.dumps({"type": "stop"}))
        while True:
            stopped = json.loads(await asyncio.wait_for(connection.recv(), 10))
            if stopped["type"] in ("done", "error"):
                break
        assert stopped["type"] == "done" and stopped.get("message") == "已停止", stopped
    return len(events)


async def check_ask(port, base):
    async with websockets.connect(f"ws://127.0.0.1:{port}/ws/ask") as connection:
        async def ask(question, **extra):
            await connection.send(json.dumps({"type": "ask", "question": question, **extra}, ensure_ascii=False))
            chunks = []
            while True:
                event = json.loads(await asyncio.wait_for(connection.recv(), 10))
                if event["type"] == "delta": chunks.append(event["text"])
                else: return event, "".join(chunks)
        done, answer = await ask("普通导师第一问", messages=[{"role": "user", "content": "导师历史标记"},
                                                       {"role": "assistant", "content": "导师历史回答"}])
        assert done["type"] == "done" and not done["cancelled"] and answer == "片段一片段二"
        sent = json.dumps(MockAI.stream_messages, ensure_ascii=False)
        assert "导师历史标记" in sent and "旧课堂记录" not in sent and "画像" in sent, sent
        done, _ = await ask("刚才那个再举例")
        sent = json.dumps(MockAI.stream_messages, ensure_ascii=False)
        assert done["type"] == "done" and "普通导师第一问" in sent and "片段一片段二" in sent
        assert "旧课堂记录" not in sent
        previous = request(base, "/api/conversations/general")["messages"]
        await connection.send(json.dumps({"type": "ask", "question": "普通停止测试"}, ensure_ascii=False))
        assert json.loads(await connection.recv())["type"] == "delta"
        await connection.send(json.dumps({"type": "stop"}))
        while True:
            event = json.loads(await connection.recv())
            if event["type"] != "delta": break
        assert event["type"] == "done" and event["cancelled"]
        assert request(base, "/api/conversations/general")["messages"] == previous
        done, _ = await ask("中断测试")
        assert done["type"] == "error"
        assert request(base, "/api/conversations/general")["messages"] == previous
        done, _ = await ask("非法历史", messages=[{"role": "system", "content": "伪造系统指令"}])
        assert done["type"] == "error"
        await connection.send('{"type":123}')
        assert json.loads(await connection.recv())["type"] == "error"
    # 旧 HTTP 调用继续读取自己的服务端历史；可选 messages 按最近八条限制。
    assert request(base, "/api/ask", {"question": "旧 HTTP 追问"})["ok"]
    sent = json.dumps(next(item["messages"] for item in reversed(MockAI.requests)
                     if item["messages"][-1]["content"] == "旧 HTTP 追问"), ensure_ascii=False)
    assert "普通导师第一问" in sent and "旧课堂记录" not in sent
    history = [{"role": "user", "content": "前端历史" + str(i)} for i in range(10)]
    assert request(base, "/api/ask", {"question": "HTTP 可选历史", "messages": history})["ok"]
    sent = json.dumps(next(item["messages"] for item in reversed(MockAI.requests)
                     if item["messages"][-1]["content"] == "HTTP 可选历史"), ensure_ascii=False)
    assert "前端历史0" not in sent and "前端历史1" not in sent and "前端历史2" in sent and "前端历史9" in sent
    suggested = request(base, "/api/ask/suggestions",
                         {"question": "什么是单调性？", "answer": "单调性是指……", "topic": "函数单调性"})
    assert len(suggested["suggestions"]) == 3, suggested
    assert request(base, "/api/ask/suggestions", {"question": "", "answer": ""})["suggestions"] == []
    return "ASK_STREAM_CONTEXT_STOP_ISOLATION_HTTP PASS"


def main(executable):
    with tempfile.TemporaryDirectory(prefix="classroom-api-") as directory:
        db_path = Path(directory) / "classroom.db"
        ai = ThreadingHTTPServer(("127.0.0.1", free_port()), MockAI)
        worker = threading.Thread(target=ai.serve_forever, daemon=True)
        worker.start()
        port = free_port()
        env = dict(os.environ, HOST="127.0.0.1", PORT=str(port), DATABASE_PATH=str(db_path),
                   AI_BASE_URL=f"http://127.0.0.1:{ai.server_port}/v1", AI_API_KEY="mock-key",
                   LOCAL_CONTROL_TOKEN="classroom-test")
        service = subprocess.Popen([str(executable)], cwd=executable.parent, env=env,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                   creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        base = f"http://127.0.0.1:{port}"
        try:
            for _ in range(100):
                try:
                    if urllib.request.urlopen(base + "/health", timeout=1).status == 200:
                        break
                except Exception:
                    time.sleep(.1)
            else:
                raise AssertionError("课堂服务未启动")
            created_plan = {"generation": {"source": "ai", "promptVersion": "ai-plan-v1"},
                            "title": "函数单调性训练", "summary": "按定义、图像、应用逐步练习函数单调性。",
                            "outcome": "完成函数应用报告", "prerequisites": ["掌握一次函数"],
                            "courseStructure": [], "roadmap": []}
            for stage, first, second in [("定义基础", "函数单调性", "函数图像"),
                                         ("图像判断", "图像上升区间", "图像下降区间"),
                                         ("应用迁移", "实际变化模型", "综合情境挑战")]:
                created_plan["courseStructure"].append({"stage": stage, "topics": [first, second]})
                created_plan["roadmap"].append({"name": stage, "description": f"完成{first}与{second}练习，提交函数报告。",
                                                   "topics": [first, second], "steps": [{"title": f"分析{first}",
                                                   "explanation": f"用区间判断{first}的变化。", "action": "完成练习",
                                                   "check": f"解释{second}的判断依据"}]})
            created = request(base, "/api/courses", {"anonymousId": "anon", "goal": "学习函数单调性", "mode": "lite",
                              "title": "函数单调性训练", "source": "ai", "payload": created_plan})
            assert created["ok"] and created["courseId"], created
            course_id = created["courseId"]
            first_week = request(base, "/api/classroom/week?courseId=" + course_id)
            assert len(first_week["availability"]) == 3 and all(slot["minutes"] == 30 for slot in first_week["availability"])
            with closing(sqlite3.connect(db_path)) as db:
                initial_snapshot_count = db.execute("SELECT COUNT(*) FROM CourseSnapshot WHERE courseId=?", (course_id,)).fetchone()[0]
            new_lesson = f"?courseId={course_id}&phaseIndex=1&topicIndex=1"
            first_diagnostic = request(base, "/api/classroom/start" + new_lesson + "&kind=diagnostic")
            assert len(first_diagnostic["questions"]) == 2
            with closing(sqlite3.connect(db_path)) as db:
                upgraded = json.loads(db.execute(
                    "SELECT payload FROM CourseSnapshot WHERE courseId=? ORDER BY version DESC LIMIT 1",
                    (course_id,)).fetchone()[0])["courseStructure"]
                assert all(len(stage["topicIds"]) == len(stage["topics"]) for stage in upgraded)
            skipped = request(base, "/api/courses", {"anonymousId": "skip-anon", "goal": "学习函数单调性",
                              "mode": "lite", "title": "跳过诊断测试", "source": "ai", "payload": created_plan})
            skip_id = skipped["courseId"]
            request(base, f"/api/classroom/start?courseId={skip_id}&phaseIndex=1&topicIndex=1&kind=diagnostic")
            assert request(base, "/api/classroom/skip", {"courseId": skip_id, "phaseIndex": 1,
                           "topicIndex": 1})["mode"] == "full"
            quiz = [{"question": f"测验{i}", "options": ["对", "错", "其他", "不确定"],
                     "answerIndex": 0} for i in range(3)]
            content = {"blocks": {"quiz": {"quiz": quiz}}}
            strong = request(base, "/api/courses", {"anonymousId": "strong-anon", "goal": "学习函数单调性",
                             "mode": "lite", "title": "强证据跳课测试", "source": "ai", "payload": created_plan})
            strong_id = strong["courseId"]
            with closing(sqlite3.connect(db_path)) as db, db:
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("l1", course_id, "anon", "函数", "lite", 1, "定义基础", 1, "函数单调性", "函数单调性", "", "", json.dumps(content, ensure_ascii=False), "[]", 0, "ai"))
                db.execute("INSERT INTO LearningInteraction VALUES(?,?,?,?,?,?,?)",
                           ("old-lesson-chat", course_id, f"lesson-{course_id}-1-1", None,
                            "chat-assistant", json.dumps({"text": "旧课堂记录"}, ensure_ascii=False), "2026-09-28"))
                db.execute("INSERT INTO LearningInteraction VALUES(?,?,?,?,?,?,?)",
                           ("other-lesson-chat", course_id, f"lesson-{course_id}-1-2", None,
                            "chat-assistant", json.dumps({"text": "另一课时记录"}, ensure_ascii=False), "2026-09-28"))
                for kind in ("diagnostic", "interaction"):
                    for index in range(3):
                        item = {"question": f"{kind}{index}", "type": "open" if kind == "interaction" and index == 1 else "choice",
                                "options": ["对", "错", "其他", "不确定"], "answerIndex": 0,
                                "rubric": "说明条件与具体推理过程", "explanation": "请检查定义中的关键条件。",
                                "status": "pending", "evidenceSource": "ai-generated"}
                        db.execute("INSERT OR REPLACE INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)",
                                   (f"classroom:{course_id}:1:1:{kind}:{index}", course_id, 1, 1, kind,
                                    json.dumps(item, ensure_ascii=False), "2026-09-28"))
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("strong-session", strong_id, "strong-anon", "函数", "lite", 1, "定义基础", 1,
                            "函数单调性", "函数单调性", "", "", json.dumps(content, ensure_ascii=False), "[]", 0, "ai"))
                for index in range(2):
                    item = {"status": "answered", "correct": True, "credible": True}
                    db.execute("INSERT INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)",
                               (f"classroom:{strong_id}:1:1:diagnostic:{index}", strong_id, 1, 1,
                                "diagnostic", json.dumps(item), "2026-09-28"))
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("skip-session", skip_id, "skip-anon", "函数", "lite", 1, "定义基础", 1,
                            "函数单调性", "函数单调性", "", "", json.dumps(content, ensure_ascii=False), "[]", 0, "ai"))
                for index in range(2):
                    item = {"status": "answered", "correct": True, "credible": True}
                    db.execute("INSERT OR REPLACE INTO ClassroomActivity VALUES(?,?,?,?,?,?,?)",
                               (f"classroom:{skip_id}:1:1:diagnostic:{index}", skip_id, 1, 1,
                                "diagnostic", json.dumps(item), "2026-09-28"))
            strong_quiz = request(base, "/api/quiz-attempts", {"courseId": strong_id, "phaseIndex": 1,
                                  "topicIndex": 1, "answers": [0, 0, 0]})
            assert strong_quiz["passed"]
            request(base, "/api/learn/progress", {"courseId": strong_id, "phaseIndex": 1,
                    "topicIndex": 1, "status": "completed"})
            strong_finish = request(base, "/api/classroom/finish", {"courseId": strong_id,
                                    "phaseIndex": 1, "topicIndex": 1})
            assert strong_finish["pathAdjustment"] == "", "完成课时不能改写课程大纲"
            with closing(sqlite3.connect(db_path)) as db:
                strong_topics = json.loads(db.execute(
                    "SELECT payload FROM CourseSnapshot WHERE courseId=? ORDER BY version DESC LIMIT 1",
                    (strong_id,)).fetchone()[0])["courseStructure"]
                assert strong_topics[0]["topics"] == ["函数单调性", "函数图像"] and len(strong_topics[1]["topics"]) == 2
                skip_versions = db.execute("SELECT COUNT(*) FROM CourseSnapshot WHERE courseId=?", (skip_id,)).fetchone()[0]
            request(base, "/api/quiz-attempts", {"courseId": skip_id, "phaseIndex": 1,
                    "topicIndex": 1, "answers": [0, 0, 0]})
            request(base, "/api/learn/progress", {"courseId": skip_id, "phaseIndex": 1,
                    "topicIndex": 1, "status": "completed"})
            skipped_finish = request(base, "/api/classroom/finish", {"courseId": skip_id,
                                     "phaseIndex": 1, "topicIndex": 1})
            assert skipped_finish["pathAdjustment"] == ""
            with closing(sqlite3.connect(db_path)) as db:
                assert db.execute("SELECT COUNT(*) FROM CourseSnapshot WHERE courseId=?", (skip_id,)).fetchone()[0] == skip_versions
            q = f"?courseId={course_id}&phaseIndex=1&topicIndex=1"
            with ThreadPoolExecutor(max_workers=2) as pool:
                lessons = list(pool.map(lambda _: request(base, "/api/learn" + q + "&block=all"), range(2)))
            assert all(len(item["blocks"]) == 6 for item in lessons) and MockAI.learn_block_calls == 6, lessons
            assert len(request(base, "/api/classroom/start" + q + "&kind=diagnostic")["questions"]) == 2
            assert request(base, "/api/classroom/submit", {"courseId": course_id, "phaseIndex": 1, "topicIndex": 1,
                       "kind": "diagnostic", "index": 0, "answer": 1})["mode"] == "pending"
            weak_quiz = request(base, "/api/quiz-attempts", {"courseId": course_id, "phaseIndex": 1,
                                      "topicIndex": 1, "answers": [1, 0, 0]})
            assert weak_quiz["passed"] is False
            assert request(base, "/api/learn/progress", {"courseId": course_id, "phaseIndex": 1,
                           "topicIndex": 1, "status": "completed"})["ok"]
            assert request(base, "/api/classroom/finish", {"courseId": course_id, "phaseIndex": 1,
                       "topicIndex": 1})["pathAdjustment"] == ""
            second = request(base, "/api/classroom/submit", {"courseId": course_id, "phaseIndex": 1,
                             "topicIndex": 1, "kind": "diagnostic", "index": 1, "answer": 0})
            assert second["mode"] == "third" and second["nextQuestion"]["question"] == "diagnostic2"
            third = request(base, "/api/classroom/submit", {"courseId": course_id, "phaseIndex": 1,
                            "topicIndex": 1, "kind": "diagnostic", "index": 2, "answer": 1})
            assert third["mode"] == "weak"
            assert len(request(base, "/api/classroom/start" + q + "&kind=diagnostic")["questions"]) == 3
            remedial_before = MockAI.remedial_calls
            with ThreadPoolExecutor(max_workers=2) as pool:
                remedials = list(pool.map(lambda _: request(base, "/api/classroom/remedial" + q), range(2)))
            assert remedials[0] == remedials[1] and MockAI.remedial_calls == remedial_before + 1, remedials
            assert len(request(base, "/api/classroom/start" + q + "&kind=interaction")["questions"]) == 3
            prediction = request(base, "/api/classroom/submit", {"courseId": course_id, "phaseIndex": 1,
                                 "topicIndex": 1, "kind": "interaction", "index": 0, "answer": 1})
            assert not prediction["correct"] and prediction["followUp"]
            hint2 = request(base, "/api/classroom/hint", {"courseId": course_id, "phaseIndex": 1,
                            "topicIndex": 1, "kind": "interaction", "index": 0})
            hint3 = request(base, "/api/classroom/hint", {"courseId": course_id, "phaseIndex": 1,
                            "topicIndex": 1, "kind": "interaction", "index": 0})
            assert hint2["level"] == 2 and hint3["level"] == 3
            open_result = request(base, "/api/classroom/submit", {"courseId": course_id, "phaseIndex": 1,
                                  "topicIndex": 1, "kind": "interaction", "index": 1, "answer": "我认为是递增。"})
            assert open_result["credible"] is False
            assert request(base, "/api/classroom/activity/skip", {"courseId": course_id, "phaseIndex": 1,
                           "topicIndex": 1, "index": 2})["item"]["status"] == "skipped"
            with ThreadPoolExecutor(max_workers=2) as pool:
                finishes = list(pool.map(lambda _: request(base, "/api/classroom/finish", {
                    "courseId": course_id, "phaseIndex": 1, "topicIndex": 1}), range(2)))
            assert all(item["pathAdjustment"] == "" for item in finishes), "补弱调整教学，不改写大纲"
            with closing(sqlite3.connect(db_path)) as db, db:
                snapshots = db.execute("SELECT version,payload FROM CourseSnapshot WHERE courseId=? ORDER BY version", (course_id,)).fetchall()
                assert snapshots[0][0] == 1 and len(snapshots) == initial_snapshot_count + 1, "只允许为旧课程补充稳定主题标识"
                adjusted_payload = json.loads(snapshots[-1][1])
                topics = adjusted_payload["courseStructure"][0]["topics"]
                assert topics == ["函数单调性", "函数图像"], topics
                assert adjusted_payload["prerequisites"] == ["掌握一次函数"]
                state_id = f"classroom:{course_id}:1:1:state"
                state = json.loads(db.execute("SELECT payload FROM ClassroomActivity WHERE id=?", (state_id,)).fetchone()[0])
                state["pathAdjusted"] = False
                db.execute("UPDATE ClassroomActivity SET payload=? WHERE id=?", (json.dumps(state, ensure_ascii=False), state_id))
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("l2", course_id, "anon", "函数", "lite", 1, "定义基础", 2, "函数单调性短补弱",
                            "短补弱", "", "", json.dumps(content, ensure_ascii=False), "[]", 0, "ai"))
            unchanged = request(base, "/api/classroom/finish", {"courseId": course_id, "phaseIndex": 1, "topicIndex": 1})
            assert unchanged["pathAdjustment"] == ""
            with closing(sqlite3.connect(db_path)) as db:
                assert db.execute("SELECT COUNT(*) FROM CourseSnapshot WHERE courseId=?", (course_id,)).fetchone()[0] == len(snapshots)
            review = request(base, "/api/classroom/review/submit", {"courseId": course_id, "phaseIndex": 1,
                             "topicIndex": 1, "day": 1, "answers": [1, 1, 1]})
            tomorrow = (datetime.date.today() + datetime.timedelta(days=1)).isoformat()
            assert not review["passed"] and review["nextDue"] == tomorrow, review
            with closing(sqlite3.connect(db_path)) as db, db:
                retry = db.execute("SELECT payload FROM ClassroomActivity WHERE id LIKE ?",
                                   (f"classroom:{course_id}:1:1:review:retry:%",)).fetchone()
                assert retry and json.loads(retry[0])["due"] == tomorrow
            refreshed = request(base, "/api/classroom/week/replan", {"courseId": course_id})
            assert refreshed["pending"] and refreshed["plan"]["status"] == "pending"
            assert request(base, "/api/classroom/week?courseId=" + course_id)["sharedBudget"]
            print(asyncio.run(check_ask(port, base)))
            assert asyncio.run(check_stream(port, course_id)) == 3
            sent_context = json.dumps([item for item in MockAI.stream_messages if item["role"] != "system"], ensure_ascii=False)
            assert "旧课堂记录" in sent_context and "另一课时记录" not in sent_context
            assert "普通导师第一问" not in sent_context and "verifiedProfile" in json.dumps(MockAI.stream_messages, ensure_ascii=False)
            history = request(base, f"/api/conversations/lesson-{course_id}-1-1")["messages"]
            assert history[0]["text"] == "旧课堂记录" and history[-1]["text"] == "片段一片段二"
            print("CLASSROOM_API_SMOKE PASS: creation, diagnosis, review retry, stable outline, shared-budget compatibility, concurrent lesson/remedial, stream, stop")
        finally:
            try:
                urllib.request.urlopen(urllib.request.Request(base + "/internal/shutdown", method="POST",
                    headers={"X-Gangyi-Control-Token": "classroom-test"}), timeout=2)
            except Exception:
                service.terminate()
            service.wait(timeout=10)
            ai.shutdown(); ai.server_close(); worker.join(timeout=2)


if __name__ == "__main__":
    import sys
    main(Path(sys.argv[1]).resolve())
