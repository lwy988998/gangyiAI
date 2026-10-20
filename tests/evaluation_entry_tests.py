"""授权门禁、隔离预算、版本化复用及未知统计的免费回归。"""
import copy
import json
import os
from pathlib import Path
import subprocess
import sqlite3
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'scripts'))
import teaching_eval_provider as provider
import evaluation_guard as guard
import evaluation_compare as compare


class EntryTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix='evaluation-entry-')
        self.root = Path(self.directory.name)
        self.driver = self.root/'driver.exe'
        self.driver.write_bytes(b'isolated-version-one')
        self.budget = self.root/'original-budget.json'
        self.budget.write_text(json.dumps({'limitTokens': 1000000, 'usedTokens': 839528, 'reservedTokens': 72075}), encoding='utf-8')
        self.original = self.budget.read_bytes()
        # CI使用公开合成决定，不依赖忽略目录内的真实预算决定或私有教学材料。
        protected = self.root/'protected-ledger'/'budget.json'
        approved = {'limitTokens': 1000000, 'knownUsedTokens': 839528, 'unknownReservedTokens': 72075,
                    'newPaidCallsAllowedThisRound': False, 'originalBudgetPath': str(protected)}
        self.decision_patch = patch.object(guard, 'decision', return_value=approved)
        self.decision_patch.start()
        self.addCleanup(self.decision_patch.stop)
        self.criteria_patch = patch.object(provider, 'reviewed_criteria', return_value={'status': '公开合成材料'})
        self.criteria_patch.start()
        self.addCleanup(self.criteria_patch.stop)
        self.case = {'id': 'synthetic', 'title': '公开合成材料', 'concept': '比较前后状态', 'questions': [],
                     'preferences': {'style': '先举例'}, 'turns': [{'type': 'teaching_request', 'requestType': 'hint', 'text': '提示'}]}
        self.config = {'variant': 'baseline', 'mode': 'preflight', 'profileName': 'GangyiAI-Library-Preview',
                       'driver': str(self.driver), 'sourceCommit': 'effb401', 'outputDir': str(self.root/'records'),
                       'budgetFile': str(self.budget)}
        self.launches = []

    def tearDown(self):
        self.assertEqual(self.budget.read_bytes(), self.original)
        self.directory.cleanup()

    def fake_run(self, command, **kwargs):
        actual = json.loads(Path(command[1]).read_text(encoding='utf-8'))
        self.launches.append((actual, kwargs['env']))
        budget = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
        self.assertNotEqual(budget.resolve(), self.budget.resolve())
        value = {'turns': [{'event': row, 'task': {'id': 'one-task', 'status': 'waiting_student'}} for row in actual['turns']],
                 'httpRequests': [], 'usage': []}
        Path(command[3]).write_text(json.dumps(value), encoding='utf-8')
        return subprocess.CompletedProcess(command, 0, b'', b'')

    def invoke(self, config=None, case=None):
        return provider.call_api('', {'config': config or self.config}, {'vars': {'case': case or self.case}})

    def test_live_refused_before_credentials_and_subprocess(self):
        config = dict(self.config, mode='live')
        with patch.object(provider, 'configured_environment', side_effect=AssertionError('禁止读取凭据')) as credential, \
                patch.object(provider.subprocess, 'run', side_effect=AssertionError('禁止启动HTTP')) as request:
            with self.assertRaises(PermissionError):
                self.invoke(config)
            credential.assert_not_called()
            request.assert_not_called()
        self.assertFalse((self.root/'records').exists())
        with self.assertRaises(PermissionError):
            provider.configured_environment()

    def test_native_protocol_zero_limit_and_no_credentials(self):
        with patch.object(provider, 'configured_environment', side_effect=AssertionError('离线不得读取凭据')), \
                patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            value = json.loads(self.invoke()['output'])
        case, env = self.launches[0]
        self.assertEqual(case['turns'][0]['type'], 'teaching_request')
        self.assertEqual(env['GANGYI_EVAL_MAX_CALLS'], '0')
        self.assertEqual(value['summary']['taskMetrics'][0]['completionMs'], None)
        self.assertFalse(value['httpRequests'])
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            self.invoke(dict(self.config, mode='fixture', fixtureUrl='http://127.0.0.1:9/v1', maxCalls=0))
        self.assertEqual(self.launches[-1][1]['GANGYI_EVAL_MAX_CALLS'], '0')

    def test_legacy_mapping_requires_explicit_protocol(self):
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            self.invoke(dict(self.config, entryProtocol='legacy-chat', sourceCommit='legacy-before-native'))
        self.assertEqual(self.launches[0][0]['turns'][0]['type'], 'chat')
        self.assertNotIn('requestType', self.launches[0][0]['turns'][0])
        self.assertEqual(self.case['turns'][0]['type'], 'teaching_request')
        with self.assertRaises(ValueError):
            self.invoke(dict(self.config, entryProtocol='legacy-chat'))

    def test_versioned_reuse_and_failed_records(self):
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            first = json.loads(self.invoke()['output'])
            config = dict(self.config, reuseDir=self.config['outputDir'])
            self.assertTrue(json.loads(self.invoke(config)['output'])['baselineReused'])
            self.assertEqual(len(self.launches), 1)
            self.invoke(dict(config, sourceCommit='different-version'))
            self.invoke(config, dict(self.case, preferences={'style': '先定义'}))
            self.driver.write_bytes(b'isolated-version-two')
            self.invoke(config)
            self.assertEqual(len(self.launches), 4)
            self.driver.write_bytes(b'isolated-version-one')
            path = Path(first['recordPath'])
            first['turns'][0]['task']['status'] = 'failed'
            path.write_text(json.dumps(first), encoding='utf-8')
            # 删除已复用的副本，确认失败原件不能作为通过记录。
            for other in Path(config['reuseDir']).glob('*/result.json'):
                if other != path:
                    saved = json.loads(other.read_text(encoding='utf-8'))
                    saved['complete'] = False
                    other.write_text(json.dumps(saved), encoding='utf-8')
            self.invoke(config)
            self.assertEqual(len(self.launches), 5)

    def test_budget_and_frozen_directories_are_protected(self):
        with self.assertRaises(ValueError):
            guard.isolated_output(guard.FROZEN/'new-output')
        with self.assertRaises(ValueError):
            guard.isolated_output(guard.decision()['originalBudgetPath'])
        self.budget.write_text(json.dumps({'limitTokens': 5000000, 'usedTokens': 839528, 'reservedTokens': 72075}), encoding='utf-8')
        self.original = self.budget.read_bytes()
        result = guard.budget_copy(self.root/'copy.json', self.budget)
        self.assertEqual(json.loads(result.read_text())['limitTokens'], 1000000)
        with self.assertRaises(FileExistsError):
            guard.budget_copy(result, self.budget)

    def test_http_failures_counted_and_task_metrics_not_summed(self):
        value = {'httpRequests': [{'status': 'failed', 'tokens': {'total_tokens': 14182}, 'durationMs': 20, 'queueMs': 30},
                                  {'status': 'completed', 'tokens': None, 'queueMs': 30}],
                 'usage': [{'tokenUsage': {'total_tokens': 1}}],
                 'turns': [{'task': {'id': 'same', 'queueMs': 30, 'completionMs': 80}},
                           {'task': {'id': 'same', 'queueMs': 30, 'completionMs': 80}}]}
        result = guard.summarize(value)
        self.assertEqual(result['knownTokens'], 14182)
        self.assertEqual(result['unknownUsageRequests'], 1)
        self.assertIsNone(result['totalTokens'])
        self.assertEqual(result['failedHttpRequests'], 1)
        self.assertEqual(len(result['taskMetrics']), 1)
        self.assertEqual(result['taskMetrics'][0]['queueMs'], 30)
        self.assertIsNone(result['taskMetrics'][0]['firstContentMs'])
        self.assertIsNone(guard.summarize({})['totalTokens'])

    def test_nonlocal_fixture_rejected(self):
        for url in ('https://api.example.org/v1', 'http://127.0.0.1:9@example.org/v1'):
            with self.assertRaises(ValueError):
                guard.local_fixture_url(url)

    def test_compare_keeps_one_cumulative_isolated_budget(self):
        cases_file, manifest_file = self.root/'cases.json', self.root/'manifest.json'
        cases_file.write_text(json.dumps([self.case]), encoding='utf-8')
        manifest_file.write_text(json.dumps({'mode': 'preflight', 'casesFile': str(cases_file),
            'shared': {'budgetFile': str(self.budget)}, 'entries': [{'variant': 'baseline'}, {'variant': 'candidate'}]}), encoding='utf-8')
        starting = []
        def fake_call(prompt, options, context):
            value = json.loads(Path(options['config']['budgetFile']).read_text(encoding='utf-8'))
            starting.append(value['usedTokens'])
            value['usedTokens'] += 100
            value['reservedTokens'] += 50
            return {'output': json.dumps({'budgetStatus': value, 'complete': False}), 'budgetStatus': value}
        with patch.object(sys, 'argv', ['evaluation_compare.py', str(manifest_file), '--output', str(self.root/'pair')]), \
                patch.object(compare, 'call_api', side_effect=fake_call):
            self.assertEqual(compare.main(), 0)
        self.assertEqual(starting, [839528, 839628])
        final = json.loads((self.root/'pair/budget-current.json').read_text())
        self.assertEqual(final['usedTokens'], 839728)
        self.assertEqual(final['reservedTokens'], 72175)

    def test_process_failure_retains_budget_reservation(self):
        def fail(command, **kwargs):
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            budget = json.loads(path.read_text())
            budget['reservedTokens'] += 20000
            path.write_text(json.dumps(budget), encoding='utf-8')
            return subprocess.CompletedProcess(command, 9, b'', b'PRIVATE-CREDENTIAL')
        with patch.object(provider.subprocess, 'run', side_effect=fail):
            result = self.invoke()
        self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)
        self.assertNotIn('PRIVATE-CREDENTIAL', json.dumps(result))
        self.assertTrue((Path(result['metadata']['recordPath'])/'run-error.json').exists())

    def test_reuse_invalidates_model_and_source_snapshot(self):
        source = self.root/'saved-source.db'
        source.write_bytes(b'fixed-snapshot-one')
        case = dict(self.case, libraryDatabase=str(source), librarySourceIds=['fixed-source'])
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            first = json.loads(self.invoke(case=case)['output'])
            config = dict(self.config, reuseDir=self.config['outputDir'])
            model = json.loads(self.invoke(dict(config, model='different-model'), case)['output'])
            source.write_bytes(b'fixed-snapshot-two')
            changed = json.loads(self.invoke(config, case)['output'])
        self.assertNotEqual(first['reuseKey'], model['reuseKey'])
        self.assertNotEqual(first['reuseKey'], changed['reuseKey'])
        self.assertEqual(len(self.launches), 3)

    def compare_arguments(self, name, case=None):
        """两版共用公开合成用例与隔离预算，允许测试直接经过真实provider收尾。"""
        cases_file, manifest_file = self.root/(name+'-cases.json'), self.root/(name+'-manifest.json')
        cases_file.write_text(json.dumps([case or self.case]), encoding='utf-8')
        shared = {key: value for key, value in self.config.items()
                  if key in ('profileName', 'driver', 'sourceCommit', 'budgetFile')}
        manifest_file.write_text(json.dumps({'mode': 'preflight', 'casesFile': str(cases_file),
            'shared': shared, 'entries': [{'variant': 'baseline'}, {'variant': 'candidate'}]}), encoding='utf-8')
        output = self.root/name
        return ['evaluation_compare.py', str(manifest_file), '--output', str(output)], output

    def test_corrupt_result_keeps_reservation_for_next_variant(self):
        arguments, output = self.compare_arguments('corrupt-result')
        starting = []
        def execute(command, **kwargs):
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            budget = guard.read_budget(path)
            starting.append(budget['reservedTokens'])
            budget['reservedTokens'] += 20000
            path.write_text(json.dumps(budget), encoding='utf-8')
            if len(starting) == 1:
                Path(command[3]).write_text('{broken-json', encoding='utf-8')
                return subprocess.CompletedProcess(command, 0)
            return self.fake_run(command, **kwargs)
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            self.assertEqual(compare.main(), 1)
        self.assertEqual(starting, [72075, 92075])
        self.assertEqual(guard.read_budget(output/'budget-current.json')['reservedTokens'], 112075)
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        self.assertIn('JSONDecodeError', report['results'][0]['result']['error'])

    def test_summary_failure_returns_budget_outside_result(self):
        def execute(command, **kwargs):
            self.fake_run(command, **kwargs)
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            budget = guard.read_budget(path)
            budget['reservedTokens'] += 20000
            path.write_text(json.dumps(budget), encoding='utf-8')
            return subprocess.CompletedProcess(command, 0)
        with patch.object(provider.subprocess, 'run', side_effect=execute), \
                patch.object(provider, 'summarize', side_effect=ValueError('PRIVATE-CREDENTIAL')):
            result = self.invoke()
        self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)
        self.assertFalse(result['complete'])
        self.assertNotIn('PRIVATE-CREDENTIAL', json.dumps(result))

    def test_unreadable_budget_stops_before_next_variant(self):
        arguments, output = self.compare_arguments('unreadable-budget')
        calls = []
        def execute(command, **kwargs):
            calls.append(command)
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            path.write_text('{budget-state-lost', encoding='utf-8')
            return self.fake_run(command, **kwargs)
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            self.assertEqual(compare.main(), 1)
        self.assertEqual(len(calls), 1)
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        self.assertIn('BudgetStateError', report['results'][0]['error'])

    def test_native_budget_save_failure_rejects_old_valid_budget_and_stops(self):
        arguments, output = self.compare_arguments('native-save-failure')
        def execute(command, **kwargs):
            self.fake_run(command, **kwargs)
            return subprocess.CompletedProcess(command, 73)
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            self.assertEqual(compare.main(), 1)
        self.assertEqual(len(self.launches), 1)
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        self.assertIn('BudgetStateError', report['results'][0]['error'])
        self.assertEqual(guard.read_budget(output/'budget-current.json')['reservedTokens'], 72075)

    def test_result_and_error_report_write_failures_keep_budget_receipt(self):
        write = Path.write_text
        output_writes = []
        def write_file(path, text, *args, **kwargs):
            if path.name == 'result.json':
                output_writes.append(path)
                if len(output_writes) > 1:
                    raise PermissionError('模拟结果后处理不可写')
            if path.name == 'run-error.json':
                raise PermissionError('模拟错误报告不可写')
            return write(path, text, *args, **kwargs)
        def execute(command, **kwargs):
            self.fake_run(command, **kwargs)
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            budget = guard.read_budget(path)
            budget['reservedTokens'] += 20000
            path.write_text(json.dumps(budget), encoding='utf-8')
            return subprocess.CompletedProcess(command, 0)
        with patch.object(provider.subprocess, 'run', side_effect=execute), patch.object(Path, 'write_text', new=write_file):
            result = self.invoke()
        self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)
        self.assertTrue(result['reportWriteFailed'])
        self.assertFalse(result['complete'])

    def test_cumulative_budget_save_failure_stops_before_next_variant(self):
        arguments, output = self.compare_arguments('unwritable-budget')
        def fail_persist(path, value):
            raise guard.BudgetStateError('模拟累计预算不可写')
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=self.fake_run), \
                patch.object(compare, 'persist_budget', side_effect=fail_persist):
            self.assertEqual(compare.main(), 1)
        self.assertEqual(len(self.launches), 1)
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        self.assertIn('BudgetStateError', report['results'][0]['error'])

    def test_persist_budget_rejects_directory_target(self):
        target = self.root/'budget-directory'
        target.mkdir()
        (target/'occupied').write_text('不能被文件替换', encoding='utf-8')
        with self.assertRaises(guard.BudgetStateError):
            guard.persist_budget(target, {'limitTokens': 1000000, 'usedTokens': 0, 'reservedTokens': 0})

    def test_new_committed_wal_stops_next_variant_without_main_db_change(self):
        source = self.root/'wal-source.db'
        with sqlite3.connect(source) as db:
            db.execute('PRAGMA journal_mode=WAL')
            db.execute('CREATE TABLE Source(text TEXT)')
            db.commit()
        db.close()
        self.assertFalse(Path(str(source)+'-wal').exists())
        before = guard.file_hash(source)
        arguments, output = self.compare_arguments('new-wal', dict(self.case, libraryDatabase=str(source)))
        connections = []
        def execute(command, **kwargs):
            db = sqlite3.connect(source)
            connections.append(db)
            db.execute("INSERT INTO Source VALUES('运行中提交的新原文')")
            db.commit()
            self.assertEqual(guard.file_hash(source), before)
            self.assertTrue(Path(str(source)+'-wal').exists())
            return self.fake_run(command, **kwargs)
        try:
            with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
                self.assertEqual(compare.main(), 1)
            self.assertEqual(len(self.launches), 1)
            report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
            self.assertIsNone(report['sourceMaterialHashes'][str(Path(str(source)+'-wal').resolve())])
            self.assertIn('资料快照', report['results'][-1]['error'])
        finally:
            for connection in connections:
                connection.close()

    def test_removed_wal_stops_next_variant(self):
        source = self.root/'removed-wal-source.db'
        source.write_bytes(b'unchanged-main-db')
        wal = Path(str(source)+'-wal')
        wal.write_bytes(b'initial-sidecar')
        arguments, output = self.compare_arguments('removed-wal', dict(self.case, libraryDatabase=str(source)))
        def execute(command, **kwargs):
            wal.unlink()
            return self.fake_run(command, **kwargs)
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            self.assertEqual(compare.main(), 1)
        self.assertEqual(len(self.launches), 1)
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        self.assertIn('资料快照', report['results'][-1]['error'])


if __name__ == '__main__':
    unittest.main()
