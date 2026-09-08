#!/usr/bin/env python3
"""使用已有环境变量在隔离实例中验收真实 AI 与联网搜索。"""

import http.client
import json
import os
from pathlib import Path
import ssl
import subprocess
import sys
import tempfile
import time
import urllib.request
import uuid


REPO_ROOT = Path(__file__).resolve().parent.parent
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else REPO_ROOT / "build" / "gangyiAI"
PORT = int(os.environ.get("GANGYI_LIVE_TEST_PORT", "39422"))
CONTROL_TOKEN = "live-acceptance-control-token"


def local_request(path, method="GET", payload=None, headers=None, expected=200, timeout=240):
    body = payload
    request_headers = dict(headers or {})
    if isinstance(payload, (dict, list)):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        request_headers.setdefault("Content-Type", "application/json")
    connection = http.client.HTTPConnection("127.0.0.1", PORT, timeout=timeout)
    connection.request(method, path, body=body, headers=request_headers)
    response = connection.getresponse()
    data = response.read()
    connection.close()
    if response.status != expected:
        detail = ""
        try:
            parsed = json.loads(data.decode("utf-8"))
            detail = parsed.get("type") or parsed.get("error") or parsed.get("message") or ""
        except (UnicodeDecodeError, json.JSONDecodeError):
            pass
        raise AssertionError(f"{method} {path} 状态应为 {expected}，实际为 {response.status}，类型：{detail}")
    return data


def local_json(path, method="GET", payload=None, expected=200, timeout=240):
    return json.loads(local_request(path, method, payload, expected=expected, timeout=timeout).decode("utf-8"))


def has_chinese(value):
    return isinstance(value, str) and any("\u4e00" <= character <= "\u9fff" for character in value)


def multipart(filename, content_type, content, fields):
    boundary = "----gangyi-live-" + uuid.uuid4().hex
    parts = []
    for name, value in fields.items():
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


def models_endpoint(base_url):
    url = base_url.rstrip("/")
    if url.endswith("/chat/completions"):
        url = url[: -len("/chat/completions")]
    return url if url.endswith("/models") else url + "/models"


def verify_model_list(environment):
    request = urllib.request.Request(
        models_endpoint(environment["AI_BASE_URL"]),
        headers={"Authorization": "Bearer " + environment["AI_API_KEY"]},
    )
    with urllib.request.urlopen(request, timeout=15, context=ssl.create_default_context()) as response:
        parsed = json.loads(response.read().decode("utf-8"))
    models = sorted({item.get("id", "") for item in parsed.get("data", []) if item.get("id")})
    if not models:
        raise AssertionError("真实服务模型列表为空")
    return len(models), environment["AI_MODEL"] in models


def main():
    required = ("AI_BASE_URL", "AI_API_KEY", "AI_MODEL", "BOCHA_API_KEY")
    missing = [name for name in required if not os.environ.get(name)]
    if missing:
        raise RuntimeError("缺少真实验收环境变量：" + "、".join(missing))
    if not BINARY.is_file():
        raise FileNotFoundError(f"服务程序不存在：{BINARY}")

    environment = os.environ.copy()
    environment.pop("AI_DEBUG", None)
    model_count, configured_model_listed = verify_model_list(environment)
    results = {"modelCount": model_count, "configuredModelListed": configured_model_listed}

    with tempfile.TemporaryDirectory(prefix="gangyiAI-live-") as temporary:
        work = Path(temporary)
        log_path = work / "service.log"
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
        goal = "高中数学函数单调性真实验收"
        with log_path.open("wb") as log_file:
            process = subprocess.Popen(
                [str(BINARY)], cwd=REPO_ROOT, env=environment,
                stdout=log_file, stderr=subprocess.STDOUT,
            )
            try:
                for _ in range(100):
                    try:
                        if local_json("/health", timeout=1).get("status") == "healthy":
                            break
                    except (OSError, AssertionError, json.JSONDecodeError):
                        time.sleep(0.1)
                else:
                    raise AssertionError("真实 AI 验收实例未能启动")

                answer = local_json("/api/ask", "POST", {"question": "用两句话解释函数单调性。"})
                assert answer.get("ok") is True and has_chinese(answer.get("answer"))
                assert answer.get("model")
                results["askModel"] = answer["model"]

                classroom = local_json(
                    "/api/learning-chat", "POST",
                    {
                        "question": "为什么要比较自变量大小？",
                        "goal": goal,
                        "mode": "deep",
                        "phaseName": "概念基础",
                        "topic": "函数单调性",
                        "contextTitle": "单调性的定义",
                        "contextSummary": "理解区间内函数值随自变量变化的规律。",
                        "messages": [],
                    },
                )
                assert classroom.get("ok") is True and has_chinese(classroom.get("answer"))
                assert classroom.get("model")
                results["classroomModel"] = classroom["model"]

                plans = {}
                for mode in ("lite", "deep"):
                    plan = local_json(
                        "/api/generate-plan", "POST",
                        {"goal": goal, "mode": mode, "bypassCache": True},
                    )
                    assert plan.get("title") and has_chinese(plan.get("summary", ""))
                    assert len(plan.get("roadmap", [])) >= (3 if mode == "lite" else 4)
                    assert plan.get("courseStructure")
                    plans[mode] = plan
                    results[mode + "Fallback"] = "qualityNotice" in plan

                if os.environ.get("GANGYI_PLAN_DIAGNOSTICS") == "1":
                    local_request(
                        "/internal/shutdown", "POST",
                        headers={"X-Gangyi-Control-Token": CONTROL_TOKEN},
                    )
                    process.wait(timeout=5)
                    log_file.flush()
                    log_contents = log_path.read_text("utf-8", errors="replace")
                    results["planEvents"] = [
                        line for line in log_contents.splitlines()
                        if line.startswith("[generate-plan]")
                    ]
                    print(json.dumps(results, ensure_ascii=False, sort_keys=True))
                    return

                assert results["liteFallback"] is False
                assert results["deepFallback"] is False

                deep_plan = plans["deep"]
                resources = deep_plan.get("resources", [])
                assert resources and all(str(item.get("href", "")).startswith(("http://", "https://")) for item in resources)
                results["resourceCount"] = len(resources)
                results["resourceMessage"] = deep_plan.get("resourceSourceMessage", "")

                saved = local_json(
                    "/api/courses", "POST",
                    {
                        "goal": goal,
                        "title": deep_plan["title"],
                        "summary": deep_plan.get("summary", ""),
                        "mode": "deep",
                        "source": "live-acceptance",
                        "anonymousId": "live-acceptance-user",
                        "payload": deep_plan,
                    },
                )
                course_id = saved["courseId"]
                first_stage = deep_plan["roadmap"][0]
                stage_name = first_stage.get("name") or first_stage.get("stage") or "第一阶段"
                topics = deep_plan.get("courseStructure", [{}])[0].get("topics", [])
                if not topics:
                    topics = [item.get("title", "") for item in first_stage.get("steps", []) if item.get("title")]
                assert topics

                phase = local_json(
                    "/api/phase-expansion", "POST",
                    {
                        "courseId": course_id,
                        "anonymousId": "live-acceptance-user",
                        "goal": goal,
                        "mode": "deep",
                        "phaseIndex": 1,
                        "stage": stage_name,
                        "topics": topics[:4],
                    },
                )
                assert phase.get("ok") is True and phase.get("phase", {}).get("steps")
                results["phaseSteps"] = len(phase["phase"]["steps"])

                lesson = local_json(
                    f"/api/learn?courseId={course_id}&anonymousId=live-acceptance-user&phaseIndex=1&topicIndex=1",
                    timeout=300,
                )
                assert lesson.get("ok") is True
                assert has_chinese(lesson.get("summary", ""))
                assert lesson.get("lessonSteps") and lesson.get("practice") and lesson.get("quiz")
                results["lessonFallback"] = bool(lesson.get("_fallbackUsed"))

                image_path = REPO_ROOT / "tests" / "fixtures" / "function-monotonicity.png"
                image_body, image_headers = multipart(
                    "函数单调性题目.png", "image/png", image_path.read_bytes(),
                    {"prompt": "请识别图片中的数学题并生成学习目标", "mode": "deep"},
                )
                image_result = json.loads(
                    local_request(
                        "/api/analyze-image-goal", "POST", image_body, image_headers,
                        timeout=300,
                    ).decode("utf-8")
                )
                if image_result.get("success") is not True:
                    raise AssertionError("真实图片识别失败，类型：" + image_result.get("type", "unknown"))
                assert has_chinese(image_result.get("goal", ""))
                results["imageRecognition"] = True

                local_request(
                    "/internal/shutdown", "POST",
                    headers={"X-Gangyi-Control-Token": CONTROL_TOKEN},
                )
                process.wait(timeout=5)
                log_file.flush()
                log_contents = log_path.read_text("utf-8", errors="replace")
                for secret in (environment["AI_API_KEY"], environment["BOCHA_API_KEY"], CONTROL_TOKEN, goal):
                    assert secret not in log_contents
                results["planEvents"] = [
                    line for line in log_contents.splitlines()
                    if line.startswith("[generate-plan]")
                ]
            finally:
                if process.poll() is None:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait(timeout=3)

    print(json.dumps(results, ensure_ascii=False, sort_keys=True))


if __name__ == "__main__":
    main()
