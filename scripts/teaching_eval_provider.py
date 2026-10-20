"""Promptfoo 的 Python 适配器：调用实际 C++ 主控，不重写教学逻辑。"""
import ctypes
from ctypes import wintypes
import json
import os
from pathlib import Path
import subprocess
import time
import uuid
import hashlib
from evaluation_guard import (SCHEMA, budget_copy, digest, file_hash, isolated_output,
                              local_fixture_url, require_live_authorization, summarize, successful,
                              BudgetStateError, read_budget)
from evaluation_guard import using_decision


def reviewed_criteria(case):
    """只有材料校验值和案例正文均匹配时，沿用用户已经确认的标准。"""
    root = Path(__file__).parents[1] / 'verification'
    try:
        source = (root / 'cases.json').read_bytes()
        receipt = json.loads((root / 'criteria-review-status.json').read_text(encoding='utf-8'))
        approved = receipt.get('status') == '用户人工核对通过并确认采用'
        if approved and receipt.get('materialSha256') == hashlib.sha256(source).hexdigest():
            if any(saved == case for saved in json.loads(source)):
                return {'status': receipt['status'], 'author': receipt['author'],
                        'materialSha256': receipt['materialSha256'],
                        'confirmationDate': receipt['confirmationDate']}
    except (OSError, ValueError, KeyError):
        pass
    return {'status': '待人工复核的AI候选材料'}


def configured_environment(config=None):
    """授权后按明确档案读取凭据；两版共用同一冻结的非密配置。"""
    approved = require_live_authorization()
    config = config or {}
    profile = config.get('profileName')
    if not isinstance(profile, str) or not profile or any(c in profile for c in '/\\'):
        raise ValueError('真实验收必须显式指定profileName。')
    frozen = json.loads(Path(config['frozenConfiguration']).read_text(encoding='utf-8-sig'))
    if frozen.get('profileName') != profile or not frozen.get('baseUrl') or not frozen.get('model'):
        raise ValueError('冻结模型配置与profileName不匹配。')
    if (approved.get('baseUrl') != frozen['baseUrl'] or approved.get('model') != frozen['model'] or
            not approved.get('activeBudgetPath') or not approved.get('searchBudgetPath')):
        raise PermissionError('模型配置与本轮冻结授权不一致。')
    env = os.environ.copy()
    env['AI_BASE_URL'], env['AI_MODEL'] = frozen['baseUrl'], frozen['model']
    class Credential(ctypes.Structure):
        _fields_ = [('Flags', wintypes.DWORD), ('Type', wintypes.DWORD), ('TargetName', wintypes.LPWSTR),
                    ('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME),
                    ('CredentialBlobSize', wintypes.DWORD), ('CredentialBlob', ctypes.POINTER(ctypes.c_ubyte)),
                    ('Persist', wintypes.DWORD), ('AttributeCount', wintypes.DWORD), ('Attributes', ctypes.c_void_p),
                    ('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]
    pointer = ctypes.POINTER(Credential)()
    api = ctypes.WinDLL('Advapi32', use_last_error=True)
    api.CredReadW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.POINTER(ctypes.POINTER(Credential))]
    api.CredReadW.restype = wintypes.BOOL
    api.CredFree.argtypes = [ctypes.c_void_p]
    require_live_authorization()
    if not api.CredReadW(profile+'/APIKey', 1, 0, ctypes.byref(pointer)):
        raise RuntimeError('当前原生模型凭据不可读，未调用模型。')
    try:
        record = pointer.contents
        env['AI_API_KEY'] = ctypes.string_at(record.CredentialBlob, record.CredentialBlobSize).decode('utf-16-le')
    finally:
        api.CredFree(pointer)
    return env


def call_api(prompt, options, context):
    with using_decision(options['config'].get('decisionFile')):
        return _call_api(prompt, options, context)


def _call_api(prompt, options, context):
    config = options['config']
    mode = config.get('mode', 'live')
    if mode not in ('live', 'fixture', 'preflight'):
        raise ValueError('mode只接受live、fixture或preflight。')
    if mode == 'live':
        require_live_authorization()
        if config.get('maxOutputTokens') is not None:
            raise ValueError('真实对照保持产品输出上限，禁止另行缩减。')
    case = context['vars']['case']
    if mode == 'live' and case.get('fixtureMode'):
        raise ValueError('真实对照禁止固定模型演示模式。')
    variant = config['variant']
    if variant not in ('baseline', 'optimized', 'candidate'):
        raise ValueError('variant只接受baseline、optimized或candidate。')
    criteria = reviewed_criteria(case)
    harness_hash = hashlib.sha256((Path(__file__).parents[1]/'tests'/'teaching_eval_driver.cpp').read_bytes()).hexdigest()
    driver_hash = hashlib.sha256(Path(config['driver']).read_bytes()).hexdigest()
    if not isinstance(case.get('id'), str) or Path(case['id']).name != case['id'] or any(c in case['id'] for c in '/\\:'):
        raise ValueError('案例id必须是单一文件名。')
    root = isolated_output(config['outputDir']) / f"{case['id']}-{variant}-{uuid.uuid4().hex[:8]}"
    root.mkdir(parents=True)
    actual = json.loads(json.dumps(case, ensure_ascii=False))
    profile = config.get('profileName')
    if not isinstance(profile, str) or not profile or any(c in profile for c in '/\\'):
        raise ValueError('验收必须显式指定profileName。')
    if config.get('preferencesFile'):
        actual['preferences'] = json.loads(Path(config['preferencesFile']).read_text(encoding='utf-8'))
    protocol = config.get('entryProtocol', 'native')
    if protocol not in ('native', 'legacy-chat'):
        raise ValueError('未知入口协议。')
    if protocol == 'legacy-chat':
        if variant != 'baseline' or not config.get('sourceCommit') or str(config['sourceCommit']).startswith('effb401'):
            raise ValueError('legacy-chat只适用于明确指定的旧基线；v5.6.3使用原生协议。')
        # 只有明确选用不支持教学请求的旧协议才映射，v5.6.3默认保持原生事件。
        for event in actual['turns']:
            if event['type'] == 'teaching_request':
                event['type'] = 'chat'
                event.pop('requestType', None)
    if mode == 'live':
        env = configured_environment(config)
    else:
        env = dict(os.environ, AI_BASE_URL=local_fixture_url(config['fixtureUrl']) if mode == 'fixture' else 'http://127.0.0.1:1/v1',
                   AI_MODEL=config.get('model', 'isolated-fixture'), AI_API_KEY='isolated-fixture-key')
    # 禁止继承可能污染复现的主机开关及外部检索凭据。
    for key in ('GANGYI_EVAL_MAX_CALLS', 'GANGYI_EVAL_MAX_OUTPUT_TOKENS', 'GANGYI_EVAL_RESUME',
                'GANGYI_EVAL_MODE', 'GANGYI_EVAL_DECISION_FILE', 'GANGYI_EVAL_SEARCH_BUDGET_FILE'):
        env.pop(key, None)
    env['BOCHA_API_KEY'] = ''
    for key in ('DEEPSEEK_SEARCH_API_KEY', 'DEEPSEEK_API_KEY'):
        env[key] = ''
    if mode == 'live':
        approved = require_live_authorization()
        if not config.get('decisionFile') or not approved.get('activeBudgetPath'):
            raise PermissionError('真实验收必须显式提供本轮授权及唯一累计账本。')
        env['GANGYI_EVAL_MODE'] = 'live'
        env['GANGYI_EVAL_DECISION_FILE'] = str(Path(config['decisionFile']).resolve())
        env['GANGYI_EVAL_BUDGET_FILE'] = approved['activeBudgetPath']
        if not approved.get('searchBudgetPath'):
            raise PermissionError('真实验收须绑定双搜索累计预算。')
    else:
        env['GANGYI_EVAL_BUDGET_FILE'] = str(budget_copy(root/'budget-copy.json', config.get('budgetFile')))
    for field,key in [('maxCalls','GANGYI_EVAL_MAX_CALLS'),('maxOutputTokens','GANGYI_EVAL_MAX_OUTPUT_TOKENS')]:
        if field in config:
            if type(config[field]) is not int or config[field] < 0:
                raise ValueError(field+'必须是非负整数。')
            env[key]=str(config[field])
    if mode == 'preflight':
        env['GANGYI_EVAL_MAX_CALLS'] = '0'
        actual['capturePrompts'] = True
        actual.pop('fixtureMode', None)
    resources = {}
    if actual.get('libraryDatabase'):
        resources = {name: file_hash(actual['libraryDatabase']+suffix) for name,suffix in
                     [('database',''), ('wal','-wal')] if Path(actual['libraryDatabase']+suffix).exists()}
    identity = {'schema': SCHEMA, 'case': actual, 'model': env['AI_MODEL'], 'baseUrl': env['AI_BASE_URL'],
                'profileName': profile, 'mode': mode, 'variant': variant, 'entryProtocol': protocol,
                'sourceCommit': config.get('sourceCommit'), 'resources': resources,
                'harnessSha256': harness_hash, 'driverSha256': driver_hash,
                'maxCalls': config.get('maxCalls'), 'maxOutputTokens': config.get('maxOutputTokens'),
                'configurationSha256': file_hash(config['frozenConfiguration']) if mode == 'live' else None}
    reuse_key = digest(identity)
    reuse = config.get('reuseDir')
    if reuse and variant == 'baseline':
        for previous in Path(reuse).glob(case['id']+'-baseline-*'):
            old_result = previous/'result.json'
            old_case = previous/'case.json'
            if old_result.exists() and old_case.exists() and json.loads(old_case.read_text(encoding='utf-8')) == actual:
                value = json.loads(old_result.read_text(encoding='utf-8'))
                if (value.get('reuseKey') == reuse_key and config.get('sourceCommit') and value.get('complete') is True and
                    value.get('processExitCode') == 0 and
                    successful(value, len(actual['turns']))):
                    value['baselineReused'] = True
                    value['criteriaStatus'] = criteria['status']
                    value['criteriaReview'] = criteria
                    output_file = root/'result.json'
                    (root/'case.json').write_text(json.dumps(actual,ensure_ascii=False),encoding='utf-8')
                    output_file.write_text(json.dumps(value,ensure_ascii=False,indent=2),encoding='utf-8')
                    return {'output':json.dumps(value,ensure_ascii=False),
                            'budgetStatus':read_budget(env['GANGYI_EVAL_BUDGET_FILE']),
                            'metadata':{'baselineReused':True,'originalRecordPath':str(old_result)}}
    case_file, output_file = root / 'case.json', root / 'result.json'
    case_file.write_text(json.dumps(actual, ensure_ascii=False), encoding='utf-8')
    started = time.monotonic()
    if mode == 'live':
        require_live_authorization()
    timed_out = False
    budget_status = None
    try:
        try:
            process = subprocess.run([config['driver'], str(case_file), str(root / 'isolated.db'), str(output_file)],
                                     env=env, capture_output=True, timeout=1800)
        except subprocess.TimeoutExpired:
            timed_out = True
            process = subprocess.CompletedProcess([], -1)
        finally:
            # 即使启动、超时或结果处理异常，也先独立确认预算；失败时禁止下一例。
            budget_status = read_budget(env['GANGYI_EVAL_BUDGET_FILE'])
        if process.returncode == 73:
            raise BudgetStateError('原生驱动预算保存失败；旧预算不是新结算，停止后续请求。')
        if process.returncode and not output_file.exists():
            raise RuntimeError('实际主控验收进程失败，退出码：'+str(process.returncode))
        result = json.loads(output_file.read_text(encoding='utf-8'))
        result['variant'] = variant
        result['model'] = env['AI_MODEL']
        result['durationSeconds'] = round(time.monotonic()-started, 3)
        result['recordPath'] = str(output_file)
        result['harnessSha256'] = harness_hash
        result['driverSha256'] = driver_hash
        result['entryProtocol'] = protocol
        result['mode'] = mode
        result['reuseKey'] = reuse_key
        result['reuseIdentity'] = identity
        result['fixtureOnly'] = mode != 'live'
        result['processExitCode'] = process.returncode
        result['timedOut'] = timed_out
        result['complete'] = process.returncode == 0 and successful(result, len(actual['turns']))
        result['summary'] = summarize(result)
        result['caseSha256'] = hashlib.sha256(json.dumps(actual,ensure_ascii=False,sort_keys=True).encode()).hexdigest()
        result['criteriaStatus'] = criteria['status']
        result['criteriaReview'] = criteria
        result['budgetStatus'] = budget_status
        output_file.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
        rows, key = (result['httpRequests'], 'tokens') if 'httpRequests' in result else (result['usage'], 'tokenUsage')
        known = [u[key] for u in rows if isinstance(u.get(key), dict) and isinstance(u[key].get('total_tokens'), int)]
        total = sum(u.get('total_tokens', 0) for u in known)
        response = {'output': json.dumps(result, ensure_ascii=False), 'budgetStatus': budget_status,
                    'metadata': {'recordPath': str(output_file)}}
        if known and result['summary']['unknownUsageRequests'] == 0:
            response['tokenUsage'] = {'total': total, 'prompt': sum(u.get('prompt_tokens', 0) for u in known),
                                      'completion': sum(u.get('completion_tokens', 0) for u in known)}
        return response
    except BudgetStateError:
        raise
    except Exception as error:
        # 错误原文可能含第三方凭据，只回传类型；预算回执不依赖结果JSON或报告能否保存。
        failed = {'error': '验收进程或结果处理失败：'+type(error).__name__, 'complete': False,
                  'timedOut': timed_out, 'budgetStatus': budget_status, 'metadata': {'recordPath': str(root)}}
        try:
            (root/'run-error.json').write_text(json.dumps(failed, ensure_ascii=False, indent=2), encoding='utf-8')
        except OSError:
            failed['reportWriteFailed'] = True
        return failed
