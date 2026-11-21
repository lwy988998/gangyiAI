"""准备或查看首轮共用人民币账本；不读取模型密钥、不发HTTP。"""

from __future__ import annotations

import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import sys
import time
from urllib.parse import urlsplit

from tutor_benchmark import parse_json

MAXIMUM = 9007199254740991
ROOT = Path(__file__).resolve().parents[1]
LIVE_FOLDER = ROOT / 'verification/vnext-P1/first-round'


def encoded(value):
    """与原生nlohmann::json的有序紧凑编码保持一致，供日志完整性校验。"""
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(',', ':'), allow_nan=False).encode('utf-8')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def integer(value):
    if type(value) is not int or not 0 <= value <= MAXIMUM:
        raise ValueError('金额和用量必须为安全范围内的非负整数。')
    return value


def load(path):
    return parse_json(Path(path).read_text(encoding='utf-8'))


@contextmanager
def ledger_lock(path):
    """Python和原生使用同一个锁文件；不等待网络或持锁调用模型。"""
    lock = Path(str(path) + '.lock')
    if os.name == 'nt':
        import ctypes
        from ctypes import wintypes
        api = ctypes.WinDLL('kernel32', use_last_error=True)
        api.CreateFileW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.c_void_p,
                                   wintypes.DWORD, wintypes.DWORD, ctypes.c_void_p]
        api.CreateFileW.restype = wintypes.HANDLE
        api.CloseHandle.argtypes = [wintypes.HANDLE]
        handle = api.CreateFileW(str(lock), 0xC0000000, 0, None, 4, 0x80, None)
        if handle == ctypes.c_void_p(-1).value:
            raise RuntimeError('费用账本正被使用或不能锁定。')
        try:
            yield
        finally:
            api.CloseHandle(handle)
    else:
        import fcntl
        with lock.open('ab') as stream:
            fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)
            try:
                yield
            finally:
                fcntl.flock(stream, fcntl.LOCK_UN)


def prepare(config_path, ledger_path, session_id, model, endpoint, pricing, *, fixture=False,
            limit_micros=10000000, valid_hours=24, review_receipt=None, authorization_source=None):
    """独占创建配置和唯一累计账本；失败留下的文件不能直接覆盖重建。"""
    config_path, ledger_path = Path(config_path).resolve(), Path(ledger_path).resolve()
    if config_path == ledger_path or config_path.exists() or ledger_path.exists():
        raise ValueError('配置和账本必须为不同的新文件；已有记录不得覆盖或清零。')
    if not isinstance(session_id, str) or not session_id.strip() or not isinstance(model, str) or not model.strip():
        raise ValueError('缺少实验轮次或实际模型标识。')
    if not 1 <= integer(limit_micros) <= 10000000 or type(valid_hours) is not int or not 1 <= valid_hours <= 24:
        raise ValueError('首轮额度不超过10元，费率有效期不超过24小时。')
    parsed = urlsplit(endpoint)
    if (parsed.username or parsed.password or parsed.query or parsed.fragment or '\\' in endpoint or
        not parsed.hostname or not parsed.path.endswith('/chat/completions')):
        raise ValueError('必须使用不带凭据的完整聊天端点。')
    if fixture:
        if parsed.scheme != 'http' or parsed.hostname not in {'127.0.0.1', '::1'} or not parsed.port:
            raise ValueError('模拟预算只允许带端口的本机HTTP端点。')
    elif model != 'deepseek-flash' or endpoint not in {
        'https://api.deepseek.com/chat/completions', 'https://api.deepseek.com/v1/chat/completions'
    } or pricing.get('contextWindowTokens') != 1048576:
        raise ValueError('首轮真实预算只接受已核对上下文上限的 DeepSeek Flash 官方端点。')
    for name in ('inputMicrosPerMillion', 'outputMicrosPerMillion'):
        if integer(pricing.get(name)) <= 0:
            raise ValueError('费率不能缺失或为零。')
    if pricing.get('outputIncludesReasoning') is not True or not pricing.get('source') or not pricing.get('checkedAt'):
        raise ValueError('缺少价格来源、核验日期或输出计费上界依据。')
    if 'contextWindowTokens' in pricing:
        if integer(pricing['contextWindowTokens']) == 0:
            raise ValueError('上下文计费上界无效。')
    elif not fixture or pricing.get('textByteUpperBoundVerified') is not True or integer(pricing.get('promptOverheadTokens')) < 4096:
        raise ValueError('输入计费上界未核实。')
    if not fixture and (not review_receipt or not authorization_source):
        raise ValueError('真实模式必须绑定本轮授权出处和H1已完成审查回执。')
    if not fixture and (config_path != (LIVE_FOLDER/'money-config.json').resolve() or
                        ledger_path != (LIVE_FOLDER/'money-ledger.jsonl').resolve()):
        raise ValueError('首轮真实实验必须复用固定账本，不能通过新目录重置10元额度。')
    receipt_hash = digest(Path(review_receipt).read_bytes()) if review_receipt else None
    if not fixture:
        receipt = load(review_receipt)
        if (receipt.get('kind') != 'hermes_money_budget_review' or receipt.get('state') != 'completed' or
            receipt.get('allBlockingFindingsClosed') is not True or not receipt.get('requestId') or
            not receipt.get('reviewedFiles')):
            raise ValueError('H1回执必须为已完成且阻断问题已关闭的独立审查。')
        for item in receipt['reviewedFiles']:
            if digest(Path(item['path']).read_bytes()) != item['sha256']:
                raise ValueError('H1审查后代码已变化，不能沿用旧审查。')
    now = int(time.time())
    config = {'schemaVersion': 1, 'currency': 'CNY', 'sessionId': session_id, 'model': model,
              'endpoint': endpoint, 'ledgerPath': str(ledger_path), 'limitMicros': limit_micros,
              'validFromUnix': now, 'expiresUnix': now + valid_hours * 3600, 'pricing': pricing,
              'fixtureOnly': fixture, 'authorized': True, 'reviewed': True,
              'authorizationSource': authorization_source if not fixture else '仅本机模拟，不涉及付费授权',
              'reviewReceiptSha256': receipt_hash,
              'reviewReceiptPath': str(Path(review_receipt).resolve()) if review_receipt else None}
    data = (json.dumps(config, ensure_ascii=False, sort_keys=True, indent=2, allow_nan=False) + '\n').encode('utf-8')
    identity = digest(data)
    row = {'kind': 'genesis', 'sessionId': session_id, 'identity': identity, 'revision': 0,
           'previousHash': '', 'limitMicros': limit_micros, 'atUnix': now}
    row['hash'] = digest(encoded(row))
    config_path.parent.mkdir(parents=True, exist_ok=True)
    ledger_path.parent.mkdir(parents=True, exist_ok=True)
    with ledger_lock(ledger_path):
        with config_path.open('xb') as stream:
            stream.write(data); stream.flush(); os.fsync(stream.fileno())
        with ledger_path.open('xb') as stream:
            stream.write(encoded(row) + b'\n'); stream.flush(); os.fsync(stream.fileno())
    return {'configPath': str(config_path), 'ledgerPath': str(ledger_path), 'configSha256': identity,
            'limitMicros': limit_micros, 'httpRequests': 0, 'paidRequests': 0,
            'environment': {'GANGYI_MONEY_CONFIG': str(config_path), 'GANGYI_MONEY_CONFIG_SHA256': identity,
                            'GANGYI_MONEY_REQUIRED': '1'}}


def inspect(config_path):
    """重放日志计算已结算与未释放预留，不按文件中的摘要数字相信余额。"""
    config_path = Path(config_path).resolve()
    config = load(config_path); identity = digest(config_path.read_bytes())
    ledger = Path(config['ledgerPath'])
    with ledger_lock(ledger):
        data = ledger.read_text(encoding='utf-8')
        if not data.endswith('\n'):
            raise ValueError('账本存在未完整写入的记录。')
        previous, spent, held, pending, seen, blocked, requests = '', 0, 0, {}, set(), False, 0
        rows = data.splitlines()
        for index, line in enumerate(rows):
            row = parse_json(line); saved_hash = row.pop('hash')
            if (saved_hash != digest(encoded(row)) or row['identity'] != identity or row['sessionId'] != config['sessionId'] or
                integer(row['revision']) != index or row['previousHash'] != previous):
                raise ValueError('账本身份、顺序或完整性校验失败。')
            kind, request_id = row['kind'], row.get('requestId')
            if index == 0:
                if kind != 'genesis' or row['limitMicros'] != config['limitMicros']:
                    raise ValueError('账本初始额度错误。')
            elif kind == 'reserve':
                if not request_id or request_id in seen:
                    raise ValueError('请求身份重复。')
                amount = integer(row['amountMicros']); pending[request_id] = amount; seen.add(request_id)
                held += amount; requests += 1
            elif kind == 'settle':
                amount = integer(row['amountMicros'])
                if request_id not in pending or amount > pending[request_id]:
                    raise ValueError('结算未匹配预留。')
                held -= pending.pop(request_id); spent += amount
            elif kind in {'hold', 'block'}:
                if request_id not in pending:
                    raise ValueError('保留费用没有原请求。')
                blocked = blocked or kind == 'block'
            else:
                raise ValueError('未知账本记录。')
            if spent + held > integer(config['limitMicros']):
                raise ValueError('累计费用超过授权。')
            previous = saved_hash
    return {'sessionId': config['sessionId'], 'currency': 'CNY', 'spentMicros': spent, 'heldMicros': held,
            'availableMicros': config['limitMicros'] - spent - held, 'limitMicros': config['limitMicros'],
            'requestReservations': requests, 'unsettledRequests': list(pending), 'blocked': blocked,
            'lastRevision': len(rows) - 1, 'lastHash': previous, 'fixtureOnly': config['fixtureOnly'],
            'qualityEvaluated': False}


def checked_live_environment(config_path, expected_hash, endpoint, model):
    """读取真实凭据前检查共用人民币身份；账本锁仍由每次原生发送重新取得。"""
    config_path = Path(config_path).resolve()
    if not expected_hash or digest(config_path.read_bytes()) != expected_hash:
        raise PermissionError('未绑定冻结的共用人民币预算。')
    config = load(config_path)
    if (config.get('fixtureOnly') is not False or config.get('authorized') is not True or config.get('reviewed') is not True or
        config.get('currency') != 'CNY' or config.get('model') != model or config.get('endpoint') != endpoint or
        not 0 < integer(config.get('limitMicros')) <= 10000000 or
        not integer(config.get('validFromUnix')) <= int(time.time()) < integer(config.get('expiresUnix'))):
        raise PermissionError('人民币预算模式、授权、模型或费率有效期不匹配。')
    receipt = Path(config['reviewReceiptPath'])
    if digest(receipt.read_bytes()) != config.get('reviewReceiptSha256'):
        raise PermissionError('H1独立审查回执已变化。')
    reviewed = load(receipt)
    for item in reviewed['reviewedFiles']:
        if digest(Path(item['path']).read_bytes()) != item['sha256']:
            raise PermissionError('费用发送代码已变化，需要复核H1覆盖范围。')
    state = inspect(config_path)
    if state['blocked']:
        raise PermissionError('人民币账本存在不确定或异常请求，停止后续调用。')
    return {'GANGYI_MONEY_CONFIG': str(config_path), 'GANGYI_MONEY_CONFIG_SHA256': expected_hash,
            'GANGYI_MONEY_REQUIRED': '1'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    create = commands.add_parser('prepare')
    create.add_argument('--config', type=Path, required=True); create.add_argument('--ledger', type=Path, required=True)
    create.add_argument('--session', required=True); create.add_argument('--model', required=True)
    create.add_argument('--endpoint', required=True); create.add_argument('--pricing', type=Path, required=True)
    create.add_argument('--fixture-only', action='store_true'); create.add_argument('--review-receipt', type=Path)
    create.add_argument('--authorization-source'); create.add_argument('--limit-micros', type=int, default=10000000)
    view = commands.add_parser('inspect'); view.add_argument('--config', type=Path, required=True)
    args = parser.parse_args()
    try:
        result = inspect(args.config) if args.command == 'inspect' else prepare(
            args.config, args.ledger, args.session, args.model, args.endpoint, load(args.pricing),
            fixture=args.fixture_only, limit_micros=args.limit_micros, review_receipt=args.review_receipt,
            authorization_source=args.authorization_source)
        print(json.dumps(result, ensure_ascii=False, indent=2)); return 0
    except (OSError, ValueError, KeyError, TypeError, RuntimeError):
        print('费用准备或检查失败；未发起模型请求。', file=sys.stderr); return 2


if __name__ == '__main__':
    raise SystemExit(main())
