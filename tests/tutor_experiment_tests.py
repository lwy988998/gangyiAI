"""评测执行器的拒绝、恢复与本机原生协议回归；不会读取真实凭据。"""
import json
import shutil
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'scripts'))
import tutor_experiment as experiment

DATASET = experiment.ROOT/'tests/fixtures/ai-tutor/candidate-v2.json'


class ExperimentTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='tutor-experiment-')
        self.root = Path(self.temporary.name)
        self.driver = self.root/'fake.exe'
        self.driver.write_bytes(b'not executable')
        Path(str(self.driver)+'.build.json').write_text('{}', encoding='utf-8')
        self.build = patch.object(experiment, 'checked_build', return_value={})
        self.build.start()
        self.addCleanup(self.build.stop)
        self.addCleanup(self.temporary.cleanup)

    def prepare(self, **options):
        folder = self.root/'experiment'
        experiment.prepare(DATASET, self.driver, folder, ['math-intersection-01'], **options)
        return folder

    def test_prepare_never_starts_process_or_reads_credentials(self):
        with patch.object(experiment.subprocess, 'Popen', side_effect=AssertionError('不应启动')):
            folder = self.prepare()
            self.assertFalse(experiment.benchmark.load_json(folder/'authorization-template.json')['newPaidCallsAllowedThisRound'])
            self.assertEqual(experiment.inspect(folder)['paidModelRequests'], 0)

    def test_checked_plan_accepts_relative_root_without_weakening_case_boundary(self):
        folder = self.prepare()
        previous = Path.cwd()
        try:
            os.chdir(folder.parent)
            relative = Path(folder.name)
            self.assertEqual(experiment.checked_plan(relative), experiment.checked_plan(folder))
            manifest = experiment.benchmark.load_json(folder / 'manifest.json')
            manifest['records'][0]['caseFile'] = '../outside.json'
            (folder / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
            with self.assertRaisesRegex(ValueError, '案例路径越界'):
                experiment.checked_plan(relative)
        finally:
            os.chdir(previous)

    def test_offline_import_binds_raw_response_and_accepts_moved_archive_without_old_binary(self):
        folder = self.prepare()
        manifest = experiment.checked_plan(folder)
        record = manifest['records'][0]
        native = experiment.benchmark.load_json(folder / record['caseFile'])
        run = experiment.benchmark.load_json(folder / 'run.json')
        before = experiment.benchmark.load_json(folder / 'budget-original.json')
        after = dict(before, httpRequests=1)
        case_dir = folder / 'runs/000'
        case_dir.mkdir(parents=True)
        result_path = case_dir / 'result.json'
        journal = case_dir / 'result.json.responses.jsonl'
        messages = [{'role': 'user', 'content': '冻结的本机模拟提示'}]
        prompt_hash = experiment.benchmark.digest(messages)
        raw = '{"message":"模型回复原文"}'
        lines = [{'kind': 'chunk', 'promptSha256': prompt_hash, 'text': raw},
                 {'completed': True, 'partial': False, 'promptSha256': prompt_hash,
                  'rawResponse': raw, 'model': 'isolated-fixture'}]
        def write_log():
            journal.write_text(''.join(json.dumps(line, ensure_ascii=False) + '\n' for line in lines), encoding='utf-8')
        write_log()
        result = {'caseId': record['caseId'], 'experimentCaseSha256': experiment.benchmark.digest(native),
                  'rawResponsesPath': str(journal), 'turns': [{'event': dict(native['turns'][0], requestId='eval-turn-0'),
                                                              'task': {'status': 'waiting_student'}}],
                  'httpRequestCount': 1, 'httpRequests': [{}], 'promptAudit': [{'messages': messages}]}
        experiment.atomic_save(result_path, result)
        row = {'caseId': record['caseId'], 'status': 'completed', 'directory': 'runs/000',
               'budgetBefore': before, 'budgetAfter': after,
               'coverage': experiment.validate_result(folder, record, case_dir, result, 'fixture')}
        for name, field in [('result.json', 'resultSha256'), ('result.json.responses.jsonl', 'rawResponsesSha256'),
                            ('stdout.log', 'stdoutSha256'), ('stderr.log', 'stderrSha256')]:
            path = case_dir / name
            if not path.exists(): path.touch()
            row[field] = experiment.file_hash(path)
        run.update(status='completed', mode='fixture', identity={'mode': 'fixture'}, records=[row])
        experiment.atomic_save(folder / 'run.json', run)
        experiment.atomic_save(folder / 'budget-current.json', after)
        self.driver.unlink()
        moved = self.root / 'moved-archive'
        shutil.copytree(folder, moved)
        report = experiment.evaluate_native(moved)
        self.assertEqual((report['sourceKind'], report['programChecksPassed']), ('offline_fixture', 1))
        self.assertFalse(report['releaseEligible'])
        self.assertEqual(report['checks'][0]['rawResponses']['entries'][0]['line'], 2)
        lines[0]['text'] += '已变化'
        write_log()
        with self.assertRaisesRegex(ValueError, '结果损坏'):
            experiment.evaluate_native(folder)
        row['rawResponsesSha256'] = experiment.file_hash(journal)
        experiment.atomic_save(folder / 'run.json', run)
        with self.assertRaisesRegex(ValueError, '分片'):
            experiment.evaluate_native(folder)

    def test_live_without_approval_never_loads_provider_or_starts(self):
        folder = self.prepare()
        with patch.object(experiment.subprocess, 'Popen', side_effect=AssertionError('不应启动')):
            with self.assertRaises(PermissionError):
                experiment.execute(folder, mode='live')
        self.assertEqual(experiment.benchmark.load_json(folder/'run.json')['status'], 'prepared')

    def test_live_cumulative_limit_must_match_bound_configuration(self):
        folder = self.prepare()
        manifest = experiment.checked_plan(folder)
        configuration = self.root / 'configuration.json'
        authorization = self.root / 'authorized.json'
        frozen = {'baseUrl': 'https://api.deepseek.com', 'model': 'deepseek-flash', 'profileName': 'isolated-fixture',
                  'moneyConfigPath': str(self.root / 'budget.json'), 'moneyConfigSha256': 'f' * 64,
                  'moneyCumulativeLimitMicros': 4_000_000}
        experiment.atomic_save(configuration, frozen)
        approved = experiment.benchmark.load_json(folder / 'authorization-template.json')
        approved.update(frozen, newPaidCallsAllowedThisRound=True, approvedBy='隔离测试', approvedAt='fixture',
                        approvalNote='不调用真实服务', configurationPath=str(configuration),
                        configurationSha256=experiment.file_hash(configuration), moneyCumulativeLimitMicros=6_000_000)
        experiment.atomic_save(authorization, approved)
        with patch('ai_money_budget.checked_live_environment', side_effect=AssertionError('不可使用不一致的上限')):
            with self.assertRaisesRegex(PermissionError, '累计上限'):
                experiment.live_environment(folder, manifest, authorization, configuration)
        approved['moneyCumulativeLimitMicros'] = 4_000_000
        experiment.atomic_save(authorization, approved)
        with patch('ai_money_budget.checked_live_environment', side_effect=PermissionError('拦截本机隔离测试')) as inspect_budget:
            with self.assertRaisesRegex(PermissionError, '拦截本机隔离测试'):
                experiment.live_environment(folder, manifest, authorization, configuration)
        self.assertEqual(inspect_budget.call_args.kwargs['cumulative_limit_micros'], 4_000_000)

    def test_fixture_rejects_remote_dns_userinfo_and_redirect_forms(self):
        for url in ('https://127.0.0.1:80/v1','http://localhost:1234/v1', 'http://127.0.0.1:80@example.invalid/v1',
                    'http://example.invalid:1234/v1','http://127.0.0.1/v1','http://127.0.0.1:1234/v1?key=secret'):
            with self.subTest(url=url), self.assertRaises(ValueError):
                experiment.fixture_endpoint(url)

    def test_clean_environment_excludes_credentials_and_proxy(self):
        with patch.dict(os.environ, {'AI_API_KEY':'synthetic', 'OPENAI_API_KEY':'synthetic', 'HTTPS_PROXY':'synthetic'}):
            env = experiment.clean_environment()
        self.assertFalse(any(key.upper() in {'AI_API_KEY','OPENAI_API_KEY','HTTPS_PROXY'} for key in env))

    def test_invalid_limits_and_budget_are_rejected(self):
        for limit in (True, -1, 0, 1.2, 2**63):
            with self.subTest(limit=limit), self.assertRaises(ValueError):
                experiment.prepare(DATASET,self.driver,self.root/'bad',limit_tokens=limit)
        folder = self.prepare()
        state = experiment.benchmark.load_json(folder/'budget-current.json')
        for field, value in [('limitRequests',-1),('reservedRequests',True),('usedTokens',2**63),('httpRequests',31)]:
            experiment.atomic_save(folder/'budget-current.json', dict(state, **{field:value}))
            with self.subTest(field=field), self.assertRaises((ValueError, RuntimeError)):
                experiment.inspect(folder)

    def test_case_material_drift_rejected_before_start(self):
        folder = self.prepare()
        (folder/'cases/000.json').write_text('{}',encoding='utf-8')
        with patch.object(experiment.subprocess,'Popen',side_effect=AssertionError('不应启动')):
            with self.assertRaises(ValueError):experiment.execute(folder)

    def test_duplicate_manifest_identity_rejected(self):
        folder = self.prepare()
        value=experiment.benchmark.load_json(folder/'manifest.json')
        value['records'].append(value['records'][0])
        experiment.atomic_save(folder/'manifest.json',value)
        with self.assertRaises(ValueError):experiment.checked_plan(folder)

    def test_old_running_becomes_uncertain_and_is_never_resent(self):
        folder=self.prepare();run=experiment.benchmark.load_json(folder/'run.json')
        run.update(status='running',mode='preflight',identity={'mode':'preflight','fixtureUrl':None,'authorizationSha256':None,'configurationSha256':None})
        run['records']=[{'caseId':'math-intersection-01','status':'running','directory':'runs/000','pid':999999,
                         'budgetBefore':experiment.benchmark.load_json(folder/'budget-original.json')}]
        experiment.atomic_save(folder/'run.json',run)
        with patch.object(experiment.subprocess,'Popen',side_effect=AssertionError('不应重发')):
            self.assertEqual(experiment.execute(folder)['status'],'uncertain')
            self.assertEqual(experiment.execute(folder)['status'],'uncertain')

    def test_recovery_rejects_budget_reset(self):
        folder=self.prepare();state=experiment.benchmark.load_json(folder/'budget-current.json')
        experiment.atomic_save(folder/'budget-current.json',dict(state,usedTokens=5))
        with self.assertRaises(ValueError):experiment.execute(folder)

    def test_completed_status_without_records_is_rejected(self):
        folder=self.prepare();run=experiment.benchmark.load_json(folder/'run.json')
        run.update(status='completed',mode='fixture')
        experiment.atomic_save(folder/'run.json',run)
        with self.assertRaises(ValueError):experiment.inspect(folder)

    def test_pid_save_failure_stops_own_child_and_keeps_uncertain(self):
        folder=self.prepare();real_save=experiment.atomic_save;real_popen=subprocess.Popen;children=[]
        def launch(*args,**kwargs):
            child=real_popen([sys.executable,'-B','-c','import time;time.sleep(30)'],
                             stdout=kwargs['stdout'],stderr=kwargs['stderr'],env=kwargs['env'])
            children.append(child);return child
        def save(path,value):
            rows=value.get('records',[]) if isinstance(value,dict) else []
            if rows and rows[-1].get('pid') and rows[-1]['status']=='running':raise OSError('模拟PID保存失败')
            real_save(path,value)
        with patch.object(experiment.subprocess,'Popen',side_effect=launch),patch.object(experiment,'atomic_save',side_effect=save):
            status=experiment.execute(folder)
        self.assertEqual(status['status'],'uncertain')
        self.assertTrue(children and children[0].poll() is not None)

    def test_os_lock_prevents_second_process(self):
        folder=self.prepare()
        code="import sys;sys.path.insert(0,sys.argv[1]);import tutor_experiment as e\nwith e.experiment_lock(sys.argv[2]):pass"
        with experiment.experiment_lock(folder):
            process=subprocess.run([sys.executable,'-B','-c',code,str(experiment.ROOT/'scripts'),str(folder)],
                                   capture_output=True,timeout=15)
        self.assertNotEqual(process.returncode,0)
        self.assertIn('RuntimeError',process.stderr.decode(errors='replace'))


if __name__=='__main__':
    unittest.main()
