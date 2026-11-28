"""仅使用内存中的虚构案例核对评审证据，绝不生成真实人工评分。"""

import copy
import importlib.util
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('tutor_benchmark', ROOT / 'scripts/tutor_benchmark.py')
benchmark = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(benchmark)


class NativeHumanReviewTests(unittest.TestCase):
    def setUp(self):
        def completed(case_id, line):
            return {'caseId': case_id, 'inputHash': '输入-' + case_id, 'status': 'completed', 'passed': True,
                    'result': {'path': case_id + '/result.json', 'sha256': '结果-' + case_id},
                    'rawResponses': {'path': case_id + '/responses.jsonl', 'sha256': '原文-' + case_id,
                                     'entries': [{'line': line, 'completed': True}]}}
        self.report = {'datasetId': '隔离样例', 'datasetHash': '数据摘要', 'manifestSha256': '清单摘要',
                       'runSha256': '运行摘要', 'sourceKind': 'offline_fixture', 'selectedCases': 3,
                       'missingCaseIds': ['未运行案例'],
                       'checks': [completed('甲', 5), completed('乙', 7),
                                  {'caseId': '丙', 'status': 'uncertain', 'passed': False}]}

    def review(self, reviewer):
        value = benchmark.native_rating_template(self.report)
        value['reviewer'] = {'id': reviewer, 'name': '虚构评审 ' + reviewer,
                             'qualification': '仅供自动测试', 'kind': 'human'}
        return value

    def rate(self, row, score, line):
        row.update(status='reviewed', reviewedAt='2026-11-28T09:00:00+08:00',
                   ratings=dict.fromkeys(benchmark.RATING_FIELDS, score),
                   evidenceLines=[line], criticalErrors=[], notes='虚构评分，仅供测试')

    def test_blank_template_and_unreviewed_cases_do_not_create_scores(self):
        blank = benchmark.native_rating_template(self.report)
        self.assertIsNone(blank['reviewer'])
        self.assertEqual([row['caseId'] for row in blank['records']], ['甲', '乙'])
        self.assertTrue(all(value is None for row in blank['records'] for value in row['ratings'].values()))
        summary = benchmark.summarize_native_ratings(self.report, [self.review('甲评审')])
        self.assertEqual(summary['reviewedCases'], 0)
        self.assertEqual(summary['unreviewedCaseIds'], ['乙', '甲'])
        self.assertEqual(summary['unavailableCaseIds'], ['丙'])
        self.assertIsNone(summary['descriptiveScore']['average'])
        self.assertFalse(summary['humanIdentityVerified'] or summary['releaseEligible'])

    def test_case_weighting_and_unfinished_critical_notes_are_preserved(self):
        first, second = self.review('甲评审'), self.review('乙评审')
        self.rate(first['records'][0], 5, 5)
        first['records'][1].update(notes='问题尚未评完，先记下明确错误。', evidenceLines=[7],
                                   criticalErrors=['虚构的关键错误，仅供测试'])
        partial = benchmark.summarize_native_ratings(self.report, [first])
        self.assertEqual(partial['unreviewedCaseIds'], ['乙'])
        self.assertEqual(partial['criticalErrors'][0]['caseId'], '乙')
        self.assertEqual(partial['pendingObservations'][0]['notes'], '问题尚未评完，先记下明确错误。')
        self.rate(second['records'][0], 1, 5)
        self.rate(second['records'][1], 5, 7)
        summary = benchmark.summarize_native_ratings(self.report, [first, second])
        self.assertEqual(summary['reviewedCases'], 2)
        self.assertEqual(summary['descriptiveScore']['average'], 4)
        self.assertEqual(summary['criticalErrors'][0]['description'], '虚构的关键错误，仅供测试')
        self.assertIsNone(summary['modelQuality'])
        self.assertFalse(summary['modelProvenanceVerified'] or summary['releaseEligible'])

    def test_wrong_archive_and_response_identity_are_rejected(self):
        review = self.review('甲评审')
        for change in ('runSha256', 'rawResponses', 'inputHash'):
            altered = copy.deepcopy(review)
            if change == 'runSha256':
                altered['runSha256'] = '旧归档'
            elif change == 'rawResponses':
                altered['records'][0]['rawResponses']['sha256'] = '不同原文'
            else:
                altered['records'][0]['inputHash'] = '其他输入'
            with self.subTest(change=change), self.assertRaises(ValueError):
                benchmark.summarize_native_ratings(self.report, [altered])

    def test_invalid_evidence_partial_scores_and_duplicate_identities_are_rejected(self):
        for failure in ('line', 'critical_without_evidence', 'pending_score', 'bool_score', 'time', 'duplicate_case'):
            review = self.review('甲评审')
            row = review['records'][0]
            if failure == 'critical_without_evidence':
                row['criticalErrors'] = ['不能定位的错误']
            elif failure == 'pending_score':
                row['ratings']['accuracy'] = 5
            else:
                self.rate(row, 5, 5)
                if failure == 'line': row['evidenceLines'] = [7]
                if failure == 'bool_score': row['ratings']['accuracy'] = True
                if failure == 'time': row['reviewedAt'] = '2026-11-28T09:00:00'
                if failure == 'duplicate_case': review['records'].append(copy.deepcopy(row))
            with self.subTest(failure=failure), self.assertRaises(ValueError):
                benchmark.summarize_native_ratings(self.report, [review])
        with self.assertRaises(ValueError):
            benchmark.summarize_native_ratings(self.report, [self.review('test'), self.review(' TEST ')])


if __name__ == '__main__':
    unittest.main()
