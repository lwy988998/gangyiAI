"""在隔离数据库上检查新版页面和关键本机 API。"""

import json
import os
import socket
import sqlite3
import subprocess
import sys
import tempfile
import threading
import time
import urllib.error
import urllib.request
from contextlib import closing
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


def request(base, path, method="GET", body=None):
    data = None if body is None else json.dumps(body, ensure_ascii=False).encode()
    req = urllib.request.Request(base + path, data=data, method=method,
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=10) as response:
        content = response.read().decode()
        return response.status, json.loads(content) if path.startswith("/api/") else content


def main(executable):
    with tempfile.TemporaryDirectory(prefix="gangyi-v030-") as directory:
        database = Path(directory) / "student.db"
        mock_state = {"score": 73, "invalid": False, "requests": []}

        class MockAI(BaseHTTPRequestHandler):
            def do_POST(self):
                raw = self.rfile.read(int(self.headers["Content-Length"]))
                mock_state["requests"].append(raw)
                messages = json.loads(raw)["messages"]
                text = next(item["content"] for item in messages if item["role"] == "user")
                user = json.loads(text) if text.startswith('{') else {}
                system = messages[0]['content']
                if '课堂作答评价教师' in system:
                    given = user['givenAnswer']
                    result = {'isAnswer': True, 'correct': isinstance(given, int) and given == user['question'].get('answerIndex'),
                        'unknown': '不会' in user.get('studentAnswer', ''), 'confidence': .95,
                        'feedback': '依据学生实际作答判断；未写过程时不能推测方法。', 'misconception': '', 'methodAnalysis': '本轮没有完整过程。'}
                elif '课堂教师' in system:
                    result = '结合这道题和实际回答讲解，并说明推导依据。'
                elif "recentQuizzes" in user:
                    result = {mode: {"continue": [f"{mode} 继续{i}" for i in range(3)],
                                     "explore": [f"{mode} 探索{i}" for i in range(2)]} for mode in ("lite", "deep")}
                elif "events" in user:
                    latest = {}
                    for event in user["events"]:
                        for answer in event.get("results", []):
                            latest[answer["questionId"]] = event["id"]
                    result = {"score": mock_state["score"], "rationale": "依据逐题测验", "weakPoints": ["二次函数"],
                              "recommendation": "复盘错题", "evidenceIds": list(dict.fromkeys(latest.values())), "nextReviewAt": ""}
                elif "topicStates" in user:
                    result = {"subjects": [{"subject": "数学", "score": mock_state["score"],
                        "rationale": "根据测验和学习记录", "weakPoints": ["二次函数"],
                        "recommendation": "复盘错题", "evidenceIds": user["topicStates"][0]["evidenceIds"]}]}
                else:
                    result = {"abilities": [{"id": key, "score": None, "rationale": "该维度缺少三道直接相关的可靠题目。",
                        "recommendation": "继续积累反馈。", "evidenceIds": []} for key in
                        ("memory", "understanding", "application", "reasoning", "expression", "transfer")]}
                content = "[]" if mock_state["invalid"] else result if isinstance(result, str) else json.dumps(result, ensure_ascii=False)
                data = json.dumps({"model": "mock-profile", "choices": [{"message": {"content": content}}]},
                                  ensure_ascii=False).encode()
                self.send_response(200)
                self.send_header("Content-Type", "application/json")
                self.send_header("Content-Length", str(len(data)))
                self.end_headers()
                self.wfile.write(data)

            def log_message(self, *_):
                pass

        ai_server = ThreadingHTTPServer(("127.0.0.1", 0), MockAI)
        ai_thread = threading.Thread(target=ai_server.serve_forever, daemon=True)
        ai_thread.start()
        with socket.socket() as listener:
            listener.bind(("127.0.0.1", 0))
            port = listener.getsockname()[1]
        env = os.environ.copy()
        env.update(PORT=str(port), HOST="127.0.0.1", DATABASE_PATH=str(database),
                   AI_BASE_URL=f"http://127.0.0.1:{ai_server.server_port}/v1", AI_API_KEY="smoke-key",
                   LOCAL_CONTROL_TOKEN="smoke-only")
        flags = subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0
        log_path = os.environ.get("GANGYI_SMOKE_LOG")
        output = open(log_path, "wb") if log_path else subprocess.DEVNULL
        process = subprocess.Popen([str(executable)], cwd=str(executable.parent), env=env,
                                   stdout=output, stderr=output,
                                   creationflags=flags)
        base = f"http://127.0.0.1:{port}"
        try:
            for _ in range(50):
                try:
                    status, profile = request(base, "/api/profile")
                    break
                except (urllib.error.URLError, TimeoutError):
                    time.sleep(.1)
            else:
                raise AssertionError("服务未启动")
            assert status == 200 and profile["subjects"] == []
            assert request(base, "/api/recent-courses")[1]["courses"] == []
            assert "home-radar" in request(base, "/")[1]
            assert "ask.js" in request(base, "/ask")[1]
            assert request(base, "/dark.css")[0] == 200
            with closing(sqlite3.connect(database)) as db, db:
                db.execute("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)",
                           ("old", "old-id", None, "学习二次函数", "deep", "数学课程", None,
                            "ai", "active", "2026-01-01", "2026-01-01"))
                quiz = {"promptVersion": "ai-block-v1", "blocks": {"quiz": {"quiz": [
                    {"question": "一", "options": ["甲", "乙", "丙"], "answerIndex": 0},
                    {"question": "二", "options": ["甲", "乙", "丙"], "answerIndex": 1},
                    {"question": "三", "options": ["甲", "乙", "丙"], "answerIndex": 2}]}}}
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("session", "old", "old-id", "学习二次函数", "deep", 1, "基础", 1,
                            "二次函数", "测试课", None, None, json.dumps(quiz), None, 0, "ai"))
                db.execute("INSERT INTO LearningInteraction VALUES(?,?,?,?,?,?,?)",
                           ("chat", None, "general", None, "chat-user", '{"text":"你好"}', "2026-01-01"))
            status, attempt = request(base, "/api/quiz-attempts", "POST",
                                      {"courseId": "old", "phaseIndex": 1, "topicIndex": 1,
                                       "answers": ["unknown", None, 2]})
            assert status == 200 and attempt["score"] == 1 and attempt["unknownCount"] == 1
            with closing(sqlite3.connect(database)) as db:
                rows = db.execute("SELECT payload FROM LearningInteraction WHERE kind='question-evaluation' ORDER BY rowid").fetchall()
                saved = [json.loads(row[0])['results'][0] for row in rows]
                assert len(saved) == 2, '漏答不形成新的评价或不会记录'
                assert saved[0]['answered'] and saved[0]['unknown'] and not saved[0]['correct']
                assert saved[1]['correct'] and not saved[1]['unknown']
            try:
                request(base, "/api/quiz-attempts", "POST", {"courseId": "old", "phaseIndex": 1,
                        "topicIndex": 1, "answers": [{"invalid": True}, 1, 2]})
                raise AssertionError("非法答案未拒绝")
            except urllib.error.HTTPError as error:
                assert error.code == 409
            status, attempt = request(base, "/api/quiz-attempts", "POST",
                                      {"courseId": "old", "phaseIndex": 1, "topicIndex": 1,
                                       "answers": [0, 1, 2]})
            assert status == 200 and attempt["score"] == 3 and attempt["total"] == 3 and attempt["passed"]
            for _ in range(80):
                profile = request(base, "/api/profile")[1]
                if profile["subjects"] and profile["subjects"][0]["score"] == 73:
                    break
                time.sleep(.1)
            else:
                raise AssertionError("画像未按模拟 AI 的分数更新")
            assert mock_state["requests"] and b'courses' in mock_state["requests"][-1]
            mock_state["invalid"] = True
            request(base, "/api/profile/refresh", "POST")
            for _ in range(80):
                profile = request(base, "/api/profile")[1]
                if profile["error"]:
                    break
                time.sleep(.1)
            else:
                raise AssertionError("无效 AI 输出未显示重试状态")
            assert profile["subjects"][0]["score"] == 73
            mock_state.update(invalid=False, score=81)
            request(base, "/api/profile/refresh", "POST")
            for _ in range(80):
                profile = request(base, "/api/profile")[1]
                if not profile["error"] and profile["subjects"] and profile["subjects"][0]["score"] == 81:
                    break
                time.sleep(.1)
            else:
                raise AssertionError("画像重试未恢复")
            assert request(base, "/api/conversations/general")[1]["messages"][0]["text"] == "你好"
            assert request(base, "/api/conversations/general", "DELETE")[1]["ok"]
            assert request(base, "/api/conversations/general")[1]["messages"] == []
            assert "数学课程" in request(base, "/my-courses")[1]
            assert "recent-courses" in request(base, "/")[1]
            assert request(base, "/plan?courseId=old")[0] == 200
            recent = request(base, "/api/recent-courses")[1]["courses"]
            assert len(recent) == 1 and recent[0]["courseId"] == "old" and recent[0]["lastActivityAt"]
            assert request(base, "/api/my-courses/old", "DELETE")[1]["ok"]
            assert request(base, "/api/recent-courses")[1]["courses"] == []
            with closing(sqlite3.connect(database)) as db, db:
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE courseId='old'").fetchone()[0] == 0
            # 删除课程不能通过新增题目或备课入口继续读取、写入或请求模型。
            calls_before = len(mock_state["requests"])
            for path, body in (
                ("/api/learn/next", {"courseId": "old", "phaseIndex": 1, "topicIndex": 1, "requestId": "deleted-next"}),
                ("/api/classroom/question/skip", {"courseId": "old", "phaseIndex": 1, "topicIndex": 1, "kind": "quiz", "index": 0}),
                ("/api/classroom/submit", {"courseId": "old", "phaseIndex": 1, "topicIndex": 1, "kind": "quiz", "index": 0, "answer": 0})):
                try:
                    request(base, path, "POST", body)
                    raise AssertionError("已删除课程仍可访问")
                except urllib.error.HTTPError as error:
                    assert error.code == 404
            assert len(mock_state["requests"]) == calls_before
            print("API_SMOKE PASS: pages, quiz persistence, AI profile, retry, conversation clear, course deletion")
        except Exception:
            print("验证失败时服务进程状态：", process.poll())
            raise
        finally:
            try:
                shutdown = urllib.request.Request(base + "/internal/shutdown", data=b"",
                    method="POST", headers={"X-Gangyi-Control-Token": "smoke-only"})
                urllib.request.urlopen(shutdown, timeout=2).close()
            except (urllib.error.URLError, ConnectionError, TimeoutError):
                # 服务关闭连接时仍要等待进程退出，避免清理正在使用的数据库。
                pass
            try:
                process.wait(timeout=20)
            except subprocess.TimeoutExpired:
                process.terminate()
                process.wait(timeout=10)
                raise AssertionError("服务未按期退出")
            ai_server.shutdown()
            ai_server.server_close()
            ai_thread.join(timeout=2)
            if log_path:
                output.close()
            assert process.returncode == 0, f"服务退出异常：{process.returncode}"


if __name__ == "__main__":
    main(Path(sys.argv[1]).resolve())
