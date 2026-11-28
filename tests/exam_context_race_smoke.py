#!/usr/bin/env python3
"""用隔离进程和本机假模型验证试卷上下文并发更新，不访问真实档案或网络。"""

import base64
import hashlib
import json
import os
from pathlib import Path
import socket
import sqlite3
import subprocess
import sys
import tempfile
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from contextlib import closing, nullcontext
from urllib.error import HTTPError, URLError
from urllib.request import ProxyHandler, Request, build_opener


PNG = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/8fkAAAAASUVORK5CYII=")
HTTP = build_opener(ProxyHandler({}))
SOURCE_URL = "https://example.org/fixture-paper"
QUESTION_QUOTE = "Question one statement with enough text"
ANSWER_QUOTE = "Answer A"


def canonical(value):
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))


def digest(value):
    return hashlib.sha256(canonical(value).encode("utf-8")).hexdigest()


def free_port():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def eventually(fn, seconds=40):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        value = fn()
        if value:
            return value
        time.sleep(0.1)
    raise AssertionError("等待测试条件超时")


class FakeModel(BaseHTTPRequestHandler):
    entered = threading.Event()
    release = threading.Event()
    first_sent = threading.Event()
    received = []

    def log_message(self, *_args):
        pass

    def do_POST(self):
        body = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
        messages = body.get("messages", [])
        system = messages[0].get("content", "") if messages else ""
        user = messages[-1].get("content", "") if messages else ""
        if not isinstance(user, str):
            user = ""
        recognition = "试卷识别器" in system
        race = next((name for name in ("race-question", "race-revoke", "race-control") if name in user), "")
        type(self).received.append({"race": race, "system": system})
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Connection", "close")
        self.end_headers()

        def send(content, finish=None):
            event = {"choices": [{"delta": {"content": content}, "finish_reason": finish}]}
            self.wfile.write(("data: " + json.dumps(event, ensure_ascii=False) + "\n\n").encode("utf-8"))
            self.wfile.flush()

        try:
            if race:
                marker = "STALE-" + race
                send('{"message":"' + marker)
                type(self).first_sent.set()
                type(self).entered.set()
                if not type(self).release.wait(30):
                    return
                send('","artifacts":[],"currentQuestionId":"q1","state":"continue"}', "stop")
            elif recognition:
                result = {"title": "隔离试卷", "questions": [{"id": "q1", "number": "1", "stem": QUESTION_QUOTE,
                    "subject": "历史", "type": "选择题", "materials": [], "subquestions": [],
                    "recognitionConfidence": 0.98, "pageIndex": 0}]}
                send(json.dumps(result, ensure_ascii=False), "stop")
            else:
                result = {"message": "隔离测试回复", "artifacts": [], "currentQuestionId": "q1", "state": "continue"}
                send(json.dumps(result, ensure_ascii=False), "stop")
            self.wfile.write(b"data: [DONE]\n\n")
            self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            pass


def seed_snapshot(database, exam_id):
    segments = [{"id": "s1", "text": QUESTION_QUOTE}, {"id": "s2", "text": ANSWER_QUOTE}]
    fingerprint = digest([SOURCE_URL, segments])
    snapshot_id = "exam-source-snapshot:" + digest([exam_id, SOURCE_URL, fingerprint])
    current_id = "exam-source-current:" + digest([exam_id, SOURCE_URL])
    snapshot = {"id": snapshot_id, "examId": exam_id, "sourceUrl": SOURCE_URL, "finalUrl": SOURCE_URL,
                "digest": fingerprint, "checkedAt": "2026-01-01T00:00:00Z", "status": "read",
                "coverage": "fixture", "segments": segments}
    current = {"examId": exam_id, "sourceUrl": SOURCE_URL, "snapshotId": snapshot_id}
    with closing(sqlite3.connect(database, timeout=15)) as db, db:
        for key, kind, payload in ((snapshot_id, "exam-source-snapshot", snapshot),
                                   (current_id, "exam-source-current", current)):
            db.execute("INSERT INTO MyAIRecord(id,kind,payload,updatedAt,version) VALUES(?,?,?,?,1)",
                       (key, kind, canonical(payload), "2026-01-01T00:00:00Z"))
    return snapshot_id


def main(executable):
    exe = Path(executable).resolve(strict=True)
    model = ThreadingHTTPServer(("127.0.0.1", 0), FakeModel)
    model.daemon_threads = True
    threading.Thread(target=model.serve_forever, daemon=True).start()
    process = None
    temporary = tempfile.TemporaryDirectory(prefix="gangyi-exam-context-race-")
    try:
        with nullcontext(temporary.name) as directory:
            database = str(Path(directory) / "fixture.db")
            port = free_port()
            env = dict(os.environ, DATABASE_PATH=database, HOST="127.0.0.1", PORT=str(port),
                       AI_BASE_URL=f"http://127.0.0.1:{model.server_port}/v1", AI_API_KEY="fixture-only",
                       AI_MODEL="fixture", BOCHA_API_KEY="", DEEPSEEK_SEARCH_API_KEY="", DEEPSEEK_API_KEY="",
                       LOCAL_CONTROL_TOKEN="fixture-only", GANGYI_LAUNCH_SESSION_ID="exam-context-race",
                       RESOURCE_SEARCH_CACHE_DIR=str(Path(directory) / "search-cache"))
            base = f"http://127.0.0.1:{port}"
            process = subprocess.Popen([str(exe)], cwd=str(exe.parent), env=env, stdout=subprocess.DEVNULL,
                                       stderr=subprocess.DEVNULL,
                                       creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)

            def request(path, payload=None, method=None):
                data = None if payload is None else json.dumps(payload, ensure_ascii=False).encode("utf-8")
                req = Request(base + path, data=data, method=method, headers={"Content-Type": "application/json"})
                try:
                    with HTTP.open(req, timeout=10) as response:
                        return response.status, json.loads(response.read())
                except HTTPError as error:
                    return error.code, json.loads(error.read())

            def ready():
                if process.poll() is not None:
                    raise AssertionError("隔离服务提前退出")
                try:
                    return request("/api/exam-explanations")[0] == 200
                except URLError:
                    return False

            def api(path, payload=None, method=None, expected=200):
                status, value = request(path, payload, method)
                if status != expected:
                    raise AssertionError(f"接口 {path} 返回 {status}: {value}")
                return value

            def task(task_id):
                return api("/api/learning-agent?taskId=" + task_id)

            def done(task_id):
                value = task(task_id)
                return value if value.get("status") not in ("pending", "running") else None

            eventually(ready)
            image = "data:image/png;base64," + base64.b64encode(PNG).decode("ascii")
            created = api("/api/exam-explanations", {"requestId": "fixture-upload", "title": "隔离试卷",
                "pages": [{"name": "page.png", "dataUrl": image}]}, expected=202)
            exam_id = created["id"]
            exam_path = "/api/exam-explanations/" + exam_id
            first = eventually(lambda: done(created["task"]["id"]))
            assert first.get("status") == "ready", first
            detail = api(exam_path)
            if detail.get("activeTaskId"):
                opening = eventually(lambda: done(detail["activeTaskId"]))
                assert opening.get("status") == "ready", opening
            assert api(exam_path)["currentQuestionId"] == "q1"
            snapshot_id = seed_snapshot(database, exam_id)
            confirm = {"questionId": "q1", "snapshotId": snapshot_id, "questionSegmentId": "s1",
                       "questionQuote": QUESTION_QUOTE, "answerSegmentId": "s2", "answerQuote": ANSWER_QUOTE,
                       "confirmed": True}

            def confirm_source():
                result = api(exam_path + "/source-confirmations", confirm)
                assert result.get("gradingEligible") is True, result
                values = api(exam_path + "/source-confirmations")["confirmations"]
                assert any(item.get("gradingEligible") for item in values)

            def race(name, change):
                FakeModel.entered.clear()
                FakeModel.release.clear()
                FakeModel.first_sent.clear()
                queued = api(exam_path + "/messages", {"requestId": name, "text": name}, expected=202)
                task_id = queued["task"]["id"]
                assert FakeModel.entered.wait(30), "假模型没有收到教学请求"
                assert FakeModel.first_sent.is_set(), "假模型没有发送首个流式片段"
                call = next(item for item in reversed(FakeModel.received) if item["race"] == name)
                assert ANSWER_QUOTE in call["system"] and '"confirmedSources"' in call["system"], "测试请求未携带确认来源"
                marker = "STALE-" + name
                deadline = time.monotonic() + 1.5
                while time.monotonic() < deadline:
                    state = task(task_id)
                    assert state.get("status") == "running", "假模型暂停时任务应保持运行"
                    assert marker not in canonical(state), "旧上下文回复在提交前已经流向界面"
                    time.sleep(0.1)
                change()
                FakeModel.release.set()
                final = eventually(lambda: done(task_id))
                assert marker not in canonical(final), "过期回复出现在任务状态或事件中"
                detail = api(exam_path)
                assert all(marker not in item.get("content", "") for item in detail["messages"]), "过期回复被保存"

            confirm_source()
            race("race-question", lambda: api(exam_path, {"questionId": "q1", "text": "用户已修正题干"}, "PATCH"))
            confirm_source()
            race("race-revoke", lambda: api(exam_path + "/source-revocations", {"questionId": "q1", "sourceUrl": SOURCE_URL}))
            current = api(exam_path + "/source-confirmations")["confirmations"]
            assert not any(item.get("gradingEligible") for item in current), "撤销后来源仍可评分"
            FakeModel.entered.clear()
            FakeModel.release.clear()
            FakeModel.first_sent.clear()
            control = api(exam_path + "/messages", {"requestId": "race-control", "text": "race-control"}, expected=202)
            control_id = control["task"]["id"]
            assert FakeModel.entered.wait(30), "普通教学的假模型请求未到达"
            def receiving_control():
                state = task(control_id)
                assert "STALE-race-control" not in canonical(state), "尚未校验的普通回复已流向界面"
                return state.get("activity", {}).get("phase") == "receiving"
            eventually(receiving_control, seconds=10)
            FakeModel.release.set()
            assert eventually(lambda: done(control_id)).get("status") == "ready", "普通教学应正常完成"
            assert any("STALE-race-control" in item.get("content", "") for item in api(exam_path)["messages"]), "对照回复未保存"
            print("试卷上下文并发测试通过：题目修正、来源撤销、流式预泄露和旧回复保存。")
    finally:
        FakeModel.release.set()
        if process and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        model.shutdown()
        model.server_close()
        temporary.cleanup()


if __name__ == "__main__":
    main(sys.argv[1])
