#!/usr/bin/env python3
"""在隔离目录中验收 Linux 服务，不接触正式数据库。"""

import http.client
import json
import os
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
import time
import uuid


REPO_ROOT = Path(__file__).resolve().parent.parent
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else REPO_ROOT / "build" / "gangyiAI"
PORT = int(os.environ.get("GANGYI_TEST_PORT", "39421"))
CONTROL_TOKEN = "acceptance-control-token"


def request(path, method="GET", payload=None, headers=None, expected=200, timeout=180):
    body = payload
    actual_headers = dict(headers or {})
    if isinstance(payload, (dict, list)):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        actual_headers.setdefault("Content-Type", "application/json")
    connection = http.client.HTTPConnection("127.0.0.1", PORT, timeout=timeout)
    connection.request(method, path, body=body, headers=actual_headers)
    response = connection.getresponse()
    data = response.read()
    response_headers = dict(response.getheaders())
    connection.close()
    if response.status != expected:
        raise AssertionError(
            f"{method} {path} 状态应为 {expected}，实际为 {response.status}："
            f"{data[:500].decode('utf-8', errors='replace')}"
        )
    return data, response_headers


def request_json(path, method="GET", payload=None, expected=200, timeout=180):
    data, _ = request(path, method, payload, expected=expected, timeout=timeout)
    return json.loads(data.decode("utf-8"))


def multipart(filename, content_type, content, fields=None):
    boundary = "----gangyi-" + uuid.uuid4().hex
    parts = []
    for name, value in (fields or {}).items():
        parts.append(
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"{name}\"\r\n\r\n"
            f"{value}\r\n".encode("utf-8")
        )
    parts.append(
        f"--{boundary}\r\nContent-Disposition: form-data; name=\"image\"; "
        f"filename=\"{filename}\"\r\nContent-Type: {content_type}\r\n\r\n".encode("utf-8")
        + content
        + b"\r\n"
    )
    parts.append(f"--{boundary}--\r\n".encode("ascii"))
    return b"".join(parts), {"Content-Type": f"multipart/form-data; boundary={boundary}"}


def contains(data, text):
    if text not in data.decode("utf-8"):
        raise AssertionError(f"响应缺少文本：{text}")


def main():
    if not BINARY.is_file():
        raise FileNotFoundError(f"服务程序不存在：{BINARY}")

    with tempfile.TemporaryDirectory(prefix="gangyiAI-runtime-") as temporary:
        work = Path(temporary)
        log_path = work / "service.log"
        environment = os.environ.copy()
        environment.update(
            {
                "HOST": "127.0.0.1",
                "PORT": str(PORT),
                "LOCAL_CONTROL_TOKEN": CONTROL_TOKEN,
                "DATABASE_PATH": str(work / "gangyiAI.db"),
                "AI_PLAN_CACHE_DIR": str(work / "plan-cache"),
                "RESOURCE_SEARCH_CACHE_DIR": str(work / "search-cache"),
            }
        )
        environment.setdefault("AI_BASE_URL", "http://127.0.0.1:9/v1")
        environment.setdefault("AI_API_KEY", "acceptance-placeholder-key")
        environment.setdefault("AI_MODEL", "acceptance-model")

        with log_path.open("wb") as log_file:
            process = subprocess.Popen(
                [str(BINARY)], cwd=REPO_ROOT, env=environment,
                stdout=log_file, stderr=subprocess.STDOUT,
            )
            try:
                for _ in range(100):
                    try:
                        if request_json("/health", timeout=1).get("status") == "healthy":
                            break
                    except (OSError, AssertionError, json.JSONDecodeError):
                        time.sleep(0.1)
                else:
                    raise AssertionError("服务未能通过健康检查")

                for path in ("/", "/ask", "/my-courses"):
                    data, _ = request(path)
                    contains(data, "钢一定制AI")
                for path, mime in (
                    ("/styles.css", "text/css"),
                    ("/school-logo.png", "image/png"),
                    ("/campus-background.jpg", "image/jpeg"),
                ):
                    _, headers = request(path)
                    if mime not in headers.get("Content-Type", ""):
                        raise AssertionError(f"{path} 的 MIME 类型错误")
                request("/../CMakeLists.txt", expected=404)

                invalid = request_json("/api/ask", "POST", b"{", expected=400)
                assert invalid["error"] == "请求内容格式不正确。"
                missing_goal = request_json("/api/generate-plan", "POST", {"mode": "lite"}, expected=400)
                assert missing_goal["error"] == "请填写学习目标。"

                body, headers = multipart("中文文件名.txt", "text/plain", b"not an image")
                request("/api/analyze-image-goal", "POST", body, headers, expected=400)
                body, headers = multipart("oversize.png", "image/png", b"0" * (6 * 1024 * 1024))
                request("/api/analyze-image-goal", "POST", body, headers, expected=413)

                goal = "高中数学函数单调性验收"
                plan = request_json(
                    "/api/generate-plan", "POST",
                    {"goal": goal, "mode": "lite", "bypassCache": True},
                )
                assert plan.get("title")
                assert len(plan.get("roadmap", [])) >= 3
                assert plan.get("courseStructure")
                saved = request_json(
                    "/api/courses", "POST",
                    {
                        "goal": goal,
                        "title": plan["title"],
                        "summary": plan.get("summary", ""),
                        "mode": "lite",
                        "source": "acceptance",
                        "anonymousId": "acceptance-user",
                        "payload": plan,
                    },
                )
                course_id = saved["courseId"]

                courses = request_json("/api/courses?anonymousId=acceptance-user")["courses"]
                assert any(item["id"] == course_id for item in courses)
                request(f"/api/courses/{course_id}?anonymousId=another-user", expected=404)
                restored = request_json(f"/api/courses/{course_id}?anonymousId=acceptance-user")
                assert restored["course"]["id"] == course_id
                for path, marker in (
                    (f"/plan?courseId={course_id}&anonymousId=acceptance-user", "课程规划"),
                    (f"/phase?courseId={course_id}&anonymousId=acceptance-user&phaseIndex=1", "阶段验收清单"),
                    (f"/learn?courseId={course_id}&anonymousId=acceptance-user&phaseIndex=1&topicIndex=1", "微课程"),
                ):
                    data, _ = request(path)
                    contains(data, marker)

                common = {
                    "courseId": course_id,
                    "anonymousId": "acceptance-user",
                    "goal": goal,
                    "mode": "lite",
                    "phaseIndex": 1,
                    "phaseName": "基础阶段",
                }
                request_json(
                    "/api/task-progress", "POST",
                    {**common, "taskIndex": 0, "taskTitle": "任务一", "status": "completed"},
                )
                tasks = request_json(
                    f"/api/task-progress?courseId={course_id}&anonymousId=acceptance-user&phaseIndex=1"
                )
                assert tasks["items"][0]["status"] == "completed"

                request_json(
                    "/api/learning-step-progress", "POST",
                    {**common, "stepIndex": 0, "stepTitle": "步骤一", "status": "understood"},
                )
                steps = request_json(
                    f"/api/learning-step-progress?courseId={course_id}&anonymousId=acceptance-user&phaseIndex=1"
                )
                assert steps["items"][0]["status"] == "understood"

                request_json(
                    "/api/learn/progress", "POST",
                    {
                        **common,
                        "topicIndex": 1,
                        "topicTitle": "函数单调性",
                        "status": "completed",
                        "lastVisitedUrl": "/learn?courseId=local",
                        "lastPageType": "learn",
                        "lastPhaseIndex": 1,
                        "lastTopicIndex": 1,
                    },
                )
                progress = request_json(
                    f"/api/course-progress?courseId={course_id}&anonymousId=acceptance-user"
                )["progress"]
                assert progress["completedCount"] == 3
                assert progress["overallPercent"] > 0

                reset = request_json(
                    "/api/course-progress", "POST",
                    {"action": "reset", "courseId": course_id,
                     "anonymousId": "acceptance-user", "goal": goal},
                )["progress"]
                assert reset["completedCount"] == 0
                assert reset["overallPercent"] == 0

                request(
                    "/internal/shutdown", "POST", headers={"X-Gangyi-Control-Token": "wrong-token"},
                    expected=403,
                )
                request(f"/api/my-courses/{course_id}?anonymousId=another-user", "DELETE", expected=404)
                request(f"/api/my-courses/{course_id}?anonymousId=acceptance-user", "DELETE")
                request(f"/api/courses/{course_id}?anonymousId=acceptance-user", expected=404)

                with sqlite3.connect(work / "gangyiAI.db") as database:
                    assert database.execute("PRAGMA quick_check").fetchone()[0] == "ok"

                request(
                    "/internal/shutdown", "POST",
                    headers={"X-Gangyi-Control-Token": CONTROL_TOKEN},
                )
                process.wait(timeout=5)
                log_file.flush()
                log_contents = log_path.read_text("utf-8", errors="replace")
                for secret in (environment["AI_API_KEY"], CONTROL_TOKEN, goal):
                    assert secret not in log_contents
            finally:
                if process.poll() is None:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait(timeout=3)

    print("[通过] Linux 运行时验收完成。")


if __name__ == "__main__":
    main()
