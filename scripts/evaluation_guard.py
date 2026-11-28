# 【逐行批注】 执行当前 Python 语句："""验收入口共用的授权、隔离与统计规则；离线路径不读取凭据。"""。
"""验收入口共用的授权、隔离与统计规则；离线路径不读取凭据。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import hashlib。
import hashlib
# 【逐行批注】 导入后续逻辑需要的模块或符号：import ipaddress。
import ipaddress
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import contextmanager。
from contextlib import contextmanager
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextvars import ContextVar。
from contextvars import ContextVar
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import urlsplit。
from urllib.parse import urlsplit

# 【逐行批注】 计算右侧结果并赋给 REPO，更新脚本状态。
REPO = Path(__file__).resolve().parents[1]
# 【逐行批注】 计算右侧结果并赋给 DECISION，更新脚本状态。
DECISION = REPO / 'verification/v564-budget-decision.json'
# 【逐行批注】 计算右侧结果并赋给 FROZEN，更新脚本状态。
FROZEN = REPO / 'verification/v564-baseline'
# 【逐行批注】 计算右侧结果并赋给 SCHEMA，更新脚本状态。
SCHEMA = 'same-protocol-v564-r1'
# 【逐行批注】 计算右侧结果并赋给 TASK_METRICS，更新脚本状态。
TASK_METRICS = ('queueMs', 'firstContentMs', 'completionMs')
# 【逐行批注】 计算右侧结果并赋给 _decision_file，更新脚本状态。
_decision_file = ContextVar('evaluation_decision_file', default=None)


# 【逐行批注】 应用装饰器 @contextmanager，扩展下一函数或测试的行为。
@contextmanager
# 【逐行批注】 定义函数 using_decision，封装可复用的处理步骤。
def using_decision(path):
    # 【逐行批注】 执行当前 Python 语句："""显式选择本轮决定；退出后恢复，避免不同验收互相污染。"""。
    """显式选择本轮决定；退出后恢复，避免不同验收互相污染。"""
    # 【逐行批注】 计算右侧结果并赋给 token，更新脚本状态。
    token = _decision_file.set(path)
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 执行当前 Python 语句：yield。
        yield
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：_decision_file.reset(token)。
        _decision_file.reset(token)


# 【逐行批注】 定义类 BudgetStateError，组织相关数据与行为。
class BudgetStateError(RuntimeError):
    # 【逐行批注】 执行当前 Python 语句："""无法确认或持久化预算时，整个运行必须停止后续发送。"""。
    """无法确认或持久化预算时，整个运行必须停止后续发送。"""


# 【逐行批注】 定义函数 read_budget，封装可复用的处理步骤。
def read_budget(path):
    # 【逐行批注】 执行当前 Python 语句："""独立读取已结算/预留的预算，不依赖模型结果是否可解析。"""。
    """独立读取已结算/预留的预算，不依赖模型结果是否可解析。"""
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = json.loads(Path(path).read_text(encoding='utf-8-sig'))
        # 【逐行批注】 遍历数据并重复执行循环体：for name in ('limitTokens', 'usedTokens', 'reservedTokens'):。
        for name in ('limitTokens', 'usedTokens', 'reservedTokens'):
            # 【逐行批注】 判断条件 type(value.get(name)) is not int or value[name] < 0:，成立时执行对应分支。
            if type(value.get(name)) is not int or value[name] < 0:
                # 【逐行批注】 在错误情形抛出异常：raise ValueError('预算字段必须是非负整数')。
                raise ValueError('预算字段必须是非负整数')
        # 【逐行批注】 结束当前函数并返回结果：return value。
        return value
    # 【逐行批注】 捕获并处理异常：except (OSError, ValueError, TypeError, AttributeError) as error:。
    except (OSError, ValueError, TypeError, AttributeError) as error:
        # 【逐行批注】 在错误情形抛出异常：raise BudgetStateError('预算状态无法读取或校验；停止后续请求。') from error。
        raise BudgetStateError('预算状态无法读取或校验；停止后续请求。') from error


# 【逐行批注】 定义函数 persist_budget，封装可复用的处理步骤。
def persist_budget(path, value):
    # 【逐行批注】 执行当前 Python 语句："""先原子保存累计账本，再允许解析结果或启动下一例。"""。
    """先原子保存累计账本，再允许解析结果或启动下一例。"""
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
        path = Path(path)
        # 【逐行批注】 计算右侧结果并赋给 temporary，更新脚本状态。
        temporary = path.with_suffix('.tmp')
        # 【逐行批注】 计算右侧结果并赋给 temporary.write_text(json.dumps(value, ensur…，更新脚本状态。
        temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 调用 read_budget 完成当前脚本或测试步骤：read_budget(temporary)。
        read_budget(temporary)
        # 【逐行批注】 执行当前 Python 语句：temporary.replace(path)。
        temporary.replace(path)
    # 【逐行批注】 捕获并处理异常：except (OSError, ValueError, TypeError) as error:。
    except (OSError, ValueError, TypeError) as error:
        # 【逐行批注】 在错误情形抛出异常：raise BudgetStateError('累计预算无法保存；停止后续请求。') from error。
        raise BudgetStateError('累计预算无法保存；停止后续请求。') from error


# 【逐行批注】 定义函数 digest，封装可复用的处理步骤。
def digest(value):
    # 【逐行批注】 执行当前 Python 语句："""使用稳定JSON生成复用指纹，不包含真实凭据。"""。
    """使用稳定JSON生成复用指纹，不包含真实凭据。"""
    # 【逐行批注】 结束当前函数并返回结果：return hashlib.sha256(json.dumps(value, ensure_ascii=False, sort_keys…。
    return hashlib.sha256(json.dumps(value, ensure_ascii=False, sort_keys=True,
                                     # 【逐行批注】 计算右侧结果并赋给 separators，更新脚本状态。
                                     separators=(',', ':')).encode('utf-8')).hexdigest()


# 【逐行批注】 定义函数 file_hash，封装可复用的处理步骤。
def file_hash(path):
    # 【逐行批注】 结束当前函数并返回结果：return hashlib.sha256(Path(path).read_bytes()).hexdigest()。
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


# 【逐行批注】 定义函数 decision，封装可复用的处理步骤。
def decision():
    # 【逐行批注】 结束当前函数并返回结果：return json.loads(Path(_decision_file.get() or DECISION).read_text(en…。
    return json.loads(Path(_decision_file.get() or DECISION).read_text(encoding='utf-8-sig'))


# 【逐行批注】 定义函数 require_live_authorization，封装可复用的处理步骤。
def require_live_authorization():
    # 【逐行批注】 执行当前 Python 语句："""必须先于读凭据、启动live子进程与每次live HTTP执行。"""。
    """必须先于读凭据、启动live子进程与每次live HTTP执行。"""
    # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
    value = decision()
    # 【逐行批注】 判断条件 value.get('newPaidCallsAllowedThisRound') is not True:，成立时执行对应分支。
    if value.get('newPaidCallsAllowedThisRound') is not True:
        # 【逐行批注】 在错误情形抛出异常：raise PermissionError('本轮授权禁止新增付费模型调用；未读取凭据、未启动真实HTTP。')。
        raise PermissionError('本轮授权禁止新增付费模型调用；未读取凭据、未启动真实HTTP。')
    # 【逐行批注】 判断条件 file_hash(value['originalBudgetPath']) != value['originalBudgetS…，成立时执行对应分支。
    if file_hash(value['originalBudgetPath']) != value['originalBudgetSha256']:
        # 【逐行批注】 在错误情形抛出异常：raise PermissionError('原预算账本校验值变化，停止真实验收。')。
        raise PermissionError('原预算账本校验值变化，停止真实验收。')
    # 【逐行批注】 计算右侧结果并赋给 active，更新脚本状态。
    active = value.get('activeBudgetPath')
    # 【逐行批注】 判断条件 active:，成立时执行对应分支。
    if active:
        # 【逐行批注】 计算右侧结果并赋给 state，更新脚本状态。
        state = read_budget(active)
        # 【逐行批注】 判断条件 (state['limitTokens'] > value['limitTokens'] or state['usedToken…，成立时执行对应分支。
        if (state['limitTokens'] > value['limitTokens'] or state['usedTokens'] < value['knownUsedTokens'] or
                # 【逐行批注】 执行当前 Python 语句：state['reservedTokens'] < value['unknownReservedTokens'] or。
                state['reservedTokens'] < value['unknownReservedTokens'] or
                # 【逐行批注】 调用 Path 完成当前脚本或测试步骤：Path(active).resolve() == Path(value['originalBudgetPath']).resolve()…。
                Path(active).resolve() == Path(value['originalBudgetPath']).resolve()):
            # 【逐行批注】 在错误情形抛出异常：raise PermissionError('累计预算不符合本轮授权，停止真实验收。')。
            raise PermissionError('累计预算不符合本轮授权，停止真实验收。')
    # 【逐行批注】 结束当前函数并返回结果：return value。
    return value


# 【逐行批注】 定义函数 isolated_output，封装可复用的处理步骤。
def isolated_output(path):
    # 【逐行批注】 执行当前 Python 语句："""冻结目录和原预算目录不能成为新结果、数据库或预算副本的落点。"""。
    """冻结目录和原预算目录不能成为新结果、数据库或预算副本的落点。"""
    # 【逐行批注】 计算右侧结果并赋给 target，更新脚本状态。
    target = Path(path).resolve()
    # 【逐行批注】 计算右侧结果并赋给 original，更新脚本状态。
    original = Path(decision()['originalBudgetPath']).resolve().parent
    # 【逐行批注】 遍历数据并重复执行循环体：for protected in (FROZEN.resolve(), original):。
    for protected in (FROZEN.resolve(), original):
        # 【逐行批注】 判断条件 target == protected or protected in target.parents:，成立时执行对应分支。
        if target == protected or protected in target.parents:
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('结果必须写入隔离目录，禁止覆盖冻结基线或原预算目录。')。
            raise ValueError('结果必须写入隔离目录，禁止覆盖冻结基线或原预算目录。')
    # 【逐行批注】 结束当前函数并返回结果：return target。
    return target


# 【逐行批注】 定义函数 local_fixture_url，封装可复用的处理步骤。
def local_fixture_url(url):
    # 【逐行批注】 计算右侧结果并赋给 host，更新脚本状态。
    host = urlsplit(url).hostname
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 local，更新脚本状态。
        local = ipaddress.ip_address(host or '').is_loopback
    # 【逐行批注】 捕获并处理异常：except ValueError:。
    except ValueError:
        # 【逐行批注】 计算右侧结果并赋给 local，更新脚本状态。
        local = host == 'localhost'
    # 【逐行批注】 判断条件 urlsplit(url).scheme != 'http' or not local:，成立时执行对应分支。
    if urlsplit(url).scheme != 'http' or not local:
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('模拟服务必须是本机HTTP地址。')。
        raise ValueError('模拟服务必须是本机HTTP地址。')
    # 【逐行批注】 结束当前函数并返回结果：return url。
    return url


# 【逐行批注】 定义函数 budget_copy，封装可复用的处理步骤。
def budget_copy(path, source=None):
    # 【逐行批注】 执行当前 Python 语句："""只创建新副本，并受已确认累计上限约束。"""。
    """只创建新副本，并受已确认累计上限约束。"""
    # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
    path = isolated_output(path)
    # 【逐行批注】 计算右侧结果并赋给 approved，更新脚本状态。
    approved = decision()
    # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
    value = json.loads(Path(source).read_text(encoding='utf-8-sig')) if source else {
        # 【逐行批注】 执行当前 Python 语句：'limitTokens': approved['limitTokens'], 'usedTokens': approved['knownUsedTokens'],。
        'limitTokens': approved['limitTokens'], 'usedTokens': approved['knownUsedTokens'],
        # 【逐行批注】 执行当前 Python 语句：'reservedTokens': approved['unknownReservedTokens']}。
        'reservedTokens': approved['unknownReservedTokens']}
    # 【逐行批注】 遍历数据并重复执行循环体：for name in ('limitTokens', 'usedTokens', 'reservedTokens'):。
    for name in ('limitTokens', 'usedTokens', 'reservedTokens'):
        # 【逐行批注】 判断条件 type(value.get(name)) is not int or value[name] < 0:，成立时执行对应分支。
        if type(value.get(name)) is not int or value[name] < 0:
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('预算字段必须是非负整数：'+name)。
            raise ValueError('预算字段必须是非负整数：'+name)
    # 【逐行批注】 计算右侧结果并赋给 value['limitTokens']，更新脚本状态。
    value['limitTokens'] = min(value['limitTokens'], approved['limitTokens'])
    # 【逐行批注】 在受控上下文中执行资源操作：with path.open('x', encoding='utf-8') as stream:。
    with path.open('x', encoding='utf-8') as stream:
        # 【逐行批注】 计算右侧结果并赋给 json.dump(value, stream, ensure_ascii，更新脚本状态。
        json.dump(value, stream, ensure_ascii=False, indent=2)
    # 【逐行批注】 结束当前函数并返回结果：return path。
    return path


# 【逐行批注】 定义函数 summarize，封装可复用的处理步骤。
def summarize(result):
    # 【逐行批注】 执行当前 Python 语句："""失败HTTP照计，逐笔Token优先；旧任务指标缺失保持未知。"""。
    """失败HTTP照计，逐笔Token优先；旧任务指标缺失保持未知。"""
    # 【逐行批注】 计算右侧结果并赋给 has_http，更新脚本状态。
    has_http = 'httpRequests' in result
    # 【逐行批注】 计算右侧结果并赋给 missing_usage，更新脚本状态。
    missing_usage = not has_http and 'usage' not in result
    # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
    rows = result.get('httpRequests', []) if has_http else result.get('usage', [])
    # 【逐行批注】 计算右侧结果并赋给 tokens_key，更新脚本状态。
    tokens_key = 'tokens' if has_http else 'tokenUsage'
    # 【逐行批注】 计算右侧结果并赋给 known, unknown，更新脚本状态。
    known, unknown = [], 0
    # 【逐行批注】 遍历数据并重复执行循环体：for row in rows:。
    for row in rows:
        # 【逐行批注】 计算右侧结果并赋给 tokens，更新脚本状态。
        tokens = row.get(tokens_key)
        # 【逐行批注】 判断条件 isinstance(tokens, dict) and type(tokens.get('total_tokens')) is…，成立时执行对应分支。
        if isinstance(tokens, dict) and type(tokens.get('total_tokens')) is int and tokens['total_tokens'] >= 0:
            # 【逐行批注】 执行当前 Python 语句：known.append(tokens)。
            known.append(tokens)
        # 【逐行批注】 执行前一条件不成立时的分支。
        else:
            # 【逐行批注】 计算右侧结果并赋给 unknown +，更新脚本状态。
            unknown += 1
    # 【逐行批注】 计算右侧结果并赋给 tasks，更新脚本状态。
    tasks = {}
    # 【逐行批注】 遍历数据并重复执行循环体：for turn in result.get('turns', []):。
    for turn in result.get('turns', []):
        # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
        task = turn.get('task', {})
        # 【逐行批注】 计算右侧结果并赋给 key，更新脚本状态。
        key = task.get('id', str(len(tasks)))
        # 【逐行批注】 计算右侧结果并赋给 tasks[key]，更新脚本状态。
        tasks[key] = {'taskId': task.get('id'), **{name: task.get(name) for name in TASK_METRICS}}
    # 【逐行批注】 结束当前函数并返回结果：return {'usageSource': 'httpRequests' if has_http else 'legacyUsage',。
    return {'usageSource': 'httpRequests' if has_http else 'legacyUsage',
            # 【逐行批注】 调用 len 完成当前脚本或测试步骤：'httpRequestCount': len(rows) if has_http else None,。
            'httpRequestCount': len(rows) if has_http else None,
            # 【逐行批注】 调用 sum 完成当前脚本或测试步骤：'failedHttpRequests': sum(row.get('status') == 'failed' for row in ro…。
            'failedHttpRequests': sum(row.get('status') == 'failed' for row in rows) if has_http else None,
            # 【逐行批注】 调用 sum 完成当前脚本或测试步骤：'knownTokens': sum(row['total_tokens'] for row in known),。
            'knownTokens': sum(row['total_tokens'] for row in known),
            # 【逐行批注】 执行当前 Python 语句：'unknownUsageRequests': None if missing_usage else unknown,。
            'unknownUsageRequests': None if missing_usage else unknown,
            # 【逐行批注】 调用 sum 完成当前脚本或测试步骤：'totalTokens': None if unknown or missing_usage else sum(row['total_t…。
            'totalTokens': None if unknown or missing_usage else sum(row['total_tokens'] for row in known),
            # 【逐行批注】 执行当前 Python 语句：'httpDurationMs': [row.get('durationMs') for row in rows] if has_http else None,。
            'httpDurationMs': [row.get('durationMs') for row in rows] if has_http else None,
            # 【逐行批注】 调用 list 完成当前脚本或测试步骤：'taskMetrics': list(tasks.values())}。
            'taskMetrics': list(tasks.values())}


# 【逐行批注】 定义函数 successful，封装可复用的处理步骤。
def successful(result, expected_turns):
    # 【逐行批注】 计算右侧结果并赋给 turns，更新脚本状态。
    turns = result.get('turns', [])
    # 【逐行批注】 结束当前函数并返回结果：return len(turns) == expected_turns and all(。
    return len(turns) == expected_turns and all(
        # 【逐行批注】 调用 in 完成当前脚本或测试步骤：row.get('task', {}).get('status') in ('completed', 'ready', 'waiting_…。
        row.get('task', {}).get('status') in ('completed', 'ready', 'waiting_student')
        # 【逐行批注】 执行当前 Python 语句：and not row.get('task', {}).get('failure') for row in turns)。
        and not row.get('task', {}).get('failure') for row in turns)
