#!/usr/bin/env python3
"""以隔离服务和档案验证试卷识别与讲解在异常回复后的恢复。"""

import base64
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
from threading import Thread
import time
from urllib.error import HTTPError, URLError
from urllib.request import Request, build_opener, ProxyHandler


PNG = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/8fkAAAAASUVORK5CYII=")
HTTP = build_opener(ProxyHandler({}))


class ExamModel(BaseHTTPRequestHandler):
    page_batches = []
    lesson_calls = 0

    def log_message(self, *_):
        pass

    def do_POST(self):
        request = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        messages = request.get('messages', [])
        system = messages[0].get('content', '') if messages else ''
        finish = 'stop'
        if '试卷识别器' in system:
            page_count = sum(1 for msg in messages if isinstance(msg.get('content'), list)
                             for part in msg['content'] if part.get('type') == 'image_url')
            self.page_batches.append(page_count)
            if self.page_batches == [4, 1]:
                body = '{"title":"截断","questions":['
                finish = 'length'
            elif self.page_batches == [4, 1, 1]:
                body = '{"title":"无效","questions":{}}'
            else:
                number = '1' if page_count == 4 else '5'
                body = json.dumps({'title': '五页隔离试卷', 'questions': [{
                    'id': 'q' + number, 'number': number, 'pageIndex': 0, 'stem': '隔离样题',
                    'recognitionConfidence': .91,
                }]}, ensure_ascii=False)
        elif '试卷讲解老师' in system:
            type(self).lesson_calls += 1
            body = '{"message":"不应展示的坏回复","artifacts":[' if self.lesson_calls == 1 else json.dumps({
                'message': '先看题中给出的条件。', 'artifacts': [], 'currentQuestionId': 'q1', 'state': 'continue',
            }, ensure_ascii=False)
        else:
            body = '{}'
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream')
        self.end_headers()
        frame = {'model': 'fixture', 'choices': [{'delta': {'content': body}, 'finish_reason': finish}]}
        self.wfile.write(('data: ' + json.dumps(frame, ensure_ascii=False) + '\n\ndata: [DONE]\n\n').encode())


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
        model = None

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

            # 改用隔离模型重现坏回复，验证第二批失败不会抹掉之前的识别结果。
            process.terminate()
            process.wait(timeout=15)
            model = ThreadingHTTPServer(('127.0.0.1', 0), ExamModel)
            Thread(target=model.serve_forever, daemon=True).start()
            env['AI_BASE_URL'] = f'http://127.0.0.1:{model.server_port}/v1'
            process = launch()
            wait_server()
            status, created = request('/api/exam-explanations', {'requestId': 'malformed-upload',
                'title': '模型格式异常',
                'pages': [{'name': f'第{i}页.png', 'dataUrl': image} for i in range(1, 6)]})
            assert status == 202
            exam_id = created['id']
            failed = wait_status(exam_id, 'failed')
            assert failed['progress']['current'] == 4, failed['progress']
            assert [q['id'] for q in failed['questions']] == ['q1']
            assert ExamModel.page_batches == [4, 1, 1], ExamModel.page_batches
            status, _ = request(f'/api/exam-explanations/{exam_id}/recognition-retries',
                                {'requestId': 'resume-malformed'})
            assert status == 202
            ready = wait_status(exam_id, 'ready')
            assert ready['progress']['current'] == 5 and [q['pageIndex'] for q in ready['questions']] == [0, 4]
            assert ExamModel.page_batches == [4, 1, 1, 1]
            end = time.monotonic() + 25
            while time.monotonic() < end:
                _, session = request(f'/api/exam-explanations/{exam_id}')
                replies = [m['content'] for m in session['messages'] if m.get('role') == 'assistant']
                if replies:
                    break
                time.sleep(.15)
            assert replies == ['先看题中给出的条件。'], replies
            assert ExamModel.lesson_calls == 2
            print('试卷格式恢复通过：先保留四页，单独恢复第五页，坏讲解没有写入，恢复后只保存一次。')
        finally:
            if process and process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=15)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            if model:
                model.shutdown()
                model.server_close()


if __name__ == "__main__":
    main(sys.argv[1])
