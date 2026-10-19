"""验收入口共用的授权、隔离与统计规则；离线路径不读取凭据。"""
import hashlib
import ipaddress
import json
from pathlib import Path
from urllib.parse import urlsplit

REPO = Path(__file__).resolve().parents[1]
DECISION = REPO / 'verification/v564-budget-decision.json'
FROZEN = REPO / 'verification/v564-baseline'
SCHEMA = 'same-protocol-v564-r1'
TASK_METRICS = ('queueMs', 'firstContentMs', 'completionMs')


def digest(value):
    """使用稳定JSON生成复用指纹，不包含真实凭据。"""
    return hashlib.sha256(json.dumps(value, ensure_ascii=False, sort_keys=True,
                                     separators=(',', ':')).encode('utf-8')).hexdigest()


def file_hash(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def decision():
    return json.loads(DECISION.read_text(encoding='utf-8-sig'))


def require_live_authorization():
    """必须先于读凭据、启动live子进程与每次live HTTP执行。"""
    value = decision()
    if value.get('newPaidCallsAllowedThisRound') is not True:
        raise PermissionError('本轮授权禁止新增付费模型调用；未读取凭据、未启动真实HTTP。')
    if file_hash(value['originalBudgetPath']) != value['originalBudgetSha256']:
        raise PermissionError('原预算账本校验值变化，停止真实验收。')
    return value


def isolated_output(path):
    """冻结目录和原预算目录不能成为新结果、数据库或预算副本的落点。"""
    target = Path(path).resolve()
    original = Path(decision()['originalBudgetPath']).resolve().parent
    for protected in (FROZEN.resolve(), original):
        if target == protected or protected in target.parents:
            raise ValueError('结果必须写入隔离目录，禁止覆盖冻结基线或原预算目录。')
    return target


def local_fixture_url(url):
    host = urlsplit(url).hostname
    try:
        local = ipaddress.ip_address(host or '').is_loopback
    except ValueError:
        local = host == 'localhost'
    if urlsplit(url).scheme != 'http' or not local:
        raise ValueError('模拟服务必须是本机HTTP地址。')
    return url


def budget_copy(path, source=None):
    """只创建新副本，并受已确认累计上限约束。"""
    path = isolated_output(path)
    approved = decision()
    value = json.loads(Path(source).read_text(encoding='utf-8-sig')) if source else {
        'limitTokens': approved['limitTokens'], 'usedTokens': approved['knownUsedTokens'],
        'reservedTokens': approved['unknownReservedTokens']}
    for name in ('limitTokens', 'usedTokens', 'reservedTokens'):
        if type(value.get(name)) is not int or value[name] < 0:
            raise ValueError('预算字段必须是非负整数：'+name)
    value['limitTokens'] = min(value['limitTokens'], approved['limitTokens'])
    with path.open('x', encoding='utf-8') as stream:
        json.dump(value, stream, ensure_ascii=False, indent=2)
    return path


def summarize(result):
    """失败HTTP照计，逐笔Token优先；旧任务指标缺失保持未知。"""
    has_http = 'httpRequests' in result
    missing_usage = not has_http and 'usage' not in result
    rows = result.get('httpRequests', []) if has_http else result.get('usage', [])
    tokens_key = 'tokens' if has_http else 'tokenUsage'
    known, unknown = [], 0
    for row in rows:
        tokens = row.get(tokens_key)
        if isinstance(tokens, dict) and type(tokens.get('total_tokens')) is int and tokens['total_tokens'] >= 0:
            known.append(tokens)
        else:
            unknown += 1
    tasks = {}
    for turn in result.get('turns', []):
        task = turn.get('task', {})
        key = task.get('id', str(len(tasks)))
        tasks[key] = {'taskId': task.get('id'), **{name: task.get(name) for name in TASK_METRICS}}
    return {'usageSource': 'httpRequests' if has_http else 'legacyUsage',
            'httpRequestCount': len(rows) if has_http else None,
            'failedHttpRequests': sum(row.get('status') == 'failed' for row in rows) if has_http else None,
            'knownTokens': sum(row['total_tokens'] for row in known),
            'unknownUsageRequests': None if missing_usage else unknown,
            'totalTokens': None if unknown or missing_usage else sum(row['total_tokens'] for row in known),
            'httpDurationMs': [row.get('durationMs') for row in rows] if has_http else None,
            'taskMetrics': list(tasks.values())}


def successful(result, expected_turns):
    turns = result.get('turns', [])
    return len(turns) == expected_turns and all(
        row.get('task', {}).get('status') in ('completed', 'ready', 'waiting_student')
        and not row.get('task', {}).get('failure') for row in turns)
