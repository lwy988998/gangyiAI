# 【逐行批注】 执行当前 Python 语句："""用隔离HTTP夹具验证验收入口真实请求统计，含用量选项兼容重试。"""。
"""用隔离HTTP夹具验证验收入口真实请求统计，含用量选项兼容重试。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import io。
import io
# 【逐行批注】 导入后续逻辑需要的模块或符号：import hashlib。
import hashlib
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：from http.server import ThreadingHTTPServer。
from http.server import ThreadingHTTPServer
# 【逐行批注】 导入后续逻辑需要的模块或符号：from threading import Thread。
from threading import Thread
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import FixtureAI。
from classroom_v560_fixture import FixtureAI


# 【逐行批注】 定义类 UsageCompatibilityFixture，组织相关数据与行为。
class UsageCompatibilityFixture(FixtureAI):
    # 【逐行批注】 计算右侧结果并赋给 posts，更新脚本状态。
    posts = 0
    # 【逐行批注】 计算右侧结果并赋给 rejected_usage，更新脚本状态。
    rejected_usage = 0
    # 【逐行批注】 计算右侧结果并赋给 block_budget_temporary，更新脚本状态。
    block_budget_temporary = None

    # 【逐行批注】 定义函数 do_POST，封装可复用的处理步骤。
    def do_POST(self):
        # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
        raw = self.rfile.read(int(self.headers['Content-Length']))
        # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
        request = json.loads(raw)
        # 【逐行批注】 计算右侧结果并赋给 type(self).posts +，更新脚本状态。
        type(self).posts += 1
        # 【逐行批注】 判断条件 type(self).block_budget_temporary is not None:，成立时执行对应分支。
        if type(self).block_budget_temporary is not None:
            # 【逐行批注】 计算右侧结果并赋给 type(self).block_budget_temporary.mkdir(exis…，更新脚本状态。
            type(self).block_budget_temporary.mkdir(exist_ok=True)
        # 【逐行批注】 判断条件 'stream_options' in request:，成立时执行对应分支。
        if 'stream_options' in request:
            # 【逐行批注】 计算右侧结果并赋给 type(self).rejected_usage +，更新脚本状态。
            type(self).rejected_usage += 1
            # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
            response = b'{"error":{"message":"unsupported stream_options include_usage"}}'
            # 【逐行批注】 执行当前 Python 语句：self.send_response(400); self.send_header('Content-Type','application/json')。
            self.send_response(400); self.send_header('Content-Type','application/json')
            # 【逐行批注】 调用 str、len 完成当前脚本或测试步骤：self.send_header('Content-Length',str(len(response))); self.end_heade…。
            self.send_header('Content-Length',str(len(response))); self.end_headers(); self.wfile.write(response)
            # 【逐行批注】 结束当前函数并返回结果：return。
            return
        # 【逐行批注】 计算右侧结果并赋给 self.rfile，更新脚本状态。
        self.rfile = io.BytesIO(raw)
        # 【逐行批注】 调用 super 完成当前脚本或测试步骤：super().do_POST()。
        super().do_POST()


# 【逐行批注】 定义函数 authorization_checks，封装可复用的处理步骤。
def authorization_checks(driver):
    # 【逐行批注】 执行当前 Python 语句："""只调用原生校验入口，合法与拒绝路径均不建立 HTTP 连接。"""。
    """只调用原生校验入口，合法与拒绝路径均不建立 HTTP 连接。"""
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='native-authorization-') as folder:。
    with tempfile.TemporaryDirectory(prefix='native-authorization-') as folder:
        # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
        root = Path(folder)
        # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
        state = {'limitTokens': 1000000, 'usedTokens': 839528, 'reservedTokens': 72075}
        # 【逐行批注】 计算右侧结果并赋给 original, active, approval，更新脚本状态。
        original, active, approval = root/'original.json', root/'active.json', root/'decision.json'
        # 【逐行批注】 计算右侧结果并赋给 original.write_text(json.dumps(state), encod…，更新脚本状态。
        original.write_text(json.dumps(state), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 active.write_text(json.dumps(state), encoding，更新脚本状态。
        active.write_text(json.dumps(state), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 decision，更新脚本状态。
        decision = {'newPaidCallsAllowedThisRound': True, 'limitTokens': 1000000,
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'knownUsedTokens': 839528, 'unknownReservedTokens': 72075, 'originalB…。
            'knownUsedTokens': 839528, 'unknownReservedTokens': 72075, 'originalBudgetPath': str(original),
            # 【逐行批注】 执行当前 Python 语句：'originalBudgetSha256': hashlib.sha256(original.read_bytes()).hexdigest(),。
            'originalBudgetSha256': hashlib.sha256(original.read_bytes()).hexdigest(),
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'activeBudgetPath': str(active), 'baseUrl': 'https://frozen.example.i…。
            'activeBudgetPath': str(active), 'baseUrl': 'https://frozen.example.invalid/v1', 'model': 'fixture-model'}
        # 【逐行批注】 计算右侧结果并赋给 approval.write_text(json.dumps(decision), en…，更新脚本状态。
        approval.write_text(json.dumps(decision), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = dict(os.environ, GANGYI_EVAL_MODE='live', GANGYI_EVAL_DECISION_FILE=str(approval),
                   # 【逐行批注】 计算右侧结果并赋给 GANGYI_EVAL_BUDGET_FILE，更新脚本状态。
                   GANGYI_EVAL_BUDGET_FILE=str(active), AI_BASE_URL=decision['baseUrl'], AI_MODEL=decision['model'])

        # 【逐行批注】 定义函数 check，封装可复用的处理步骤。
        def check(expected, **overrides):
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.run([str(Path(driver).resolve()), '--check-live-authorization'],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=dict(env, **overrides), capture_output=True, timeout=10)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert (process.returncode == 0) == expected, process.stderr.decode('utf-8', er…。
            assert (process.returncode == 0) == expected, process.stderr.decode('utf-8', errors='replace')

        # 【逐行批注】 调用 check 完成当前脚本或测试步骤：check(True)。
        check(True)
        # 【逐行批注】 计算右侧结果并赋给 blocked，更新脚本状态。
        blocked = subprocess.run([str(Path(driver).resolve()), '--check-live-search-block'], env=env, capture_output=True, timeout=10)
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert blocked.returncode == 0, blocked.stderr.decode('utf-8', errors='replace')。
        assert blocked.returncode == 0, blocked.stderr.decode('utf-8', errors='replace')
        # 【逐行批注】 计算右侧结果并赋给 check(False, AI_MODEL，更新脚本状态。
        check(False, AI_MODEL='different')
        # 【逐行批注】 计算右侧结果并赋给 check(False, AI_BASE_URL，更新脚本状态。
        check(False, AI_BASE_URL='https://different.example.invalid/v1')
        # 【逐行批注】 计算右侧结果并赋给 check(False, GANGYI_EVAL_DECISION_FILE，更新脚本状态。
        check(False, GANGYI_EVAL_DECISION_FILE='')
        # 【逐行批注】 计算右侧结果并赋给 check(False, GANGYI_EVAL_MODE，更新脚本状态。
        check(False, GANGYI_EVAL_MODE='')
        # 追加额度必须写入明确授权，单独抬高账本不能放行，历史用量也不能归零。
        # 【逐行批注】 计算右侧结果并赋给 active.write_text(json.dumps(dict(state, lim…，更新脚本状态。
        active.write_text(json.dumps(dict(state, limitTokens=1200000)), encoding='utf-8')
        # 【逐行批注】 调用 check 完成当前脚本或测试步骤：check(False)。
        check(False)
        # 【逐行批注】 计算右侧结果并赋给 decision['limitTokens']，更新脚本状态。
        decision['limitTokens'] = 1200000
        # 【逐行批注】 计算右侧结果并赋给 approval.write_text(json.dumps(decision), en…，更新脚本状态。
        approval.write_text(json.dumps(decision), encoding='utf-8')
        # 【逐行批注】 调用 check 完成当前脚本或测试步骤：check(True)。
        check(True)
        # 【逐行批注】 计算右侧结果并赋给 decision['limitTokens']，更新脚本状态。
        decision['limitTokens'] = 1000000
        # 【逐行批注】 计算右侧结果并赋给 approval.write_text(json.dumps(decision), en…，更新脚本状态。
        approval.write_text(json.dumps(decision), encoding='utf-8')
        # 【逐行批注】 遍历数据并重复执行循环体：for field in ('usedTokens', 'reservedTokens'):。
        for field in ('usedTokens', 'reservedTokens'):
            # 【逐行批注】 计算右侧结果并赋给 active.write_text(json.dumps(dict(state, **{…，更新脚本状态。
            active.write_text(json.dumps(dict(state, **{field: 0})), encoding='utf-8')
            # 【逐行批注】 调用 check 完成当前脚本或测试步骤：check(False)。
            check(False)
        # 【逐行批注】 计算右侧结果并赋给 active.write_text(json.dumps(state), encoding，更新脚本状态。
        active.write_text(json.dumps(state), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 original.write_text('{}', encoding，更新脚本状态。
        original.write_text('{}', encoding='utf-8')
        # 【逐行批注】 调用 check 完成当前脚本或测试步骤：check(False)。
        check(False)


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(driver):
    # 【逐行批注】 调用 authorization_checks 完成当前脚本或测试步骤：authorization_checks(driver)。
    authorization_checks(driver)
    # 【逐行批注】 计算右侧结果并赋给 repo，更新脚本状态。
    repo = Path(__file__).parents[1]
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-eval-usage-') as directory:。
    with tempfile.TemporaryDirectory(prefix='gangyi-eval-usage-') as directory:
        # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
        root = Path(directory)
        # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
        case = {'id':'usage-accounting','title':'HTTP统计测试','concept':'比较化合价前后变化。',
                # 【逐行批注】 执行当前 Python 语句：'questions':[{'question':'Fe变成Fe²⁺，化合价如何变化？','type':'open','expectedAnswer':'升高'}],。
                'questions':[{'question':'Fe变成Fe²⁺，化合价如何变化？','type':'open','expectedAnswer':'升高'}],
                # 【逐行批注】 执行当前 Python 语句：'turns':[{'type':'question_answer','sectionId':'question-0','text':'由0升高到+2。'}]}。
                'turns':[{'type':'question_answer','sectionId':'question-0','text':'由0升高到+2。'}]}
        # 【逐行批注】 计算右侧结果并赋给 (root/'case.json').write_text(json.dumps(cas…，更新脚本状态。
        (root/'case.json').write_text(json.dumps(case,ensure_ascii=False),encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 server，更新脚本状态。
        server = ThreadingHTTPServer(('127.0.0.1',0),UsageCompatibilityFixture)
        # 【逐行批注】 计算右侧结果并赋给 Thread(target，更新脚本状态。
        Thread(target=server.serve_forever,daemon=True).start()
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = dict(os.environ,AI_BASE_URL=f'http://127.0.0.1:{server.server_port}/v1',
                   # 【逐行批注】 计算右侧结果并赋给 AI_API_KEY，更新脚本状态。
                   AI_API_KEY='isolated-fixture',AI_MODEL='fixture-v560')
        # 【逐行批注】 计算右侧结果并赋给 budget_file，更新脚本状态。
        budget_file=root/'budget.json'
        # 【逐行批注】 计算右侧结果并赋给 budget_file.write_text(json.dumps({'limitTok…，更新脚本状态。
        budget_file.write_text(json.dumps({'limitTokens':1000000,'usedTokens':0,'reservedTokens':0}),encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 env['GANGYI_EVAL_BUDGET_FILE']，更新脚本状态。
        env['GANGYI_EVAL_BUDGET_FILE']=str(budget_file)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'isolated.db'),str(root/'result.json')],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=env,cwd=repo,capture_output=True,timeout=90,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert process.returncode == 0,process.stderr.decode('utf-8',errors='replace')。
            assert process.returncode == 0,process.stderr.decode('utf-8',errors='replace')
            # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
            result = json.loads((root/'result.json').read_text(encoding='utf-8'))
            # 【逐行批注】 计算右侧结果并赋给 initial_compatibility_retries，更新脚本状态。
            initial_compatibility_retries = UsageCompatibilityFixture.rejected_usage
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert result['turns'][0]['task']['status']=='waiting_student',result['turns'][…。
            assert result['turns'][0]['task']['status']=='waiting_student',result['turns'][0]['task'].get('failure')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert UsageCompatibilityFixture.rejected_usage>0。
            assert UsageCompatibilityFixture.rejected_usage>0
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert result['httpRequestCount']==UsageCompatibilityFixture.posts。
            assert result['httpRequestCount']==UsageCompatibilityFixture.posts
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert result['httpRequestCount']>len(result['usage'])。
            assert result['httpRequestCount']>len(result['usage'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert sum(request['status']=='failed' for request in result['httpRequests'])==…。
            assert sum(request['status']=='failed' for request in result['httpRequests'])==UsageCompatibilityFixture.rejected_usage
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(request['status'] in ('failed','completed') for request in result['h…。
            assert all(request['status'] in ('failed','completed') for request in result['httpRequests'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(request['tokens'] is None for request in result['httpRequests'])。
            assert all(request['tokens'] is None for request in result['httpRequests'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all('PRIVATE-' not in json.dumps(request) for request in result['httpReq…。
            assert all('PRIVATE-' not in json.dumps(request) for request in result['httpRequests'])
            # 【逐行批注】 计算右侧结果并赋给 budget，更新脚本状态。
            budget=json.loads(budget_file.read_text(encoding='utf-8'))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert budget['usedTokens']==0 and budget['reservedTokens']>0 and budget['unkno…。
            assert budget['usedTokens']==0 and budget['reservedTokens']>0 and budget['unknownUsageRequests']==result['httpRequestCount']
            # 【逐行批注】 计算右侧结果并赋给 budget_file.write_text(json.dumps({'limitTok…，更新脚本状态。
            budget_file.write_text(json.dumps({'limitTokens':1,'usedTokens':0,'reservedTokens':0}),encoding='utf-8')
            # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
            calls_before=UsageCompatibilityFixture.posts
            # 【逐行批注】 计算右侧结果并赋给 stopped，更新脚本状态。
            stopped=subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'budget-stopped.db'),str(root/'budget-stopped.json')],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert stopped.returncode==0,stopped.stderr.decode('utf-8',errors='replace')。
            assert stopped.returncode==0,stopped.stderr.decode('utf-8',errors='replace')
            # 【逐行批注】 计算右侧结果并赋给 denied，更新脚本状态。
            denied=json.loads((root/'budget-stopped.json').read_text(encoding='utf-8'))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert denied['turns'][0]['task']['failure']['kind']=='eval_budget'。
            assert denied['turns'][0]['task']['failure']['kind']=='eval_budget'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert UsageCompatibilityFixture.posts==calls_before。
            assert UsageCompatibilityFixture.posts==calls_before
            # 零调用预检必须真的进入失败轮恢复，不能返回原来的失败记录而漏采提示。
            # 【逐行批注】 计算右侧结果并赋给 recovery_env，更新脚本状态。
            recovery_env=dict(env,GANGYI_EVAL_RESUME='1',GANGYI_EVAL_MAX_CALLS='0')
            # 【逐行批注】 计算右侧结果并赋给 preview，更新脚本状态。
            preview=subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'budget-stopped.db'),str(root/'recovery-preview.json')],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=recovery_env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert preview.returncode==0,preview.stderr.decode('utf-8',errors='replace')。
            assert preview.returncode==0,preview.stderr.decode('utf-8',errors='replace')
            # 【逐行批注】 计算右侧结果并赋给 captured，更新脚本状态。
            captured=json.loads((root/'recovery-preview.json').read_text(encoding='utf-8'))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert captured['promptAudit'] and captured['turns'][0]['task']['failure']['kin…。
            assert captured['promptAudit'] and captured['turns'][0]['task']['failure']['kind']=='eval_call_limit'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert UsageCompatibilityFixture.posts==calls_before。
            assert UsageCompatibilityFixture.posts==calls_before
            # 【逐行批注】 计算右侧结果并赋给 budget_file.write_text(json.dumps({'limitTok…，更新脚本状态。
            budget_file.write_text(json.dumps({'limitTokens':1000000,'usedTokens':0,'reservedTokens':0}),encoding='utf-8')
            # 【逐行批注】 执行当前 Python 语句：recovery_env.pop('GANGYI_EVAL_MAX_CALLS')。
            recovery_env.pop('GANGYI_EVAL_MAX_CALLS')
            # 【逐行批注】 计算右侧结果并赋给 recovered，更新脚本状态。
            recovered=subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'budget-stopped.db'),str(root/'recovered.json')],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=recovery_env,cwd=repo,capture_output=True,timeout=90,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert recovered.returncode==0,recovered.stderr.decode('utf-8',errors='replace')。
            assert recovered.returncode==0,recovered.stderr.decode('utf-8',errors='replace')
            # 【逐行批注】 计算右侧结果并赋给 resumed，更新脚本状态。
            resumed=json.loads((root/'recovered.json').read_text(encoding='utf-8'))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert resumed['turns'][0]['task']['id']==denied['turns'][0]['task']['id']。
            assert resumed['turns'][0]['task']['id']==denied['turns'][0]['task']['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert resumed['turns'][0]['task']['status']=='waiting_student'。
            assert resumed['turns'][0]['task']['status']=='waiting_student'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert sum(item['kind']=='chat-user' for item in resumed['interactions'])==1。
            assert sum(item['kind']=='chat-user' for item in resumed['interactions'])==1
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert all(isinstance(turn['externalProcessMs'], int) for turn in resumed['turn…。
            assert all(isinstance(turn['externalProcessMs'], int) for turn in resumed['turns'])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert isinstance(resumed['externalElapsedMs'], int)。
            assert isinstance(resumed['externalElapsedMs'], int)
            # 直接执行驱动也不得凭外部地址启动本轮付费请求；防止URL用户信息伪装本机地址。
            # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
            calls_before = UsageCompatibilityFixture.posts
            # 【逐行批注】 计算右侧结果并赋给 remote_env，更新脚本状态。
            remote_env = dict(env, AI_BASE_URL='http://127.0.0.1:9@example.invalid/v1')
            # 【逐行批注】 计算右侧结果并赋给 rejected，更新脚本状态。
            rejected = subprocess.run([str(Path(driver).resolve()), str(root/'case.json'), str(root/'remote-blocked.db'), str(root/'remote-blocked.json')],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=remote_env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert rejected.returncode == 0, rejected.stderr.decode('utf-8',errors='replace…。
            assert rejected.returncode == 0, rejected.stderr.decode('utf-8',errors='replace')
            # 【逐行批注】 计算右侧结果并赋给 blocked，更新脚本状态。
            blocked = json.loads((root/'remote-blocked.json').read_text(encoding='utf-8'))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert blocked['turns'][0]['task']['failure']['kind'] == 'eval_authorization'。
            assert blocked['turns'][0]['task']['failure']['kind'] == 'eval_authorization'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert blocked['httpRequestCount'] == 0 and UsageCompatibilityFixture.posts == …。
            assert blocked['httpRequestCount'] == 0 and UsageCompatibilityFixture.posts == calls_before
            # 预留保存失败时不发送HTTP，原有效预算逐字节保留；退出码不能伪装成成功结算。
            # 【逐行批注】 计算右侧结果并赋给 original_budget，更新脚本状态。
            original_budget = json.dumps({'limitTokens':1000000,'usedTokens':0,'reservedTokens':0}).encode()
            # 【逐行批注】 执行当前 Python 语句：budget_file.write_bytes(original_budget)。
            budget_file.write_bytes(original_budget)
            # 【逐行批注】 计算右侧结果并赋给 temporary_budget，更新脚本状态。
            temporary_budget = Path(str(budget_file)+'.tmp')
            # 【逐行批注】 执行当前 Python 语句：temporary_budget.mkdir()。
            temporary_budget.mkdir()
            # 【逐行批注】 计算右侧结果并赋给 save_failed，更新脚本状态。
            save_failed = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'save-failed.db'),str(root/'save-failed.json')],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert save_failed.returncode == 73, save_failed.stderr.decode('utf-8', errors=…。
            assert save_failed.returncode == 73, save_failed.stderr.decode('utf-8', errors='replace')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert UsageCompatibilityFixture.posts == calls_before and budget_file.read_byt…。
            assert UsageCompatibilityFixture.posts == calls_before and budget_file.read_bytes() == original_budget
            # 【逐行批注】 执行当前 Python 语句：temporary_budget.rmdir()。
            temporary_budget.rmdir()
            # 结算写盘失败时保留请求前预留，停止后续调用并返回专用预算失败退出码。
            # 【逐行批注】 计算右侧结果并赋给 UsageCompatibilityFixture.block_budget_tempo…，更新脚本状态。
            UsageCompatibilityFixture.block_budget_temporary = temporary_budget
            # 【逐行批注】 计算右侧结果并赋给 settle_failed，更新脚本状态。
            settle_failed = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'settle-failed.db'),str(root/'settle-failed.json')],
                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                env=env,cwd=repo,capture_output=True,timeout=90,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            # 【逐行批注】 计算右侧结果并赋给 UsageCompatibilityFixture.block_budget_tempo…，更新脚本状态。
            UsageCompatibilityFixture.block_budget_temporary = None
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert settle_failed.returncode == 73, settle_failed.stderr.decode('utf-8', err…。
            assert settle_failed.returncode == 73, settle_failed.stderr.decode('utf-8', errors='replace')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert UsageCompatibilityFixture.posts-calls_before == 2, '结算失败后不得进入下一逻辑请求'。
            assert UsageCompatibilityFixture.posts-calls_before == 2, '结算失败后不得进入下一逻辑请求'
            # 【逐行批注】 计算右侧结果并赋给 saved_reservation，更新脚本状态。
            saved_reservation = json.loads(budget_file.read_text(encoding='utf-8'))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert saved_reservation['reservedTokens'] > 0 and saved_reservation['usedToken…。
            assert saved_reservation['reservedTokens'] > 0 and saved_reservation['usedTokens'] == 0
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'httpRequests' not in saved_reservation。
            assert 'httpRequests' not in saved_reservation
            # 【逐行批注】 执行当前 Python 语句：temporary_budget.rmdir()。
            temporary_budget.rmdir()
            # Windows允许读取但拒绝删除共享时，原子替换必须失败；不能先删除旧账本。
            # 【逐行批注】 判断条件 os.name == 'nt':，成立时执行对应分支。
            if os.name == 'nt':
                # 【逐行批注】 导入后续逻辑需要的模块或符号：import ctypes。
                import ctypes
                # 【逐行批注】 计算右侧结果并赋给 api，更新脚本状态。
                api = ctypes.WinDLL('kernel32', use_last_error=True)
                # 【逐行批注】 计算右侧结果并赋给 api.CreateFileW.argtypes，更新脚本状态。
                api.CreateFileW.argtypes = [ctypes.c_wchar_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p]
                # 【逐行批注】 计算右侧结果并赋给 api.CreateFileW.restype，更新脚本状态。
                api.CreateFileW.restype = ctypes.c_void_p
                # 【逐行批注】 计算右侧结果并赋给 api.CloseHandle.argtypes，更新脚本状态。
                api.CloseHandle.argtypes = [ctypes.c_void_p]
                # 【逐行批注】 执行当前 Python 语句：budget_file.write_bytes(original_budget)。
                budget_file.write_bytes(original_budget)
                # 【逐行批注】 计算右侧结果并赋给 handle，更新脚本状态。
                handle = api.CreateFileW(str(budget_file), 0x80000000, 3, None, 3, 0x80, None)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert handle not in (None, ctypes.c_void_p(-1).value)。
                assert handle not in (None, ctypes.c_void_p(-1).value)
                # 【逐行批注】 计算右侧结果并赋给 calls_before，更新脚本状态。
                calls_before = UsageCompatibilityFixture.posts
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 commit_failed，更新脚本状态。
                    commit_failed = subprocess.run([str(Path(driver).resolve()),str(root/'case.json'),str(root/'commit-failed.db'),str(root/'commit-failed.json')],
                        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                        env=env,cwd=repo,capture_output=True,timeout=30,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert commit_failed.returncode == 73, commit_failed.stderr.decode('utf-8', err…。
                    assert commit_failed.returncode == 73, commit_failed.stderr.decode('utf-8', errors='replace')
                    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert UsageCompatibilityFixture.posts == calls_before and budget_file.read_byt…。
                    assert UsageCompatibilityFixture.posts == calls_before and budget_file.read_bytes() == original_budget
                # 【逐行批注】 执行当前 Python 语句：finally:。
                finally:
                    # 【逐行批注】 执行当前 Python 语句：api.CloseHandle(handle)。
                    api.CloseHandle(handle)
            # 【逐行批注】 输出当前运行结果：print(json.dumps({'passed':True,'logicalCalls':len(result['usage']),'httpR…。
            print(json.dumps({'passed':True,'logicalCalls':len(result['usage']),'httpRequests':result['httpRequestCount'],
                # 【逐行批注】 执行当前 Python 语句：'allFixtureHttpRequests':UsageCompatibilityFixture.posts,。
                'allFixtureHttpRequests':UsageCompatibilityFixture.posts,
                # 【逐行批注】 执行当前 Python 语句：'compatibilityRetries':initial_compatibility_retries,'totalCompatibilityRetries':UsageCom…。
                'compatibilityRetries':initial_compatibility_retries,'totalCompatibilityRetries':UsageCompatibilityFixture.rejected_usage,
                # 【逐行批注】 执行当前 Python 语句：'unknownUsagePreserved':True,'budgetStopsBeforeRequest':True,'directExternalHttpDenied':T…。
                'unknownUsagePreserved':True,'budgetStopsBeforeRequest':True,'directExternalHttpDenied':True,
                # 【逐行批注】 执行当前 Python 语句：'failedTaskResumeVerified':True,'zeroCallRecoveryCapturesPrompt':True,。
                'failedTaskResumeVerified':True,'zeroCallRecoveryCapturesPrompt':True,
                # 【逐行批注】 执行当前 Python 语句：'budgetSaveFailureHttpZero':True,'settlementFailureStopsDriver':True,'oldBudgetNeverAccep…。
                'budgetSaveFailureHttpZero':True,'settlementFailureStopsDriver':True,'oldBudgetNeverAcceptedAsNewSettlement':True,
                # 【逐行批注】 计算右侧结果并赋给 'paidRequests':0},ensure_ascii，更新脚本状态。
                'paidRequests':0},ensure_ascii=False))
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 执行当前 Python 语句：server.shutdown();server.server_close()。
            server.shutdown();server.server_close()


# 【逐行批注】 判断条件 __name__=='__main__':，成立时执行对应分支。
if __name__=='__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encodin…，成立时执行对应分支。
    if hasattr(sys.stdout,'reconfigure'):sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main(sys.argv[1])。
    main(sys.argv[1])
