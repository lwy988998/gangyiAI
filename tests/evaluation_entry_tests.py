# 【逐行批注】 执行当前 Python 语句："""授权门禁、隔离预算、版本化复用及未知统计的免费回归。"""。
"""授权门禁、隔离预算、版本化复用及未知统计的免费回归。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import copy。
import copy
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import unittest。
import unittest
# 【逐行批注】 导入后续逻辑需要的模块或符号：from unittest.mock import patch。
from unittest.mock import patch

# 【逐行批注】 调用 str、Path 完成当前脚本或测试步骤：sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'scripts'))。
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'scripts'))
# 【逐行批注】 导入后续逻辑需要的模块或符号：import teaching_eval_provider as provider。
import teaching_eval_provider as provider
# 【逐行批注】 导入后续逻辑需要的模块或符号：import evaluation_guard as guard。
import evaluation_guard as guard
# 【逐行批注】 导入后续逻辑需要的模块或符号：import evaluation_compare as compare。
import evaluation_compare as compare


# 【逐行批注】 定义类 LiveAuthorizationTests，组织相关数据与行为。
class LiveAuthorizationTests(unittest.TestCase):
    # 【逐行批注】 定义函数 setUp，封装可复用的处理步骤。
    def setUp(self):
        # 【逐行批注】 计算右侧结果并赋给 self.directory，更新脚本状态。
        self.directory = tempfile.TemporaryDirectory(prefix='evaluation-authorization-')
        # 【逐行批注】 执行当前 Python 语句：self.addCleanup(self.directory.cleanup)。
        self.addCleanup(self.directory.cleanup)
        # 【逐行批注】 计算右侧结果并赋给 self.root，更新脚本状态。
        self.root = Path(self.directory.name)
        # 【逐行批注】 计算右侧结果并赋给 self.original，更新脚本状态。
        self.original = self.root/'original.json'
        # 【逐行批注】 计算右侧结果并赋给 self.active，更新脚本状态。
        self.active = self.root/'active.json'
        # 【逐行批注】 计算右侧结果并赋给 self.state，更新脚本状态。
        self.state = {'limitTokens': 1000000, 'usedTokens': 839528, 'reservedTokens': 72075}
        # 【逐行批注】 计算右侧结果并赋给 self.original.write_text(json.dumps(self.sta…，更新脚本状态。
        self.original.write_text(json.dumps(self.state), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 self.active.write_text(json.dumps(self.state…，更新脚本状态。
        self.active.write_text(json.dumps(self.state), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 self.approval，更新脚本状态。
        self.approval = {'newPaidCallsAllowedThisRound': True, 'limitTokens': 1000000,
            # 【逐行批注】 执行当前 Python 语句：'knownUsedTokens': 839528, 'unknownReservedTokens': 72075,。
            'knownUsedTokens': 839528, 'unknownReservedTokens': 72075,
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'originalBudgetPath': str(self.original), 'originalBudgetSha256': gua…。
            'originalBudgetPath': str(self.original), 'originalBudgetSha256': guard.file_hash(self.original),
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'activeBudgetPath': str(self.active), 'baseUrl': 'https://frozen.exam…。
            'activeBudgetPath': str(self.active), 'baseUrl': 'https://frozen.example/v1', 'model': 'frozen-model'}
        # 【逐行批注】 计算右侧结果并赋给 self.path，更新脚本状态。
        self.path = self.root/'decision.json'

    # 【逐行批注】 定义函数 write_decision，封装可复用的处理步骤。
    def write_decision(self):
        # 【逐行批注】 计算右侧结果并赋给 self.path.write_text(json.dumps(self.approva…，更新脚本状态。
        self.path.write_text(json.dumps(self.approval), encoding='utf-8')

    # 【逐行批注】 定义函数 test_explicit_decision_restores_previous_context，封装可复用的处理步骤。
    def test_explicit_decision_restores_previous_context(self):
        # 【逐行批注】 执行当前 Python 语句：self.write_decision()。
        self.write_decision()
        # 【逐行批注】 计算右侧结果并赋给 denied，更新脚本状态。
        denied = self.root/'denied.json'
        # 【逐行批注】 计算右侧结果并赋给 denied.write_text('{"newPaidCallsAllowedThis…，更新脚本状态。
        denied.write_text('{"newPaidCallsAllowedThisRound":false}', encoding='utf-8')
        # 【逐行批注】 在受控上下文中执行资源操作：with guard.using_decision(denied):。
        with guard.using_decision(denied):
            # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
            with self.assertRaises(PermissionError):
                # 【逐行批注】 执行当前 Python 语句：guard.require_live_authorization()。
                guard.require_live_authorization()
            # 【逐行批注】 在受控上下文中执行资源操作：with guard.using_decision(self.path):。
            with guard.using_decision(self.path):
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤：self.assertEqual(guard.require_live_authorization()['activeBudgetPath…。
                self.assertEqual(guard.require_live_authorization()['activeBudgetPath'], str(self.active))
            # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
            with self.assertRaises(PermissionError):
                # 【逐行批注】 执行当前 Python 语句：guard.require_live_authorization()。
                guard.require_live_authorization()

    # 【逐行批注】 定义函数 test_reset_budget_and_original_mutation_are_rejected，封装可复用的处理步骤。
    def test_reset_budget_and_original_mutation_are_rejected(self):
        # 【逐行批注】 执行当前 Python 语句：self.write_decision()。
        self.write_decision()
        # 【逐行批注】 在受控上下文中执行资源操作：with guard.using_decision(self.path):。
        with guard.using_decision(self.path):
            # 【逐行批注】 遍历数据并重复执行循环体：for field in ('usedTokens', 'reservedTokens'):。
            for field in ('usedTokens', 'reservedTokens'):
                # 【逐行批注】 计算右侧结果并赋给 reduced，更新脚本状态。
                reduced = dict(self.state, **{field: 0})
                # 【逐行批注】 计算右侧结果并赋给 self.active.write_text(json.dumps(reduced), …，更新脚本状态。
                self.active.write_text(json.dumps(reduced), encoding='utf-8')
                # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
                with self.assertRaises(PermissionError):
                    # 【逐行批注】 执行当前 Python 语句：guard.require_live_authorization()。
                    guard.require_live_authorization()
            # 【逐行批注】 计算右侧结果并赋给 self.active.write_text(json.dumps(self.state…，更新脚本状态。
            self.active.write_text(json.dumps(self.state), encoding='utf-8')
            # 【逐行批注】 计算右侧结果并赋给 self.original.write_text('{}', encoding，更新脚本状态。
            self.original.write_text('{}', encoding='utf-8')
            # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
            with self.assertRaises(PermissionError):
                # 【逐行批注】 执行当前 Python 语句：guard.require_live_authorization()。
                guard.require_live_authorization()

    # 【逐行批注】 定义函数 test_budget_increase_requires_explicit_matching_authorization，封装可复用的处理步骤。
    def test_budget_increase_requires_explicit_matching_authorization(self):
        # 【逐行批注】 执行当前 Python 语句：self.write_decision()。
        self.write_decision()
        # 【逐行批注】 计算右侧结果并赋给 self.active.write_text(json.dumps(dict(self.…，更新脚本状态。
        self.active.write_text(json.dumps(dict(self.state, limitTokens=1200000)), encoding='utf-8')
        # 【逐行批注】 在受控上下文中执行资源操作：with guard.using_decision(self.path):。
        with guard.using_decision(self.path):
            # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
            with self.assertRaises(PermissionError):
                # 【逐行批注】 执行当前 Python 语句：guard.require_live_authorization()。
                guard.require_live_authorization()
            # 【逐行批注】 计算右侧结果并赋给 self.approval['limitTokens']，更新脚本状态。
            self.approval['limitTokens'] = 1200000
            # 【逐行批注】 执行当前 Python 语句：self.write_decision()。
            self.write_decision()
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(guard.require_live_authorization()['limitTokens'], 1200000)。
            self.assertEqual(guard.require_live_authorization()['limitTokens'], 1200000)
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(guard.read_budget(self.active)['usedTokens'], self.state['usedTokens'])。
            self.assertEqual(guard.read_budget(self.active)['usedTokens'], self.state['usedTokens'])
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(guard.read_budget(self.active)['reservedTokens'], self.state['reservedTo…。
            self.assertEqual(guard.read_budget(self.active)['reservedTokens'], self.state['reservedTokens'])

    # 【逐行批注】 定义函数 test_frozen_model_mismatch_precedes_credentials，封装可复用的处理步骤。
    def test_frozen_model_mismatch_precedes_credentials(self):
        # 【逐行批注】 执行当前 Python 语句：self.write_decision()。
        self.write_decision()
        # 【逐行批注】 计算右侧结果并赋给 frozen，更新脚本状态。
        frozen = self.root/'configuration.json'
        # 【逐行批注】 计算右侧结果并赋给 frozen.write_text(json.dumps({'profileName':…，更新脚本状态。
        frozen.write_text(json.dumps({'profileName': 'fixture', 'baseUrl': self.approval['baseUrl'], 'model': 'different'}), encoding='utf-8')
        # 【逐行批注】 在受控上下文中执行资源操作：with guard.using_decision(self.path), patch.object(provider.ctypes, 'WinDLL', c…。
        with guard.using_decision(self.path), patch.object(provider.ctypes, 'WinDLL', create=True) as credential:
            # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
            with self.assertRaises(PermissionError):
                # 【逐行批注】 调用 str 完成当前脚本或测试步骤：provider.configured_environment({'profileName': 'fixture', 'frozenCon…。
                provider.configured_environment({'profileName': 'fixture', 'frozenConfiguration': str(frozen)})
            # 【逐行批注】 执行当前 Python 语句：credential.assert_not_called()。
            credential.assert_not_called()


# 【逐行批注】 定义类 EntryTests，组织相关数据与行为。
class EntryTests(unittest.TestCase):
    # 【逐行批注】 定义函数 setUp，封装可复用的处理步骤。
    def setUp(self):
        # 【逐行批注】 计算右侧结果并赋给 self.directory，更新脚本状态。
        self.directory = tempfile.TemporaryDirectory(prefix='evaluation-entry-')
        # 【逐行批注】 计算右侧结果并赋给 self.root，更新脚本状态。
        self.root = Path(self.directory.name)
        # 【逐行批注】 计算右侧结果并赋给 self.driver，更新脚本状态。
        self.driver = self.root/'driver.exe'
        # 【逐行批注】 执行当前 Python 语句：self.driver.write_bytes(b'isolated-version-one')。
        self.driver.write_bytes(b'isolated-version-one')
        # 【逐行批注】 计算右侧结果并赋给 self.budget，更新脚本状态。
        self.budget = self.root/'original-budget.json'
        # 【逐行批注】 计算右侧结果并赋给 self.budget.write_text(json.dumps({'limitTok…，更新脚本状态。
        self.budget.write_text(json.dumps({'limitTokens': 1000000, 'usedTokens': 839528, 'reservedTokens': 72075}), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 self.original，更新脚本状态。
        self.original = self.budget.read_bytes()
        # CI使用公开合成决定，不依赖忽略目录内的真实预算决定或私有教学材料。
        # 【逐行批注】 计算右侧结果并赋给 protected，更新脚本状态。
        protected = self.root/'protected-ledger'/'budget.json'
        # 【逐行批注】 计算右侧结果并赋给 approved，更新脚本状态。
        approved = {'limitTokens': 1000000, 'knownUsedTokens': 839528, 'unknownReservedTokens': 72075,
                    # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'newPaidCallsAllowedThisRound': False, 'originalBudgetPath': str(prot…。
                    'newPaidCallsAllowedThisRound': False, 'originalBudgetPath': str(protected)}
        # 【逐行批注】 计算右侧结果并赋给 self.decision_patch，更新脚本状态。
        self.decision_patch = patch.object(guard, 'decision', return_value=approved)
        # 【逐行批注】 执行当前 Python 语句：self.decision_patch.start()。
        self.decision_patch.start()
        # 【逐行批注】 执行当前 Python 语句：self.addCleanup(self.decision_patch.stop)。
        self.addCleanup(self.decision_patch.stop)
        # 【逐行批注】 计算右侧结果并赋给 self.criteria_patch，更新脚本状态。
        self.criteria_patch = patch.object(provider, 'reviewed_criteria', return_value={'status': '公开合成材料'})
        # 【逐行批注】 执行当前 Python 语句：self.criteria_patch.start()。
        self.criteria_patch.start()
        # 【逐行批注】 执行当前 Python 语句：self.addCleanup(self.criteria_patch.stop)。
        self.addCleanup(self.criteria_patch.stop)
        # 【逐行批注】 计算右侧结果并赋给 self.case，更新脚本状态。
        self.case = {'id': 'synthetic', 'title': '公开合成材料', 'concept': '比较前后状态', 'questions': [],
                     # 【逐行批注】 执行当前 Python 语句：'preferences': {'style': '先举例'}, 'turns': [{'type': 'teaching_request', 'requestType': 'h…。
                     'preferences': {'style': '先举例'}, 'turns': [{'type': 'teaching_request', 'requestType': 'hint', 'text': '提示'}]}
        # 【逐行批注】 计算右侧结果并赋给 self.config，更新脚本状态。
        self.config = {'variant': 'baseline', 'mode': 'preflight', 'profileName': 'GangyiAI-Library-Preview',
                       # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'driver': str(self.driver), 'sourceCommit': 'effb401', 'outputDir': s…。
                       'driver': str(self.driver), 'sourceCommit': 'effb401', 'outputDir': str(self.root/'records'),
                       # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'budgetFile': str(self.budget)}。
                       'budgetFile': str(self.budget)}
        # 【逐行批注】 计算右侧结果并赋给 self.launches，更新脚本状态。
        self.launches = []

    # 【逐行批注】 定义函数 tearDown，封装可复用的处理步骤。
    def tearDown(self):
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(self.budget.read_bytes(), self.original)。
        self.assertEqual(self.budget.read_bytes(), self.original)
        # 【逐行批注】 执行当前 Python 语句：self.directory.cleanup()。
        self.directory.cleanup()

    # 【逐行批注】 定义函数 fake_run，封装可复用的处理步骤。
    def fake_run(self, command, **kwargs):
        # 【逐行批注】 计算右侧结果并赋给 actual，更新脚本状态。
        actual = json.loads(Path(command[1]).read_text(encoding='utf-8'))
        # 【逐行批注】 执行当前 Python 语句：self.launches.append((actual, kwargs['env']))。
        self.launches.append((actual, kwargs['env']))
        # 【逐行批注】 计算右侧结果并赋给 budget，更新脚本状态。
        budget = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
        # 【逐行批注】 执行当前 Python 语句：self.assertNotEqual(budget.resolve(), self.budget.resolve())。
        self.assertNotEqual(budget.resolve(), self.budget.resolve())
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = {'turns': [{'event': row, 'task': {'id': 'one-task', 'status': 'waiting_student'}} for row in actual['turns']],
                 # 【逐行批注】 执行当前 Python 语句：'httpRequests': [], 'usage': []}。
                 'httpRequests': [], 'usage': []}
        # 【逐行批注】 计算右侧结果并赋给 Path(command[3]).write_text(json.dumps(value…，更新脚本状态。
        Path(command[3]).write_text(json.dumps(value), encoding='utf-8')
        # 【逐行批注】 结束当前函数并返回结果：return subprocess.CompletedProcess(command, 0, b'', b'')。
        return subprocess.CompletedProcess(command, 0, b'', b'')

    # 【逐行批注】 定义函数 invoke，封装可复用的处理步骤。
    def invoke(self, config=None, case=None):
        # 【逐行批注】 结束当前函数并返回结果：return provider.call_api('', {'config': config or self.config}, {'var…。
        return provider.call_api('', {'config': config or self.config}, {'vars': {'case': case or self.case}})

    # 【逐行批注】 定义函数 test_live_refused_before_credentials_and_subprocess，封装可复用的处理步骤。
    def test_live_refused_before_credentials_and_subprocess(self):
        # 【逐行批注】 计算右侧结果并赋给 config，更新脚本状态。
        config = dict(self.config, mode='live')
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider, 'configured_environment', side_effect=AssertionErro…。
        with patch.object(provider, 'configured_environment', side_effect=AssertionError('禁止读取凭据')) as credential, \
                patch.object(provider.subprocess, 'run', side_effect=AssertionError('禁止启动HTTP')) as request:
            # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
            with self.assertRaises(PermissionError):
                # 【逐行批注】 执行当前 Python 语句：self.invoke(config)。
                self.invoke(config)
            # 【逐行批注】 执行当前 Python 语句：credential.assert_not_called()。
            credential.assert_not_called()
            # 【逐行批注】 执行当前 Python 语句：request.assert_not_called()。
            request.assert_not_called()
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse((self.root/'records').exists())。
        self.assertFalse((self.root/'records').exists())
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(PermissionError):。
        with self.assertRaises(PermissionError):
            # 【逐行批注】 执行当前 Python 语句：provider.configured_environment()。
            provider.configured_environment()

    # 【逐行批注】 定义函数 test_incomplete_baseline_stops_before_candidate，封装可复用的处理步骤。
    def test_incomplete_baseline_stops_before_candidate(self):
        # 【逐行批注】 计算右侧结果并赋给 cases，更新脚本状态。
        cases = self.root/'cases.json'
        # 【逐行批注】 计算右侧结果并赋给 cases.write_text(json.dumps([self.case]), en…，更新脚本状态。
        cases.write_text(json.dumps([self.case]), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 manifest，更新脚本状态。
        manifest = self.root/'manifest.json'
        # 【逐行批注】 调用 str 完成当前脚本或测试步骤：manifest.write_text(json.dumps({'mode': 'fixture', 'casesFile': str(c…。
        manifest.write_text(json.dumps({'mode': 'fixture', 'casesFile': str(cases),
            # 【逐行批注】 计算右侧结果并赋给 'entries': [{'variant': 'baseline'}, {'varia…，更新脚本状态。
            'entries': [{'variant': 'baseline'}, {'variant': 'candidate'}]}), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 incomplete，更新脚本状态。
        incomplete = {'output': json.dumps({'complete': False, 'turns': [], 'usage': []}),
            # 【逐行批注】 执行当前 Python 语句：'budgetStatus': {'limitTokens': 1000000, 'usedTokens': 839528, 'reservedTokens': 72075}}。
            'budgetStatus': {'limitTokens': 1000000, 'usedTokens': 839528, 'reservedTokens': 72075}}
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(compare, 'call_api', return_value=incomplete) as request, \。
        with patch.object(compare, 'call_api', return_value=incomplete) as request, \
                patch.object(sys, 'argv', ['evaluation_compare.py', str(manifest), '--output', str(self.root/'incomplete')]):
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 1)。
            self.assertEqual(compare.main(), 1)
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(request.call_count, 1)。
            self.assertEqual(request.call_count, 1)

    # 【逐行批注】 定义函数 test_native_protocol_zero_limit_and_no_credentials，封装可复用的处理步骤。
    def test_native_protocol_zero_limit_and_no_credentials(self):
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider, 'configured_environment', side_effect=AssertionErro…。
        with patch.object(provider, 'configured_environment', side_effect=AssertionError('离线不得读取凭据')), \
                patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
            value = json.loads(self.invoke()['output'])
        # 【逐行批注】 计算右侧结果并赋给 case, env，更新脚本状态。
        case, env = self.launches[0]
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(case['turns'][0]['type'], 'teaching_request')。
        self.assertEqual(case['turns'][0]['type'], 'teaching_request')
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(env['GANGYI_EVAL_MAX_CALLS'], '0')。
        self.assertEqual(env['GANGYI_EVAL_MAX_CALLS'], '0')
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(value['summary']['taskMetrics'][0]['completionMs'], None)。
        self.assertEqual(value['summary']['taskMetrics'][0]['completionMs'], None)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(value['httpRequests'])。
        self.assertFalse(value['httpRequests'])
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):。
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            # 【逐行批注】 计算右侧结果并赋给 self.invoke(dict(self.config, mode，更新脚本状态。
            self.invoke(dict(self.config, mode='fixture', fixtureUrl='http://127.0.0.1:9/v1', maxCalls=0))
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(self.launches[-1][1]['GANGYI_EVAL_MAX_CALLS'], '0')。
        self.assertEqual(self.launches[-1][1]['GANGYI_EVAL_MAX_CALLS'], '0')

    # 【逐行批注】 定义函数 test_legacy_mapping_requires_explicit_protocol，封装可复用的处理步骤。
    def test_legacy_mapping_requires_explicit_protocol(self):
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):。
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            # 【逐行批注】 计算右侧结果并赋给 self.invoke(dict(self.config, entryProtocol，更新脚本状态。
            self.invoke(dict(self.config, entryProtocol='legacy-chat', sourceCommit='legacy-before-native'))
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(self.launches[0][0]['turns'][0]['type'], 'chat')。
        self.assertEqual(self.launches[0][0]['turns'][0]['type'], 'chat')
        # 【逐行批注】 执行当前 Python 语句：self.assertNotIn('requestType', self.launches[0][0]['turns'][0])。
        self.assertNotIn('requestType', self.launches[0][0]['turns'][0])
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(self.case['turns'][0]['type'], 'teaching_request')。
        self.assertEqual(self.case['turns'][0]['type'], 'teaching_request')
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(ValueError):。
        with self.assertRaises(ValueError):
            # 【逐行批注】 计算右侧结果并赋给 self.invoke(dict(self.config, entryProtocol，更新脚本状态。
            self.invoke(dict(self.config, entryProtocol='legacy-chat'))

    # 【逐行批注】 定义函数 test_versioned_reuse_and_failed_records，封装可复用的处理步骤。
    def test_versioned_reuse_and_failed_records(self):
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):。
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
            first = json.loads(self.invoke()['output'])
            # 【逐行批注】 计算右侧结果并赋给 config，更新脚本状态。
            config = dict(self.config, reuseDir=self.config['outputDir'])
            # 【逐行批注】 执行当前 Python 语句：self.assertTrue(json.loads(self.invoke(config)['output'])['baselineReused'])。
            self.assertTrue(json.loads(self.invoke(config)['output'])['baselineReused'])
            # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 1)。
            self.assertEqual(len(self.launches), 1)
            # 【逐行批注】 计算右侧结果并赋给 self.invoke(dict(config, sourceCommit，更新脚本状态。
            self.invoke(dict(config, sourceCommit='different-version'))
            # 【逐行批注】 计算右侧结果并赋给 self.invoke(config, dict(self.case, preferen…，更新脚本状态。
            self.invoke(config, dict(self.case, preferences={'style': '先定义'}))
            # 【逐行批注】 执行当前 Python 语句：self.driver.write_bytes(b'isolated-version-two')。
            self.driver.write_bytes(b'isolated-version-two')
            # 【逐行批注】 执行当前 Python 语句：self.invoke(config)。
            self.invoke(config)
            # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 4)。
            self.assertEqual(len(self.launches), 4)
            # 【逐行批注】 执行当前 Python 语句：self.driver.write_bytes(b'isolated-version-one')。
            self.driver.write_bytes(b'isolated-version-one')
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = Path(first['recordPath'])
            # 【逐行批注】 计算右侧结果并赋给 first['turns'][0]['task']['status']，更新脚本状态。
            first['turns'][0]['task']['status'] = 'failed'
            # 【逐行批注】 计算右侧结果并赋给 path.write_text(json.dumps(first), encoding，更新脚本状态。
            path.write_text(json.dumps(first), encoding='utf-8')
            # 删除已复用的副本，确认失败原件不能作为通过记录。
            # 【逐行批注】 遍历数据并重复执行循环体：for other in Path(config['reuseDir']).glob('* /result.json'):。
            for other in Path(config['reuseDir']).glob('*/result.json'):
                # 【逐行批注】 判断条件 other != path:，成立时执行对应分支。
                if other != path:
                    # 【逐行批注】 计算右侧结果并赋给 saved，更新脚本状态。
                    saved = json.loads(other.read_text(encoding='utf-8'))
                    # 【逐行批注】 计算右侧结果并赋给 saved['complete']，更新脚本状态。
                    saved['complete'] = False
                    # 【逐行批注】 计算右侧结果并赋给 other.write_text(json.dumps(saved), encoding，更新脚本状态。
                    other.write_text(json.dumps(saved), encoding='utf-8')
            # 【逐行批注】 执行当前 Python 语句：self.invoke(config)。
            self.invoke(config)
            # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 5)。
            self.assertEqual(len(self.launches), 5)

    # 【逐行批注】 定义函数 test_budget_and_frozen_directories_are_protected，封装可复用的处理步骤。
    def test_budget_and_frozen_directories_are_protected(self):
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(ValueError):。
        with self.assertRaises(ValueError):
            # 【逐行批注】 执行当前 Python 语句：guard.isolated_output(guard.FROZEN/'new-output')。
            guard.isolated_output(guard.FROZEN/'new-output')
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(ValueError):。
        with self.assertRaises(ValueError):
            # 【逐行批注】 执行当前 Python 语句：guard.isolated_output(guard.decision()['originalBudgetPath'])。
            guard.isolated_output(guard.decision()['originalBudgetPath'])
        # 【逐行批注】 计算右侧结果并赋给 self.budget.write_text(json.dumps({'limitTok…，更新脚本状态。
        self.budget.write_text(json.dumps({'limitTokens': 5000000, 'usedTokens': 839528, 'reservedTokens': 72075}), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 self.original，更新脚本状态。
        self.original = self.budget.read_bytes()
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = guard.budget_copy(self.root/'copy.json', self.budget)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(json.loads(result.read_text())['limitTokens'], 1000000)。
        self.assertEqual(json.loads(result.read_text())['limitTokens'], 1000000)
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(FileExistsError):。
        with self.assertRaises(FileExistsError):
            # 【逐行批注】 执行当前 Python 语句：guard.budget_copy(result, self.budget)。
            guard.budget_copy(result, self.budget)

    # 【逐行批注】 定义函数 test_http_failures_counted_and_task_metrics_not_summed，封装可复用的处理步骤。
    def test_http_failures_counted_and_task_metrics_not_summed(self):
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = {'httpRequests': [{'status': 'failed', 'tokens': {'total_tokens': 14182}, 'durationMs': 20, 'queueMs': 30},
                                  # 【逐行批注】 执行当前 Python 语句：{'status': 'completed', 'tokens': None, 'queueMs': 30}],。
                                  {'status': 'completed', 'tokens': None, 'queueMs': 30}],
                 # 【逐行批注】 执行当前 Python 语句：'usage': [{'tokenUsage': {'total_tokens': 1}}],。
                 'usage': [{'tokenUsage': {'total_tokens': 1}}],
                 # 【逐行批注】 执行当前 Python 语句：'turns': [{'task': {'id': 'same', 'queueMs': 30, 'completionMs': 80}},。
                 'turns': [{'task': {'id': 'same', 'queueMs': 30, 'completionMs': 80}},
                           # 【逐行批注】 执行当前 Python 语句：{'task': {'id': 'same', 'queueMs': 30, 'completionMs': 80}}]}。
                           {'task': {'id': 'same', 'queueMs': 30, 'completionMs': 80}}]}
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = guard.summarize(value)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['knownTokens'], 14182)。
        self.assertEqual(result['knownTokens'], 14182)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['unknownUsageRequests'], 1)。
        self.assertEqual(result['unknownUsageRequests'], 1)
        # 【逐行批注】 执行当前 Python 语句：self.assertIsNone(result['totalTokens'])。
        self.assertIsNone(result['totalTokens'])
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['failedHttpRequests'], 1)。
        self.assertEqual(result['failedHttpRequests'], 1)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(result['taskMetrics']), 1)。
        self.assertEqual(len(result['taskMetrics']), 1)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['taskMetrics'][0]['queueMs'], 30)。
        self.assertEqual(result['taskMetrics'][0]['queueMs'], 30)
        # 【逐行批注】 执行当前 Python 语句：self.assertIsNone(result['taskMetrics'][0]['firstContentMs'])。
        self.assertIsNone(result['taskMetrics'][0]['firstContentMs'])
        # 【逐行批注】 执行当前 Python 语句：self.assertIsNone(guard.summarize({})['totalTokens'])。
        self.assertIsNone(guard.summarize({})['totalTokens'])

    # 【逐行批注】 定义函数 test_nonlocal_fixture_rejected，封装可复用的处理步骤。
    def test_nonlocal_fixture_rejected(self):
        # 【逐行批注】 遍历数据并重复执行循环体：for url in ('https://api.example.org/v1', 'http://127.0.0.1:9@example.org/…。
        for url in ('https://api.example.org/v1', 'http://127.0.0.1:9@example.org/v1'):
            # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(ValueError):。
            with self.assertRaises(ValueError):
                # 【逐行批注】 执行当前 Python 语句：guard.local_fixture_url(url)。
                guard.local_fixture_url(url)

    # 【逐行批注】 定义函数 test_compare_keeps_one_cumulative_isolated_budget，封装可复用的处理步骤。
    def test_compare_keeps_one_cumulative_isolated_budget(self):
        # 【逐行批注】 计算右侧结果并赋给 cases_file, manifest_file，更新脚本状态。
        cases_file, manifest_file = self.root/'cases.json', self.root/'manifest.json'
        # 【逐行批注】 计算右侧结果并赋给 cases_file.write_text(json.dumps([self.case]…，更新脚本状态。
        cases_file.write_text(json.dumps([self.case]), encoding='utf-8')
        # 【逐行批注】 调用 str 完成当前脚本或测试步骤：manifest_file.write_text(json.dumps({'mode': 'preflight', 'casesFile'…。
        manifest_file.write_text(json.dumps({'mode': 'preflight', 'casesFile': str(cases_file),
            # 【逐行批注】 计算右侧结果并赋给 'shared': {'budgetFile': str(self.budget)}, …，更新脚本状态。
            'shared': {'budgetFile': str(self.budget)}, 'entries': [{'variant': 'baseline'}, {'variant': 'candidate'}]}), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 starting，更新脚本状态。
        starting = []
        # 【逐行批注】 定义函数 fake_call，封装可复用的处理步骤。
        def fake_call(prompt, options, context):
            # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
            value = json.loads(Path(options['config']['budgetFile']).read_text(encoding='utf-8'))
            # 【逐行批注】 执行当前 Python 语句：starting.append(value['usedTokens'])。
            starting.append(value['usedTokens'])
            # 【逐行批注】 计算右侧结果并赋给 value['usedTokens'] +，更新脚本状态。
            value['usedTokens'] += 100
            # 【逐行批注】 计算右侧结果并赋给 value['reservedTokens'] +，更新脚本状态。
            value['reservedTokens'] += 50
            # 【逐行批注】 结束当前函数并返回结果：return {'output': json.dumps({'budgetStatus': value, 'complete': Fals…。
            return {'output': json.dumps({'budgetStatus': value, 'complete': False}), 'budgetStatus': value}
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(sys, 'argv', ['evaluation_compare.py', str(manifest_file), '-…。
        with patch.object(sys, 'argv', ['evaluation_compare.py', str(manifest_file), '--output', str(self.root/'pair')]), \
                patch.object(compare, 'call_api', side_effect=fake_call):
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 0)。
            self.assertEqual(compare.main(), 0)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(starting, [839528, 839628])。
        self.assertEqual(starting, [839528, 839628])
        # 【逐行批注】 计算右侧结果并赋给 final，更新脚本状态。
        final = json.loads((self.root/'pair/budget-current.json').read_text())
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(final['usedTokens'], 839728)。
        self.assertEqual(final['usedTokens'], 839728)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(final['reservedTokens'], 72175)。
        self.assertEqual(final['reservedTokens'], 72175)

    # 【逐行批注】 定义函数 test_process_failure_retains_budget_reservation，封装可复用的处理步骤。
    def test_process_failure_retains_budget_reservation(self):
        # 【逐行批注】 定义函数 fail，封装可复用的处理步骤。
        def fail(command, **kwargs):
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            # 【逐行批注】 计算右侧结果并赋给 budget，更新脚本状态。
            budget = json.loads(path.read_text())
            # 【逐行批注】 计算右侧结果并赋给 budget['reservedTokens'] +，更新脚本状态。
            budget['reservedTokens'] += 20000
            # 【逐行批注】 计算右侧结果并赋给 path.write_text(json.dumps(budget), encoding，更新脚本状态。
            path.write_text(json.dumps(budget), encoding='utf-8')
            # 【逐行批注】 结束当前函数并返回结果：return subprocess.CompletedProcess(command, 9, b'', b'PRIVATE-CREDENT…。
            return subprocess.CompletedProcess(command, 9, b'', b'PRIVATE-CREDENTIAL')
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider.subprocess, 'run', side_effect=fail):。
        with patch.object(provider.subprocess, 'run', side_effect=fail):
            # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
            result = self.invoke()
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)。
        self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)
        # 【逐行批注】 执行当前 Python 语句：self.assertNotIn('PRIVATE-CREDENTIAL', json.dumps(result))。
        self.assertNotIn('PRIVATE-CREDENTIAL', json.dumps(result))
        # 【逐行批注】 调用 Path 完成当前脚本或测试步骤：self.assertTrue((Path(result['metadata']['recordPath'])/'run-error.js…。
        self.assertTrue((Path(result['metadata']['recordPath'])/'run-error.json').exists())

    # 【逐行批注】 定义函数 test_reuse_invalidates_model_and_source_snapshot，封装可复用的处理步骤。
    def test_reuse_invalidates_model_and_source_snapshot(self):
        # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
        source = self.root/'saved-source.db'
        # 【逐行批注】 执行当前 Python 语句：source.write_bytes(b'fixed-snapshot-one')。
        source.write_bytes(b'fixed-snapshot-one')
        # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
        case = dict(self.case, libraryDatabase=str(source), librarySourceIds=['fixed-source'])
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):。
        with patch.object(provider.subprocess, 'run', side_effect=self.fake_run):
            # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
            first = json.loads(self.invoke(case=case)['output'])
            # 【逐行批注】 计算右侧结果并赋给 config，更新脚本状态。
            config = dict(self.config, reuseDir=self.config['outputDir'])
            # 【逐行批注】 计算右侧结果并赋给 model，更新脚本状态。
            model = json.loads(self.invoke(dict(config, model='different-model'), case)['output'])
            # 【逐行批注】 执行当前 Python 语句：source.write_bytes(b'fixed-snapshot-two')。
            source.write_bytes(b'fixed-snapshot-two')
            # 【逐行批注】 计算右侧结果并赋给 changed，更新脚本状态。
            changed = json.loads(self.invoke(config, case)['output'])
        # 【逐行批注】 执行当前 Python 语句：self.assertNotEqual(first['reuseKey'], model['reuseKey'])。
        self.assertNotEqual(first['reuseKey'], model['reuseKey'])
        # 【逐行批注】 执行当前 Python 语句：self.assertNotEqual(first['reuseKey'], changed['reuseKey'])。
        self.assertNotEqual(first['reuseKey'], changed['reuseKey'])
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 3)。
        self.assertEqual(len(self.launches), 3)

    # 【逐行批注】 定义函数 compare_arguments，封装可复用的处理步骤。
    def compare_arguments(self, name, case=None):
        # 【逐行批注】 执行当前 Python 语句："""两版共用公开合成用例与隔离预算，允许测试直接经过真实provider收尾。"""。
        """两版共用公开合成用例与隔离预算，允许测试直接经过真实provider收尾。"""
        # 【逐行批注】 计算右侧结果并赋给 cases_file, manifest_file，更新脚本状态。
        cases_file, manifest_file = self.root/(name+'-cases.json'), self.root/(name+'-manifest.json')
        # 【逐行批注】 计算右侧结果并赋给 cases_file.write_text(json.dumps([case or se…，更新脚本状态。
        cases_file.write_text(json.dumps([case or self.case]), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 shared，更新脚本状态。
        shared = {key: value for key, value in self.config.items()
                  # 【逐行批注】 判断条件 key in ('profileName', 'driver', 'sourceCommit', 'budgetFile')}，成立时执行对应分支。
                  if key in ('profileName', 'driver', 'sourceCommit', 'budgetFile')}
        # 【逐行批注】 调用 str 完成当前脚本或测试步骤：manifest_file.write_text(json.dumps({'mode': 'preflight', 'casesFile'…。
        manifest_file.write_text(json.dumps({'mode': 'preflight', 'casesFile': str(cases_file),
            # 【逐行批注】 计算右侧结果并赋给 'shared': shared, 'entries': [{'variant': 'b…，更新脚本状态。
            'shared': shared, 'entries': [{'variant': 'baseline'}, {'variant': 'candidate'}]}), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 output，更新脚本状态。
        output = self.root/name
        # 【逐行批注】 结束当前函数并返回结果：return ['evaluation_compare.py', str(manifest_file), '--output', str(…。
        return ['evaluation_compare.py', str(manifest_file), '--output', str(output)], output

    # 【逐行批注】 定义函数 test_corrupt_result_keeps_reservation_for_next_variant，封装可复用的处理步骤。
    def test_corrupt_result_keeps_reservation_for_next_variant(self):
        # 【逐行批注】 计算右侧结果并赋给 arguments, output，更新脚本状态。
        arguments, output = self.compare_arguments('corrupt-result')
        # 【逐行批注】 计算右侧结果并赋给 starting，更新脚本状态。
        starting = []
        # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
        def execute(command, **kwargs):
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            # 【逐行批注】 计算右侧结果并赋给 budget，更新脚本状态。
            budget = guard.read_budget(path)
            # 【逐行批注】 执行当前 Python 语句：starting.append(budget['reservedTokens'])。
            starting.append(budget['reservedTokens'])
            # 【逐行批注】 计算右侧结果并赋给 budget['reservedTokens'] +，更新脚本状态。
            budget['reservedTokens'] += 20000
            # 【逐行批注】 计算右侧结果并赋给 path.write_text(json.dumps(budget), encoding，更新脚本状态。
            path.write_text(json.dumps(budget), encoding='utf-8')
            # 【逐行批注】 判断条件 len(starting) == 1:，成立时执行对应分支。
            if len(starting) == 1:
                # 【逐行批注】 计算右侧结果并赋给 Path(command[3]).write_text('{broken-json', …，更新脚本状态。
                Path(command[3]).write_text('{broken-json', encoding='utf-8')
                # 【逐行批注】 结束当前函数并返回结果：return subprocess.CompletedProcess(command, 0)。
                return subprocess.CompletedProcess(command, 0)
            # 【逐行批注】 结束当前函数并返回结果：return self.fake_run(command, **kwargs)。
            return self.fake_run(command, **kwargs)
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'r…。
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 1)。
            self.assertEqual(compare.main(), 1)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(starting, [72075, 92075])。
        self.assertEqual(starting, [72075, 92075])
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(guard.read_budget(output/'budget-current.json')['reservedTokens'], 11207…。
        self.assertEqual(guard.read_budget(output/'budget-current.json')['reservedTokens'], 112075)
        # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        # 【逐行批注】 执行当前 Python 语句：self.assertIn('JSONDecodeError', report['results'][0]['result']['error'])。
        self.assertIn('JSONDecodeError', report['results'][0]['result']['error'])

    # 【逐行批注】 定义函数 test_summary_failure_returns_budget_outside_result，封装可复用的处理步骤。
    def test_summary_failure_returns_budget_outside_result(self):
        # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
        def execute(command, **kwargs):
            # 【逐行批注】 执行当前 Python 语句：self.fake_run(command, **kwargs)。
            self.fake_run(command, **kwargs)
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            # 【逐行批注】 计算右侧结果并赋给 budget，更新脚本状态。
            budget = guard.read_budget(path)
            # 【逐行批注】 计算右侧结果并赋给 budget['reservedTokens'] +，更新脚本状态。
            budget['reservedTokens'] += 20000
            # 【逐行批注】 计算右侧结果并赋给 path.write_text(json.dumps(budget), encoding，更新脚本状态。
            path.write_text(json.dumps(budget), encoding='utf-8')
            # 【逐行批注】 结束当前函数并返回结果：return subprocess.CompletedProcess(command, 0)。
            return subprocess.CompletedProcess(command, 0)
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider.subprocess, 'run', side_effect=execute), \。
        with patch.object(provider.subprocess, 'run', side_effect=execute), \
                patch.object(provider, 'summarize', side_effect=ValueError('PRIVATE-CREDENTIAL')):
            # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
            result = self.invoke()
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)。
        self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(result['complete'])。
        self.assertFalse(result['complete'])
        # 【逐行批注】 执行当前 Python 语句：self.assertNotIn('PRIVATE-CREDENTIAL', json.dumps(result))。
        self.assertNotIn('PRIVATE-CREDENTIAL', json.dumps(result))

    # 【逐行批注】 定义函数 test_unreadable_budget_stops_before_next_variant，封装可复用的处理步骤。
    def test_unreadable_budget_stops_before_next_variant(self):
        # 【逐行批注】 计算右侧结果并赋给 arguments, output，更新脚本状态。
        arguments, output = self.compare_arguments('unreadable-budget')
        # 【逐行批注】 计算右侧结果并赋给 calls，更新脚本状态。
        calls = []
        # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
        def execute(command, **kwargs):
            # 【逐行批注】 执行当前 Python 语句：calls.append(command)。
            calls.append(command)
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            # 【逐行批注】 计算右侧结果并赋给 path.write_text('{budget-state-lost', encodi…，更新脚本状态。
            path.write_text('{budget-state-lost', encoding='utf-8')
            # 【逐行批注】 结束当前函数并返回结果：return self.fake_run(command, **kwargs)。
            return self.fake_run(command, **kwargs)
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'r…。
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 1)。
            self.assertEqual(compare.main(), 1)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(calls), 1)。
        self.assertEqual(len(calls), 1)
        # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        # 【逐行批注】 执行当前 Python 语句：self.assertIn('BudgetStateError', report['results'][0]['error'])。
        self.assertIn('BudgetStateError', report['results'][0]['error'])

    # 【逐行批注】 定义函数 test_native_budget_save_failure_rejects_old_valid_budget_and_stops，封装可复用的处理步骤。
    def test_native_budget_save_failure_rejects_old_valid_budget_and_stops(self):
        # 【逐行批注】 计算右侧结果并赋给 arguments, output，更新脚本状态。
        arguments, output = self.compare_arguments('native-save-failure')
        # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
        def execute(command, **kwargs):
            # 【逐行批注】 执行当前 Python 语句：self.fake_run(command, **kwargs)。
            self.fake_run(command, **kwargs)
            # 【逐行批注】 结束当前函数并返回结果：return subprocess.CompletedProcess(command, 73)。
            return subprocess.CompletedProcess(command, 73)
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'r…。
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 1)。
            self.assertEqual(compare.main(), 1)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 1)。
        self.assertEqual(len(self.launches), 1)
        # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        # 【逐行批注】 执行当前 Python 语句：self.assertIn('BudgetStateError', report['results'][0]['error'])。
        self.assertIn('BudgetStateError', report['results'][0]['error'])
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(guard.read_budget(output/'budget-current.json')['reservedTokens'], 72075)。
        self.assertEqual(guard.read_budget(output/'budget-current.json')['reservedTokens'], 72075)

    # 【逐行批注】 定义函数 test_result_and_error_report_write_failures_keep_budget_receipt，封装可复用的处理步骤。
    def test_result_and_error_report_write_failures_keep_budget_receipt(self):
        # 【逐行批注】 计算右侧结果并赋给 write，更新脚本状态。
        write = Path.write_text
        # 【逐行批注】 计算右侧结果并赋给 output_writes，更新脚本状态。
        output_writes = []
        # 【逐行批注】 定义函数 write_file，封装可复用的处理步骤。
        def write_file(path, text, *args, **kwargs):
            # 【逐行批注】 判断条件 path.name == 'result.json':，成立时执行对应分支。
            if path.name == 'result.json':
                # 【逐行批注】 执行当前 Python 语句：output_writes.append(path)。
                output_writes.append(path)
                # 【逐行批注】 判断条件 len(output_writes) > 1:，成立时执行对应分支。
                if len(output_writes) > 1:
                    # 【逐行批注】 在错误情形抛出异常：raise PermissionError('模拟结果后处理不可写')。
                    raise PermissionError('模拟结果后处理不可写')
            # 【逐行批注】 判断条件 path.name == 'run-error.json':，成立时执行对应分支。
            if path.name == 'run-error.json':
                # 【逐行批注】 在错误情形抛出异常：raise PermissionError('模拟错误报告不可写')。
                raise PermissionError('模拟错误报告不可写')
            # 【逐行批注】 结束当前函数并返回结果：return write(path, text, *args, **kwargs)。
            return write(path, text, *args, **kwargs)
        # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
        def execute(command, **kwargs):
            # 【逐行批注】 执行当前 Python 语句：self.fake_run(command, **kwargs)。
            self.fake_run(command, **kwargs)
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = Path(kwargs['env']['GANGYI_EVAL_BUDGET_FILE'])
            # 【逐行批注】 计算右侧结果并赋给 budget，更新脚本状态。
            budget = guard.read_budget(path)
            # 【逐行批注】 计算右侧结果并赋给 budget['reservedTokens'] +，更新脚本状态。
            budget['reservedTokens'] += 20000
            # 【逐行批注】 计算右侧结果并赋给 path.write_text(json.dumps(budget), encoding，更新脚本状态。
            path.write_text(json.dumps(budget), encoding='utf-8')
            # 【逐行批注】 结束当前函数并返回结果：return subprocess.CompletedProcess(command, 0)。
            return subprocess.CompletedProcess(command, 0)
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(provider.subprocess, 'run', side_effect=execute), patch.objec…。
        with patch.object(provider.subprocess, 'run', side_effect=execute), patch.object(Path, 'write_text', new=write_file):
            # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
            result = self.invoke()
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)。
        self.assertEqual(result['budgetStatus']['reservedTokens'], 92075)
        # 【逐行批注】 执行当前 Python 语句：self.assertTrue(result['reportWriteFailed'])。
        self.assertTrue(result['reportWriteFailed'])
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(result['complete'])。
        self.assertFalse(result['complete'])

    # 【逐行批注】 定义函数 test_cumulative_budget_save_failure_stops_before_next_variant，封装可复用的处理步骤。
    def test_cumulative_budget_save_failure_stops_before_next_variant(self):
        # 【逐行批注】 计算右侧结果并赋给 arguments, output，更新脚本状态。
        arguments, output = self.compare_arguments('unwritable-budget')
        # 【逐行批注】 定义函数 fail_persist，封装可复用的处理步骤。
        def fail_persist(path, value):
            # 【逐行批注】 在错误情形抛出异常：raise guard.BudgetStateError('模拟累计预算不可写')。
            raise guard.BudgetStateError('模拟累计预算不可写')
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'r…。
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=self.fake_run), \
                patch.object(compare, 'persist_budget', side_effect=fail_persist):
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 1)。
            self.assertEqual(compare.main(), 1)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 1)。
        self.assertEqual(len(self.launches), 1)
        # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        # 【逐行批注】 执行当前 Python 语句：self.assertIn('BudgetStateError', report['results'][0]['error'])。
        self.assertIn('BudgetStateError', report['results'][0]['error'])

    # 【逐行批注】 定义函数 test_persist_budget_rejects_directory_target，封装可复用的处理步骤。
    def test_persist_budget_rejects_directory_target(self):
        # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
        target = self.root/'budget-directory'
        # 【逐行批注】 执行当前 Python 语句：target.mkdir()。
        target.mkdir()
        # 【逐行批注】 计算右侧结果并赋给 (target/'occupied').write_text('不能被文件替换', en…，更新脚本状态。
        (target/'occupied').write_text('不能被文件替换', encoding='utf-8')
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(guard.BudgetStateError):。
        with self.assertRaises(guard.BudgetStateError):
            # 【逐行批注】 执行当前 Python 语句：guard.persist_budget(target, {'limitTokens': 1000000, 'usedTokens': 0, 'reservedTokens': …。
            guard.persist_budget(target, {'limitTokens': 1000000, 'usedTokens': 0, 'reservedTokens': 0})

    # 【逐行批注】 定义函数 test_new_committed_wal_stops_next_variant_without_main_db_change，封装可复用的处理步骤。
    def test_new_committed_wal_stops_next_variant_without_main_db_change(self):
        # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
        source = self.root/'wal-source.db'
        # 【逐行批注】 在受控上下文中执行资源操作：with sqlite3.connect(source) as db:。
        with sqlite3.connect(source) as db:
            # 【逐行批注】 计算右侧结果并赋给 db.execute('PRAGMA journal_mode，更新脚本状态。
            db.execute('PRAGMA journal_mode=WAL')
            # 【逐行批注】 调用 Source 完成当前脚本或测试步骤：db.execute('CREATE TABLE Source(text TEXT)')。
            db.execute('CREATE TABLE Source(text TEXT)')
            # 【逐行批注】 执行当前 Python 语句：db.commit()。
            db.commit()
        # 【逐行批注】 执行当前 Python 语句：db.close()。
        db.close()
        # 【逐行批注】 调用 Path、str 完成当前脚本或测试步骤：self.assertFalse(Path(str(source)+'-wal').exists())。
        self.assertFalse(Path(str(source)+'-wal').exists())
        # 【逐行批注】 计算右侧结果并赋给 before，更新脚本状态。
        before = guard.file_hash(source)
        # 【逐行批注】 计算右侧结果并赋给 arguments, output，更新脚本状态。
        arguments, output = self.compare_arguments('new-wal', dict(self.case, libraryDatabase=str(source)))
        # 【逐行批注】 计算右侧结果并赋给 connections，更新脚本状态。
        connections = []
        # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
        def execute(command, **kwargs):
            # 【逐行批注】 计算右侧结果并赋给 db，更新脚本状态。
            db = sqlite3.connect(source)
            # 【逐行批注】 执行当前 Python 语句：connections.append(db)。
            connections.append(db)
            # 【逐行批注】 调用 VALUES 完成当前脚本或测试步骤：db.execute("INSERT INTO Source VALUES('运行中提交的新原文')")。
            db.execute("INSERT INTO Source VALUES('运行中提交的新原文')")
            # 【逐行批注】 执行当前 Python 语句：db.commit()。
            db.commit()
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(guard.file_hash(source), before)。
            self.assertEqual(guard.file_hash(source), before)
            # 【逐行批注】 调用 Path、str 完成当前脚本或测试步骤：self.assertTrue(Path(str(source)+'-wal').exists())。
            self.assertTrue(Path(str(source)+'-wal').exists())
            # 【逐行批注】 结束当前函数并返回结果：return self.fake_run(command, **kwargs)。
            return self.fake_run(command, **kwargs)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'r…。
            with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
                # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 1)。
                self.assertEqual(compare.main(), 1)
            # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 1)。
            self.assertEqual(len(self.launches), 1)
            # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
            report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
            # 【逐行批注】 调用 str、Path 完成当前脚本或测试步骤：self.assertIsNone(report['sourceMaterialHashes'][str(Path(str(source)…。
            self.assertIsNone(report['sourceMaterialHashes'][str(Path(str(source)+'-wal').resolve())])
            # 【逐行批注】 执行当前 Python 语句：self.assertIn('资料快照', report['results'][-1]['error'])。
            self.assertIn('资料快照', report['results'][-1]['error'])
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 遍历数据并重复执行循环体：for connection in connections:。
            for connection in connections:
                # 【逐行批注】 执行当前 Python 语句：connection.close()。
                connection.close()

    # 【逐行批注】 定义函数 test_removed_wal_stops_next_variant，封装可复用的处理步骤。
    def test_removed_wal_stops_next_variant(self):
        # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
        source = self.root/'removed-wal-source.db'
        # 【逐行批注】 执行当前 Python 语句：source.write_bytes(b'unchanged-main-db')。
        source.write_bytes(b'unchanged-main-db')
        # 【逐行批注】 计算右侧结果并赋给 wal，更新脚本状态。
        wal = Path(str(source)+'-wal')
        # 【逐行批注】 执行当前 Python 语句：wal.write_bytes(b'initial-sidecar')。
        wal.write_bytes(b'initial-sidecar')
        # 【逐行批注】 计算右侧结果并赋给 arguments, output，更新脚本状态。
        arguments, output = self.compare_arguments('removed-wal', dict(self.case, libraryDatabase=str(source)))
        # 【逐行批注】 定义函数 execute，封装可复用的处理步骤。
        def execute(command, **kwargs):
            # 【逐行批注】 执行当前 Python 语句：wal.unlink()。
            wal.unlink()
            # 【逐行批注】 结束当前函数并返回结果：return self.fake_run(command, **kwargs)。
            return self.fake_run(command, **kwargs)
        # 【逐行批注】 在受控上下文中执行资源操作：with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'r…。
        with patch.object(sys, 'argv', arguments), patch.object(provider.subprocess, 'run', side_effect=execute):
            # 【逐行批注】 执行当前 Python 语句：self.assertEqual(compare.main(), 1)。
            self.assertEqual(compare.main(), 1)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(self.launches), 1)。
        self.assertEqual(len(self.launches), 1)
        # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
        report = json.loads((output/'summary.json').read_text(encoding='utf-8'))
        # 【逐行批注】 执行当前 Python 语句：self.assertIn('资料快照', report['results'][-1]['error'])。
        self.assertIn('资料快照', report['results'][-1]['error'])


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 执行当前 Python 语句：unittest.main()。
    unittest.main()
