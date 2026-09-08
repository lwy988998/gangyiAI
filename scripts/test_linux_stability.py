#!/usr/bin/env python3
"""在隔离实例上执行并发稳定性验收，不接触正式数据库。"""

import http.client
import json
import os
from pathlib import Path
import sqlite3
import statistics
import subprocess
import sys
import tempfile
import threading
import time


REPO_ROOT = Path(__file__).resolve().parent.parent
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else REPO_ROOT / "build" / "gangyiAI"
PORT = int(os.environ.get("GANGYI_STABILITY_PORT", "39425"))
DURATION_SECONDS = int(os.environ.get("GANGYI_STABILITY_SECONDS", "1800"))
CONTROL_TOKEN = "stability-control-token"
ANONYMOUS_ID = "stability-user"
GOAL = "高中数学函数单调性稳定性验收"
AI_QUESTION = "请用两句话说明函数单调性的判断思路。"


def request(path, method="GET", payload=None, headers=None, expected=200, timeout=120):
    body = payload
    actual_headers = dict(headers or {})
    if isinstance(payload, (dict, list)):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        actual_headers.setdefault("Content-Type", "application/json")
    connection = http.client.HTTPConnection("127.0.0.1", PORT, timeout=timeout)
    connection.request(method, path, body=body, headers=actual_headers)
    response = connection.getresponse()
    data = response.read()
    status = response.status
    connection.close()
    if status != expected:
        raise AssertionError(f"{method} {path} 状态应为 {expected}，实际为 {status}")
    return data


def request_json(path, method="GET", payload=None, expected=200, timeout=120):
    return json.loads(request(path, method, payload, expected=expected, timeout=timeout).decode("utf-8"))


def process_metrics(pid):
    status = Path(f"/proc/{pid}/status").read_text("utf-8")
    rss_kib = 0
    for line in status.splitlines():
        if line.startswith("VmRSS:"):
            rss_kib = int(line.split()[1])
            break
    return rss_kib, len(list(Path(f"/proc/{pid}/fd").iterdir()))


def main():
    if not BINARY.is_file():
        raise FileNotFoundError(f"服务程序不存在：{BINARY}")
    if not os.environ.get("AI_API_KEY"):
        raise RuntimeError("稳定性验收需要由运行环境提供 AI_API_KEY")

    with tempfile.TemporaryDirectory(prefix="gangyiAI-stability-") as temporary:
        work = Path(temporary)
        log_path = work / "service.log"
        environment = os.environ.copy()
        environment.update({
            "HOST": "127.0.0.1",
            "PORT": str(PORT),
            "LOCAL_CONTROL_TOKEN": CONTROL_TOKEN,
            "DATABASE_PATH": str(work / "gangyiAI.db"),
            "AI_PLAN_CACHE_DIR": str(work / "plan-cache"),
            "RESOURCE_SEARCH_CACHE_DIR": str(work / "search-cache"),
        })

        with log_path.open("wb") as log_file:
            process = subprocess.Popen(
                [str(BINARY)], cwd=REPO_ROOT, env=environment,
                stdout=log_file, stderr=subprocess.STDOUT,
            )
            stop = threading.Event()
            errors = []
            counters = {"read": 0, "write": 0, "ai": 0}
            lock = threading.Lock()

            def record_error(message):
                with lock:
                    errors.append(message)
                stop.set()

            try:
                for _ in range(150):
                    try:
                        if request_json("/health", timeout=1).get("status") == "healthy":
                            break
                    except (OSError, AssertionError, json.JSONDecodeError):
                        time.sleep(0.1)
                else:
                    raise AssertionError("服务未能通过健康检查")

                plan = request_json(
                    "/api/generate-plan", "POST",
                    {"goal": GOAL, "mode": "lite", "bypassCache": True}, timeout=180,
                )
                saved = request_json(
                    "/api/courses", "POST",
                    {
                        "goal": GOAL,
                        "title": plan["title"],
                        "summary": plan.get("summary", ""),
                        "mode": "lite",
                        "source": "stability",
                        "anonymousId": ANONYMOUS_ID,
                        "payload": plan,
                    },
                )
                course_id = saved["courseId"]
                common = {
                    "courseId": course_id,
                    "anonymousId": ANONYMOUS_ID,
                    "goal": GOAL,
                    "mode": "lite",
                    "phaseIndex": 1,
                    "phaseName": "基础阶段",
                }
                encoded_query = f"courseId={course_id}&anonymousId={ANONYMOUS_ID}"
                read_paths = (
                    "/health",
                    "/",
                    "/styles.css",
                    "/school-logo.png",
                    f"/api/courses?anonymousId={ANONYMOUS_ID}",
                    f"/api/courses/{course_id}?anonymousId={ANONYMOUS_ID}",
                    f"/api/task-progress?{encoded_query}&phaseIndex=1",
                    f"/api/course-progress?{encoded_query}",
                )

                # 预热网络、模板和 SQLite 路径后再采集基线。
                for path in read_paths:
                    request(path)
                request_json(
                    "/api/task-progress", "POST",
                    {**common, "taskIndex": 0, "taskTitle": "稳定性任务", "status": "in_progress"},
                )
                baseline_rss, baseline_fds = process_metrics(process.pid)
                samples = []

                def reader(offset):
                    index = offset
                    while not stop.is_set():
                        try:
                            request(read_paths[index % len(read_paths)], timeout=10)
                            with lock:
                                counters["read"] += 1
                            index += 1
                            time.sleep(0.1)
                        except Exception as error:
                            record_error(f"并发读取失败：{error}")

                def writer():
                    index = 0
                    statuses = ("not_started", "in_progress", "completed")
                    while not stop.is_set():
                        try:
                            request_json(
                                "/api/task-progress", "POST",
                                {**common, "taskIndex": 0, "taskTitle": "稳定性任务",
                                 "status": statuses[index % len(statuses)]},
                            )
                            request_json(
                                "/api/learning-step-progress", "POST",
                                {**common, "stepIndex": 0, "stepTitle": "稳定性步骤",
                                 "status": "understood" if index % 2 else "unset"},
                            )
                            with lock:
                                counters["write"] += 2
                            index += 1
                            time.sleep(0.25)
                        except Exception as error:
                            record_error(f"并发写入失败：{error}")

                workers = [threading.Thread(target=reader, args=(index,), daemon=True) for index in range(4)]
                workers.append(threading.Thread(target=writer, daemon=True))
                for worker in workers:
                    worker.start()

                started = time.monotonic()
                checkpoints = [0, max(1, DURATION_SECONDS // 2), max(2, DURATION_SECONDS - 60)]
                completed_checkpoints = set()
                while not stop.is_set() and time.monotonic() - started < DURATION_SECONDS:
                    if process.poll() is not None:
                        record_error(f"服务进程意外退出，退出码 {process.returncode}")
                        break
                    elapsed = int(time.monotonic() - started)
                    for checkpoint in checkpoints:
                        if checkpoint <= elapsed and checkpoint not in completed_checkpoints:
                            answer = request_json(
                                "/api/ask", "POST", {"question": AI_QUESTION}, timeout=120,
                            )
                            if not answer.get("answer") or answer.get("model") != environment.get("AI_MODEL"):
                                raise AssertionError("稳定性检查点的 AI 回答或模型信息不正确")
                            completed_checkpoints.add(checkpoint)
                            counters["ai"] += 1
                    samples.append(process_metrics(process.pid))
                    time.sleep(5)

                stop.set()
                for worker in workers:
                    worker.join(timeout=15)
                if errors:
                    raise AssertionError(errors[0])
                if len(completed_checkpoints) != len(checkpoints):
                    raise AssertionError("未完成全部 AI 稳定性检查点")

                final_rss, final_fds = process_metrics(process.pid)
                rss_growth_kib = final_rss - baseline_rss
                if rss_growth_kib > 50 * 1024:
                    raise AssertionError(f"RSS 增长超过 50 MiB：{rss_growth_kib / 1024:.1f} MiB")
                tail_fds = [fds for _, fds in samples[-12:]] or [final_fds]
                if statistics.median(tail_fds) > baseline_fds + 8:
                    raise AssertionError(
                        f"文件描述符存在持续增长：基线 {baseline_fds}，末段中位数 {statistics.median(tail_fds):.0f}"
                    )
                with sqlite3.connect(work / "gangyiAI.db") as database:
                    if database.execute("PRAGMA quick_check").fetchone()[0] != "ok":
                        raise AssertionError("SQLite quick_check 未通过")

                request(
                    "/internal/shutdown", "POST",
                    headers={"X-Gangyi-Control-Token": CONTROL_TOKEN},
                )
                process.wait(timeout=10)
                log_file.flush()
                log_contents = log_path.read_text("utf-8", errors="replace")
                sensitive_values = (
                    environment["AI_API_KEY"], CONTROL_TOKEN, GOAL, AI_QUESTION,
                )
                if any(value and value in log_contents for value in sensitive_values):
                    raise AssertionError("服务日志包含密钥、控制令牌或完整用户内容")

                print(json.dumps({
                    "durationSeconds": DURATION_SECONDS,
                    "requests": counters,
                    "baselineRssMiB": round(baseline_rss / 1024, 1),
                    "finalRssMiB": round(final_rss / 1024, 1),
                    "rssGrowthMiB": round(rss_growth_kib / 1024, 1),
                    "baselineFileDescriptors": baseline_fds,
                    "finalFileDescriptors": final_fds,
                    "sqliteQuickCheck": "ok",
                    "unexpected5xx": 0,
                    "logSensitiveData": False,
                }, ensure_ascii=False))
            finally:
                stop.set()
                if process.poll() is None:
                    process.terminate()
                    try:
                        process.wait(timeout=5)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait(timeout=5)


if __name__ == "__main__":
    main()
