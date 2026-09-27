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
                mock_state["requests"].append(self.rfile.read(int(self.headers["Content-Length"])))
                profile = {"subjects": [{"subject": "数学", "score": mock_state["score"],
                    "rationale": "根据测验和学习记录", "weakPoints": ["二次函数"],
                    "recommendation": "复盘错题", "evidenceCount": 1}]}
                content = "[]" if mock_state["invalid"] else json.dumps(profile, ensure_ascii=False)
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
        process = subprocess.Popen([str(executable)], cwd=str(executable.parent), env=env,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
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
            assert "home-radar" in request(base, "/")[1]
            assert "ask.js" in request(base, "/ask")[1]
            assert request(base, "/dark.css")[0] == 200
            with closing(sqlite3.connect(database)) as db, db:
                db.execute("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)",
                           ("old", "old-id", None, "学习二次函数", "deep", "数学课程", None,
                            "ai", "active", "2026-01-01", "2026-01-01"))
                quiz = {"promptVersion": "ai-block-v1", "blocks": {"quiz": {"quiz": [
                    {"question": "一", "answerIndex": 0}, {"question": "二", "answerIndex": 1},
                    {"question": "三", "answerIndex": 2}]}}}
                db.execute("INSERT INTO LearningSession VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                           ("session", "old", "old-id", "学习二次函数", "deep", 1, "基础", 1,
                            "二次函数", "测试课", None, None, json.dumps(quiz), None, 0, "ai"))
                db.execute("INSERT INTO LearningInteraction VALUES(?,?,?,?,?,?,?)",
                           ("chat", None, "general", None, "chat-user", '{"text":"你好"}', "2026-01-01"))
            status, attempt = request(base, "/api/quiz-attempts", "POST",
                                      {"courseId": "old", "phaseIndex": 1, "topicIndex": 1,
                                       "answers": [0, 1, None]})
            assert status == 200 and attempt["score"] == 2 and attempt["total"] == 3 and not attempt["passed"]
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
            assert request(base, "/api/my-courses/old", "DELETE")[1]["ok"]
            with closing(sqlite3.connect(database)) as db, db:
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE courseId='old'").fetchone()[0] == 0
            print("API_SMOKE PASS: pages, quiz persistence, AI profile, retry, conversation clear, course deletion")
        finally:
            try:
                shutdown = urllib.request.Request(base + "/internal/shutdown", data=b"",
                    method="POST", headers={"X-Gangyi-Control-Token": "smoke-only"})
                urllib.request.urlopen(shutdown, timeout=2).close()
                process.wait(timeout=10)
            except (urllib.error.URLError, subprocess.TimeoutExpired):
                process.terminate()
                process.wait(timeout=10)
            ai_server.shutdown()
            ai_server.server_close()
            ai_thread.join(timeout=2)


if __name__ == "__main__":
    main(Path(sys.argv[1]).resolve())
