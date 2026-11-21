#!/usr/bin/env python3
"""试卷讲解接口 smoke 与离线契约夹具校验。"""

from __future__ import annotations

import argparse
import base64
import json
import os
import sys
import uuid
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen


ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / "tests" / "fixtures" / "exam-explanation-contract.json"
PNG_BYTES = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/8fkAAAAASUVORK5CYII="
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def validate_fixture(path: Path) -> dict:
    data = json.loads(path.read_text(encoding="utf-8"))
    require(data.get("schemaVersion") == 1, "夹具 schemaVersion 必须为 1")
    subjects = data.get("subjects")
    require(isinstance(subjects, list) and len(subjects) >= 10, "需要覆盖高中主要学科")
    names = [subject.get("name") for subject in subjects]
    require(len(set(names)) == len(names), "学科名称不得重复")
    require(all(subject.get("questionTypes") for subject in subjects), "每个学科都要列出代表题型")

    cases = {case["id"]: case for case in data.get("sourceCases", [])}
    require(cases.get("confirmed-exact-paper", {}).get("gradingEligible") is True, "确认同卷才允许作为批改依据")
    require(cases.get("similar-paper-only", {}).get("gradingEligible") is False, "相似试卷不得作为批改依据")
    require(cases.get("no-match", {}).get("expected") == "continueFromUploadedPaperWithoutBlocking", "搜不到资料不能阻断教学")

    score = next(case for case in data.get("gradingCases", []) if case["id"] == "estimated-score")
    require(score.get("requiredLabel") == "AI 估分，非官方", "估分必须明确非官方")
    require(set(score.get("requiredContext", [])) == {"scoringBasis", "uncertainty"}, "估分须说明依据与不确定性")

    controls = set(data.get("conversationControls", []))
    require({"directAnswer", "skip", "previousQuestion", "pause", "resume", "editAnswer"}.issubset(controls), "自然对话控制不完整")
    artifacts = {item["type"]: item for item in data.get("artifactCases", [])}
    require({"table", "code", "math", "chart", "functionPlot", "flowchart", "mindmap"}.issubset(artifacts), "富内容类型不完整")
    require(artifacts["code"].get("execution") == "onlyAfterExplicitRequestAndInIsolation", "代码执行必须显式请求且隔离")
    require(data.get("persistence", {}).get("storage") == "localOnly", "学习材料与档案默认仅本机保存")
    require("generativeImageCreation" in data.get("deferred", []), "生成式配图应列为暂缓项")
    print(f"契约夹具通过：{len(subjects)} 个学科，{len(artifacts)} 类回复内容。")
    return data


def request_json(base_url: str, path: str, method: str = "GET", payload: dict | None = None) -> tuple[int, object, dict]:
    body = None if payload is None else json.dumps(payload, ensure_ascii=False).encode("utf-8")
    headers = {"Accept": "application/json"}
    if body is not None:
        headers["Content-Type"] = "application/json; charset=utf-8"
    request = Request(base_url.rstrip("/") + path, data=body, headers=headers, method=method)
    try:
        with urlopen(request, timeout=15) as response:
            raw = response.read()
            content_type = response.headers.get("Content-Type", "")
            parsed = json.loads(raw.decode("utf-8")) if "json" in content_type else raw
            return response.status, parsed, dict(response.headers.items())
    except HTTPError as error:
        raw = error.read()
        try:
            parsed = json.loads(raw.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            parsed = raw.decode("utf-8", errors="replace")
        return error.code, parsed, dict(error.headers.items())
    except URLError as error:
        raise RuntimeError(f"无法连接试卷讲解服务 {base_url}: {error.reason}") from error


def api_smoke(base_url: str) -> None:
    run_id = uuid.uuid4().hex
    status, listing, _ = request_json(base_url, "/api/exam-explanations")
    require(status == 200, f"会话列表 HTTP 状态应为 200，实际 {status}: {listing}")
    require(isinstance(listing, dict) and isinstance(listing.get("sessions"), list), "列表响应必须为 {sessions: []}")

    page_data = "data:image/png;base64," + base64.b64encode(PNG_BYTES).decode("ascii")
    payload = {
        "requestId": "smoke-upload-" + run_id,
        "title": "接口验收合成试卷",
        "pages": [{"name": "smoke-page-1.png", "mimeType": "image/png", "dataUrl": page_data, "order": 0}],
    }
    status, created, _ = request_json(base_url, "/api/exam-explanations", "POST", payload)
    require(status in (200, 201, 202), f"创建会话 HTTP 状态异常 {status}: {created}")
    require(isinstance(created, dict) and created.get("id"), "创建会话响应缺少 id")
    session_id = str(created["id"])
    require(created.get("status"), "创建会话响应缺少 status")

    status, detail, _ = request_json(base_url, "/api/exam-explanations/" + session_id)
    require(status == 200, f"读取会话详情失败 {status}: {detail}")
    require(isinstance(detail, dict) and detail.get("id") == session_id, "详情 id 不匹配")
    require(isinstance(detail.get("pages"), list) and detail["pages"], "详情没有保存上传页面")

    first_page = detail["pages"][0]
    page_order = int(first_page.get("order", 0)) if isinstance(first_page, dict) else 0
    status, raw_page, headers = request_json(base_url, f"/api/exam-explanations/{session_id}/pages/{page_order}")
    require(status == 200, f"读取本地原页失败 {status}: {raw_page}")
    require(isinstance(raw_page, bytes) and raw_page.startswith(b"\x89PNG\r\n\x1a\n"), "原页必须以 PNG 原始字节返回")
    require("image/png" in headers.get("Content-Type", ""), "原页 Content-Type 必须为 image/png")

    ai_status = str(detail.get("recognitionStatus", detail.get("status", ""))).lower()
    questions = detail.get("questions", [])
    patch_checked = False
    if ai_status in {"completed", "ready", "recognized"} and questions:
        question = questions[0]
        question_id = question.get("questionId", question.get("id"))
        require(question_id, "已识别题目缺少稳定 questionId")
        edit = {"questionId": question_id, "text": "接口 smoke 校正后的题干"}
        status, result, _ = request_json(base_url, "/api/exam-explanations/" + session_id, "PATCH", edit)
        require(status == 200, f"修正识别题干失败 {status}: {result}")
        status, detail, _ = request_json(base_url, "/api/exam-explanations/" + session_id)
        require(status == 200, f"校正后详情读取失败 {status}: {detail}")
        patched = next((item for item in detail.get("questions", []) if item.get("questionId", item.get("id")) == question_id), None)
        require(patched is not None, "校正后的题目丢失")
        require(patched.get("text", patched.get("stem")) == edit["text"], "题干修正未持久化")
        patch_checked = True
    else:
        print(f"SKIP 模型识别/题目校正断言：当前识别状态为 {ai_status or '未知'}，题目数 {len(questions) if isinstance(questions, list) else '未知'}。")

    message_payload = {"requestId": "smoke-message-" + run_id, "text": "跳过当前题，直接说明下一题考查点。"}
    status, queued, _ = request_json(base_url, f"/api/exam-explanations/{session_id}/messages", "POST", message_payload)
    message_checked = status in (200, 202)
    task_id = None
    if message_checked:
        task = queued.get("task", {}) if isinstance(queued, dict) else {}
        task_id = task.get("id", queued.get("taskId") if isinstance(queued, dict) else None)
        require(task_id, "消息提交响应缺少异步 task id")
    else:
        require(status == 400 and isinstance(queued, dict) and "识别" in queued.get("error", ""),
                f"识别期间应拒绝消息并说明原因，实际 {status}: {queued}")

    status, after_send, _ = request_json(base_url, "/api/exam-explanations/" + session_id)
    require(status == 200, f"消息发送后详情读取失败 {status}: {after_send}")
    messages = after_send.get("messages", [])
    saved = any(message_payload["requestId"] == item.get("requestId") for item in messages)
    require(saved == message_checked, "识别期间应保留识别任务，识别完成后应保存消息")

    status, listing_after, _ = request_json(base_url, "/api/exam-explanations")
    require(status == 200 and any(item.get("id") == session_id for item in listing_after.get("sessions", [])), "新会话未出现在本机列表")
    checks = ["列表", "创建", "详情", "本地原页读取", "消息持久化" if message_checked else "识别期间消息保护", "列表回查"]
    if patch_checked:
        checks.append("识别题干校正")
    print("HTTP smoke 通过：" + "、".join(checks) + f"；会话 id={session_id}，异步任务 id={task_id}。")
    print("提示：会话写入当前测试服务数据库；请使用隔离的临时数据库运行此脚本。")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-url", default=os.environ.get("EXAM_EXPLANATIONS_BASE_URL"), help="本地隔离测试服务地址")
    parser.add_argument("--fixture", type=Path, default=FIXTURE, help="试卷讲解契约 JSON")
    parser.add_argument("--fixture-only", action="store_true", help="只校验离线验收夹具，不连接服务")
    args = parser.parse_args()

    validate_fixture(args.fixture)
    if args.fixture_only:
        return 0
    if not args.base_url:
        print("SKIP HTTP smoke：未提供 --base-url 或 EXAM_EXPLANATIONS_BASE_URL；离线契约夹具校验已完成。")
        return 0
    try:
        api_smoke(args.base_url)
    except (AssertionError, RuntimeError, TimeoutError) as error:
        print(f"FAIL HTTP smoke：{error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
