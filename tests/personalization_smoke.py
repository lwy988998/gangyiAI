"""用模拟博查与模型验证逐题证据、主题状态、画像和降级。"""

import json
import os
import socket
import sqlite3
import subprocess
import sys
import tempfile
import threading
import time
import urllib.request
from urllib.parse import quote
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from contextlib import closing
from pathlib import Path


def request(base, path, method="GET", body=None):
    payload = None if body is None else json.dumps(body, ensure_ascii=False).encode()
    req = urllib.request.Request(base + path, data=payload, method=method,
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=15) as response:
        return json.loads(response.read())


def wait_for(predicate, seconds=15):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        try:
            value = predicate()
            if value:
                return value
        except Exception:
            pass
        time.sleep(.2)
    raise AssertionError("后台评估未在期限内完成")


def main(executable):
    with tempfile.TemporaryDirectory(prefix="gangyi-personalization-") as directory:
        root = Path(directory)
        ai_requests, search_queries = [], []
        search_state = {"fail": False}
        ai_state = {"bad_evidence": False}

        class MockAI(BaseHTTPRequestHandler):
            def do_POST(self):
                body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
                ai_requests.append(body)
                system = " ".join(x["content"] for x in body["messages"] if x["role"] == "system")
                user = next((x["content"] for x in reversed(body["messages"]) if x["role"] == "user"), "")
                if "首页学习目标推荐助手" in system:
                    result = {mode: {"continue": [f"{mode} 继续{i}" for i in range(3)],
                                     "explore": [f"{mode} 探索{i}" for i in range(2)]} for mode in ("lite", "deep")}
                elif "学习诊断教师" in system:
                    events = json.loads(user)["events"]
                    last = next(item for item in reversed(events) if item["kind"] == "quiz")
                    correct = sum(x["correct"] for x in last["results"])
                    score = 0 if all(item.get("unknown") for item in last["results"]) else 91 if correct >= 3 else 42
                    result = {"score": score, "rationale": f"依据记录 {last['id']} 的逐题结果",
                              "weakPoints": [] if score > 70 else ["二次函数"],
                              "recommendation": "进阶迁移练习" if score > 70 else "补讲基础并完成基础练习",
                              "evidenceIds": ["fabricated-id"] if ai_state["bad_evidence"] else [last["id"]],
                              "nextReviewAt": "2026-10-01"}
                elif "画像评估 AI" in system:
                    state = json.loads(user)["topicStates"][0]
                    result = {"subjects": [{"subject": "数学", "score": state["score"],
                        "rationale": "依据二次函数主题测验", "weakPoints": [],
                        "recommendation": "继续练习", "evidenceCount": 1,
                        "evidenceIds": state["evidenceIds"]}]}
                elif "专业高中教师" in system:
                    data = json.loads(user)
                    result = {"title": "学习二次函数 基础 二次函数",
                              "summary": "学习二次函数，在基础阶段理解二次函数。",
                              "inferredDomain": "高中数学",
                              "keyConcepts": ["学习二次函数", "基础", "二次函数"]}
                else:
                    result = "模拟问答"
                response = json.dumps({"model": "mock-ai", "choices": [{"message": {
                    "content": json.dumps(result, ensure_ascii=False) if not isinstance(result, str) else result}}]},
                    ensure_ascii=False).encode()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(response)))
                self.end_headers()
                self.wfile.write(response)

            def log_message(self, *_):
                pass

        class MockBocha(BaseHTTPRequestHandler):
            def do_POST(self):
                body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
                search_queries.append(body["query"])
                if search_state["fail"]:
                    self.send_response(503)
                    self.end_headers()
                    return
                response = json.dumps({"code": 200, "data": {"webPages": {"value": [
                    {"name": "公开教材", "url": "https://example.edu/lesson", "summary": "公开概念"}
                ]}}}, ensure_ascii=False).encode()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(response)))
                self.end_headers()
                self.wfile.write(response)

            def log_message(self, *_):
                pass

        ai = ThreadingHTTPServer(("127.0.0.1", 0), MockAI)
        bocha = ThreadingHTTPServer(("127.0.0.1", 0), MockBocha)
        threads = [threading.Thread(target=s.serve_forever, daemon=True) for s in (ai, bocha)]
        for thread in threads:
            thread.start()
        with socket.socket() as listener:
            listener.bind(("127.0.0.1", 0))
            port = listener.getsockname()[1]
        env = os.environ.copy()
        env.update(PORT=str(port), HOST="127.0.0.1", DATABASE_PATH=str(root / "student.db"),
                   AI_BASE_URL=f"http://127.0.0.1:{ai.server_port}/v1", AI_API_KEY="test-ai-key",
                   BOCHA_BASE_URL=f"http://127.0.0.1:{bocha.server_port}", BOCHA_API_KEY="test-search-key",
                   SEARCH_FALLBACK_PROVIDER="", RESOURCE_SEARCH_CACHE_DIR=str(root / "cache"),
                   RESOURCE_SEARCH_TIMEOUT_MS="1000", LOCAL_CONTROL_TOKEN="smoke-token")
        service_log = (root / "service.log").open("wb")
        process = subprocess.Popen([str(executable)], cwd=str(executable.parent), env=env,
                                   stdout=service_log, stderr=service_log,
                                   creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)
        base = f"http://127.0.0.1:{port}"
        try:
            wait_for(lambda: request(base, "/api/profile"), 10)
            with closing(sqlite3.connect(root / "student.db")) as db, db:
                db.execute("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)",
                           ("course", None, None, "学习二次函数", "deep", "数学课程", None,
                            "ai", "active", "2026-01-01", "2026-01-01"))
                quiz = {"promptVersion": "ai-block-v1", "blocks": {"quiz": {"quiz": [
                    {"question": str(i), "options": ["甲", "乙", "丙", "丁"], "answerIndex": 0}
                    for i in range(3)]}}}
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("session", "course", None, "学习二次函数", "deep", 1, "基础", 1,
                            "二次函数", "测试课", None, None, json.dumps(quiz), None, 0, "ai"))
            request(base, "/api/learning-interactions", "POST",
                    {"courseId": "course", "phaseIndex": 1, "topicIndex": 1,
                     "kind": "practice", "state": "completed"})
            wait_for(lambda: request(base, "/api/topic-mastery?courseId=course")["topics"])
            assert request(base, "/api/topic-mastery?courseId=course")["topics"][0]["score"] is None
            assert request(base, "/api/profile")["subjects"] == []
            for answers, expected in ((["unknown", "unknown", "unknown"], 0), ([1, 1, 1], 42), ([0, 0, 0], 91)):
                attempt = request(base, "/api/quiz-attempts", "POST",
                                  {"courseId": "course", "phaseIndex": 1, "topicIndex": 1, "answers": answers})
                assert attempt["ok"] and attempt["total"] == 3
                state = wait_for(lambda: next((x for x in request(base, "/api/topic-mastery?courseId=course")["topics"]
                                              if x["score"] == expected), None))
                assert state["evidenceIds"] and state["status"] == "ready"
                profile = wait_for(lambda: next((x for x in request(base, "/api/profile")["subjects"]
                                                if x["score"] == expected), None))
                assert state["evidenceIds"][0] in profile["rationale"]
                recommendation = request(base, "/api/next-learning?courseId=course")
                assert recommendation["score"] == expected and recommendation["evidenceIds"]
                if expected == 0:
                    diagnosis = [json.loads(message["content"]) for call in ai_requests
                                 for message in call["messages"] if message["role"] == "user"
                                 and '"events"' in message["content"]][-1]
                    assert all(item["unknown"] for item in diagnosis["events"][-1]["results"])
                state = "too_hard" if expected < 70 else "too_easy"
                request(base, "/api/learning-interactions", "POST",
                        {"courseId": "course", "phaseIndex": 1, "topicIndex": 1,
                         "kind": "review", "state": state})
                recommendation = request(base, "/api/next-learning?courseId=course")
                assert ("补讲基础" if expected < 70 else "进阶迁移") in recommendation["action"]
            ai_state["bad_evidence"] = True
            request(base, "/api/quiz-attempts", "POST",
                    {"courseId": "course", "phaseIndex": 1, "topicIndex": 1, "answers": [0, 0, 0]})
            wait_for(lambda: request(base, "/api/profile")["error"])
            assert request(base, "/api/topic-mastery?courseId=course")["topics"][0]["score"] == 91
            ai_state["bad_evidence"] = False
            with closing(sqlite3.connect(root / "student.db")) as db, db:
                db.execute("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)",
                           ("course2", None, None, "学习二次函数", "deep", "数学课程二", None,
                            "ai", "active", "2026-01-01", "2026-01-01"))
                plan = {"generation": {"source": "ai", "promptVersion": "ai-plan-v1"},
                        "roadmap": [{"name": "基础", "topics": ["二次函数"]}],
                        "courseStructure": [{"stage": "基础", "topics": ["二次函数"]}]}
                db.execute("INSERT INTO CourseSnapshot VALUES(?,?,?,?,?)",
                           ("snapshot2", "course2", 1, json.dumps(plan, ensure_ascii=False), "2026-01-01"))
                cached = {"promptVersion": "ai-block-v1", "profileVersion": 0,
                          "blocks": {"overview": {"title": "旧讲解"}},
                          "generations": {"overview": {"source": "ai", "promptVersion": "ai-block-v1",
                                                       "model": "mock-ai"}}, "references": []}
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("session2", "course2", None, "学习二次函数", "deep", 1, "基础", 1,
                            "二次函数", "旧讲解", None, None, json.dumps(cached, ensure_ascii=False), None, 0, "ai"))
            lesson_url = "/api/learn?courseId=course2&phaseIndex=1&topicIndex=1&block=overview"
            refreshed = request(base, lesson_url + "&phaseName=" + quote("篡改阶段") +
                                "&topic=" + quote("篡改主题"))
            assert not refreshed["cached"] and refreshed["content"]["title"] != "旧讲解"
            lesson_inputs = [json.loads(message["content"]) for call in ai_requests
                             for message in call["messages"] if message["role"] == "user"
                             and '"block":"overview"' in message["content"]]
            assert lesson_inputs[-1]["topic"] == "二次函数" and lesson_inputs[-1]["phase"] == "基础"
            assert any("personalLearning" in json.dumps(item, ensure_ascii=False) for item in ai_requests)
            with closing(sqlite3.connect(root / "student.db")) as db, db:
                db.execute("INSERT INTO LearningCardProgress VALUES(?,?,?,?,?,?,?,?,?,?)",
                           ("card2", "course2", None, "学习二次函数", "deep", 1, "基础", 1,
                            "二次函数", "completed"))
            locked = request(base, lesson_url + "&regenerate=1")
            assert locked["cached"] and locked["content"] == refreshed["content"]
            locked_retry = request(base, lesson_url + "&retry=1")
            assert locked_retry["cached"] and locked_retry["content"] == refreshed["content"]
            first = request(base, "/api/ask", "POST", {"question": "数学问题甲"})
            count = len(search_queries)
            second = request(base, "/api/ask", "POST", {"question": "数学问题乙"})
            assert first["searchStatus"] != "unavailable" and second["searchStatus"] == "cache"
            assert len(search_queries) == count
            search_state["fail"] = True
            fallback = request(base, "/api/ask", "POST", {"question": "英语问题丙"})
            assert fallback["answer"] == "模拟问答" and fallback["searchStatus"] == "unavailable"
            search_state["fail"] = False
            count = len(search_queries)
            generic = request(base, "/api/ask", "POST", {"question": "怎么安排明天的学习？"})
            assert generic["answer"] == "模拟问答" and len(search_queries) == count + 1
            assert search_queries[-1] == "高中学科知识"
            count = len(search_queries)
            classroom = request(base, "/api/learning-chat", "POST", {
                "courseId": "course2", "question": "这题为何这样解？", "topic": "二次函数",
                "conversationId": "classroom-check"})
            assert classroom["answer"] == "模拟问答" and classroom["searchStatus"] != "unavailable"
            assert (search_queries[-1].startswith("学习二次函数") if len(search_queries) > count
                    else classroom["searchStatus"] == "cache")
            request(base, "/api/learning-interactions", "POST",
                    {"courseId": "course", "phaseIndex": 1, "topicIndex": 1,
                     "kind": "review", "state": "unsuitable"})
            assert request(base, "/api/next-learning?courseId=course")["status"] == "insufficient"
            assert all("问题" not in query and "answers" not in query for query in search_queries)
            print("PERSONALIZATION_SMOKE PASS: evidence, mastery, profile, recommendation, cache, fallback")
        except Exception:
            service_log.flush()
            print(f"SERVICE_EXIT={process.poll()} LOG_TAIL={((root / 'service.log').read_text(errors='replace'))[-1500:]}",
                  file=sys.stderr)
            raise
        finally:
            try:
                req = urllib.request.Request(base + "/internal/shutdown", data=b"", method="POST",
                                             headers={"X-Gangyi-Control-Token": "smoke-token"})
                urllib.request.urlopen(req, timeout=2).close()
                process.wait(timeout=10)
            except Exception:
                process.terminate()
                process.wait(timeout=10)
            for server in (ai, bocha):
                server.shutdown()
                server.server_close()
            for thread in threads:
                thread.join(timeout=2)
            service_log.close()


if __name__ == "__main__":
    main(Path(sys.argv[1]).resolve())
