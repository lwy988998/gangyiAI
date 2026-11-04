#!/usr/bin/env python3
# 【逐行批注】 执行当前 Python 语句："""在隔离实例上执行并发稳定性验收，不接触正式数据库。"""。
"""在隔离实例上执行并发稳定性验收，不接触正式数据库。"""

# 【逐行批注】 导入后续逻辑需要的模块或符号：import http.client。
import http.client
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import statistics。
import statistics
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import threading。
import threading
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time


# 【逐行批注】 计算右侧结果并赋给 REPO_ROOT，更新脚本状态。
REPO_ROOT = Path(__file__).resolve().parent.parent
# 【逐行批注】 计算右侧结果并赋给 BINARY，更新脚本状态。
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else REPO_ROOT / "build" / "gangyiAI"
# 【逐行批注】 计算右侧结果并赋给 PORT，更新脚本状态。
PORT = int(os.environ.get("GANGYI_STABILITY_PORT", "39425"))
# 【逐行批注】 计算右侧结果并赋给 DURATION_SECONDS，更新脚本状态。
DURATION_SECONDS = int(os.environ.get("GANGYI_STABILITY_SECONDS", "1800"))
# 【逐行批注】 计算右侧结果并赋给 CONTROL_TOKEN，更新脚本状态。
CONTROL_TOKEN = "stability-control-token"
# 【逐行批注】 计算右侧结果并赋给 ANONYMOUS_ID，更新脚本状态。
ANONYMOUS_ID = "stability-user"
# 【逐行批注】 计算右侧结果并赋给 GOAL，更新脚本状态。
GOAL = "高中数学函数单调性稳定性验收"
# 【逐行批注】 计算右侧结果并赋给 AI_QUESTION，更新脚本状态。
AI_QUESTION = "请用两句话说明函数单调性的判断思路。"


# 【逐行批注】 定义函数 request，封装可复用的处理步骤。
def request(path, method="GET", payload=None, headers=None, expected=200, timeout=120):
    # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
    body = payload
    # 【逐行批注】 计算右侧结果并赋给 actual_headers，更新脚本状态。
    actual_headers = dict(headers or {})
    # 【逐行批注】 判断条件 isinstance(payload, (dict, list)):，成立时执行对应分支。
    if isinstance(payload, (dict, list)):
        # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        # 【逐行批注】 执行当前 Python 语句：actual_headers.setdefault("Content-Type", "application/json")。
        actual_headers.setdefault("Content-Type", "application/json")
    # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
    connection = http.client.HTTPConnection("127.0.0.1", PORT, timeout=timeout)
    # 【逐行批注】 计算右侧结果并赋给 connection.request(method, path, body，更新脚本状态。
    connection.request(method, path, body=body, headers=actual_headers)
    # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
    response = connection.getresponse()
    # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
    data = response.read()
    # 【逐行批注】 计算右侧结果并赋给 status，更新脚本状态。
    status = response.status
    # 【逐行批注】 执行当前 Python 语句：connection.close()。
    connection.close()
    # 【逐行批注】 判断条件 status != expected:，成立时执行对应分支。
    if status != expected:
        # 【逐行批注】 在错误情形抛出异常：raise AssertionError(f"{method} {path} 状态应为 {expected}，实际为 {status}")。
        raise AssertionError(f"{method} {path} 状态应为 {expected}，实际为 {status}")
    # 【逐行批注】 结束当前函数并返回结果：return data。
    return data


# 【逐行批注】 定义函数 request_json，封装可复用的处理步骤。
def request_json(path, method="GET", payload=None, expected=200, timeout=120):
    # 【逐行批注】 结束当前函数并返回结果：return json.loads(request(path, method, payload, expected=expected, t…。
    return json.loads(request(path, method, payload, expected=expected, timeout=timeout).decode("utf-8"))


# 【逐行批注】 定义函数 process_metrics，封装可复用的处理步骤。
def process_metrics(pid):
    # 【逐行批注】 计算右侧结果并赋给 status，更新脚本状态。
    status = Path(f"/proc/{pid}/status").read_text("utf-8")
    # 【逐行批注】 计算右侧结果并赋给 rss_kib，更新脚本状态。
    rss_kib = 0
    # 【逐行批注】 遍历数据并重复执行循环体：for line in status.splitlines():。
    for line in status.splitlines():
        # 【逐行批注】 判断条件 line.startswith("VmRSS:"):，成立时执行对应分支。
        if line.startswith("VmRSS:"):
            # 【逐行批注】 计算右侧结果并赋给 rss_kib，更新脚本状态。
            rss_kib = int(line.split()[1])
            # 【逐行批注】 执行当前 Python 语句：break。
            break
    # 【逐行批注】 结束当前函数并返回结果：return rss_kib, len(list(Path(f"/proc/{pid}/fd").iterdir()))。
    return rss_kib, len(list(Path(f"/proc/{pid}/fd").iterdir()))


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 判断条件 not BINARY.is_file():，成立时执行对应分支。
    if not BINARY.is_file():
        # 【逐行批注】 在错误情形抛出异常：raise FileNotFoundError(f"服务程序不存在：{BINARY}")。
        raise FileNotFoundError(f"服务程序不存在：{BINARY}")
    # 【逐行批注】 判断条件 not os.environ.get("AI_API_KEY"):，成立时执行对应分支。
    if not os.environ.get("AI_API_KEY"):
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError("稳定性验收需要由运行环境提供 AI_API_KEY")。
        raise RuntimeError("稳定性验收需要由运行环境提供 AI_API_KEY")

    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix="gangyiAI-stability-") as temporary:。
    with tempfile.TemporaryDirectory(prefix="gangyiAI-stability-") as temporary:
        # 【逐行批注】 计算右侧结果并赋给 work，更新脚本状态。
        work = Path(temporary)
        # 【逐行批注】 计算右侧结果并赋给 log_path，更新脚本状态。
        log_path = work / "service.log"
        # 【逐行批注】 计算右侧结果并赋给 environment，更新脚本状态。
        environment = os.environ.copy()
        # 【逐行批注】 执行当前 Python 语句：environment.update({。
        environment.update({
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
        # 【逐行批注】 执行当前 Python 语句：})。
        })

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
            # 【逐行批注】 计算右侧结果并赋给 stop，更新脚本状态。
            stop = threading.Event()
            # 【逐行批注】 计算右侧结果并赋给 errors，更新脚本状态。
            errors = []
            # 【逐行批注】 计算右侧结果并赋给 counters，更新脚本状态。
            counters = {"read": 0, "write": 0, "ai": 0}
            # 【逐行批注】 计算右侧结果并赋给 lock，更新脚本状态。
            lock = threading.Lock()

            # 【逐行批注】 定义函数 record_error，封装可复用的处理步骤。
            def record_error(message):
                # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
                with lock:
                    # 【逐行批注】 执行当前 Python 语句：errors.append(message)。
                    errors.append(message)
                # 【逐行批注】 执行当前 Python 语句：stop.set()。
                stop.set()

            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(150):。
                for _ in range(150):
                    # 【逐行批注】 开始可能抛出异常的保护代码块。
                    try:
                        # 【逐行批注】 判断条件 request_json("/health", timeout=1).get("status") == "healthy":，成立时执行对应分支。
                        if request_json("/health", timeout=1).get("status") == "healthy":
                            # 【逐行批注】 执行当前 Python 语句：break。
                            break
                    # 【逐行批注】 捕获并处理异常：except (OSError, AssertionError, json.JSONDecodeError):。
                    except (OSError, AssertionError, json.JSONDecodeError):
                        # 【逐行批注】 执行当前 Python 语句：time.sleep(0.1)。
                        time.sleep(0.1)
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError("服务未能通过健康检查")。
                    raise AssertionError("服务未能通过健康检查")

                # 【逐行批注】 计算右侧结果并赋给 plan，更新脚本状态。
                plan = request_json(
                    # 【逐行批注】 执行当前 Python 语句："/api/generate-plan", "POST",。
                    "/api/generate-plan", "POST",
                    # 【逐行批注】 计算右侧结果并赋给 {"goal": GOAL, "mode": "lite", "bypassCache"…，更新脚本状态。
                    {"goal": GOAL, "mode": "lite", "bypassCache": True}, timeout=180,
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 计算右侧结果并赋给 saved，更新脚本状态。
                saved = request_json(
                    # 【逐行批注】 执行当前 Python 语句："/api/courses", "POST",。
                    "/api/courses", "POST",
                    # 【逐行批注】 执行当前 Python 语句：{。
                    {
                        # 【逐行批注】 执行当前 Python 语句："goal": GOAL,。
                        "goal": GOAL,
                        # 【逐行批注】 执行当前 Python 语句："title": plan["title"],。
                        "title": plan["title"],
                        # 【逐行批注】 执行当前 Python 语句："summary": plan.get("summary", ""),。
                        "summary": plan.get("summary", ""),
                        # 【逐行批注】 执行当前 Python 语句："mode": "lite",。
                        "mode": "lite",
                        # 【逐行批注】 执行当前 Python 语句："source": "stability",。
                        "source": "stability",
                        # 【逐行批注】 执行当前 Python 语句："anonymousId": ANONYMOUS_ID,。
                        "anonymousId": ANONYMOUS_ID,
                        # 【逐行批注】 执行当前 Python 语句："payload": plan,。
                        "payload": plan,
                    # 【逐行批注】 执行当前 Python 语句：},。
                    },
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 计算右侧结果并赋给 course_id，更新脚本状态。
                course_id = saved["courseId"]
                # 【逐行批注】 计算右侧结果并赋给 common，更新脚本状态。
                common = {
                    # 【逐行批注】 执行当前 Python 语句："courseId": course_id,。
                    "courseId": course_id,
                    # 【逐行批注】 执行当前 Python 语句："anonymousId": ANONYMOUS_ID,。
                    "anonymousId": ANONYMOUS_ID,
                    # 【逐行批注】 执行当前 Python 语句："goal": GOAL,。
                    "goal": GOAL,
                    # 【逐行批注】 执行当前 Python 语句："mode": "lite",。
                    "mode": "lite",
                    # 【逐行批注】 执行当前 Python 语句："phaseIndex": 1,。
                    "phaseIndex": 1,
                    # 【逐行批注】 执行当前 Python 语句："phaseName": "基础阶段",。
                    "phaseName": "基础阶段",
                # 【逐行批注】 执行当前 Python 语句：}。
                }
                # 【逐行批注】 计算右侧结果并赋给 encoded_query，更新脚本状态。
                encoded_query = f"courseId={course_id}&anonymousId={ANONYMOUS_ID}"
                # 【逐行批注】 计算右侧结果并赋给 read_paths，更新脚本状态。
                read_paths = (
                    # 【逐行批注】 执行当前 Python 语句："/health",。
                    "/health",
                    # 【逐行批注】 执行当前 Python 语句："/",。
                    "/",
                    # 【逐行批注】 执行当前 Python 语句："/styles.css",。
                    "/styles.css",
                    # 【逐行批注】 执行当前 Python 语句："/school-logo.png",。
                    "/school-logo.png",
                    # 【逐行批注】 计算右侧结果并赋给 f"/api/courses?anonymousId，更新脚本状态。
                    f"/api/courses?anonymousId={ANONYMOUS_ID}",
                    # 【逐行批注】 计算右侧结果并赋给 f"/api/courses/{course_id}?anonymousId，更新脚本状态。
                    f"/api/courses/{course_id}?anonymousId={ANONYMOUS_ID}",
                    # 【逐行批注】 计算右侧结果并赋给 f"/api/task-progress?{encoded_query}&phaseIn…，更新脚本状态。
                    f"/api/task-progress?{encoded_query}&phaseIndex=1",
                    # 【逐行批注】 执行当前 Python 语句：f"/api/course-progress?{encoded_query}",。
                    f"/api/course-progress?{encoded_query}",
                # 【逐行批注】 执行当前 Python 语句：)。
                )

                # 预热网络、模板和 SQLite 路径后再采集基线。
                # 【逐行批注】 遍历数据并重复执行循环体：for path in read_paths:。
                for path in read_paths:
                    # 【逐行批注】 调用 request 完成当前脚本或测试步骤：request(path)。
                    request(path)
                # 【逐行批注】 调用 request_json 完成当前脚本或测试步骤：request_json(。
                request_json(
                    # 【逐行批注】 执行当前 Python 语句："/api/task-progress", "POST",。
                    "/api/task-progress", "POST",
                    # 【逐行批注】 执行当前 Python 语句：{**common, "taskIndex": 0, "taskTitle": "稳定性任务", "status": "in_progress"},。
                    {**common, "taskIndex": 0, "taskTitle": "稳定性任务", "status": "in_progress"},
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 计算右侧结果并赋给 baseline_rss, baseline_fds，更新脚本状态。
                baseline_rss, baseline_fds = process_metrics(process.pid)
                # 【逐行批注】 计算右侧结果并赋给 samples，更新脚本状态。
                samples = []

                # 【逐行批注】 定义函数 reader，封装可复用的处理步骤。
                def reader(offset):
                    # 【逐行批注】 计算右侧结果并赋给 index，更新脚本状态。
                    index = offset
                    # 【逐行批注】 在条件成立期间重复执行：while not stop.is_set():。
                    while not stop.is_set():
                        # 【逐行批注】 开始可能抛出异常的保护代码块。
                        try:
                            # 【逐行批注】 计算右侧结果并赋给 request(read_paths[index % len(read_paths)],…，更新脚本状态。
                            request(read_paths[index % len(read_paths)], timeout=10)
                            # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
                            with lock:
                                # 【逐行批注】 计算右侧结果并赋给 counters["read"] +，更新脚本状态。
                                counters["read"] += 1
                            # 【逐行批注】 计算右侧结果并赋给 index +，更新脚本状态。
                            index += 1
                            # 【逐行批注】 执行当前 Python 语句：time.sleep(0.1)。
                            time.sleep(0.1)
                        # 【逐行批注】 捕获并处理异常：except Exception as error:。
                        except Exception as error:
                            # 【逐行批注】 调用 record_error 完成当前脚本或测试步骤：record_error(f"并发读取失败：{error}")。
                            record_error(f"并发读取失败：{error}")

                # 【逐行批注】 定义函数 writer，封装可复用的处理步骤。
                def writer():
                    # 【逐行批注】 计算右侧结果并赋给 index，更新脚本状态。
                    index = 0
                    # 【逐行批注】 计算右侧结果并赋给 statuses，更新脚本状态。
                    statuses = ("not_started", "in_progress", "completed")
                    # 【逐行批注】 在条件成立期间重复执行：while not stop.is_set():。
                    while not stop.is_set():
                        # 【逐行批注】 开始可能抛出异常的保护代码块。
                        try:
                            # 【逐行批注】 调用 request_json 完成当前脚本或测试步骤：request_json(。
                            request_json(
                                # 【逐行批注】 执行当前 Python 语句："/api/task-progress", "POST",。
                                "/api/task-progress", "POST",
                                # 【逐行批注】 执行当前 Python 语句：{**common, "taskIndex": 0, "taskTitle": "稳定性任务",。
                                {**common, "taskIndex": 0, "taskTitle": "稳定性任务",
                                 # 【逐行批注】 调用 len 完成当前脚本或测试步骤："status": statuses[index % len(statuses)]},。
                                 "status": statuses[index % len(statuses)]},
                            # 【逐行批注】 执行当前 Python 语句：)。
                            )
                            # 【逐行批注】 调用 request_json 完成当前脚本或测试步骤：request_json(。
                            request_json(
                                # 【逐行批注】 执行当前 Python 语句："/api/learning-step-progress", "POST",。
                                "/api/learning-step-progress", "POST",
                                # 【逐行批注】 执行当前 Python 语句：{**common, "stepIndex": 0, "stepTitle": "稳定性步骤",。
                                {**common, "stepIndex": 0, "stepTitle": "稳定性步骤",
                                 # 【逐行批注】 执行当前 Python 语句："status": "understood" if index % 2 else "unset"},。
                                 "status": "understood" if index % 2 else "unset"},
                            # 【逐行批注】 执行当前 Python 语句：)。
                            )
                            # 【逐行批注】 在受控上下文中执行资源操作：with lock:。
                            with lock:
                                # 【逐行批注】 计算右侧结果并赋给 counters["write"] +，更新脚本状态。
                                counters["write"] += 2
                            # 【逐行批注】 计算右侧结果并赋给 index +，更新脚本状态。
                            index += 1
                            # 【逐行批注】 执行当前 Python 语句：time.sleep(0.25)。
                            time.sleep(0.25)
                        # 【逐行批注】 捕获并处理异常：except Exception as error:。
                        except Exception as error:
                            # 【逐行批注】 调用 record_error 完成当前脚本或测试步骤：record_error(f"并发写入失败：{error}")。
                            record_error(f"并发写入失败：{error}")

                # 【逐行批注】 计算右侧结果并赋给 workers，更新脚本状态。
                workers = [threading.Thread(target=reader, args=(index,), daemon=True) for index in range(4)]
                # 【逐行批注】 计算右侧结果并赋给 workers.append(threading.Thread(target，更新脚本状态。
                workers.append(threading.Thread(target=writer, daemon=True))
                # 【逐行批注】 遍历数据并重复执行循环体：for worker in workers:。
                for worker in workers:
                    # 【逐行批注】 执行当前 Python 语句：worker.start()。
                    worker.start()

                # 【逐行批注】 计算右侧结果并赋给 started，更新脚本状态。
                started = time.monotonic()
                # 【逐行批注】 计算右侧结果并赋给 checkpoints，更新脚本状态。
                checkpoints = [0, max(1, DURATION_SECONDS // 2), max(2, DURATION_SECONDS - 60)]
                # 【逐行批注】 计算右侧结果并赋给 completed_checkpoints，更新脚本状态。
                completed_checkpoints = set()
                # 【逐行批注】 在条件成立期间重复执行：while not stop.is_set() and time.monotonic() - started < DURATION_SECONDS:。
                while not stop.is_set() and time.monotonic() - started < DURATION_SECONDS:
                    # 【逐行批注】 判断条件 process.poll() is not None:，成立时执行对应分支。
                    if process.poll() is not None:
                        # 【逐行批注】 调用 record_error 完成当前脚本或测试步骤：record_error(f"服务进程意外退出，退出码 {process.returncode}")。
                        record_error(f"服务进程意外退出，退出码 {process.returncode}")
                        # 【逐行批注】 执行当前 Python 语句：break。
                        break
                    # 【逐行批注】 计算右侧结果并赋给 elapsed，更新脚本状态。
                    elapsed = int(time.monotonic() - started)
                    # 【逐行批注】 遍历数据并重复执行循环体：for checkpoint in checkpoints:。
                    for checkpoint in checkpoints:
                        # 【逐行批注】 判断条件 checkpoint <= elapsed and checkpoint not in completed_checkpoint…，成立时执行对应分支。
                        if checkpoint <= elapsed and checkpoint not in completed_checkpoints:
                            # 【逐行批注】 计算右侧结果并赋给 answer，更新脚本状态。
                            answer = request_json(
                                # 【逐行批注】 计算右侧结果并赋给 "/api/ask", "POST", {"question": AI_QUESTION…，更新脚本状态。
                                "/api/ask", "POST", {"question": AI_QUESTION}, timeout=120,
                            # 【逐行批注】 执行当前 Python 语句：)。
                            )
                            # 【逐行批注】 判断条件 not answer.get("answer") or answer.get("model") != environment.g…，成立时执行对应分支。
                            if not answer.get("answer") or answer.get("model") != environment.get("AI_MODEL"):
                                # 【逐行批注】 在错误情形抛出异常：raise AssertionError("稳定性检查点的 AI 回答或模型信息不正确")。
                                raise AssertionError("稳定性检查点的 AI 回答或模型信息不正确")
                            # 【逐行批注】 执行当前 Python 语句：completed_checkpoints.add(checkpoint)。
                            completed_checkpoints.add(checkpoint)
                            # 【逐行批注】 计算右侧结果并赋给 counters["ai"] +，更新脚本状态。
                            counters["ai"] += 1
                    # 【逐行批注】 调用 process_metrics 完成当前脚本或测试步骤：samples.append(process_metrics(process.pid))。
                    samples.append(process_metrics(process.pid))
                    # 【逐行批注】 执行当前 Python 语句：time.sleep(5)。
                    time.sleep(5)

                # 【逐行批注】 执行当前 Python 语句：stop.set()。
                stop.set()
                # 【逐行批注】 遍历数据并重复执行循环体：for worker in workers:。
                for worker in workers:
                    # 【逐行批注】 计算右侧结果并赋给 worker.join(timeout，更新脚本状态。
                    worker.join(timeout=15)
                # 【逐行批注】 判断条件 errors:，成立时执行对应分支。
                if errors:
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError(errors[0])。
                    raise AssertionError(errors[0])
                # 【逐行批注】 判断条件 len(completed_checkpoints) != len(checkpoints):，成立时执行对应分支。
                if len(completed_checkpoints) != len(checkpoints):
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError("未完成全部 AI 稳定性检查点")。
                    raise AssertionError("未完成全部 AI 稳定性检查点")

                # 【逐行批注】 计算右侧结果并赋给 final_rss, final_fds，更新脚本状态。
                final_rss, final_fds = process_metrics(process.pid)
                # 【逐行批注】 计算右侧结果并赋给 rss_growth_kib，更新脚本状态。
                rss_growth_kib = final_rss - baseline_rss
                # 【逐行批注】 判断条件 rss_growth_kib > 50 * 1024:，成立时执行对应分支。
                if rss_growth_kib > 50 * 1024:
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError(f"RSS 增长超过 50 MiB：{rss_growth_kib / 1024:.1f} Mi…。
                    raise AssertionError(f"RSS 增长超过 50 MiB：{rss_growth_kib / 1024:.1f} MiB")
                # 【逐行批注】 计算右侧结果并赋给 tail_fds，更新脚本状态。
                tail_fds = [fds for _, fds in samples[-12:]] or [final_fds]
                # 【逐行批注】 判断条件 statistics.median(tail_fds) > baseline_fds + 8:，成立时执行对应分支。
                if statistics.median(tail_fds) > baseline_fds + 8:
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError(。
                    raise AssertionError(
                        # 【逐行批注】 执行当前 Python 语句：f"文件描述符存在持续增长：基线 {baseline_fds}，末段中位数 {statistics.median(tail_fds):.0f}"。
                        f"文件描述符存在持续增长：基线 {baseline_fds}，末段中位数 {statistics.median(tail_fds):.0f}"
                    # 【逐行批注】 执行当前 Python 语句：)。
                    )
                # 【逐行批注】 在受控上下文中执行资源操作：with sqlite3.connect(work / "gangyiAI.db") as database:。
                with sqlite3.connect(work / "gangyiAI.db") as database:
                    # 【逐行批注】 判断条件 database.execute("PRAGMA quick_check").fetchone()[0] != "ok":，成立时执行对应分支。
                    if database.execute("PRAGMA quick_check").fetchone()[0] != "ok":
                        # 【逐行批注】 在错误情形抛出异常：raise AssertionError("SQLite quick_check 未通过")。
                        raise AssertionError("SQLite quick_check 未通过")

                # 【逐行批注】 调用 request 完成当前脚本或测试步骤：request(。
                request(
                    # 【逐行批注】 执行当前 Python 语句："/internal/shutdown", "POST",。
                    "/internal/shutdown", "POST",
                    # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                    headers={"X-Gangyi-Control-Token": CONTROL_TOKEN},
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                process.wait(timeout=10)
                # 【逐行批注】 执行当前 Python 语句：log_file.flush()。
                log_file.flush()
                # 【逐行批注】 计算右侧结果并赋给 log_contents，更新脚本状态。
                log_contents = log_path.read_text("utf-8", errors="replace")
                # 【逐行批注】 计算右侧结果并赋给 sensitive_values，更新脚本状态。
                sensitive_values = (
                    # 【逐行批注】 执行当前 Python 语句：environment["AI_API_KEY"], CONTROL_TOKEN, GOAL, AI_QUESTION,。
                    environment["AI_API_KEY"], CONTROL_TOKEN, GOAL, AI_QUESTION,
                # 【逐行批注】 执行当前 Python 语句：)。
                )
                # 【逐行批注】 判断条件 any(value and value in log_contents for value in sensitive_value…，成立时执行对应分支。
                if any(value and value in log_contents for value in sensitive_values):
                    # 【逐行批注】 在错误情形抛出异常：raise AssertionError("服务日志包含密钥、控制令牌或完整用户内容")。
                    raise AssertionError("服务日志包含密钥、控制令牌或完整用户内容")

                # 【逐行批注】 输出当前运行结果：print(json.dumps({。
                print(json.dumps({
                    # 【逐行批注】 执行当前 Python 语句："durationSeconds": DURATION_SECONDS,。
                    "durationSeconds": DURATION_SECONDS,
                    # 【逐行批注】 执行当前 Python 语句："requests": counters,。
                    "requests": counters,
                    # 【逐行批注】 调用 round 完成当前脚本或测试步骤："baselineRssMiB": round(baseline_rss / 1024, 1),。
                    "baselineRssMiB": round(baseline_rss / 1024, 1),
                    # 【逐行批注】 调用 round 完成当前脚本或测试步骤："finalRssMiB": round(final_rss / 1024, 1),。
                    "finalRssMiB": round(final_rss / 1024, 1),
                    # 【逐行批注】 调用 round 完成当前脚本或测试步骤："rssGrowthMiB": round(rss_growth_kib / 1024, 1),。
                    "rssGrowthMiB": round(rss_growth_kib / 1024, 1),
                    # 【逐行批注】 执行当前 Python 语句："baselineFileDescriptors": baseline_fds,。
                    "baselineFileDescriptors": baseline_fds,
                    # 【逐行批注】 执行当前 Python 语句："finalFileDescriptors": final_fds,。
                    "finalFileDescriptors": final_fds,
                    # 【逐行批注】 执行当前 Python 语句："sqliteQuickCheck": "ok",。
                    "sqliteQuickCheck": "ok",
                    # 【逐行批注】 执行当前 Python 语句："unexpected5xx": 0,。
                    "unexpected5xx": 0,
                    # 【逐行批注】 执行当前 Python 语句："logSensitiveData": False,。
                    "logSensitiveData": False,
                # 【逐行批注】 计算右侧结果并赋给 }, ensure_ascii，更新脚本状态。
                }, ensure_ascii=False))
            # 【逐行批注】 执行当前 Python 语句：finally:。
            finally:
                # 【逐行批注】 执行当前 Python 语句：stop.set()。
                stop.set()
                # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
                if process.poll() is None:
                    # 【逐行批注】 执行当前 Python 语句：process.terminate()。
                    process.terminate()
                    # 【逐行批注】 开始可能抛出异常的保护代码块。
                    try:
                        # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                        process.wait(timeout=5)
                    # 【逐行批注】 捕获并处理异常：except subprocess.TimeoutExpired:。
                    except subprocess.TimeoutExpired:
                        # 【逐行批注】 执行当前 Python 语句：process.kill()。
                        process.kill()
                        # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                        process.wait(timeout=5)


# 【逐行批注】 判断条件 __name__ == "__main__":，成立时执行对应分支。
if __name__ == "__main__":
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()
