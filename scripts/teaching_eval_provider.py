"""Promptfoo 的 Python 适配器：调用实际 C++ 主控，不重写教学逻辑。"""
import ctypes
from ctypes import wintypes
import json
import os
from pathlib import Path
import subprocess
import sqlite3
import time
import uuid
import winreg
from contextlib import closing
import hashlib


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


def configured_environment():
    """沿用原生启动器的当前模型及凭据，密钥只放在子进程环境。"""
    env = os.environ.copy()
    if all(env.get(k) for k in ('AI_BASE_URL', 'AI_MODEL', 'AI_API_KEY')):
        return env
    with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r'Software\GangyiAI') as key:
        env['AI_BASE_URL'] = winreg.QueryValueEx(key, 'AIBaseUrl')[0]
        env['AI_MODEL'] = winreg.QueryValueEx(key, 'AIModel')[0]
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
    if not api.CredReadW('GangyiAI/APIKey', 1, 0, ctypes.byref(pointer)):
        raise RuntimeError('当前原生模型凭据不可读，未调用模型。')
    try:
        record = pointer.contents
        env['AI_API_KEY'] = ctypes.string_at(record.CredentialBlob, record.CredentialBlobSize).decode('utf-16-le')
    finally:
        api.CredFree(pointer)
    return env


def call_api(prompt, options, context):
    config = options['config']
    case = context['vars']['case']
    variant = config['variant']
    criteria = reviewed_criteria(case)
    harness_hash = hashlib.sha256((Path(__file__).parents[1]/'tests'/'teaching_eval_driver.cpp').read_bytes()).hexdigest()
    driver_hash = hashlib.sha256(Path(config['driver']).read_bytes()).hexdigest()
    root = Path(config['outputDir']).resolve() / f"{case['id']}-{variant}-{uuid.uuid4().hex[:8]}"
    root.mkdir(parents=True)
    actual = json.loads(json.dumps(case, ensure_ascii=False))
    source = Path(os.environ['LOCALAPPDATA']) / 'GangyiAI' / 'data' / 'gangyiAI.db'
    if config.get('preferencesFile'):
        actual['preferences'] = json.loads(Path(config['preferencesFile']).read_text(encoding='utf-8'))
    elif source.exists():
        with closing(sqlite3.connect(source.as_uri()+'?mode=ro', uri=True)) as db:
            row = db.execute("SELECT payload FROM MyAIRecord WHERE id='preferences'").fetchone()
            if row:
                actual['preferences'] = json.loads(row[0])
    if variant == 'baseline':
        # 旧界面没有教学请求类型，按同样的学生要求经现有 chat 入口交给实际旧主控。
        for event in actual['turns']:
            if event['type'] == 'teaching_request':
                event['type'] = 'chat'
                event.pop('requestType', None)
    env = configured_environment()
    if config.get('budgetFile'):
        env['GANGYI_EVAL_BUDGET_FILE'] = str(Path(config['budgetFile']).resolve())
    for field,key in [('maxCalls','GANGYI_EVAL_MAX_CALLS'),('maxOutputTokens','GANGYI_EVAL_MAX_OUTPUT_TOKENS')]:
        if config.get(field):env[key]=str(config[field])
    reuse = config.get('reuseDir')
    if reuse and variant == 'baseline':
        for previous in Path(reuse).glob(case['id']+'-baseline-*'):
            old_result = previous/'result.json'
            old_case = previous/'case.json'
            if old_result.exists() and old_case.exists() and json.loads(old_case.read_text(encoding='utf-8')) == actual:
                value = json.loads(old_result.read_text(encoding='utf-8'))
                if (value.get('model') == env['AI_MODEL'] and value.get('harnessSha256') == harness_hash and
                    value.get('driverSha256') == driver_hash and len(value['turns']) == len(actual['turns']) and
                    all(turn['task']['status'] not in ('failed','superseded') for turn in value['turns'])):
                    value['baselineReused'] = True
                    value['criteriaStatus'] = criteria['status']
                    value['criteriaReview'] = criteria
                    output_file = root/'result.json'
                    output_file.write_text(json.dumps(value,ensure_ascii=False,indent=2),encoding='utf-8')
                    return {'output':json.dumps(value,ensure_ascii=False),'metadata':{'baselineReused':True,'originalRecordPath':str(old_result)}}
    case_file, output_file = root / 'case.json', root / 'result.json'
    case_file.write_text(json.dumps(actual, ensure_ascii=False), encoding='utf-8')
    started = time.monotonic()
    process = subprocess.run([config['driver'], str(case_file), str(root / 'isolated.db'), str(output_file)],
                             env=env, capture_output=True, timeout=1800)
    if process.returncode:
        return {'error': '实际主控验收进程失败：'+process.stderr.decode('utf-8', errors='replace')[-1500:]}
    result = json.loads(output_file.read_text(encoding='utf-8'))
    result['variant'] = variant
    result['model'] = env['AI_MODEL']
    result['durationSeconds'] = round(time.monotonic()-started, 3)
    result['recordPath'] = str(output_file)
    result['harnessSha256'] = harness_hash
    result['driverSha256'] = driver_hash
    result['caseSha256'] = hashlib.sha256(json.dumps(actual,ensure_ascii=False,sort_keys=True).encode()).hexdigest()
    result['criteriaStatus'] = criteria['status']
    result['criteriaReview'] = criteria
    if config.get('budgetFile'):
        result['budgetStatus'] = json.loads(Path(config['budgetFile']).read_text(encoding='utf-8'))
    output_file.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
    if 'httpRequests' in result:
        known = [u['tokens'] for u in result['httpRequests'] if isinstance(u.get('tokens'), dict)]
    else:
        known = [u['tokenUsage'] for u in result['usage'] if isinstance(u.get('tokenUsage'), dict)]
    total = sum(u.get('total_tokens', 0) for u in known)
    response = {'output': json.dumps(result, ensure_ascii=False), 'metadata': {'recordPath': str(output_file)}}
    if known:
        response['tokenUsage'] = {'total': total, 'prompt': sum(u.get('prompt_tokens', 0) for u in known),
                                  'completion': sum(u.get('completion_tokens', 0) for u in known)}
    return response
