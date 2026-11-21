"""九科评测执行器：复用实际C++主控与逐请求预算，默认零调用预检。"""

from __future__ import annotations

import argparse
import copy
from contextlib import contextmanager
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
from urllib.parse import urlsplit

import tutor_benchmark as benchmark
from tutor_build_identity import checked_build
from evaluation_guard import read_budget, require_live_authorization, using_decision

ROOT = Path(__file__).resolve().parents[1]
SUBJECT_NAMES = {'mathematics':'数学','physics':'物理','chemistry':'化学','biology':'生物','history':'历史',
                 'geography':'地理','chinese':'语文','english':'英语','computer_science':'计算机'}


def file_hash(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def now():
    return datetime.now(timezone.utc).isoformat()


def atomic_save(path, value):
    """检查点可原子更新，逐例文件和原始回复永不覆盖。"""
    path = Path(path)
    temporary = path.with_suffix(path.suffix + '.tmp')
    with temporary.open('w', encoding='utf-8') as stream:
        json.dump(value, stream, ensure_ascii=False, indent=2, allow_nan=False)
        stream.flush(); os.fsync(stream.fileno())
    temporary.replace(path)


@contextmanager
def experiment_lock(folder):
    """使用OS锁阻止并发，进程退出自动释放；不会依据文件时间戳重启请求。"""
    path = Path(folder) / 'run.lock'
    with path.open('a+b') as stream:
        stream.seek(0)
        if os.name == 'nt':
            import msvcrt
            if path.stat().st_size == 0:
                stream.write(b'0'); stream.flush(); stream.seek(0)
            try:msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)
            except OSError:raise RuntimeError('另一个运行持有实验锁，未启动新请求。') from None
        else:
            import fcntl
            try:fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except OSError:raise RuntimeError('另一个运行持有实验锁，未启动新请求。') from None
        try:yield
        finally:
            stream.seek(0)
            if os.name == 'nt':msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
            else:fcntl.flock(stream, fcntl.LOCK_UN)


def native_case(case):
    """原生重放用户历史，不预填虚构教师回复；私有参考只作为题目评分依据。"""
    events = []
    for turn in case['turns']:
        if turn['role'] != 'user':raise ValueError('原生重放不接受预填教师回复。')
        events.append({'type':'chat','text':turn['text']})
    events.append({'type':'question_answer' if case['formalSubmission'] else 'chat',
                   'text':case['studentResponse'] if case['formalSubmission'] else case['prompt'],
                   **({'sectionId':'question-0'} if case['formalSubmission'] else {})})
    from tutor_component_scenes import validate_scene_plan, component_event
    plan = validate_scene_plan(case)
    if plan and plan['mode'] == 'interactive':
        events[-1].update(component_event(plan))
    result = {'id':case['id'],'title':SUBJECT_NAMES[case['subject']]+' · '+case['knowledgePoint'],'subject':case['subject'],
            'concept':'本课任务：'+case['prompt'],
            'questions':[{'type':'open','question':case['prompt'],'expectedAnswer':case['reference'],
                          'rubric':'候选评分依据，待专家审核：'+case['reference']}],
            'turns':events,'capturePrompts':True,
            'benchmarkIdentity':{'caseId':case['id'],'inputHash':benchmark.digest(case),'binding':case['binding']}}
    if plan:
        result['componentFixtures'] = copy.deepcopy(plan['fixtures'])
        result['concept'] += '\n组件材料来源：工程合成夹具，非真实学生操作。适配边界：' + plan['reason']
    return result


def prepare(dataset_path, driver, output, case_ids=None, limit_tokens=100000, limit_requests=30):
    dataset = benchmark.load_json(dataset_path);coverage = benchmark.validate_dataset(dataset)
    if type(limit_tokens) is not int or not 0<limit_tokens<=9007199254740991 or type(limit_requests) is not int or not 0<limit_requests<=9007199254740991:
        raise ValueError('Token与HTTP上限必须为正整数。')
    driver = Path(driver).resolve()
    checked_build(driver)
    cases = dataset['cases']
    if case_ids:
        by_id={case['id']:case for case in cases}
        if len(set(case_ids))!=len(case_ids) or set(case_ids)-set(by_id):raise ValueError('案例选择重复或不存在。')
        cases=[by_id[key] for key in case_ids]
    converted=[native_case(case) for case in cases]
    if any(native.get('componentFixtures') for native in converted):
        from tutor_component_scenes import inspect_fixtures
        checked = inspect_fixtures(driver, cases, clean_environment())
        if not checked['valid']:
            raise ValueError('组件配置或合成状态未通过原生校验，未准备实验。')
    folder=Path(output).resolve();folder.mkdir(parents=True,exist_ok=False)
    benchmark.write_json(folder/'dataset.json',dataset)
    records=[]
    for index,(case,native) in enumerate(zip(cases,converted)):
        relative=f'cases/{index:03d}.json';benchmark.write_json(folder/relative,native)
        records.append({'caseId':case['id'],'inputHash':benchmark.digest(case),'caseFile':relative,
                        'caseSha256':file_hash(folder/relative),'expectedTurns':len(native['turns'])})
    budget={'limitTokens':limit_tokens,'usedTokens':0,'reservedTokens':0,'limitRequests':limit_requests,
            'httpRequests':0,'reservedRequests':0,'unknownUsageRequests':0}
    for name in ('budget-original.json','budget-current.json'):benchmark.write_json(folder/name,budget)
    benchmark.write_json(folder/'search-disabled.json',{'limit':0,'used':0,'allowed':False})
    manifest={'schemaVersion':1,'kind':'native_tutor_experiment','datasetId':dataset['datasetId'],
              'datasetHash':benchmark.digest(dataset),'datasetFileSha256':file_hash(folder/'dataset.json'),
              'driverPath':str(driver),'driverSha256':file_hash(driver),'driverSourceSha256':file_hash(ROOT/'tests/teaching_eval_driver.cpp'),
              'buildReceiptSha256':file_hash(str(driver)+'.build.json'),
              'executorSha256':file_hash(Path(__file__)), 'searchPolicySha256':file_hash(folder/'search-disabled.json'),
              'createdAt':now(),'reviewStatus':coverage['reviewStatus'],'records':records,
              'limits':{'tokens':limit_tokens,'httpRequests':limit_requests},
              'originalBudgetSha256':file_hash(folder/'budget-original.json'),'acceptanceComplete':False}
    benchmark.write_json(folder/'manifest.json',manifest)
    approval={'newPaidCallsAllowedThisRound':False,'approvedBy':None,'approvedAt':None,'approvalNote':None,
              'manifestSha256':file_hash(folder/'manifest.json'),'driverSha256':manifest['driverSha256'],'datasetHash':manifest['datasetHash'],
              'baseUrl':None,'model':None,'profileName':None,'configurationSha256':None,
              'manifestPath':str(folder/'manifest.json'),'configurationPath':None,
              'frozenFiles':[{'path':str(folder/name),'sha256':file_hash(folder/name)} for name in
                ['dataset.json','search-disabled.json',*[row['caseFile'] for row in records]]] +
                [{'path':str(driver),'sha256':manifest['driverSha256']},
                 {'path':str(driver)+'.build.json','sha256':manifest['buildReceiptSha256']}] +
                [{'path':str(ROOT/name),'sha256':file_hash(ROOT/name)} for name in
                  ['scripts/tutor_experiment.py','scripts/tutor_build_identity.py','scripts/evaluation_guard.py',
                   'scripts/teaching_eval_provider.py','scripts/tutor_benchmark.py','scripts/tutor_component_scenes.py','scripts/ai_money_budget.py']],
              'limitTokens':limit_tokens,'limitRequests':limit_requests,'knownUsedTokens':0,'unknownReservedTokens':0,
              'originalBudgetPath':str(folder/'budget-original.json'),'originalBudgetSha256':manifest['originalBudgetSha256'],
              'activeBudgetPath':str(folder/'budget-current.json'),'searchBudgetPath':str(folder/'search-disabled.json')}
    benchmark.write_json(folder/'authorization-template.json',approval)
    benchmark.write_json(folder/'run.json',{'manifestSha256':file_hash(folder/'manifest.json'),'identity':None,'mode':None,
                                         'startedAt':None,'status':'prepared','records':[],
                                         'authorizationTemplateSha256':file_hash(folder/'authorization-template.json'),'acceptanceComplete':False})
    return {'folder':str(folder),'selectedCases':len(records),'paidModelRequests':0,'status':'prepared'}


def checked_plan(folder):
    manifest=benchmark.load_json(folder/'manifest.json')
    if manifest.get('schemaVersion')!=1 or manifest.get('kind')!='native_tutor_experiment':raise ValueError('实验清单版本无效。')
    checked_build(manifest['driverPath'])
    if (file_hash(str(manifest['driverPath'])+'.build.json')!=manifest['buildReceiptSha256'] or
        file_hash(Path(__file__))!=manifest['executorSha256'] or file_hash(folder/'search-disabled.json')!=manifest['searchPolicySha256']):
        raise ValueError('构建凭据、执行器或搜索策略已变化。')
    dataset=benchmark.load_json(folder/'dataset.json');benchmark.validate_dataset(dataset)
    if (file_hash(folder/'dataset.json')!=manifest['datasetFileSha256'] or benchmark.digest(dataset)!=manifest['datasetHash'] or
        file_hash(manifest['driverPath'])!=manifest['driverSha256'] or file_hash(ROOT/'tests/teaching_eval_driver.cpp')!=manifest['driverSourceSha256']):
        raise ValueError('实验材料、入口或源码已变化，请重新准备。')
    if file_hash(folder/'budget-original.json')!=manifest['originalBudgetSha256']:raise ValueError('原预算快照已变化。')
    source={case['id']:case for case in dataset['cases']}
    seen=set()
    for row in manifest['records']:
        if row['caseId'] in seen or row['caseId'] not in source:raise ValueError('案例清单身份不唯一。')
        seen.add(row['caseId'])
        original=source[row['caseId']]
        if row['inputHash']!=benchmark.digest(original):raise ValueError('案例来源身份无效。')
        relative=Path(row['caseFile'])
        if relative.is_absolute() or '..' in relative.parts or not (folder/relative).resolve().is_relative_to(folder):raise ValueError('案例路径越界。')
        if (file_hash(folder/relative)!=row['caseSha256'] or
            benchmark.load_json(folder/relative)!=native_case(original) or row['expectedTurns']!=len(native_case(original)['turns'])):
            raise ValueError('原生案例已变化。')
    if not seen:raise ValueError('案例清单为空。')
    return manifest


def fixture_endpoint(url):
    parsed=urlsplit(url)
    if (parsed.scheme!='http' or parsed.hostname not in {'127.0.0.1','::1'} or not parsed.port or
        parsed.username or parsed.password or parsed.query or parsed.fragment):raise ValueError('模拟模式仅允许带端口的本机HTTP地址。')
    return url


def live_environment(folder,manifest,authorization,configuration):
    """严格核对授权和冻结身份后，才调用既有凭据读取器。"""
    if authorization is None or configuration is None:raise PermissionError('缺少真实实验授权与冻结配置，未读取凭据。')
    approved=benchmark.load_json(authorization);frozen=benchmark.load_json(configuration)
    if approved.get('newPaidCallsAllowedThisRound') is not True or not all(benchmark.nonblank(approved.get(key)) for key in ('approvedBy','approvedAt','approvalNote')):
        raise PermissionError('没有本轮明确授权，未读取凭据。')
    expected={'manifestSha256':file_hash(folder/'manifest.json'),'driverSha256':manifest['driverSha256'],'datasetHash':manifest['datasetHash'],
              'configurationSha256':file_hash(configuration),'limitTokens':manifest['limits']['tokens'],
              'limitRequests':manifest['limits']['httpRequests'],'originalBudgetSha256':manifest['originalBudgetSha256']}
    if any(approved.get(key)!=value for key,value in expected.items()):raise PermissionError('授权未绑定当前材料、配置、入口与预算。')
    if (Path(approved.get('manifestPath','')).resolve()!=folder/'manifest.json' or
        Path(approved.get('configurationPath','')).resolve()!=Path(configuration).resolve()):
        raise PermissionError('授权未绑定固定材料路径。')
    template=benchmark.load_json(folder/'authorization-template.json')
    if approved.get('frozenFiles')!=template['frozenFiles']:raise PermissionError('授权未绑定完整材料。')
    for item in approved['frozenFiles']:
        if file_hash(item['path'])!=item['sha256']:raise PermissionError('授权材料已变化。')
    for key,name in [('originalBudgetPath','budget-original.json'),('activeBudgetPath','budget-current.json'),('searchBudgetPath','search-disabled.json')]:
        if Path(approved.get(key,'')).resolve()!=folder/name:raise PermissionError('预算目录与本实验不一致。')
    for key in ('baseUrl','model','profileName'):
        if not benchmark.nonblank(frozen.get(key)) or frozen[key]!=approved.get(key):raise PermissionError('模型配置与授权不一致。')
    url=urlsplit(frozen['baseUrl'])
    if url.scheme!='https' or not url.hostname or url.username or url.password or url.query or url.fragment:raise PermissionError('真实模型地址须为无凭据的HTTPS地址。')
    # Token预算不能替代人民币总额：读取密钥之前绑定同一轮累计账本。
    from ai_money_budget import checked_live_environment
    for key in ('moneyConfigPath', 'moneyConfigSha256'):
        if not benchmark.nonblank(frozen.get(key)) or frozen[key] != approved.get(key):
            raise PermissionError('真实实验缺少已冻结的共用人民币预算。')
    endpoint = frozen['baseUrl'].rstrip('/')
    if not endpoint.endswith('/chat/completions'): endpoint += '/chat/completions'
    money_env = checked_live_environment(frozen['moneyConfigPath'], frozen['moneyConfigSha256'], endpoint, frozen['model'])
    from teaching_eval_provider import configured_environment
    with using_decision(authorization):
        require_live_authorization()
        configured=configured_environment({'profileName':frozen['profileName'],'frozenConfiguration':str(configuration)})
    env=clean_environment();env.update(AI_API_KEY=configured['AI_API_KEY'],AI_BASE_URL=frozen['baseUrl'],AI_MODEL=frozen['model'])
    env.update(money_env)
    return env


def clean_environment():
    """模拟运行只继承运行程序必要的系统变量，不读取或复制模型/搜索凭据。"""
    allowed={'SYSTEMROOT','WINDIR','SYSTEMDRIVE','PATH','PATHEXT','TEMP','TMP','COMSPEC','USERPROFILE','APPDATA','LOCALAPPDATA'}
    return {key:value for key,value in os.environ.items() if key.upper() in allowed}


def budget_state(folder,manifest):
    state=read_budget(folder/'budget-current.json')
    for field in ('limitRequests','httpRequests','reservedRequests','unknownUsageRequests'):
        if type(state.get(field)) is not int or not 0<=state[field]<=9007199254740991:raise ValueError('预算账本字段无效。')
    if any(type(state.get(key)) is not int or not 0<=state[key]<=9007199254740991 for key in ('limitTokens','usedTokens','reservedTokens')):
        raise ValueError('Token预算超出安全范围。')
    if state['limitTokens']!=manifest['limits']['tokens'] or state['limitRequests']!=manifest['limits']['httpRequests']:raise ValueError('预算上限与冻结清单不符。')
    if state.get('unexpectedUsage') or state['usedTokens']+state['reservedTokens']>state['limitTokens'] or state['httpRequests']+state['reservedRequests']>state['limitRequests']:
        raise ValueError('用量超出预留，停止后续请求。')
    return state


def checked_run(folder, manifest, run):
    """已完成结果必须与原案例、原始日志、预算链逐一匹配后才能跳过。"""
    if run.get('manifestSha256') != file_hash(folder/'manifest.json'):
        raise ValueError('恢复记录不属于当前清单。')
    if run.get('authorizationTemplateSha256')!=file_hash(folder/'authorization-template.json'):
        raise ValueError('授权模板已变化。')
    rows = run.get('records')
    if not isinstance(rows, list) or len(rows) > len(manifest['records']):raise ValueError('恢复记录数量无效。')
    if run.get('status') not in {'prepared','running','completed','failed','uncertain'}:raise ValueError('运行状态无效。')
    if run.get('identity') is None:
        if rows or run.get('mode') is not None or run['status']!='prepared':raise ValueError('运行身份缺失。')
    elif (run.get('mode') not in {'preflight','fixture','live'} or
          run['identity'].get('mode')!=run['mode'] or run['status']=='prepared'):
        raise ValueError('运行身份和模式不一致。')
    if run['status']=='completed' and (len(rows)!=len(manifest['records']) or
        any(row.get('status') not in {'preflight','completed'} for row in rows)):
        raise ValueError('完成状态与记录不一致。')
    if run['status'] in {'uncertain','failed'} and (not rows or rows[-1].get('status')!=run['status']):
        raise ValueError('终止状态与记录不一致。')
    previous = benchmark.load_json(folder/'budget-original.json')
    for index, row in enumerate(rows):
        record = manifest['records'][index]
        if row.get('caseId') != record['caseId'] or row.get('directory') != f'runs/{index:03d}':
            raise ValueError('恢复记录顺序或目录不匹配。')
        if row.get('budgetBefore') != previous:raise ValueError('预算链断裂。')
        if row.get('status') in {'completed', 'preflight'}:
            case_dir = folder/row['directory']
            for name, key in [('result.json','resultSha256'), ('result.json.responses.jsonl','rawResponsesSha256'),
                              ('stdout.log','stdoutSha256'), ('stderr.log','stderrSha256')]:
                if file_hash(case_dir/name) != row.get(key):raise ValueError('已有评测结果损坏，拒绝跳过或重发。')
            result = benchmark.load_json(case_dir/'result.json')
            coverage=validate_result(folder, record, case_dir, result, run['mode'])
            if row.get('coverage')!=coverage or (row['status']=='preflight')!=(run['mode']=='preflight'):
                raise ValueError('保存的覆盖范围不匹配。')
            previous = row['budgetAfter']
        elif row.get('status') in {'running', 'uncertain', 'failed'}:
            if index != len(rows)-1:raise ValueError('失败或不确定案例之后存在异常记录。')
            return
        else:raise ValueError('未知恢复状态。')
    if benchmark.load_json(folder/'budget-current.json') != previous:
        raise ValueError('累计预算与已保存结果不一致。')


def validate_result(folder, record, case_dir, result, mode):
    native = benchmark.load_json(folder/record['caseFile'])
    if (result.get('caseId') != record['caseId'] or result.get('experimentCaseSha256') != benchmark.digest(native) or
        Path(result.get('rawResponsesPath','')).resolve() != (case_dir/'result.json.responses.jsonl').resolve()):
        raise ValueError('原生结果身份无效。')
    turns = result.get('turns', [])
    if not turns or len(turns)>record['expectedTurns']:raise ValueError('结果轮数无效。')
    for index, turn in enumerate(turns):
        event=turn['event'];expected=native['turns'][index]
        if any(event.get(key)!=value for key,value in expected.items()) or event.get('requestId')!=f'eval-turn-{index}':
            raise ValueError('原生结果事件未匹配冻结输入。')
    if mode=='preflight':
        if (result.get('httpRequestCount')!=0 or not result.get('promptAudit') or
            turns[-1]['task'].get('failure',{}).get('kind')!='eval_call_limit'):
            raise ValueError('零调用预检未在主控发送前停止。')
    elif any(turn.get('task', {}).get('status') == 'failed' for turn in turns):
        failed = next(turn['task'] for turn in reversed(turns) if turn.get('task', {}).get('status') == 'failed')
        failure = failed.get('failure', {}) if isinstance(failed.get('failure', {}), dict) else {}
        return {'observedTurns':len(turns), 'expectedTurns':record['expectedTurns'],
                'allTurnsExecuted':False, 'httpRequests':result['httpRequestCount'],
                'qualityEvaluated':False, 'executionFailureKind':failure.get('kind', 'unknown')}
    elif len(turns)!=record['expectedTurns'] or any(turn['task']['status'] not in {'ready','waiting_student','completed'} for turn in turns):
        raise ValueError('主控未完整完成，不记教学通过。')
    return {'observedTurns':len(turns), 'expectedTurns':record['expectedTurns'],
            'allTurnsExecuted':mode!='preflight' and len(turns)==record['expectedTurns'],
            'httpRequests':result['httpRequestCount'], 'qualityEvaluated':False}


def execute(folder,mode='preflight',fixture_url=None,authorization=None,configuration=None,timeout=180):
    folder=Path(folder).resolve()
    with experiment_lock(folder):
        manifest=checked_plan(folder);run=benchmark.load_json(folder/'run.json')
        checked_run(folder,manifest,run)
        if mode not in {'preflight','fixture','live'} or not 1<=timeout<=1800:raise ValueError('模式或超时无效。')
        endpoint=fixture_endpoint(fixture_url) if mode=='fixture' else 'http://127.0.0.1:9/v1'
        identity={'mode':mode,'fixtureUrl':endpoint if mode=='fixture' else None,
                  'authorizationSha256':file_hash(authorization) if authorization else None,'configurationSha256':file_hash(configuration) if configuration else None}
        if run['identity'] not in (None,identity):raise ValueError('恢复必须使用相同模式、端点、授权与配置。')
        if any(row['status']=='running' for row in run['records']):
            # 原生子进程可能仍在执行或已发送，恢复不能自动重发。
            for row in run['records']:
                if row['status']=='running':row['status']='uncertain'
            run['status']='uncertain';atomic_save(folder/'run.json',run);return inspect(folder)
        if any(row['status'] in {'uncertain','failed'} for row in run['records']):return inspect(folder)
        budget_state(folder,manifest)
        if mode=='live':env=live_environment(folder,manifest,authorization,configuration)
        else:
            env=clean_environment();env.update(AI_BASE_URL=endpoint,AI_MODEL='isolated-tutor-fixture',AI_API_KEY='isolated-fixture-key')
        env.update(BOCHA_API_KEY='',DEEPSEEK_API_KEY='',DEEPSEEK_SEARCH_API_KEY='',OPENAI_API_KEY='',AI_RETRY_ATTEMPTS='1',
                   GANGYI_EVAL_BUDGET_FILE=str(folder/'budget-current.json'),GANGYI_EVAL_MODE=mode)
        if mode=='live':env.update(GANGYI_EVAL_DECISION_FILE=str(Path(authorization).resolve()),
            GANGYI_TUTOR_EXPERIMENT='1',GANGYI_TUTOR_AUTH_SHA256=identity['authorizationSha256'])
        if mode=='preflight':env['GANGYI_EVAL_MAX_CALLS']='0'
        run.update(mode=mode,identity=identity,status='running',startedAt=run['startedAt'] or now());atomic_save(folder/'run.json',run)
        done={row['caseId'] for row in run['records']}
        for record in manifest['records']:
            if record['caseId'] in done:continue
            checked_run(folder,manifest,run)
            if mode=='live':
                checked_plan(folder)
                if file_hash(authorization)!=identity['authorizationSha256'] or file_hash(configuration)!=identity['configurationSha256']:
                    raise PermissionError('授权或配置在运行中发生变化。')
                env=live_environment(folder,manifest,authorization,configuration)
                env.update(GANGYI_EVAL_BUDGET_FILE=str(folder/'budget-current.json'), GANGYI_EVAL_MODE='live',
                           GANGYI_EVAL_DECISION_FILE=str(Path(authorization).resolve()), AI_RETRY_ATTEMPTS='1',
                           GANGYI_TUTOR_EXPERIMENT='1',GANGYI_TUTOR_AUTH_SHA256=identity['authorizationSha256'])
            before=budget_state(folder,manifest);case_dir=folder/'runs'/f'{len(run["records"]):03d}';case_dir.mkdir(parents=True,exist_ok=False)
            row={'caseId':record['caseId'],'status':'running','startedAt':now(),'directory':case_dir.relative_to(folder).as_posix(),'budgetBefore':before,'pid':None}
            run['records'].append(row);atomic_save(folder/'run.json',run);started=time.monotonic();process=None
            try:
                with (case_dir/'stdout.log').open('xb') as out,(case_dir/'stderr.log').open('xb') as error:
                    process=subprocess.Popen([manifest['driverPath'],str(folder/record['caseFile']),str(case_dir/'isolated.db'),str(case_dir/'result.json')],
                                             cwd=ROOT,env=env,stdout=out,stderr=error,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                    row['pid']=process.pid;atomic_save(folder/'run.json',run)
                    try:code=process.wait(timeout=timeout)
                    except subprocess.TimeoutExpired:
                        process.kill();process.wait(timeout=15);row.update(status='uncertain',reason='超时保留预留，不自动重发');code=None
                row.update(exitCode=code,elapsedMs=round((time.monotonic()-started)*1000),budgetAfter=budget_state(folder,manifest))
                if row['status']=='uncertain':break
                if code!=0 or not (case_dir/'result.json').is_file():row.update(status='failed',reason='原生入口失败，停止后续案例');break
                result=benchmark.load_json(case_dir/'result.json');row['resultSha256']=file_hash(case_dir/'result.json')
                row['rawResponsesSha256']=file_hash(case_dir/'result.json.responses.jsonl')
                row['stdoutSha256']=file_hash(case_dir/'stdout.log');row['stderrSha256']=file_hash(case_dir/'stderr.log')
                row['coverage']=validate_result(folder,record,case_dir,result,mode)
                if row['coverage'].get('executionFailureKind'):
                    row.update(status='failed', reason='原生主控确定性失败：' + row['coverage']['executionFailureKind'])
                    break
                row['status']='preflight' if mode=='preflight' else 'completed'
            except (Exception, KeyboardInterrupt) as error:
                row.update(status='uncertain',reason='执行或保存异常：'+type(error).__name__);break
            finally:
                # Popen之后的任何保存/等待故障都清理本人子进程；不得让它继续发送下一轮。
                if process is not None and process.poll() is None:
                    process.kill();process.wait(timeout=15)
                    row.update(status='uncertain',reason='父执行器未完成，已停止本人子进程，保留预留')
                row['endedAt']=now();atomic_save(folder/'run.json',run)
        run['status']='completed' if len(run['records'])==len(manifest['records']) and all(row['status'] in {'completed','preflight'} for row in run['records']) else run['records'][-1]['status']
        run['updatedAt']=now();atomic_save(folder/'run.json',run);return inspect(folder)


def inspect(folder):
    folder=Path(folder).resolve();manifest=checked_plan(folder);run=benchmark.load_json(folder/'run.json');checked_run(folder,manifest,run);state=budget_state(folder,manifest)
    return {'folder':str(folder),'status':run['status'],'mode':run['mode'],'selectedCases':len(manifest['records']),
            'finishedCases':sum(row['status'] in {'completed','preflight'} for row in run['records']),
            'uncertainCases':[row['caseId'] for row in run['records'] if row['status']=='uncertain'],'budget':state,
            'observedTurns':sum(row.get('coverage',{}).get('observedTurns',0) for row in run['records']),
            'expectedTurns':sum(row['expectedTurns'] for row in manifest['records']),
            'allTurnsExecuted':run['mode'] in {'fixture','live'} and run['status']=='completed' and
                len(run['records'])==len(manifest['records']),
            'paidModelRequests':0 if run['mode']!='live' else None,'acceptanceComplete':False,'qualityMetrics':None,'reviewStatus':manifest['reviewStatus']}


def main():
    parser=argparse.ArgumentParser(description=__doc__);sub=parser.add_subparsers(dest='command',required=True)
    plan=sub.add_parser('prepare');plan.add_argument('--dataset',type=Path,default=benchmark.DEFAULT_DATASET);plan.add_argument('--driver',type=Path,required=True)
    plan.add_argument('--output',type=Path,required=True);plan.add_argument('--case',action='append');plan.add_argument('--token-limit',type=int,default=100000);plan.add_argument('--request-limit',type=int,default=30)
    for name in ('execute','inspect'):
        command=sub.add_parser(name);command.add_argument('folder',type=Path)
        if name=='execute':
            command.add_argument('--mode',choices=['preflight','fixture','live'],default='preflight');command.add_argument('--fixture-url')
            command.add_argument('--authorization',type=Path);command.add_argument('--configuration',type=Path);command.add_argument('--timeout',type=int,default=180)
    args=parser.parse_args()
    try:
        if args.command=='prepare':value=prepare(args.dataset,args.driver,args.output,args.case,args.token_limit,args.request_limit)
        elif args.command=='execute':value=execute(args.folder,args.mode,args.fixture_url,args.authorization,args.configuration,args.timeout)
        else:value=inspect(args.folder)
        print(json.dumps(value,ensure_ascii=False,indent=2));return 0 if value['status'] in {'prepared','completed'} else 1
    except (ValueError,OSError,RuntimeError,KeyError,TypeError) as error:
        print('评测未执行或已停止：'+type(error).__name__,file=sys.stderr);return 2


if __name__=='__main__':raise SystemExit(main())
