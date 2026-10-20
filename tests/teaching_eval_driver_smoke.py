"""用隔离HTTP夹具验证验收入口真实请求统计，含用量选项兼容重试。"""
import io
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from http.server import ThreadingHTTPServer
from threading import Thread
from classroom_v560_fixture import FixtureAI


class UsageCompatibilityFixture(FixtureAI):
    posts = 0
    rejected_usage = 0
    block_budget_temporary = None

    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        request = json.loads(raw)
        type(self).posts += 1
        if type(self).block_budget_temporary is not None:
            type(self).block_budget_temporary.mkdir(exist_ok=True)
        if 'stream_options' in request:
            type(self).rejected_usage += 1
            response = b'{"error":{"message":"unsupported stream_options include_usage"}}'
            self.send_response(400); self.send_header('Content-Type','application/json')
            self.send_header('Content-Length',str(len(response))); self.end_headers(); self.wfile.write(response)
            return
        self.rfile = io.BytesIO(raw)
        super().do_POST()


def authorization_checks(driver):
    """只调用原生校验入口，合法与拒绝路径均不建立 HTTP 连接。"""
    with tempfile.TemporaryDirectory(prefix='native-authorization-') as folder:
        root = Path(folder)
        state = {'limitTokens': 1000000, 'usedTokens': 839528, 'reservedTokens': 72075}
        original, active, approval = root/'original.json', root/'active.json', root/'decision.json'
        original.write_text(json.dumps(state), encoding='utf-8')
        active.write_text(json.dumps(state), encoding='utf-8')
        decision = {'newPaidCallsAllowedThisRound': True, 'limitTokens': 1000000,
            'knownUsedTokens': 839528, 'unknownReservedTokens': 72075, 'originalBudgetPath': str(original),
            'originalBudgetSha256': hashlib.sha256(original.read_bytes()).hexdigest(),
            'activeBudgetPath': str(active), 'baseUrl': 'https://frozen.example.invalid/v1', 'model': 'fixture-model'}
        approval.write_text(json.dumps(decision), encoding='utf-8')
        env = dict(os.environ, GANGYI_EVAL_MODE='live', GANGYI_EVAL_DECISION_FILE=str(approval),
                   GANGYI_EVAL_BUDGET_FILE=str(active), AI_BASE_URL=decision['baseUrl'], AI_MODEL=decision['model'])

        def check(expected, **overrides):
            process = subprocess.run([str(Path(driver).resolve()), '--check-live-authorization'],
                env=dict(env, **overrides), capture_output=True, timeout=10)
            assert (process.returncode == 0) == expected, process.stderr.decode('utf-8', errors='replace')

        check(True)
        blocked = subprocess.run([str(Path(driver).resolve()), '--check-live-search-block'], env=env, capture_output=True, timeout=10)
        assert blocked.returncode == 0, blocked.stderr.decode('utf-8', errors='replace')
        check(False, AI_MODEL='different')
        check(False, AI_BASE_URL='https://different.example.invalid/v1')
        check(False, GANGYI_EVAL_DECISION_FILE='')
        check(False, GANGYI_EVAL_MODE='')
        for field in ('usedTokens', 'reservedTokens'):
            active.write_text(json.dumps(dict(state, **{field: 0})), encoding='utf-8')
            check(False)
        active.write_text(json.dumps(state), encoding='utf-8')
        original.write_text('{}', encoding='utf-8')
        check(False)


def main(driver):
    authorization_checks(driver)
    repo = Path(__file__).parents[1]
    with tempfile.TemporaryDirectory(prefix='gangyi-eval-usage-') as directory:
        root = Path(directory)
        case = {'id':'usage-accounting','title':'HTTP统计测试','concept':'比较化合价前后变化。',
                'questions':[{'question':'Fe变成Fe²⁺，化合价如何变化？','type':'open','expectedAnswer':'升高'}],
                'turns':[{'type':'question_answer','sectionId':'question-0','text':'由0升高到+2。'}]}
        (root/'case.json').write_text(json.dumps(case,ensure_ascii=False),encoding='utf-8')
        server = ThreadingHTTPServer(('127.0.0.1',0),UsageCompatibilityFixture)
        Thread(target=server.serve_forever,daemon=True).start()
        env = dict(os.environ,AI_BASE_URL=f'http://127.0.0.1:{server.server_port}/v1',
                   AI_API_KEY='isolated-fixture',AI_MODEL='fixture-v560')
        budget_file=root/'budget.json'
        budget_file.write_text(json.dumps({'limitTokens':1000000,'usedTokens':0,'reservedTokens':0}),encoding='utf-8')
        env['GANGYI_EVAL_BUDGET_FILE']=str(budget_file)
        try:
            process = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'isolated.db'),str(root/'result.json')],
                env=env,cwd=repo,capture_output=True,timeout=90,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            assert process.returncode == 0,process.stderr.decode('utf-8',errors='replace')
            result = json.loads((root/'result.json').read_text(encoding='utf-8'))
            initial_compatibility_retries = UsageCompatibilityFixture.rejected_usage
            assert result['turns'][0]['task']['status']=='waiting_student',result['turns'][0]['task'].get('failure')
            assert UsageCompatibilityFixture.rejected_usage>0
            assert result['httpRequestCount']==UsageCompatibilityFixture.posts
            assert result['httpRequestCount']>len(result['usage'])
            assert sum(request['status']=='failed' for request in result['httpRequests'])==UsageCompatibilityFixture.rejected_usage
            assert all(request['status'] in ('failed','completed') for request in result['httpRequests'])
            assert all(request['tokens'] is None for request in result['httpRequests'])
            assert all('PRIVATE-' not in json.dumps(request) for request in result['httpRequests'])
            budget=json.loads(budget_file.read_text(encoding='utf-8'))
            assert budget['usedTokens']==0 and budget['reservedTokens']>0 and budget['unknownUsageRequests']==result['httpRequestCount']
            budget_file.write_text(json.dumps({'limitTokens':1,'usedTokens':0,'reservedTokens':0}),encoding='utf-8')
            calls_before=UsageCompatibilityFixture.posts
            stopped=subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'budget-stopped.db'),str(root/'budget-stopped.json')],
                env=env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            assert stopped.returncode==0,stopped.stderr.decode('utf-8',errors='replace')
            denied=json.loads((root/'budget-stopped.json').read_text(encoding='utf-8'))
            assert denied['turns'][0]['task']['failure']['kind']=='eval_budget'
            assert UsageCompatibilityFixture.posts==calls_before
            # 零调用预检必须真的进入失败轮恢复，不能返回原来的失败记录而漏采提示。
            recovery_env=dict(env,GANGYI_EVAL_RESUME='1',GANGYI_EVAL_MAX_CALLS='0')
            preview=subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'budget-stopped.db'),str(root/'recovery-preview.json')],
                env=recovery_env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            assert preview.returncode==0,preview.stderr.decode('utf-8',errors='replace')
            captured=json.loads((root/'recovery-preview.json').read_text(encoding='utf-8'))
            assert captured['promptAudit'] and captured['turns'][0]['task']['failure']['kind']=='eval_call_limit'
            assert UsageCompatibilityFixture.posts==calls_before
            budget_file.write_text(json.dumps({'limitTokens':1000000,'usedTokens':0,'reservedTokens':0}),encoding='utf-8')
            recovery_env.pop('GANGYI_EVAL_MAX_CALLS')
            recovered=subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'budget-stopped.db'),str(root/'recovered.json')],
                env=recovery_env,cwd=repo,capture_output=True,timeout=90,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            assert recovered.returncode==0,recovered.stderr.decode('utf-8',errors='replace')
            resumed=json.loads((root/'recovered.json').read_text(encoding='utf-8'))
            assert resumed['turns'][0]['task']['id']==denied['turns'][0]['task']['id']
            assert resumed['turns'][0]['task']['status']=='waiting_student'
            assert sum(item['kind']=='chat-user' for item in resumed['interactions'])==1
            assert all(isinstance(turn['externalProcessMs'], int) for turn in resumed['turns'])
            assert isinstance(resumed['externalElapsedMs'], int)
            # 直接执行驱动也不得凭外部地址启动本轮付费请求；防止URL用户信息伪装本机地址。
            calls_before = UsageCompatibilityFixture.posts
            remote_env = dict(env, AI_BASE_URL='http://127.0.0.1:9@example.invalid/v1')
            rejected = subprocess.run([str(Path(driver).resolve()), str(root/'case.json'), str(root/'remote-blocked.db'), str(root/'remote-blocked.json')],
                env=remote_env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            assert rejected.returncode == 0, rejected.stderr.decode('utf-8',errors='replace')
            blocked = json.loads((root/'remote-blocked.json').read_text(encoding='utf-8'))
            assert blocked['turns'][0]['task']['failure']['kind'] == 'eval_authorization'
            assert blocked['httpRequestCount'] == 0 and UsageCompatibilityFixture.posts == calls_before
            # 预留保存失败时不发送HTTP，原有效预算逐字节保留；退出码不能伪装成成功结算。
            original_budget = json.dumps({'limitTokens':1000000,'usedTokens':0,'reservedTokens':0}).encode()
            budget_file.write_bytes(original_budget)
            temporary_budget = Path(str(budget_file)+'.tmp')
            temporary_budget.mkdir()
            save_failed = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'save-failed.db'),str(root/'save-failed.json')],
                env=env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            assert save_failed.returncode == 73, save_failed.stderr.decode('utf-8', errors='replace')
            assert UsageCompatibilityFixture.posts == calls_before and budget_file.read_bytes() == original_budget
            temporary_budget.rmdir()
            # 结算写盘失败时保留请求前预留，停止后续调用并返回专用预算失败退出码。
            UsageCompatibilityFixture.block_budget_temporary = temporary_budget
            settle_failed = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'settle-failed.db'),str(root/'settle-failed.json')],
                env=env,cwd=repo,capture_output=True,timeout=90,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            UsageCompatibilityFixture.block_budget_temporary = None
            assert settle_failed.returncode == 73, settle_failed.stderr.decode('utf-8', errors='replace')
            assert UsageCompatibilityFixture.posts-calls_before == 2, '结算失败后不得进入下一逻辑请求'
            saved_reservation = json.loads(budget_file.read_text(encoding='utf-8'))
            assert saved_reservation['reservedTokens'] > 0 and saved_reservation['usedTokens'] == 0
            assert 'httpRequests' not in saved_reservation
            temporary_budget.rmdir()
            # Windows允许读取但拒绝删除共享时，原子替换必须失败；不能先删除旧账本。
            if os.name == 'nt':
                import ctypes
                api = ctypes.WinDLL('kernel32', use_last_error=True)
                api.CreateFileW.argtypes = [ctypes.c_wchar_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p]
                api.CreateFileW.restype = ctypes.c_void_p
                api.CloseHandle.argtypes = [ctypes.c_void_p]
                budget_file.write_bytes(original_budget)
                handle = api.CreateFileW(str(budget_file), 0x80000000, 3, None, 3, 0x80, None)
                assert handle not in (None, ctypes.c_void_p(-1).value)
                calls_before = UsageCompatibilityFixture.posts
                try:
                    commit_failed = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'commit-failed.db'),str(root/'commit-failed.json')],
                        env=env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                    assert commit_failed.returncode == 73, commit_failed.stderr.decode('utf-8', errors='replace')
                    assert UsageCompatibilityFixture.posts == calls_before and budget_file.read_bytes() == original_budget
                finally:
                    api.CloseHandle(handle)
            print(json.dumps({'passed':True,'logicalCalls':len(result['usage']),'httpRequests':result['httpRequestCount'],
                'allFixtureHttpRequests':UsageCompatibilityFixture.posts,
                'compatibilityRetries':initial_compatibility_retries,'totalCompatibilityRetries':UsageCompatibilityFixture.rejected_usage,
                'unknownUsagePreserved':True,'budgetStopsBeforeRequest':True,'directExternalHttpDenied':True,
                'failedTaskResumeVerified':True,'zeroCallRecoveryCapturesPrompt':True,
                'budgetSaveFailureHttpZero':True,'settlementFailureStopsDriver':True,'oldBudgetNeverAcceptedAsNewSettlement':True,
                'paidRequests':0},ensure_ascii=False))
        finally:
            server.shutdown();server.server_close()


if __name__=='__main__':
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])
