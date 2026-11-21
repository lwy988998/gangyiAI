"""离线教学评测准备和记录检查；不联网、不读取凭据、不执行模型代码。"""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DATASET = ROOT / "tests/fixtures/ai-tutor/seed-v1.json"
SUBJECTS = {
    "mathematics", "physics", "chemistry", "biology", "history",
    "geography", "chinese", "english", "computer_science",
}
ACTIONS = {"explain", "explore", "ask", "feedback", "hint", "clarify", "pause", "resume", "text_fallback"}
MASTERY = {
    "UNKNOWN", "NOT_STARTED", "LEARNING", "PARTIAL", "MISUNDERSTOOD",
    "PRACTICING", "PROVISIONAL_MASTERY", "VERIFIED_MASTERY",
}
ERROR_TYPES = {"conceptual", "condition_omission", "unit_conversion", "reasoning", "boundary", "calculation", "expression", "unknown"}
BINDING_FIELDS = {"courseId", "lessonId", "questionId", "interactionId"}
RATING_FIELDS = ["accuracy", "clarity", "diagnosticRelevance", "adaptation", "guidance", "interactionDesign"]


def require(condition: bool, message: str) -> None:
    """遇到不可信评测数据时明确拒绝。"""
    if not condition:
        raise ValueError(message)


def nonblank(value: object) -> bool:
    return isinstance(value, str) and bool(value.strip())


def digest(value: object) -> str:
    """身份摘要覆盖完整案例，避免不同输入混入同一评测。"""
    data = json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
    return hashlib.sha256(data.encode("utf-8")).hexdigest()


def parse_json(data: str) -> object:
    """不接受重复键和非 JSON 数值，防止读入时静默覆盖证据。"""
    def object_pairs(pairs: list[tuple[str, object]]) -> dict:
        result = {}
        for key, value in pairs:
            require(key not in result, f"JSON 字段重复：{key}")
            result[key] = value
        return result

    def invalid_constant(value: str) -> None:
        raise ValueError(f"JSON 含无效数值：{value}")

    return json.loads(data, object_pairs_hook=object_pairs, parse_constant=invalid_constant)


def load_json(path: Path) -> object:
    return parse_json(path.read_text(encoding="utf-8-sig"))


def validate_dataset(dataset: object) -> dict:
    """核对覆盖与证据身份，不将参考答案提升为专家标准。"""
    require(isinstance(dataset, dict) and type(dataset.get("schemaVersion")) is int and dataset["schemaVersion"] == 1, "数据集版本必须为1")
    require(nonblank(dataset.get("datasetId")), "缺少数据集身份")
    review = dataset.get("review")
    require(isinstance(review, dict) and review.get("status") in {"pending_expert_review", "expert_reviewed"}, "缺少参考标准审核状态")
    if review["status"] == "expert_reviewed":
        require(nonblank(review.get("reviewer")) and nonblank(review.get("reviewedAt")), "专家审核必须记录审核人和时间")
    cases = dataset.get("cases")
    require(isinstance(cases, list) and len(cases) >= 18, "首批至少18例")
    seen, subjects, scenarios = set(), set(), set()
    for case in cases:
        require(isinstance(case, dict), "案例必须为对象")
        case_id = case.get("id")
        require(nonblank(case_id) and case_id not in seen, "案例身份为空或重复")
        seen.add(case_id)
        require(case.get("subject") in SUBJECTS, f"{case_id} 学科无效")
        subjects.add(case["subject"])
        for field in ["knowledgePoint", "prompt", "reference"]:
            require(nonblank(case.get(field)), f"{case_id} 缺少 {field}")
        require(type(case.get("formalSubmission")) is bool, f"{case_id} 缺少明确提交状态")
        require(case.get("studentResponse") is None or nonblank(case["studentResponse"]), f"{case_id} 学生回答无效")
        if case["formalSubmission"]:
            require(nonblank(case.get("studentResponse")), f"{case_id} 正式提交缺少实际回答")
        for field in ["scenarios", "expectedAction", "errorTypes", "forbidden", "componentTypes"]:
            require(isinstance(case.get(field), list) and all(nonblank(x) for x in case[field]), f"{case_id} 的 {field} 必须为文字列表")
        require(bool(case["scenarios"]) and bool(case["forbidden"]), f"{case_id} 缺少场景或禁止行为")
        scenarios.update(case["scenarios"])
        require(bool(case["expectedAction"]) and set(case["expectedAction"]) <= ACTIONS, f"{case_id} 行动无效")
        require(set(case["errorTypes"]) <= ERROR_TYPES, f"{case_id} 诊断类型无效")
        binding = case.get("binding")
        require(isinstance(binding, dict) and set(binding) == BINDING_FIELDS and all(nonblank(v) for v in binding.values()), f"{case_id} 证据绑定不完整")
        require(isinstance(case.get("turns"), list), f"{case_id} 多轮上下文无效")
        for turn in case["turns"]:
            require(isinstance(turn, dict) and turn.get("role") in {"user", "assistant"} and nonblank(turn.get("text")), f"{case_id} 对话轮次无效")
        if 'componentPlan' in case:
            from tutor_component_scenes import validate_scene_plan
            validate_scene_plan(case)
    require(subjects == SUBJECTS, "首批须覆盖九个学科")
    required = {"diagnosis", "partial_answer", "multi_turn", "ambiguity", "image_uncertainty", "interaction_selection", "invalid_component_parameter", "course_interruption", "context_recovery", "independent_exploration", "model_failure"}
    require(required <= scenarios, "缺少场景：" + ", ".join(sorted(required - scenarios)))
    return {"cases": len(cases), "subjects": len(subjects), "scenarios": sorted(scenarios), "reviewStatus": review["status"]}


def prepare(dataset: dict) -> dict:
    """生成待执行请求清单；私有参考标准仅供隔离实验，不能发送到学生界面。"""
    validate_dataset(dataset)
    return {
        "schemaVersion": 1, "kind": "request_plan", "datasetId": dataset["datasetId"],
        "datasetHash": digest(dataset), "executed": False,
        "instructions": "隔离实验内部请求；依据当前问题和实际回答教学。输出仅用于评测，禁止本地操作。",
        "records": [{
            "caseId": case["id"], "inputHash": digest(case),
            "input": {field: case[field] for field in ["subject", "knowledgePoint", "binding", "turns", "prompt", "studentResponse", "formalSubmission", "reference", *(['componentPlan'] if 'componentPlan' in case else [])]},
            "outputContract": {
                "binding": case["binding"], "message": "非空教学反馈", "nextAction": sorted(ACTIONS),
                "score": "正式作答且可靠时0..100整数，否则null",
                "masteryStatus": sorted(MASTERY),
                "diagnosis": {"summary": "具体诊断或不确定性", "evidence": "逐字引用实际回答，不能编造", "errorType": "错误类型或null", "needsClarification": "布尔值"},
            },
        } for case in dataset["cases"]],
    }


def check_output(case: dict, output: object) -> list[str]:
    """验证可确定的协议与证据边界；不以关键词替代知识质量或人工评分。"""
    errors = []
    if not isinstance(output, dict):
        return ["输出不是JSON对象"]
    if output.get("binding") != case["binding"]:
        errors.append("课程/课时/题目/回答绑定错误")
    if not nonblank(output.get("message")):
        errors.append("教学文字缺失")
    if output.get("nextAction") not in ACTIONS:
        errors.append("行动类型无效")
    if output.get("masteryStatus") not in MASTERY:
        errors.append("掌握状态无效")
    if output.get("masteryStatus") == "VERIFIED_MASTERY":
        errors.append("种子案例没有多次独立验证证据，不得认定已验证掌握")
    score = output.get("score")
    if "score" not in output or not (score is None or type(score) is int and 0 <= score <= 100):
        errors.append("成绩必须是0..100整数或null")
    if not case["formalSubmission"] and score is not None:
        errors.append("未正式作答或探索不得评分")
    diagnosis = output.get("diagnosis")
    if not isinstance(diagnosis, dict):
        return errors + ["缺少诊断对象"]
    if not nonblank(diagnosis.get("summary")) or type(diagnosis.get("needsClarification")) is not bool:
        errors.append("诊断说明或澄清状态无效")
    error_type = diagnosis.get("errorType")
    if error_type is not None and error_type not in ERROR_TYPES:
        errors.append("诊断类型无效")
    evidence = diagnosis.get("evidence")
    if not isinstance(evidence, list):
        errors.append("证据必须为原话列表")
    else:
        response = case["studentResponse"] or ""
        if any(not nonblank(quote) or quote not in response for quote in evidence):
            errors.append("诊断编造原话或引用无实际内容")
        if score is not None and not evidence:
            errors.append("正式成绩缺少实际回答依据")
    if diagnosis.get("needsClarification") is True and score is not None:
        errors.append("需澄清时不得提交正式成绩")
    return errors


def evaluate(dataset: dict, results: object) -> dict:
    """导入记录只产生程序检查；真实教学指标等待独立审核。"""
    validate_dataset(dataset)
    require(isinstance(results, dict) and type(results.get("schemaVersion")) is int and results["schemaVersion"] == 1, "结果版本无效")
    require(results.get("kind") in {"offline_fixture", "recorded_model"}, "必须区分模拟夹具与模型记录")
    require(results.get("datasetId") == dataset["datasetId"] and results.get("datasetHash") == digest(dataset), "结果不属于本版数据集")
    rows = results.get("records")
    require(isinstance(rows, list) and bool(rows), "结果没有任何记录")
    cases = {case["id"]: case for case in dataset["cases"]}
    seen, checks = set(), []
    for row in rows:
        require(isinstance(row, dict), "结果记录无效")
        case_id = row.get("caseId")
        require(isinstance(case_id, str) and case_id in cases and case_id not in seen, "未知或重复案例结果")
        seen.add(case_id)
        require(row.get("inputHash") == digest(cases[case_id]), f"{case_id} 输入已变化")
        require(nonblank(row.get("rawResponse")), f"{case_id} 缺少原始模型/夹具输出")
        try:
            output = parse_json(row["rawResponse"])
            errors = check_output(cases[case_id], output)
        except (ValueError, TypeError, OverflowError) as error:
            errors = ["原始输出无法按协议检查：" + str(error)]
        checks.append({"caseId": case_id, "passed": not errors, "errors": errors})
    return {
        "schemaVersion": 1, "kind": "program_contract_report", "sourceKind": results["kind"],
        "datasetId": dataset["datasetId"], "datasetHash": digest(dataset),
        "generatedAt": datetime.now(timezone.utc).isoformat(),
        "executedCases": len(rows), "missingCaseIds": sorted(set(cases) - seen),
        "programChecksPassed": sum(row["passed"] for row in checks), "checks": checks,
        "modelProvenanceVerified": False, "releaseEligible": False,
        "modelQuality": {"knowledgeAccuracy": None, "diagnosisAccuracy": None, "teachingQuality": None, "componentSuitability": None},
        "limitations": ["导入标签不构成真实模型运行证明", "程序检查不评价知识结论和教学效果", "没有专家审核与独立人工评分不得宣布教学达标"],
    }


def rating_template(dataset: dict) -> dict:
    """人工评分不预填成功值，也不以模型自评分代替审核。"""
    validate_dataset(dataset)
    return {
        "schemaVersion": 1, "kind": "human_review_template", "datasetId": dataset["datasetId"],
        "datasetHash": digest(dataset), "reviewer": None,
        "records": [{"caseId": case["id"], "inputHash": digest(case), "status": "pending",
                     "ratings": {field: None for field in RATING_FIELDS},
                     "criticalErrors": [], "notes": None} for case in dataset["cases"]],
    }


def write_json(path: Path, value: dict) -> None:
    """结果文件禁止覆盖已有原始实验记录。"""
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("x", encoding="utf-8") as handle:
        json.dump(value, handle, ensure_ascii=False, indent=2, allow_nan=False)
        handle.write("\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["validate", "prepare", "evaluate", "rating-template"])
    parser.add_argument("--dataset", type=Path, default=DEFAULT_DATASET)
    parser.add_argument("--results", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    try:
        dataset = load_json(args.dataset)
        if args.command == "validate":
            report = {"kind": "dataset_validation", **validate_dataset(dataset), "modelExperimentExecuted": False}
        elif args.command == "prepare":
            report = prepare(dataset)
        elif args.command == "rating-template":
            report = rating_template(dataset)
        else:
            require(args.results is not None, "evaluate 需要 --results")
            report = evaluate(dataset, load_json(args.results))
        if args.output:
            write_json(args.output, report)
            print(json.dumps({"saved": str(args.output), "kind": report["kind"]}, ensure_ascii=False))
        else:
            print(json.dumps(report, ensure_ascii=False, indent=2, allow_nan=False))
        return 1 if args.command == "evaluate" and (report["missingCaseIds"] or report["programChecksPassed"] != report["executedCases"]) else 0
    except (ValueError, OSError, TypeError) as error:
        print("教学评测未完成：" + str(error), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
