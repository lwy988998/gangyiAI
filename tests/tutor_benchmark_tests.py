"""教学评测的证据污染、错误绑定和假验收回归。"""

import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("tutor_benchmark", ROOT / "scripts/tutor_benchmark.py")
benchmark = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(benchmark)


class TutorBenchmarkTests(unittest.TestCase):
    """固定种子覆盖与导入记录必须严格隔离。"""

    def setUp(self):
        self.dataset = benchmark.load_json(benchmark.DEFAULT_DATASET)
        self.case = self.dataset["cases"][1]

    def valid_output(self, case=None):
        case = case or self.case
        return {
            "binding": case["binding"], "message": "先保留你答对的部分，再检查具体依据。",
            "nextAction": "feedback", "score": 30 if case["formalSubmission"] else None,
            "masteryStatus": "PARTIAL" if case["formalSubmission"] else "LEARNING",
            "diagnosis": {"summary": "需要进一步核对概念。", "evidence": [case["studentResponse"]] if case["studentResponse"] else [],
                          "errorType": "conceptual" if case["formalSubmission"] else None, "needsClarification": False},
        }

    def results(self, output=None):
        return {"schemaVersion": 1, "kind": "offline_fixture", "datasetId": self.dataset["datasetId"],
                "datasetHash": benchmark.digest(self.dataset), "records": [{"caseId": self.case["id"],
                "inputHash": benchmark.digest(self.case), "rawResponse": json.dumps(output or self.valid_output(), ensure_ascii=False)}]}

    def test_seed_covers_nine_subjects_without_expert_claim(self):
        info = benchmark.validate_dataset(self.dataset)
        self.assertEqual((info["cases"], info["subjects"]), (18, 9))
        self.assertEqual(info["reviewStatus"], "pending_expert_review")

    def test_duplicate_case_cannot_hide_missing_scenario(self):
        self.dataset["cases"][0] = copy.deepcopy(self.dataset["cases"][1])
        with self.assertRaises(ValueError):
            benchmark.validate_dataset(self.dataset)

    def test_unanswered_cannot_be_formal_submission(self):
        self.dataset["cases"][0]["formalSubmission"] = True
        with self.assertRaises(ValueError):
            benchmark.validate_dataset(self.dataset)

    def test_expert_label_requires_reviewer_and_time(self):
        self.dataset["review"]["status"] = "expert_reviewed"
        with self.assertRaises(ValueError):
            benchmark.validate_dataset(self.dataset)

    def test_cross_lesson_binding_is_rejected(self):
        output = self.valid_output()
        output["binding"] = {**output["binding"], "lessonId": "another-lesson"}
        self.assertTrue(benchmark.check_output(self.case, output))

    def test_invented_or_whitespace_evidence_is_rejected(self):
        for evidence in [["学生没有说过的句子"], [" "], [123]]:
            with self.subTest(evidence=evidence):
                output = self.valid_output()
                output["diagnosis"]["evidence"] = evidence
                self.assertTrue(benchmark.check_output(self.case, output))

    def test_exploration_and_clarification_cannot_be_scored(self):
        case = self.dataset["cases"][0]
        output = self.valid_output(case)
        output["score"] = 100
        self.assertTrue(benchmark.check_output(case, output))
        output = self.valid_output()
        output["diagnosis"]["needsClarification"] = True
        self.assertTrue(benchmark.check_output(self.case, output))

    def test_boolean_and_out_of_range_scores_are_rejected(self):
        for score in [True, 101, -1, 1.5, "100"]:
            output = self.valid_output()
            output["score"] = score
            self.assertTrue(benchmark.check_output(self.case, output))

    def test_one_correct_answer_cannot_be_verified_mastery(self):
        output = self.valid_output()
        output["masteryStatus"] = "VERIFIED_MASTERY"
        self.assertTrue(benchmark.check_output(self.case, output))

    def test_missing_and_duplicate_runs_cannot_count_as_full_pass(self):
        results = self.results()
        report = benchmark.evaluate(self.dataset, results)
        self.assertEqual(len(report["missingCaseIds"]), 17)
        results["records"].append(copy.deepcopy(results["records"][0]))
        with self.assertRaises(ValueError):
            benchmark.evaluate(self.dataset, results)

    def test_stale_dataset_and_input_are_rejected(self):
        results = self.results()
        results["records"][0]["inputHash"] = "old-input"
        with self.assertRaises(ValueError):
            benchmark.evaluate(self.dataset, results)
        results = self.results()
        results["datasetHash"] = "old-dataset"
        with self.assertRaises(ValueError):
            benchmark.evaluate(self.dataset, results)

    def test_raw_output_is_required_and_parse_failure_is_preserved(self):
        results = self.results()
        results["records"][0]["rawResponse"] = "{unfinished"
        report = benchmark.evaluate(self.dataset, results)
        self.assertEqual(report["programChecksPassed"], 0)
        self.assertTrue(report["checks"][0]["errors"])
        del results["records"][0]["rawResponse"]
        with self.assertRaises(ValueError):
            benchmark.evaluate(self.dataset, results)

    def test_raw_duplicate_keys_and_nonfinite_numbers_are_rejected(self):
        for raw in ['{"score":0,"score":100}', '{"score":NaN}']:
            results = self.results()
            results["records"][0]["rawResponse"] = raw
            self.assertEqual(benchmark.evaluate(self.dataset, results)["programChecksPassed"], 0)

    def test_program_success_never_becomes_model_quality(self):
        results = self.results()
        report = benchmark.evaluate(self.dataset, results)
        self.assertEqual(report["programChecksPassed"], 1)
        self.assertFalse(report["releaseEligible"])
        self.assertFalse(report["modelProvenanceVerified"])
        self.assertTrue(all(value is None for value in report["modelQuality"].values()))
        results["kind"] = "recorded_model"
        self.assertFalse(benchmark.evaluate(self.dataset, results)["modelProvenanceVerified"])

    def test_preparation_does_not_execute_and_ratings_are_empty(self):
        self.assertFalse(benchmark.prepare(self.dataset)["executed"])
        template = benchmark.rating_template(self.dataset)
        self.assertTrue(all(rating is None for row in template["records"] for rating in row["ratings"].values()))

    def test_raw_files_and_duplicate_json_keys_are_protected(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "raw.json"
            benchmark.write_json(path, {"original": True})
            with self.assertRaises(FileExistsError):
                benchmark.write_json(path, {"overwritten": True})
            path.write_text('{"score":1,"score":100}', encoding="utf-8")
            with self.assertRaises(ValueError):
                benchmark.load_json(path)


if __name__ == "__main__":
    unittest.main()
