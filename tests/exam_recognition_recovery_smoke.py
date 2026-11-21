#!/usr/bin/env python3
"""以独立进程和数据库验证模型不可用时的识别恢复，不使用用户模型或档案。"""

import base64
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time
from urllib.error import HTTPError, URLError
from urllib.request import Request, build_opener, ProxyHandler


PNG = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/8fkAAAAASUVORK5CYII=")
HTTP = build_opener(ProxyHandler({}))


def free_port():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def main(executable):
    with tempfile.TemporaryDirectory(prefix="gangyi-exam-recovery-") as directory:
        env = dict(os.environ, DATABASE_PATH=str(Path(directory) / "fixture.db"), HOST="127.0.0.1",
                   PORT=str(free_port()), AI_BASE_URL="http://127.0.0.1:1/v1", AI_API_KEY="fixture-only",
                   AI_MODEL="fixture", BOCHA_API_KEY="", DEEPSEEK_SEARCH_API_KEY="", DEEPSEEK_API_KEY="",
                   LOCAL_CONTROL_TOKEN="fixture-only", GANGYI_LAUNCH_SESSION_ID="exam-recovery-fixture")
        base = "http://127.0.0.1:" + env["PORT"]
        process = None

        def launch():
            return subprocess.Popen([str(Path(executable).resolve())], cwd=str(Path(executable).resolve().parent), env=env,
                                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                                    creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)

        def request(path, payload=None):
            data = None if payload is None else json.dumps(payload, ensure_ascii=False).encode("utf-8")
            req = Request(base + path, data=data, headers={"Content-Type": "application/json"})
            try:
                with HTTP.open(req, timeout=10) as response:
                    return response.status, json.loads(response.read())
            except HTTPError as error:
                return error.code, json.loads(error.read())

        def wait_server():
            end = time.monotonic() + 30
            while time.monotonic() < end:
                if process.poll() is not None:
                    raise AssertionError("隔离服务提前退出")
                try:
                    if request("/api/exam-explanations")[0] == 200:
                        return
                except URLError:
                    time.sleep(0.2)
            raise AssertionError("隔离服务启动超时")

        def wait_status(exam_id, expected):
            end = time.monotonic() + 45
            while time.monotonic() < end:
                status, value = request("/api/exam-explanations/" + exam_id)
                assert status == 200
                if value.get("status") == expected:
                    return value
                time.sleep(0.25)
            raise AssertionError("会话没有进入预期状态：" + expected)

        try:
            process = launch()
            wait_server()
            image = "data:image/png;base64," + base64.b64encode(PNG).decode("ascii")
            status, created = request("/api/exam-explanations", {"requestId": "upload-1", "title": "隔离测试",
                "pages": [{"name": "第1页.png", "dataUrl": image}],
                "originalFiles": [{"name": "原图.png", "dataUrl": image}]})
            assert status == 202 and created.get("id")
            exam_id = created["id"]
            failed = wait_status(exam_id, "failed")
            assert failed["progress"]["current"] == 0 and len(failed["pages"]) == 1
            status, refusal = request(f"/api/exam-explanations/{exam_id}/messages", {"requestId": "message-1", "text": "下一题"})
            assert status == 400 and "识别" in refusal.get("error", "")
            process.terminate()
            process.wait(timeout=15)
            process = launch()
            wait_server()
            restored = wait_status(exam_id, "failed")
            assert not restored["messages"] and len(restored["pages"]) == 1
            with HTTP.open(base + f"/api/exam-explanations/{exam_id}/files/0", timeout=10) as response:
                assert response.read() == PNG
            status, retry = request(f"/api/exam-explanations/{exam_id}/recognition-retries", {"requestId": "retry-1"})
            assert status == 202 and retry.get("task", {}).get("id")
            failed_again = wait_status(exam_id, "failed")
            assert failed_again["progress"]["current"] == 0
            status, duplicate = request(f"/api/exam-explanations/{exam_id}/recognition-retries", {"requestId": "retry-1"})
            assert status == 200 and duplicate.get("duplicate") is True
            assert len(request("/api/exam-explanations")[1]["sessions"]) == 1
            print("识别恢复接口通过：失败保留、禁止中断、重启恢复、原件一致、重试幂等。")
        finally:
            if process and process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=15)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()


if __name__ == "__main__":
    main(sys.argv[1])
