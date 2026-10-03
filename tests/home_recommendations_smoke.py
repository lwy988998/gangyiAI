"""用隔离档案验证启动推荐只调用一次、跨重启缓存和失败回退。"""

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
from contextlib import closing
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


def request(base, path, data=None, token=""):
    body = None if data is None else json.dumps(data).encode()
    with urllib.request.urlopen(urllib.request.Request(base + path, data=body,
            headers={"Content-Type": "application/json", "X-Gangyi-Control-Token": token}), timeout=5) as response:
        return json.loads(response.read())


def wait_for(predicate):
    deadline = time.monotonic() + 20
    while time.monotonic() < deadline:
        try:
            result = predicate()
            if result:
                return result
        except (OSError, ValueError):
            pass
        time.sleep(.1)
    raise AssertionError("推荐未在期限内完成")


def main(executable):
    calls = []
    mock = {"invalid": False}

    class Model(BaseHTTPRequestHandler):
        def log_message(self, *_):
            pass

        def do_POST(self):
            body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
            assert "首页学习目标推荐助手" in body["messages"][0]["content"]
            calls.append(body)
            result = {mode: {"continue": [f"{mode} 继续学习{i}" for i in range(3)],
                             "explore": [f"{mode} 探索新方向{i}" for i in range(2)]} for mode in ("lite", "deep")}
            if mock["invalid"]:
                result["deep"]["explore"] = ["重复", "重复"]
            payload = json.dumps({"choices": [{"message": {"content": json.dumps(result)}}]}).encode()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)

    model = ThreadingHTTPServer(("127.0.0.1", 0), Model)
    threading.Thread(target=model.serve_forever, daemon=True).start()
    with tempfile.TemporaryDirectory(prefix="gangyi-recommendations-") as directory:
        database = Path(directory) / "isolated.db"
        with socket.socket() as sock:
            sock.bind(("127.0.0.1", 0))
            port = sock.getsockname()[1]
        base = f"http://127.0.0.1:{port}"
        token = "recommendations-test"

        def run(launch, expected, key="test-only"):
            env = dict(os.environ, HOST="127.0.0.1", PORT=str(port), DATABASE_PATH=str(database),
                AI_BASE_URL=f"http://127.0.0.1:{model.server_port}/v1", AI_API_KEY=key,
                GANGYI_LAUNCH_SESSION_ID=launch, LOCAL_CONTROL_TOKEN=token)
            process = subprocess.Popen([str(executable)], cwd=executable.parent, env=env,
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            try:
                wait_for(lambda: request(base, "/health"))
                result = wait_for(lambda: (value if not value["updating"] else None)
                                  if (value := request(base, "/api/home/recommendations")) else None)
                for _ in range(12):
                    assert request(base, "/api/home/recommendations")["items"] == result["items"]
                assert len(calls) == expected, (launch, len(calls), expected)
                assert all(len(result["items"][mode]) == 5 for mode in ("lite", "deep"))
                return result
            finally:
                if process.poll() is None:
                    request(base, "/internal/shutdown", {}, token)
                    process.wait(timeout=20)
                assert process.returncode == 0, process.returncode

        first = run("launch-a", 1)
        assert first["status"] == "ready"
        assert json.loads(calls[0]["messages"][-1]["content"])["recentCourses"] == []
        assert run("launch-a", 1)["items"] == first["items"], "后台重启不得重复调用"
        with closing(sqlite3.connect(database)) as db, db:
            db.execute("INSERT INTO Course VALUES(?,?,?,?,?,?,?,?,?,?,?)", (
                "synthetic-course", None, None, "虚构测试目标", "deep", "虚构测试课程", None,
                "ai", "active", "2026-09-28", "2026-09-28"))
        run("launch-b", 2)
        context = json.loads(calls[1]["messages"][-1]["content"])
        assert context["recentCourses"][0]["courseId"] == "synthetic-course"
        assert "apiKey" not in json.dumps(context)
        mock["invalid"] = True
        failed = run("launch-c", 3)
        assert failed["status"] == "cached" and failed["items"] == first["items"]
        assert run("launch-c", 3)["items"] == first["items"]
        assert run("launch-d", 3, key="")["status"] == "cached"
        (Path(directory) / "home-recommendations.json").write_text("损坏的缓存", encoding="utf-8")
        assert run("launch-e", 4)["status"] == "fallback"
        print("HOME_RECOMMENDATIONS_SMOKE PASS：启动一次、刷新复用、后台重启、重新启动、隔离摘要及失败回退")
    model.shutdown()
    model.server_close()


if __name__ == "__main__":
    # Windows CI 默认西文编码，统一日志编码以保留中文验收信息。
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
    main(Path(sys.argv[1]).resolve())
