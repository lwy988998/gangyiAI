# 【逐行批注】 执行当前 Python 语句："""Promptfoo 的 Python 适配器：调用实际 C++ 主控，不重写教学逻辑。"""。
"""Promptfoo 的 Python 适配器：调用实际 C++ 主控，不重写教学逻辑。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import ctypes。
import ctypes
# 【逐行批注】 导入后续逻辑需要的模块或符号：from ctypes import wintypes。
from ctypes import wintypes
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：import uuid。
import uuid
# 【逐行批注】 导入后续逻辑需要的模块或符号：import hashlib。
import hashlib
# 【逐行批注】 导入后续逻辑需要的模块或符号：from evaluation_guard import (SCHEMA, budget_copy, digest, file_hash, isolated_…。
from evaluation_guard import (SCHEMA, budget_copy, digest, file_hash, isolated_output,
                              # 【逐行批注】 执行当前 Python 语句：local_fixture_url, require_live_authorization, summarize, successful,。
                              local_fixture_url, require_live_authorization, summarize, successful,
                              # 【逐行批注】 执行当前 Python 语句：BudgetStateError, read_budget)。
                              BudgetStateError, read_budget)
# 【逐行批注】 导入后续逻辑需要的模块或符号：from evaluation_guard import using_decision。
from evaluation_guard import using_decision


# 【逐行批注】 定义函数 reviewed_criteria，封装可复用的处理步骤。
def reviewed_criteria(case):
    # 【逐行批注】 执行当前 Python 语句："""只有材料校验值和案例正文均匹配时，沿用用户已经确认的标准。"""。
    """只有材料校验值和案例正文均匹配时，沿用用户已经确认的标准。"""
    # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
    root = Path(__file__).parents[1] / 'verification'
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
        source = (root / 'cases.json').read_bytes()
        # 【逐行批注】 计算右侧结果并赋给 receipt，更新脚本状态。
        receipt = json.loads((root / 'criteria-review-status.json').read_text(encoding='utf-8'))
        # 【逐行批注】 计算右侧结果并赋给 approved，更新脚本状态。
        approved = receipt.get('status') == '用户人工核对通过并确认采用'
        # 【逐行批注】 判断条件 approved and receipt.get('materialSha256') == hashlib.sha256(sou…，成立时执行对应分支。
        if approved and receipt.get('materialSha256') == hashlib.sha256(source).hexdigest():
            # 【逐行批注】 判断条件 any(saved == case for saved in json.loads(source)):，成立时执行对应分支。
            if any(saved == case for saved in json.loads(source)):
                # 【逐行批注】 结束当前函数并返回结果：return {'status': receipt['status'], 'author': receipt['author'],。
                return {'status': receipt['status'], 'author': receipt['author'],
                        # 【逐行批注】 执行当前 Python 语句：'materialSha256': receipt['materialSha256'],。
                        'materialSha256': receipt['materialSha256'],
                        # 【逐行批注】 执行当前 Python 语句：'confirmationDate': receipt['confirmationDate']}。
                        'confirmationDate': receipt['confirmationDate']}
    # 【逐行批注】 捕获并处理异常：except (OSError, ValueError, KeyError):。
    except (OSError, ValueError, KeyError):
        # 【逐行批注】 执行当前 Python 语句：pass。
        pass
    # 【逐行批注】 结束当前函数并返回结果：return {'status': '待人工复核的AI候选材料'}。
    return {'status': '待人工复核的AI候选材料'}


# 【逐行批注】 定义函数 configured_environment，封装可复用的处理步骤。
def configured_environment(config=None):
    # 【逐行批注】 执行当前 Python 语句："""授权后按明确档案读取凭据；两版共用同一冻结的非密配置。"""。
    """授权后按明确档案读取凭据；两版共用同一冻结的非密配置。"""
    # 【逐行批注】 计算右侧结果并赋给 approved，更新脚本状态。
    approved = require_live_authorization()
    # 【逐行批注】 计算右侧结果并赋给 config，更新脚本状态。
    config = config or {}
    # 【逐行批注】 计算右侧结果并赋给 profile，更新脚本状态。
    profile = config.get('profileName')
    # 【逐行批注】 判断条件 not isinstance(profile, str) or not profile or any(c in profile …，成立时执行对应分支。
    if not isinstance(profile, str) or not profile or any(c in profile for c in '/\\'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('真实验收必须显式指定profileName。')。
        raise ValueError('真实验收必须显式指定profileName。')
    # 【逐行批注】 计算右侧结果并赋给 frozen，更新脚本状态。
    frozen = json.loads(Path(config['frozenConfiguration']).read_text(encoding='utf-8-sig'))
    # 【逐行批注】 判断条件 frozen.get('profileName') != profile or not frozen.get('baseUrl'…，成立时执行对应分支。
    if frozen.get('profileName') != profile or not frozen.get('baseUrl') or not frozen.get('model'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('冻结模型配置与profileName不匹配。')。
        raise ValueError('冻结模型配置与profileName不匹配。')
    # 【逐行批注】 判断条件 (approved.get('baseUrl') != frozen['baseUrl'] or approved.get('m…，成立时执行对应分支。
    if (approved.get('baseUrl') != frozen['baseUrl'] or approved.get('model') != frozen['model'] or
            # 【逐行批注】 执行当前 Python 语句：not approved.get('activeBudgetPath') or not approved.get('searchBudgetPath')):。
            not approved.get('activeBudgetPath') or not approved.get('searchBudgetPath')):
        # 【逐行批注】 在错误情形抛出异常：raise PermissionError('模型配置与本轮冻结授权不一致。')。
        raise PermissionError('模型配置与本轮冻结授权不一致。')
    # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
    env = os.environ.copy()
    # 【逐行批注】 计算右侧结果并赋给 env['AI_BASE_URL'], env['AI_MODEL']，更新脚本状态。
    env['AI_BASE_URL'], env['AI_MODEL'] = frozen['baseUrl'], frozen['model']
    # 【逐行批注】 定义类 Credential，组织相关数据与行为。
    class Credential(ctypes.Structure):
        # 【逐行批注】 计算右侧结果并赋给 _fields_，更新脚本状态。
        _fields_ = [('Flags', wintypes.DWORD), ('Type', wintypes.DWORD), ('TargetName', wintypes.LPWSTR),
                    # 【逐行批注】 执行当前 Python 语句：('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME),。
                    ('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME),
                    # 【逐行批注】 执行当前 Python 语句：('CredentialBlobSize', wintypes.DWORD), ('CredentialBlob', ctypes.POINTER(ctypes.c_ubyte)…。
                    ('CredentialBlobSize', wintypes.DWORD), ('CredentialBlob', ctypes.POINTER(ctypes.c_ubyte)),
                    # 【逐行批注】 执行当前 Python 语句：('Persist', wintypes.DWORD), ('AttributeCount', wintypes.DWORD), ('Attributes', ctypes.c_…。
                    ('Persist', wintypes.DWORD), ('AttributeCount', wintypes.DWORD), ('Attributes', ctypes.c_void_p),
                    # 【逐行批注】 执行当前 Python 语句：('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]。
                    ('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]
    # 【逐行批注】 计算右侧结果并赋给 pointer，更新脚本状态。
    pointer = ctypes.POINTER(Credential)()
    # 【逐行批注】 计算右侧结果并赋给 api，更新脚本状态。
    api = ctypes.WinDLL('Advapi32', use_last_error=True)
    # 【逐行批注】 计算右侧结果并赋给 api.CredReadW.argtypes，更新脚本状态。
    api.CredReadW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.POINTER(ctypes.POINTER(Credential))]
    # 【逐行批注】 计算右侧结果并赋给 api.CredReadW.restype，更新脚本状态。
    api.CredReadW.restype = wintypes.BOOL
    # 【逐行批注】 计算右侧结果并赋给 api.CredFree.argtypes，更新脚本状态。
    api.CredFree.argtypes = [ctypes.c_void_p]
    # 【逐行批注】 调用 require_live_authorization 完成当前脚本或测试步骤：require_live_authorization()。
    require_live_authorization()
    # 【逐行批注】 判断条件 not api.CredReadW(profile+'/APIKey', 1, 0, ctypes.byref(pointer)…，成立时执行对应分支。
    if not api.CredReadW(profile+'/APIKey', 1, 0, ctypes.byref(pointer)):
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('当前原生模型凭据不可读，未调用模型。')。
        raise RuntimeError('当前原生模型凭据不可读，未调用模型。')
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 record，更新脚本状态。
        record = pointer.contents
        # 【逐行批注】 计算右侧结果并赋给 env['AI_API_KEY']，更新脚本状态。
        env['AI_API_KEY'] = ctypes.string_at(record.CredentialBlob, record.CredentialBlobSize).decode('utf-16-le')
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：api.CredFree(pointer)。
        api.CredFree(pointer)
    # 【逐行批注】 结束当前函数并返回结果：return env。
    return env


# 【逐行批注】 定义函数 call_api，封装可复用的处理步骤。
def call_api(prompt, options, context):
    # 【逐行批注】 在受控上下文中执行资源操作：with using_decision(options['config'].get('decisionFile')):。
    with using_decision(options['config'].get('decisionFile')):
        # 【逐行批注】 结束当前函数并返回结果：return _call_api(prompt, options, context)。
        return _call_api(prompt, options, context)


# 【逐行批注】 定义函数 _call_api，封装可复用的处理步骤。
def _call_api(prompt, options, context):
    # 【逐行批注】 计算右侧结果并赋给 config，更新脚本状态。
    config = options['config']
    # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
    mode = config.get('mode', 'live')
    # 【逐行批注】 判断条件 mode not in ('live', 'fixture', 'preflight'):，成立时执行对应分支。
    if mode not in ('live', 'fixture', 'preflight'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('mode只接受live、fixture或preflight。')。
        raise ValueError('mode只接受live、fixture或preflight。')
    # 【逐行批注】 判断条件 mode == 'live':，成立时执行对应分支。
    if mode == 'live':
        # 【逐行批注】 调用 require_live_authorization 完成当前脚本或测试步骤：require_live_authorization()。
        require_live_authorization()
        # 【逐行批注】 判断条件 config.get('maxOutputTokens') is not None:，成立时执行对应分支。
        if config.get('maxOutputTokens') is not None:
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('真实对照保持产品输出上限，禁止另行缩减。')。
            raise ValueError('真实对照保持产品输出上限，禁止另行缩减。')
    # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
    case = context['vars']['case']
    # 【逐行批注】 判断条件 mode == 'live' and case.get('fixtureMode'):，成立时执行对应分支。
    if mode == 'live' and case.get('fixtureMode'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('真实对照禁止固定模型演示模式。')。
        raise ValueError('真实对照禁止固定模型演示模式。')
    # 【逐行批注】 计算右侧结果并赋给 variant，更新脚本状态。
    variant = config['variant']
    # 【逐行批注】 判断条件 variant not in ('baseline', 'optimized', 'candidate'):，成立时执行对应分支。
    if variant not in ('baseline', 'optimized', 'candidate'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('variant只接受baseline、optimized或candidate。')。
        raise ValueError('variant只接受baseline、optimized或candidate。')
    # 【逐行批注】 计算右侧结果并赋给 criteria，更新脚本状态。
    criteria = reviewed_criteria(case)
    # 【逐行批注】 计算右侧结果并赋给 harness_hash，更新脚本状态。
    harness_hash = hashlib.sha256((Path(__file__).parents[1]/'tests'/'teaching_eval_driver.cpp').read_bytes()).hexdigest()
    # 【逐行批注】 计算右侧结果并赋给 driver_hash，更新脚本状态。
    driver_hash = hashlib.sha256(Path(config['driver']).read_bytes()).hexdigest()
    # 【逐行批注】 判断条件 not isinstance(case.get('id'), str) or Path(case['id']).name != …，成立时执行对应分支。
    if not isinstance(case.get('id'), str) or Path(case['id']).name != case['id'] or any(c in case['id'] for c in '/\\:'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('案例id必须是单一文件名。')。
        raise ValueError('案例id必须是单一文件名。')
    # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
    root = isolated_output(config['outputDir']) / f"{case['id']}-{variant}-{uuid.uuid4().hex[:8]}"
    # 【逐行批注】 计算右侧结果并赋给 root.mkdir(parents，更新脚本状态。
    root.mkdir(parents=True)
    # 【逐行批注】 计算右侧结果并赋给 actual，更新脚本状态。
    actual = json.loads(json.dumps(case, ensure_ascii=False))
    # 【逐行批注】 计算右侧结果并赋给 profile，更新脚本状态。
    profile = config.get('profileName')
    # 【逐行批注】 判断条件 not isinstance(profile, str) or not profile or any(c in profile …，成立时执行对应分支。
    if not isinstance(profile, str) or not profile or any(c in profile for c in '/\\'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('验收必须显式指定profileName。')。
        raise ValueError('验收必须显式指定profileName。')
    # 【逐行批注】 判断条件 config.get('preferencesFile'):，成立时执行对应分支。
    if config.get('preferencesFile'):
        # 【逐行批注】 计算右侧结果并赋给 actual['preferences']，更新脚本状态。
        actual['preferences'] = json.loads(Path(config['preferencesFile']).read_text(encoding='utf-8'))
    # 【逐行批注】 计算右侧结果并赋给 protocol，更新脚本状态。
    protocol = config.get('entryProtocol', 'native')
    # 【逐行批注】 判断条件 protocol not in ('native', 'legacy-chat'):，成立时执行对应分支。
    if protocol not in ('native', 'legacy-chat'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('未知入口协议。')。
        raise ValueError('未知入口协议。')
    # 【逐行批注】 判断条件 protocol == 'legacy-chat':，成立时执行对应分支。
    if protocol == 'legacy-chat':
        # 【逐行批注】 判断条件 variant != 'baseline' or not config.get('sourceCommit') or str(c…，成立时执行对应分支。
        if variant != 'baseline' or not config.get('sourceCommit') or str(config['sourceCommit']).startswith('effb401'):
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('legacy-chat只适用于明确指定的旧基线；v5.6.3使用原生协议。')。
            raise ValueError('legacy-chat只适用于明确指定的旧基线；v5.6.3使用原生协议。')
        # 只有明确选用不支持教学请求的旧协议才映射，v5.6.3默认保持原生事件。
        # 【逐行批注】 遍历数据并重复执行循环体：for event in actual['turns']:。
        for event in actual['turns']:
            # 【逐行批注】 判断条件 event['type'] == 'teaching_request':，成立时执行对应分支。
            if event['type'] == 'teaching_request':
                # 【逐行批注】 计算右侧结果并赋给 event['type']，更新脚本状态。
                event['type'] = 'chat'
                # 【逐行批注】 执行当前 Python 语句：event.pop('requestType', None)。
                event.pop('requestType', None)
    # 【逐行批注】 判断条件 mode == 'live':，成立时执行对应分支。
    if mode == 'live':
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = configured_environment(config)
    # 【逐行批注】 执行前一条件不成立时的分支。
    else:
        # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
        env = dict(os.environ, AI_BASE_URL=local_fixture_url(config['fixtureUrl']) if mode == 'fixture' else 'http://127.0.0.1:1/v1',
                   # 【逐行批注】 计算右侧结果并赋给 AI_MODEL，更新脚本状态。
                   AI_MODEL=config.get('model', 'isolated-fixture'), AI_API_KEY='isolated-fixture-key')
    # 禁止继承可能污染复现的主机开关及外部检索凭据。
    # 【逐行批注】 遍历数据并重复执行循环体：for key in ('GANGYI_EVAL_MAX_CALLS', 'GANGYI_EVAL_MAX_OUTPUT_TOKENS', 'GAN…。
    for key in ('GANGYI_EVAL_MAX_CALLS', 'GANGYI_EVAL_MAX_OUTPUT_TOKENS', 'GANGYI_EVAL_RESUME',
                # 【逐行批注】 执行当前 Python 语句：'GANGYI_EVAL_MODE', 'GANGYI_EVAL_DECISION_FILE', 'GANGYI_EVAL_SEARCH_BUDGET_FILE'):。
                'GANGYI_EVAL_MODE', 'GANGYI_EVAL_DECISION_FILE', 'GANGYI_EVAL_SEARCH_BUDGET_FILE'):
        # 【逐行批注】 执行当前 Python 语句：env.pop(key, None)。
        env.pop(key, None)
    # 【逐行批注】 计算右侧结果并赋给 env['BOCHA_API_KEY']，更新脚本状态。
    env['BOCHA_API_KEY'] = ''
    # 【逐行批注】 遍历数据并重复执行循环体：for key in ('DEEPSEEK_SEARCH_API_KEY', 'DEEPSEEK_API_KEY'):。
    for key in ('DEEPSEEK_SEARCH_API_KEY', 'DEEPSEEK_API_KEY'):
        # 【逐行批注】 计算右侧结果并赋给 env[key]，更新脚本状态。
        env[key] = ''
    # 【逐行批注】 判断条件 mode == 'live':，成立时执行对应分支。
    if mode == 'live':
        # 【逐行批注】 计算右侧结果并赋给 approved，更新脚本状态。
        approved = require_live_authorization()
        # 【逐行批注】 判断条件 not config.get('decisionFile') or not approved.get('activeBudget…，成立时执行对应分支。
        if not config.get('decisionFile') or not approved.get('activeBudgetPath'):
            # 【逐行批注】 在错误情形抛出异常：raise PermissionError('真实验收必须显式提供本轮授权及唯一累计账本。')。
            raise PermissionError('真实验收必须显式提供本轮授权及唯一累计账本。')
        # 【逐行批注】 计算右侧结果并赋给 env['GANGYI_EVAL_MODE']，更新脚本状态。
        env['GANGYI_EVAL_MODE'] = 'live'
        # 【逐行批注】 计算右侧结果并赋给 env['GANGYI_EVAL_DECISION_FILE']，更新脚本状态。
        env['GANGYI_EVAL_DECISION_FILE'] = str(Path(config['decisionFile']).resolve())
        # 【逐行批注】 计算右侧结果并赋给 env['GANGYI_EVAL_BUDGET_FILE']，更新脚本状态。
        env['GANGYI_EVAL_BUDGET_FILE'] = approved['activeBudgetPath']
        # 【逐行批注】 判断条件 not approved.get('searchBudgetPath'):，成立时执行对应分支。
        if not approved.get('searchBudgetPath'):
            # 【逐行批注】 在错误情形抛出异常：raise PermissionError('真实验收须绑定双搜索累计预算。')。
            raise PermissionError('真实验收须绑定双搜索累计预算。')
    # 【逐行批注】 执行前一条件不成立时的分支。
    else:
        # 【逐行批注】 计算右侧结果并赋给 env['GANGYI_EVAL_BUDGET_FILE']，更新脚本状态。
        env['GANGYI_EVAL_BUDGET_FILE'] = str(budget_copy(root/'budget-copy.json', config.get('budgetFile')))
    # 【逐行批注】 遍历数据并重复执行循环体：for field,key in [('maxCalls','GANGYI_EVAL_MAX_CALLS'),('maxOutputTokens',…。
    for field,key in [('maxCalls','GANGYI_EVAL_MAX_CALLS'),('maxOutputTokens','GANGYI_EVAL_MAX_OUTPUT_TOKENS')]:
        # 【逐行批注】 判断条件 field in config:，成立时执行对应分支。
        if field in config:
            # 【逐行批注】 判断条件 type(config[field]) is not int or config[field] < 0:，成立时执行对应分支。
            if type(config[field]) is not int or config[field] < 0:
                # 【逐行批注】 在错误情形抛出异常：raise ValueError(field+'必须是非负整数。')。
                raise ValueError(field+'必须是非负整数。')
            # 【逐行批注】 计算右侧结果并赋给 env[key]，更新脚本状态。
            env[key]=str(config[field])
    # 【逐行批注】 判断条件 mode == 'preflight':，成立时执行对应分支。
    if mode == 'preflight':
        # 【逐行批注】 计算右侧结果并赋给 env['GANGYI_EVAL_MAX_CALLS']，更新脚本状态。
        env['GANGYI_EVAL_MAX_CALLS'] = '0'
        # 【逐行批注】 计算右侧结果并赋给 actual['capturePrompts']，更新脚本状态。
        actual['capturePrompts'] = True
        # 【逐行批注】 执行当前 Python 语句：actual.pop('fixtureMode', None)。
        actual.pop('fixtureMode', None)
    # 【逐行批注】 计算右侧结果并赋给 resources，更新脚本状态。
    resources = {}
    # 【逐行批注】 判断条件 actual.get('libraryDatabase'):，成立时执行对应分支。
    if actual.get('libraryDatabase'):
        # 【逐行批注】 计算右侧结果并赋给 resources，更新脚本状态。
        resources = {name: file_hash(actual['libraryDatabase']+suffix) for name,suffix in
                     # 【逐行批注】 调用 Path 完成当前脚本或测试步骤：[('database',''), ('wal','-wal')] if Path(actual['libraryDatabase']+s…。
                     [('database',''), ('wal','-wal')] if Path(actual['libraryDatabase']+suffix).exists()}
    # 【逐行批注】 计算右侧结果并赋给 identity，更新脚本状态。
    identity = {'schema': SCHEMA, 'case': actual, 'model': env['AI_MODEL'], 'baseUrl': env['AI_BASE_URL'],
                # 【逐行批注】 执行当前 Python 语句：'profileName': profile, 'mode': mode, 'variant': variant, 'entryProtocol': protocol,。
                'profileName': profile, 'mode': mode, 'variant': variant, 'entryProtocol': protocol,
                # 【逐行批注】 执行当前 Python 语句：'sourceCommit': config.get('sourceCommit'), 'resources': resources,。
                'sourceCommit': config.get('sourceCommit'), 'resources': resources,
                # 【逐行批注】 执行当前 Python 语句：'harnessSha256': harness_hash, 'driverSha256': driver_hash,。
                'harnessSha256': harness_hash, 'driverSha256': driver_hash,
                # 【逐行批注】 执行当前 Python 语句：'maxCalls': config.get('maxCalls'), 'maxOutputTokens': config.get('maxOutputTokens'),。
                'maxCalls': config.get('maxCalls'), 'maxOutputTokens': config.get('maxOutputTokens'),
                # 【逐行批注】 调用 file_hash 完成当前脚本或测试步骤：'configurationSha256': file_hash(config['frozenConfiguration']) if mo…。
                'configurationSha256': file_hash(config['frozenConfiguration']) if mode == 'live' else None}
    # 【逐行批注】 计算右侧结果并赋给 reuse_key，更新脚本状态。
    reuse_key = digest(identity)
    # 【逐行批注】 计算右侧结果并赋给 reuse，更新脚本状态。
    reuse = config.get('reuseDir')
    # 【逐行批注】 判断条件 reuse and variant == 'baseline':，成立时执行对应分支。
    if reuse and variant == 'baseline':
        # 【逐行批注】 遍历数据并重复执行循环体：for previous in Path(reuse).glob(case['id']+'-baseline-*'):。
        for previous in Path(reuse).glob(case['id']+'-baseline-*'):
            # 【逐行批注】 计算右侧结果并赋给 old_result，更新脚本状态。
            old_result = previous/'result.json'
            # 【逐行批注】 计算右侧结果并赋给 old_case，更新脚本状态。
            old_case = previous/'case.json'
            # 【逐行批注】 判断条件 old_result.exists() and old_case.exists() and json.loads(old_cas…，成立时执行对应分支。
            if old_result.exists() and old_case.exists() and json.loads(old_case.read_text(encoding='utf-8')) == actual:
                # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
                value = json.loads(old_result.read_text(encoding='utf-8'))
                # 【逐行批注】 判断条件 (value.get('reuseKey') == reuse_key and config.get('sourceCommit…，成立时执行对应分支。
                if (value.get('reuseKey') == reuse_key and config.get('sourceCommit') and value.get('complete') is True and
                    # 【逐行批注】 执行当前 Python 语句：value.get('processExitCode') == 0 and。
                    value.get('processExitCode') == 0 and
                    # 【逐行批注】 调用 successful、len 完成当前脚本或测试步骤：successful(value, len(actual['turns']))):。
                    successful(value, len(actual['turns']))):
                    # 【逐行批注】 计算右侧结果并赋给 value['baselineReused']，更新脚本状态。
                    value['baselineReused'] = True
                    # 【逐行批注】 计算右侧结果并赋给 value['criteriaStatus']，更新脚本状态。
                    value['criteriaStatus'] = criteria['status']
                    # 【逐行批注】 计算右侧结果并赋给 value['criteriaReview']，更新脚本状态。
                    value['criteriaReview'] = criteria
                    # 【逐行批注】 计算右侧结果并赋给 output_file，更新脚本状态。
                    output_file = root/'result.json'
                    # 【逐行批注】 计算右侧结果并赋给 (root/'case.json').write_text(json.dumps(act…，更新脚本状态。
                    (root/'case.json').write_text(json.dumps(actual,ensure_ascii=False),encoding='utf-8')
                    # 【逐行批注】 计算右侧结果并赋给 output_file.write_text(json.dumps(value,ensu…，更新脚本状态。
                    output_file.write_text(json.dumps(value,ensure_ascii=False,indent=2),encoding='utf-8')
                    # 【逐行批注】 结束当前函数并返回结果：return {'output':json.dumps(value,ensure_ascii=False),。
                    return {'output':json.dumps(value,ensure_ascii=False),
                            # 【逐行批注】 调用 read_budget 完成当前脚本或测试步骤：'budgetStatus':read_budget(env['GANGYI_EVAL_BUDGET_FILE']),。
                            'budgetStatus':read_budget(env['GANGYI_EVAL_BUDGET_FILE']),
                            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'metadata':{'baselineReused':True,'originalRecordPath':str(old_result…。
                            'metadata':{'baselineReused':True,'originalRecordPath':str(old_result)}}
    # 【逐行批注】 计算右侧结果并赋给 case_file, output_file，更新脚本状态。
    case_file, output_file = root / 'case.json', root / 'result.json'
    # 【逐行批注】 计算右侧结果并赋给 case_file.write_text(json.dumps(actual, ensu…，更新脚本状态。
    case_file.write_text(json.dumps(actual, ensure_ascii=False), encoding='utf-8')
    # 【逐行批注】 计算右侧结果并赋给 started，更新脚本状态。
    started = time.monotonic()
    # 【逐行批注】 判断条件 mode == 'live':，成立时执行对应分支。
    if mode == 'live':
        # 【逐行批注】 调用 require_live_authorization 完成当前脚本或测试步骤：require_live_authorization()。
        require_live_authorization()
    # 【逐行批注】 计算右侧结果并赋给 timed_out，更新脚本状态。
    timed_out = False
    # 【逐行批注】 计算右侧结果并赋给 budget_status，更新脚本状态。
    budget_status = None
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.run([config['driver'], str(case_file), str(root / 'isolated.db'), str(output_file)],
                                     # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                                     env=env, capture_output=True, timeout=1800)
        # 【逐行批注】 捕获并处理异常：except subprocess.TimeoutExpired:。
        except subprocess.TimeoutExpired:
            # 【逐行批注】 计算右侧结果并赋给 timed_out，更新脚本状态。
            timed_out = True
            # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
            process = subprocess.CompletedProcess([], -1)
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 即使启动、超时或结果处理异常，也先独立确认预算；失败时禁止下一例。
            # 【逐行批注】 计算右侧结果并赋给 budget_status，更新脚本状态。
            budget_status = read_budget(env['GANGYI_EVAL_BUDGET_FILE'])
        # 【逐行批注】 判断条件 process.returncode == 73:，成立时执行对应分支。
        if process.returncode == 73:
            # 【逐行批注】 在错误情形抛出异常：raise BudgetStateError('原生驱动预算保存失败；旧预算不是新结算，停止后续请求。')。
            raise BudgetStateError('原生驱动预算保存失败；旧预算不是新结算，停止后续请求。')
        # 【逐行批注】 判断条件 process.returncode and not output_file.exists():，成立时执行对应分支。
        if process.returncode and not output_file.exists():
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('实际主控验收进程失败，退出码：'+str(process.returncode))。
            raise RuntimeError('实际主控验收进程失败，退出码：'+str(process.returncode))
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = json.loads(output_file.read_text(encoding='utf-8'))
        # 【逐行批注】 计算右侧结果并赋给 result['variant']，更新脚本状态。
        result['variant'] = variant
        # 【逐行批注】 计算右侧结果并赋给 result['model']，更新脚本状态。
        result['model'] = env['AI_MODEL']
        # 【逐行批注】 计算右侧结果并赋给 result['durationSeconds']，更新脚本状态。
        result['durationSeconds'] = round(time.monotonic()-started, 3)
        # 【逐行批注】 计算右侧结果并赋给 result['recordPath']，更新脚本状态。
        result['recordPath'] = str(output_file)
        # 【逐行批注】 计算右侧结果并赋给 result['harnessSha256']，更新脚本状态。
        result['harnessSha256'] = harness_hash
        # 【逐行批注】 计算右侧结果并赋给 result['driverSha256']，更新脚本状态。
        result['driverSha256'] = driver_hash
        # 【逐行批注】 计算右侧结果并赋给 result['entryProtocol']，更新脚本状态。
        result['entryProtocol'] = protocol
        # 【逐行批注】 计算右侧结果并赋给 result['mode']，更新脚本状态。
        result['mode'] = mode
        # 【逐行批注】 计算右侧结果并赋给 result['reuseKey']，更新脚本状态。
        result['reuseKey'] = reuse_key
        # 【逐行批注】 计算右侧结果并赋给 result['reuseIdentity']，更新脚本状态。
        result['reuseIdentity'] = identity
        # 【逐行批注】 计算右侧结果并赋给 result['fixtureOnly']，更新脚本状态。
        result['fixtureOnly'] = mode != 'live'
        # 【逐行批注】 计算右侧结果并赋给 result['processExitCode']，更新脚本状态。
        result['processExitCode'] = process.returncode
        # 【逐行批注】 计算右侧结果并赋给 result['timedOut']，更新脚本状态。
        result['timedOut'] = timed_out
        # 【逐行批注】 计算右侧结果并赋给 result['complete']，更新脚本状态。
        result['complete'] = process.returncode == 0 and successful(result, len(actual['turns']))
        # 【逐行批注】 计算右侧结果并赋给 result['summary']，更新脚本状态。
        result['summary'] = summarize(result)
        # 【逐行批注】 计算右侧结果并赋给 result['caseSha256']，更新脚本状态。
        result['caseSha256'] = hashlib.sha256(json.dumps(actual,ensure_ascii=False,sort_keys=True).encode()).hexdigest()
        # 【逐行批注】 计算右侧结果并赋给 result['criteriaStatus']，更新脚本状态。
        result['criteriaStatus'] = criteria['status']
        # 【逐行批注】 计算右侧结果并赋给 result['criteriaReview']，更新脚本状态。
        result['criteriaReview'] = criteria
        # 【逐行批注】 计算右侧结果并赋给 result['budgetStatus']，更新脚本状态。
        result['budgetStatus'] = budget_status
        # 【逐行批注】 计算右侧结果并赋给 output_file.write_text(json.dumps(result, en…，更新脚本状态。
        output_file.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 rows, key，更新脚本状态。
        rows, key = (result['httpRequests'], 'tokens') if 'httpRequests' in result else (result['usage'], 'tokenUsage')
        # 【逐行批注】 计算右侧结果并赋给 known，更新脚本状态。
        known = [u[key] for u in rows if isinstance(u.get(key), dict) and isinstance(u[key].get('total_tokens'), int)]
        # 【逐行批注】 计算右侧结果并赋给 total，更新脚本状态。
        total = sum(u.get('total_tokens', 0) for u in known)
        # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
        response = {'output': json.dumps(result, ensure_ascii=False), 'budgetStatus': budget_status,
                    # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'metadata': {'recordPath': str(output_file)}}。
                    'metadata': {'recordPath': str(output_file)}}
        # 【逐行批注】 判断条件 known and result['summary']['unknownUsageRequests'] == 0:，成立时执行对应分支。
        if known and result['summary']['unknownUsageRequests'] == 0:
            # 【逐行批注】 计算右侧结果并赋给 response['tokenUsage']，更新脚本状态。
            response['tokenUsage'] = {'total': total, 'prompt': sum(u.get('prompt_tokens', 0) for u in known),
                                      # 【逐行批注】 调用 sum 完成当前脚本或测试步骤：'completion': sum(u.get('completion_tokens', 0) for u in known)}。
                                      'completion': sum(u.get('completion_tokens', 0) for u in known)}
        # 【逐行批注】 结束当前函数并返回结果：return response。
        return response
    # 【逐行批注】 捕获并处理异常：except BudgetStateError:。
    except BudgetStateError:
        # 【逐行批注】 在错误情形抛出异常：raise。
        raise
    # 【逐行批注】 捕获并处理异常：except Exception as error:。
    except Exception as error:
        # 错误原文可能含第三方凭据，只回传类型；预算回执不依赖结果JSON或报告能否保存。
        # 【逐行批注】 计算右侧结果并赋给 failed，更新脚本状态。
        failed = {'error': '验收进程或结果处理失败：'+type(error).__name__, 'complete': False,
                  # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'timedOut': timed_out, 'budgetStatus': budget_status, 'metadata': {'r…。
                  'timedOut': timed_out, 'budgetStatus': budget_status, 'metadata': {'recordPath': str(root)}}
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 (root/'run-error.json').write_text(json.dump…，更新脚本状态。
            (root/'run-error.json').write_text(json.dumps(failed, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 捕获并处理异常：except OSError:。
        except OSError:
            # 【逐行批注】 计算右侧结果并赋给 failed['reportWriteFailed']，更新脚本状态。
            failed['reportWriteFailed'] = True
        # 【逐行批注】 结束当前函数并返回结果：return failed。
        return failed
