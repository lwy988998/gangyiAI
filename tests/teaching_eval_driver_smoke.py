"""用隔离HTTP夹具验证验收入口真实请求统计，含用量选项兼容重试。"""
import io
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

    def do_POST(self):
        raw = self.rfile.read(int(self.headers['Content-Length']))
        request = json.loads(raw)
        type(self).posts += 1
        if 'stream_options' in request:
            type(self).rejected_usage += 1
            response = b'{"error":{"message":"unsupported stream_options include_usage"}}'
            self.send_response(400); self.send_header('Content-Type','application/json')
            self.send_header('Content-Length',str(len(response))); self.end_headers(); self.wfile.write(response)
            return
        self.rfile = io.BytesIO(raw)
        super().do_POST()


def main(driver):
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
            print(json.dumps({'passed':True,'logicalCalls':len(result['usage']),'httpRequests':result['httpRequestCount'],
                'compatibilityRetries':initial_compatibility_retries,'totalCompatibilityRetries':UsageCompatibilityFixture.rejected_usage,
                'unknownUsagePreserved':True,'budgetStopsBeforeRequest':True,'directExternalHttpDenied':True,
                'failedTaskResumeVerified':True,'zeroCallRecoveryCapturesPrompt':True,
                'paidRequests':0},ensure_ascii=False))
        finally:
            server.shutdown();server.server_close()


if __name__=='__main__':
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    main(sys.argv[1])
