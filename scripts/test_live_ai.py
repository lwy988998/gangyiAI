#!/usr/bin/env python3
# 【逐行批注】 执行当前 Python 语句："""使用已有环境变量在隔离实例中验收真实 AI 与联网搜索。"""。
"""使用已有环境变量在隔离实例中验收真实 AI 与联网搜索。"""

# 【逐行批注】 导入后续逻辑需要的模块或符号：import http.client。
import http.client
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import ssl。
import ssl
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：import urllib.request。
import urllib.request
# 【逐行批注】 导入后续逻辑需要的模块或符号：import uuid。
import uuid


# 【逐行批注】 计算右侧结果并赋给 REPO_ROOT，更新脚本状态。
REPO_ROOT = Path(__file__).resolve().parent.parent
# 【逐行批注】 计算右侧结果并赋给 BINARY，更新脚本状态。
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else REPO_ROOT / "build" / "gangyiAI"
# 【逐行批注】 计算右侧结果并赋给 PORT，更新脚本状态。
PORT = int(os.environ.get("GANGYI_LIVE_TEST_PORT", "39422"))
# 【逐行批注】 计算右侧结果并赋给 CONTROL_TOKEN，更新脚本状态。
CONTROL_TOKEN = "live-acceptance-control-token"


# 【逐行批注】 定义函数 local_request，封装可复用的处理步骤。
def local_request(path, method="GET", payload=None, headers=None, expected=200, timeout=240):
    # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
    body = payload
    # 【逐行批注】 计算右侧结果并赋给 request_headers，更新脚本状态。
    request_headers = dict(headers or {})
    # 【逐行批注】 判断条件 isinstance(payload, (dict, list)):，成立时执行对应分支。
    if isinstance(payload, (dict, list)):
        # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        # 【逐行批注】 执行当前 Python 语句：request_headers.setdefault("Content-Type", "application/json")。
        request_headers.setdefault("Content-Type", "application/json")
    # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
    connection = http.client.HTTPConnection("127.0.0.1", PORT, timeout=timeout)
    # 【逐行批注】 计算右侧结果并赋给 connection.request(method, path, body，更新脚本状态。
    connection.request(method, path, body=body, headers=request_headers)
    # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
    response = connection.getresponse()
    # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
    data = response.read()
    # 【逐行批注】 执行当前 Python 语句：connection.close()。
    connection.close()
    # 【逐行批注】 判断条件 response.status != expected:，成立时执行对应分支。
    if response.status != expected:
        # 【逐行批注】 计算右侧结果并赋给 detail，更新脚本状态。
        detail = ""
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 parsed，更新脚本状态。
            parsed = json.loads(data.decode("utf-8"))
            # 【逐行批注】 计算右侧结果并赋给 detail，更新脚本状态。
            detail = parsed.get("type") or parsed.get("error") or parsed.get("message") or ""
        # 【逐行批注】 捕获并处理异常：except (UnicodeDecodeError, json.JSONDecodeError):。
        except (UnicodeDecodeError, json.JSONDecodeError):
            # 【逐行批注】 执行当前 Python 语句：pass。
            pass
        # 【逐行批注】 在错误情形抛出异常：raise AssertionError(f"{method} {path} 状态应为 {expected}，实际为 {response.…。
        raise AssertionError(f"{method} {path} 状态应为 {expected}，实际为 {response.status}，类型：{detail}")
    # 【逐行批注】 结束当前函数并返回结果：return data。
    return data


# 【逐行批注】 定义函数 local_json，封装可复用的处理步骤。
def local_json(path, method="GET", payload=None, expected=200, timeout=240):
    # 【逐行批注】 结束当前函数并返回结果：return json.loads(local_request(path, method, payload, expected=expec…。
    return json.loads(local_request(path, method, payload, expected=expected, timeout=timeout).decode("utf-8"))


# 【逐行批注】 定义函数 has_chinese，封装可复用的处理步骤。
def has_chinese(value):
    # 【逐行批注】 结束当前函数并返回结果：return isinstance(value, str) and any("\u4e00" <= character <= "\u9ff…。
    return isinstance(value, str) and any("\u4e00" <= character <= "\u9fff" for character in value)


# 【逐行批注】 定义函数 multipart，封装可复用的处理步骤。
def multipart(filename, content_type, content, fields):
    # 【逐行批注】 计算右侧结果并赋给 boundary，更新脚本状态。
    boundary = "----gangyi-live-" + uuid.uuid4().hex
    # 【逐行批注】 计算右侧结果并赋给 parts，更新脚本状态。
    parts = []
    # 【逐行批注】 遍历数据并重复执行循环体：for name, value in fields.items():。
    for name, value in fields.items():
        # 【逐行批注】 执行当前 Python 语句：parts.append(。
        parts.append(
            # 【逐行批注】 计算右侧结果并赋给 f"--{boundary}\r\nContent-Disposition: form-…，更新脚本状态。
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"{name}\"\r\n\r\n"
            # 【逐行批注】 执行当前 Python 语句：f"{value}\r\n".encode("utf-8")。
            f"{value}\r\n".encode("utf-8")
        # 【逐行批注】 执行当前 Python 语句：)。
        )
    # 【逐行批注】 执行当前 Python 语句：parts.append(。
    parts.append(
        # 【逐行批注】 计算右侧结果并赋给 f"--{boundary}\r\nContent-Disposition: form-…，更新脚本状态。
        f"--{boundary}\r\nContent-Disposition: form-data; name=\"image\"; "
        # 【逐行批注】 计算右侧结果并赋给 f"filename，更新脚本状态。
        f"filename=\"{filename}\"\r\nContent-Type: {content_type}\r\n\r\n".encode("utf-8")
        # 【逐行批注】 执行当前 Python 语句：+ content。
        + content
        # 【逐行批注】 执行当前 Python 语句：+ b"\r\n"。
        + b"\r\n"
    # 【逐行批注】 执行当前 Python 语句：)。
    )
    # 【逐行批注】 执行当前 Python 语句：parts.append(f"--{boundary}--\r\n".encode("ascii"))。
    parts.append(f"--{boundary}--\r\n".encode("ascii"))
    # 【逐行批注】 结束当前函数并返回结果：return b"".join(parts), {"Content-Type": f"multipart/form-data; bound…。
    return b"".join(parts), {"Content-Type": f"multipart/form-data; boundary={boundary}"}


# 【逐行批注】 定义函数 models_endpoint，封装可复用的处理步骤。
def models_endpoint(base_url):
    # 【逐行批注】 计算右侧结果并赋给 url，更新脚本状态。
    url = base_url.rstrip("/")
    # 【逐行批注】 判断条件 url.endswith("/chat/completions"):，成立时执行对应分支。
    if url.endswith("/chat/completions"):
        # 【逐行批注】 计算右侧结果并赋给 url，更新脚本状态。
        url = url[: -len("/chat/completions")]
    # 【逐行批注】 结束当前函数并返回结果：return url if url.endswith("/models") else url + "/models"。
    return url if url.endswith("/models") else url + "/models"


# 【逐行批注】 定义函数 verify_model_list，封装可复用的处理步骤。
def verify_model_list(environment):
    # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
    request = urllib.request.Request(
        # 【逐行批注】 调用 models_endpoint 完成当前脚本或测试步骤：models_endpoint(environment["AI_BASE_URL"]),。
        models_endpoint(environment["AI_BASE_URL"]),
        # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
        headers={"Authorization": "Bearer " + environment["AI_API_KEY"]},
    # 【逐行批注】 执行当前 Python 语句：)。
    )
    # 【逐行批注】 在受控上下文中执行资源操作：with urllib.request.urlopen(request, timeout=15, context=ssl.create_default_con…。
    with urllib.request.urlopen(request, timeout=15, context=ssl.create_default_context()) as response:
        # 【逐行批注】 计算右侧结果并赋给 parsed，更新脚本状态。
        parsed = json.loads(response.read().decode("utf-8"))
    # 【逐行批注】 计算右侧结果并赋给 models，更新脚本状态。
    models = sorted({item.get("id", "") for item in parsed.get("data", []) if item.get("id")})
    # 【逐行批注】 判断条件 not models:，成立时执行对应分支。
    if not models:
        # 【逐行批注】 在错误情形抛出异常：raise AssertionError("真实服务模型列表为空")。
        raise AssertionError("真实服务模型列表为空")
    # 【逐行批注】 结束当前函数并返回结果：return len(models), environment["AI_MODEL"] in models。
    return len(models), environment["AI_MODEL"] in models


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 required，更新脚本状态。
    required = ("AI_BASE_URL", "AI_API_KEY", "AI_MODEL", "BOCHA_API_KEY")
    # 【逐行批注】 计算右侧结果并赋给 missing，更新脚本状态。
    missing = [name for name in required if not os.environ.get(name)]
    # 【逐行批注】 判断条件 missing:，成立时执行对应分支。
    if missing:
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError("缺少真实验收环境变量：" + "、".join(missing))。
        raise RuntimeError("缺少真实验收环境变量：" + "、".join(missing))
    # 【逐行批注】 判断条件 not BINARY.is_file():，成立时执行对应分支。
    if not BINARY.is_file():
        # 【逐行批注】 在错误情形抛出异常：raise FileNotFoundError(f"服务程序不存在：{BINARY}")。
        raise FileNotFoundError(f"服务程序不存在：{BINARY}")

    # 【逐行批注】 计算右侧结果并赋给 environment，更新脚本状态。
    environment = os.environ.copy()
    # 【逐行批注】 执行当前 Python 语句：environment.pop("AI_DEBUG", None)。
    environment.pop("AI_DEBUG", None)
    # 【逐行批注】 计算右侧结果并赋给 model_count, configured_model_listed，更新脚本状态。
    model_count, configured_model_listed = verify_model_list(environment)
    # 【逐行批注】 计算右侧结果并赋给 results，更新脚本状态。
    results = {"modelCount": model_count, "configuredModelListed": configured_model_listed}

    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix="gangyiAI-live-") as temporary:。
    with tempfile.TemporaryDirectory(prefix="gangyiAI-live-") as temporary:
        # 【逐行批注】 计算右侧结果并赋给 work，更新脚本状态。
        work = Path(temporary)
        # 【逐行批注】 计算右侧结果并赋给 log_path，更新脚本状态。
        log_path = work / "service.log"
        # 【逐行批注】 执行当前 Python 语句：environment.update(。
        environment.update(
            # 【逐行批注】 执行当前 Python 语句：{。
            {
                # 【逐行批注】 执行当前 Python 语句："HOST": "127.0.0.1",。
                "HOST": "127.0.0.1",
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤："PORT": str(PORT),。
                "PORT": str(PORT),
                # 【逐行批注】 执行当前 Python 语句："LOCAL_CONTROL_TOKEN": CONTROL_TOKEN,。
                "LOCAL_CONTROL_TOKEN": CONTROL_TOKEN,
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤："DATABASE_PATH": str(work / "gangyiAI.db"),。
                "DATABASE_PATH": str(work / "gangyiAI.db"),
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤："AI_PLAN_CACHE_DIR": str(work / "plan-cache"),。
                "AI_PLAN_CACHE_DIR": str(work / "plan-cache"),
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤："RESOURCE_SEARCH_CACHE_DIR": str(work / "search-cache"),。
                "RESOURCE_SEARCH_CACHE_DIR": str(work / "search-cache"),
            # 【逐行批注】 执行当前 Python 语句：}。
            }
        # 【逐行批注】 执行当前 Python 语句：)。
        )
        # 【逐行批注】 计算右侧结果并赋给 goal，更新脚本状态。
        goal = "高中数学函数单调性真实验收"
        # 【逐行批注】 在受控上下文中执行资源操作：with log_path.open("wb") as log_file:。
        with log_path.open("wb") as log_file:
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.Popen(
                # 【逐行批注】 计算右侧结果并赋给 [str(BINARY)], cwd，更新脚本状态。
                [str(BINARY)], cwd=REPO_ROOT, env=environment,
                # 【逐行批注】 计算右侧结果并赋给 stdout，更新脚本状态。
                stdout=log_file, stderr=subprocess.STDOUT,
            # 【逐行批注】 执行当前 Python 语句：)。
            )
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(100):。
                for _ in range(100):
                    # 【逐行批注】 开始可能抛出异常的保护代码块。
                    try:
                        # 【逐行批注】 判断条件 local_json("/health", timeout=1).get("status") == "healthy":，成立时执行对应分支。
                        if local_json("/health", timeout=1).get("status") == "healthy":
                            # 【逐行批注】 执行当前 Python 语句：break。
                            break
                    # 【逐行批注】 捕获并处理异常：except (OSError, AssertionError, json.JSONDecodeError):。
                    except (OSError, AssertionError, json.JSONDecodeError):
                        # 【逐行批注】 执行当前 Python 语句：time.sleep(0.1)。
                        time.sleep(0.1)
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError("真实 AI 验收实例未能启动")。
                    raise AssertionError("真实 AI 验收实例未能启动")

                # 【逐行批注】 计算右侧结果并赋给 answer，更新脚本状态。
                answer = local_json("/api/ask", "POST", {"question": "用两句话解释函数单调性。"})
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer.get("ok") is True and has_chinese(answer.get("answer"))。
                assert answer.get("ok") is True and has_chinese(answer.get("answer"))
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert answer.get("model")。
                assert answer.get("model")
                # 【逐行批注】 计算右侧结果并赋给 results["askModel"]，更新脚本状态。
                results["askModel"] = answer["model"]

                # 【逐行批注】 计算右侧结果并赋给 classroom，更新脚本状态。
                classroom = local_json(
                    # 【逐行批注】 执行当前 Python 语句："/api/learning-chat", "POST",。
                    "/api/learning-chat", "POST",
                    # 【逐行批注】 执行当前 Python 语句：{。
                    {
                        # 【逐行批注】 执行当前 Python 语句："question": "为什么要比较自变量大小？",。
                        "question": "为什么要比较自变量大小？",
                        # 【逐行批注】 执行当前 Python 语句："goal": goal,。
                        "goal": goal,
                        # 【逐行批注】 执行当前 Python 语句："mode": "deep",。
                        "mode": "deep",
                        # 【逐行批注】 执行当前 Python 语句："phaseName": "概念基础",。
                        "phaseName": "概念基础",
                        # 【逐行批注】 执行当前 Python 语句："topic": "函数单调性",。
                        "topic": "函数单调性",
                        # 【逐行批注】 执行当前 Python 语句："contextTitle": "单调性的定义",。
                        "contextTitle": "单调性的定义",
                        # 【逐行批注】 执行当前 Python 语句："contextSummary": "理解区间内函数值随自变量变化的规律。",。
                        "contextSummary": "理解区间内函数值随自变量变化的规律。",
                        # 【逐行批注】 执行当前 Python 语句："messages": [],。
                        "messages": [],
                    # 【逐行批注】 执行当前 Python 语句：},。
                    },
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert classroom.get("ok") is True and has_chinese(classroom.get("answer"))。
                assert classroom.get("ok") is True and has_chinese(classroom.get("answer"))
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert classroom.get("model")。
                assert classroom.get("model")
                # 【逐行批注】 计算右侧结果并赋给 results["classroomModel"]，更新脚本状态。
                results["classroomModel"] = classroom["model"]

                # 【逐行批注】 计算右侧结果并赋给 plans，更新脚本状态。
                plans = {}
                # 【逐行批注】 遍历数据并重复执行循环体：for mode in ("lite", "deep"):。
                for mode in ("lite", "deep"):
                    # 【逐行批注】 计算右侧结果并赋给 plan，更新脚本状态。
                    plan = local_json(
                        # 【逐行批注】 执行当前 Python 语句："/api/generate-plan", "POST",。
                        "/api/generate-plan", "POST",
                        # 【逐行批注】 执行当前 Python 语句：{"goal": goal, "mode": mode, "bypassCache": True},。
                        {"goal": goal, "mode": mode, "bypassCache": True},
                    # 【逐行批注】 执行当前 Python 语句：)。
                    )
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert plan.get("title") and has_chinese(plan.get("summary", ""))。
                    assert plan.get("title") and has_chinese(plan.get("summary", ""))
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(plan.get("roadmap", [])) >= (3 if mode == "lite" else 4)。
                    assert len(plan.get("roadmap", [])) >= (3 if mode == "lite" else 4)
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert plan.get("courseStructure")。
                    assert plan.get("courseStructure")
                    # 【逐行批注】 计算右侧结果并赋给 plans[mode]，更新脚本状态。
                    plans[mode] = plan
                    # 【逐行批注】 计算右侧结果并赋给 results[mode + "Fallback"]，更新脚本状态。
                    results[mode + "Fallback"] = "qualityNotice" in plan

                # 【逐行批注】 判断条件 os.environ.get("GANGYI_PLAN_DIAGNOSTICS") == "1":，成立时执行对应分支。
                if os.environ.get("GANGYI_PLAN_DIAGNOSTICS") == "1":
                    # 【逐行批注】 调用 local_request 完成当前脚本或测试步骤：local_request(。
                    local_request(
                        # 【逐行批注】 执行当前 Python 语句："/internal/shutdown", "POST",。
                        "/internal/shutdown", "POST",
                        # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                        headers={"X-Gangyi-Control-Token": CONTROL_TOKEN},
                    # 【逐行批注】 执行当前 Python 语句：)。
                    )
                    # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                    process.wait(timeout=5)
                    # 【逐行批注】 执行当前 Python 语句：log_file.flush()。
                    log_file.flush()
                    # 【逐行批注】 计算右侧结果并赋给 log_contents，更新脚本状态。
                    log_contents = log_path.read_text("utf-8", errors="replace")
                    # 【逐行批注】 计算右侧结果并赋给 results["planEvents"]，更新脚本状态。
                    results["planEvents"] = [
                        # 【逐行批注】 执行当前 Python 语句：line for line in log_contents.splitlines()。
                        line for line in log_contents.splitlines()
                        # 【逐行批注】 判断条件 line.startswith("[generate-plan]")，成立时执行对应分支。
                        if line.startswith("[generate-plan]")
                    # 【逐行批注】 执行当前 Python 语句：]。
                    ]
                    # 【逐行批注】 输出当前运行结果：print(json.dumps(results, ensure_ascii=False, sort_keys=True))。
                    print(json.dumps(results, ensure_ascii=False, sort_keys=True))
                    # 【逐行批注】 结束当前函数并返回结果：return。
                    return

                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert results["liteFallback"] is False。
                assert results["liteFallback"] is False
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert results["deepFallback"] is False。
                assert results["deepFallback"] is False

                # 【逐行批注】 计算右侧结果并赋给 deep_plan，更新脚本状态。
                deep_plan = plans["deep"]
                # 【逐行批注】 计算右侧结果并赋给 resources，更新脚本状态。
                resources = deep_plan.get("resources", [])
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert resources and all(str(item.get("href", "")).startswith(("http://", "http…。
                assert resources and all(str(item.get("href", "")).startswith(("http://", "https://")) for item in resources)
                # 【逐行批注】 计算右侧结果并赋给 results["resourceCount"]，更新脚本状态。
                results["resourceCount"] = len(resources)
                # 【逐行批注】 计算右侧结果并赋给 results["resourceMessage"]，更新脚本状态。
                results["resourceMessage"] = deep_plan.get("resourceSourceMessage", "")

                # 【逐行批注】 计算右侧结果并赋给 saved，更新脚本状态。
                saved = local_json(
                    # 【逐行批注】 执行当前 Python 语句："/api/courses", "POST",。
                    "/api/courses", "POST",
                    # 【逐行批注】 执行当前 Python 语句：{。
                    {
                        # 【逐行批注】 执行当前 Python 语句："goal": goal,。
                        "goal": goal,
                        # 【逐行批注】 执行当前 Python 语句："title": deep_plan["title"],。
                        "title": deep_plan["title"],
                        # 【逐行批注】 执行当前 Python 语句："summary": deep_plan.get("summary", ""),。
                        "summary": deep_plan.get("summary", ""),
                        # 【逐行批注】 执行当前 Python 语句："mode": "deep",。
                        "mode": "deep",
                        # 【逐行批注】 执行当前 Python 语句："source": "live-acceptance",。
                        "source": "live-acceptance",
                        # 【逐行批注】 执行当前 Python 语句："anonymousId": "live-acceptance-user",。
                        "anonymousId": "live-acceptance-user",
                        # 【逐行批注】 执行当前 Python 语句："payload": deep_plan,。
                        "payload": deep_plan,
                    # 【逐行批注】 执行当前 Python 语句：},。
                    },
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 计算右侧结果并赋给 course_id，更新脚本状态。
                course_id = saved["courseId"]
                # 【逐行批注】 计算右侧结果并赋给 first_stage，更新脚本状态。
                first_stage = deep_plan["roadmap"][0]
                # 【逐行批注】 计算右侧结果并赋给 stage_name，更新脚本状态。
                stage_name = first_stage.get("name") or first_stage.get("stage") or "第一阶段"
                # 【逐行批注】 计算右侧结果并赋给 topics，更新脚本状态。
                topics = deep_plan.get("courseStructure", [{}])[0].get("topics", [])
                # 【逐行批注】 判断条件 not topics:，成立时执行对应分支。
                if not topics:
                    # 【逐行批注】 计算右侧结果并赋给 topics，更新脚本状态。
                    topics = [item.get("title", "") for item in first_stage.get("steps", []) if item.get("title")]
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert topics。
                assert topics

                # 【逐行批注】 计算右侧结果并赋给 phase，更新脚本状态。
                phase = local_json(
                    # 【逐行批注】 执行当前 Python 语句："/api/phase-expansion", "POST",。
                    "/api/phase-expansion", "POST",
                    # 【逐行批注】 执行当前 Python 语句：{。
                    {
                        # 【逐行批注】 执行当前 Python 语句："courseId": course_id,。
                        "courseId": course_id,
                        # 【逐行批注】 执行当前 Python 语句："anonymousId": "live-acceptance-user",。
                        "anonymousId": "live-acceptance-user",
                        # 【逐行批注】 执行当前 Python 语句："goal": goal,。
                        "goal": goal,
                        # 【逐行批注】 执行当前 Python 语句："mode": "deep",。
                        "mode": "deep",
                        # 【逐行批注】 执行当前 Python 语句："phaseIndex": 1,。
                        "phaseIndex": 1,
                        # 【逐行批注】 执行当前 Python 语句："stage": stage_name,。
                        "stage": stage_name,
                        # 【逐行批注】 执行当前 Python 语句："topics": topics[:4],。
                        "topics": topics[:4],
                    # 【逐行批注】 执行当前 Python 语句：},。
                    },
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert phase.get("ok") is True and phase.get("phase", {}).get("steps")。
                assert phase.get("ok") is True and phase.get("phase", {}).get("steps")
                # 【逐行批注】 计算右侧结果并赋给 results["phaseSteps"]，更新脚本状态。
                results["phaseSteps"] = len(phase["phase"]["steps"])

                # 【逐行批注】 计算右侧结果并赋给 lesson，更新脚本状态。
                lesson = local_json(
                    # 【逐行批注】 计算右侧结果并赋给 f"/api/learn?courseId，更新脚本状态。
                    f"/api/learn?courseId={course_id}&anonymousId=live-acceptance-user&phaseIndex=1&topicIndex=1",
                    # 【逐行批注】 计算右侧结果并赋给 timeout，更新脚本状态。
                    timeout=300,
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert lesson.get("ok") is True。
                assert lesson.get("ok") is True
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert has_chinese(lesson.get("summary", ""))。
                assert has_chinese(lesson.get("summary", ""))
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert lesson.get("lessonSteps") and lesson.get("practice") and lesson.get("qui…。
                assert lesson.get("lessonSteps") and lesson.get("practice") and lesson.get("quiz")
                # 【逐行批注】 计算右侧结果并赋给 results["lessonFallback"]，更新脚本状态。
                results["lessonFallback"] = bool(lesson.get("_fallbackUsed"))

                # 【逐行批注】 计算右侧结果并赋给 image_path，更新脚本状态。
                image_path = REPO_ROOT / "tests" / "fixtures" / "function-monotonicity.png"
                # 【逐行批注】 计算右侧结果并赋给 image_body, image_headers，更新脚本状态。
                image_body, image_headers = multipart(
                    # 【逐行批注】 执行当前 Python 语句："函数单调性题目.png", "image/png", image_path.read_bytes(),。
                    "函数单调性题目.png", "image/png", image_path.read_bytes(),
                    # 【逐行批注】 执行当前 Python 语句：{"prompt": "请识别图片中的数学题并生成学习目标", "mode": "deep"},。
                    {"prompt": "请识别图片中的数学题并生成学习目标", "mode": "deep"},
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 计算右侧结果并赋给 image_result，更新脚本状态。
                image_result = json.loads(
                    # 【逐行批注】 调用 local_request 完成当前脚本或测试步骤：local_request(。
                    local_request(
                        # 【逐行批注】 执行当前 Python 语句："/api/analyze-image-goal", "POST", image_body, image_headers,。
                        "/api/analyze-image-goal", "POST", image_body, image_headers,
                        # 【逐行批注】 计算右侧结果并赋给 timeout，更新脚本状态。
                        timeout=300,
                    # 【逐行批注】 执行当前 Python 语句：).decode("utf-8")。
                    ).decode("utf-8")
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 判断条件 image_result.get("success") is not True:，成立时执行对应分支。
                if image_result.get("success") is not True:
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError("真实图片识别失败，类型：" + image_result.get("type", "unkno…。
                    raise AssertionError("真实图片识别失败，类型：" + image_result.get("type", "unknown"))
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert has_chinese(image_result.get("goal", ""))。
                assert has_chinese(image_result.get("goal", ""))
                # 【逐行批注】 计算右侧结果并赋给 results["imageRecognition"]，更新脚本状态。
                results["imageRecognition"] = True

                # 【逐行批注】 调用 local_request 完成当前脚本或测试步骤：local_request(。
                local_request(
                    # 【逐行批注】 执行当前 Python 语句："/internal/shutdown", "POST",。
                    "/internal/shutdown", "POST",
                    # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                    headers={"X-Gangyi-Control-Token": CONTROL_TOKEN},
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                process.wait(timeout=5)
                # 【逐行批注】 执行当前 Python 语句：log_file.flush()。
                log_file.flush()
                # 【逐行批注】 计算右侧结果并赋给 log_contents，更新脚本状态。
                log_contents = log_path.read_text("utf-8", errors="replace")
                # 【逐行批注】 遍历数据并重复执行循环体：for secret in (environment["AI_API_KEY"], environment["BOCHA_API_KEY"], CO…。
                for secret in (environment["AI_API_KEY"], environment["BOCHA_API_KEY"], CONTROL_TOKEN, goal):
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert secret not in log_contents。
                    assert secret not in log_contents
                # 【逐行批注】 计算右侧结果并赋给 results["planEvents"]，更新脚本状态。
                results["planEvents"] = [
                    # 【逐行批注】 执行当前 Python 语句：line for line in log_contents.splitlines()。
                    line for line in log_contents.splitlines()
                    # 【逐行批注】 判断条件 line.startswith("[generate-plan]")，成立时执行对应分支。
                    if line.startswith("[generate-plan]")
                # 【逐行批注】 执行当前 Python 语句：]。
                ]
            # 【逐行批注】 执行当前 Python 语句：finally:。
            finally:
                # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
                if process.poll() is None:
                    # 【逐行批注】 执行当前 Python 语句：process.terminate()。
                    process.terminate()
                    # 【逐行批注】 开始可能抛出异常的保护代码块。
                    try:
                        # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                        process.wait(timeout=3)
                    # 【逐行批注】 捕获并处理异常：except subprocess.TimeoutExpired:。
                    except subprocess.TimeoutExpired:
                        # 【逐行批注】 执行当前 Python 语句：process.kill()。
                        process.kill()
                        # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                        process.wait(timeout=3)

    # 【逐行批注】 输出当前运行结果：print(json.dumps(results, ensure_ascii=False, sort_keys=True))。
    print(json.dumps(results, ensure_ascii=False, sort_keys=True))


# 【逐行批注】 判断条件 __name__ == "__main__":，成立时执行对应分支。
if __name__ == "__main__":
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()
