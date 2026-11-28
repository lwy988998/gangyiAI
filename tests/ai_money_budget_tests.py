"""实际AIClient和跨进程人民币账本验证；网络仅使用本机HTTP夹具。"""

import concurrent.futures
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from threading import Thread
import time
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import ai_money_budget as budget

DRIVER = str(Path(sys.argv.pop(1)).resolve())


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def do_POST(self):
        data = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        self.server.received.append(data)
        behavior = self.server.behavior
        if behavior == 'disconnect':
            self.connection.close(); return
        usage = None if behavior in {'unknown', 'compatibility'} else {
            'prompt_tokens': 11, 'completion_tokens': 7, 'total_tokens': 18}
        if behavior == 'oversize': usage['completion_tokens'] = 100000; usage['total_tokens'] = 100011
        if behavior == 'bad-total': usage['total_tokens'] = 1
        if behavior == 'compatibility' and 'stream_options' in data:
            self.send_response(400); self.end_headers()
            self.wfile.write(b'{"error":{"message":"unsupported stream_options include_usage"}}'); return
        model = 'wrong-money-model' if behavior == 'wrong-model' else 'fixture-money'
        self.send_response(200)
        self.send_header('Content-Type', 'text/event-stream' if data.get('stream') else 'application/json')
        self.end_headers()
        if data.get('stream'):
            event = {'model': model, 'choices': [{'delta': {'content': 'actual-local-response'}, 'finish_reason': 'stop'}], 'usage': usage}
            content = 'data: ' + json.dumps(event) + '\n\ndata: [DONE]\n\n'
        else:
            content = json.dumps({'model': model, 'usage': usage, 'choices': [{'message': {'content': 'actual-local-response'}, 'finish_reason': 'stop'}]})
        self.wfile.write(content.encode())


class MoneyTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='money-中文-')
        self.root = Path(self.temporary.name)
        self.addCleanup(self.temporary.cleanup)
        self.server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        self.server.received = []; self.server.behavior = 'normal'
        self.worker = Thread(target=self.server.serve_forever, daemon=True); self.worker.start()
        self.addCleanup(self.stop)
        self.endpoint = f'http://127.0.0.1:{self.server.server_port}/v1'
        self.config = self.root / '配置.json'; self.ledger = self.root / '累计.jsonl'
        self.pricing = {'inputMicrosPerMillion': 1000000, 'outputMicrosPerMillion': 2000000,
                        'promptOverheadTokens': 4096, 'textByteUpperBoundVerified': True,
                        'outputIncludesReasoning': True, 'source': '本机模拟价格，不用于真实模型', 'checkedAt': 'fixture'}
        self.prepared = self.prepare()

    def stop(self):
        self.server.shutdown(); self.server.server_close(); self.worker.join(timeout=2)

    def prepare(self, **options):
        return budget.prepare(self.config, self.ledger, 'fixture-one-round', 'fixture-money',
                              self.endpoint + '/chat/completions', self.pricing, fixture=True, **options)

    def run_driver(self, command='chat', env=None, text='预算验证'):
        current = dict(os.environ)
        current.update(self.prepared['environment'])
        current.update(GANGYI_AI_RETRY_BASE_MS='1', GANGYI_AI_SHARED_BACKOFF='0')
        if env: current.update(env)
        return subprocess.run([DRIVER, command, self.endpoint, text], env=current, capture_output=True, timeout=15)

    def state(self):
        return budget.inspect(self.config)

    def write_rate_evidence(self, config_path, path, quote=None):
        original = budget.load(config_path)
        data = {key: original[key] for key in ('currency', 'model', 'endpoint', 'fixtureOnly')}
        data.update(checkedAtUnix=int(time.time()), terms=quote if quote is not None else budget._pricing_terms(original),
                    source='fixture' if original['fixtureOnly'] else 'https://api-docs.deepseek.com/zh-cn/quick_start/pricing/')
        path.write_bytes(budget.encoded(data))
        return path

    def renew_fixture(self):
        path = self.root / '续期.json'
        receipt = budget.prepare_revalidation(self.config, self.prepared['configSha256'], path,
            self.write_rate_evidence(self.config, self.root / '费率证据.json'))
        return {'GANGYI_MONEY_REVALIDATION': str(path), 'GANGYI_MONEY_REVALIDATION_SHA256': receipt['revalidationSha256']}

    def test_revalidation_keeps_historical_spend_after_expiry(self):
        self.config, self.ledger = self.root / '短期配置.json', self.root / '短期累计.jsonl'
        # 建立即将到期的独立模拟预算，使用真实时钟等待窗口结束。
        with patch.object(budget.time, 'time', return_value=int(time.time()) - 3595):
            self.prepared = self.prepare(valid_hours=1)
        self.assertEqual(self.run_driver().returncode, 0)
        before = self.ledger.read_bytes()
        frozen = self.config.read_bytes()
        time.sleep(max(0, budget.load(self.config)['expiresUnix'] - time.time()) + .05)
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 1)
        env = self.renew_fixture()
        self.assertEqual(self.ledger.read_bytes(), before)
        self.assertEqual(self.run_driver('stream', env).returncode, 0)
        self.assertEqual(len(self.server.received), 2)
        self.assertEqual(self.config.read_bytes(), frozen)
        self.assertTrue(self.ledger.read_bytes().startswith(before))
        rows = [json.loads(line) for line in self.ledger.read_bytes().splitlines()]
        self.assertEqual(rows[-2]['pricingRevalidationSha256'], env['GANGYI_MONEY_REVALIDATION_SHA256'])
        self.assertEqual(self.state()['spentMicros'], 50)

    def test_revalidation_denies_changed_evidence_and_uncertain_request(self):
        env = self.renew_fixture()
        evidence = self.root / '费率证据.json'
        raw = evidence.read_bytes()
        evidence.write_bytes(raw + b' ')
        self.assertNotEqual(self.run_driver(env=env).returncode, 0)
        self.assertEqual(len(self.server.received), 0)
        evidence.write_bytes(raw)
        self.server.behavior = 'disconnect'
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 1)
        prior = self.state()
        self.server.behavior = 'normal'
        self.assertNotEqual(self.run_driver(env=env).returncode, 0)
        self.assertEqual(len(self.server.received), 1)
        self.assertEqual(self.state(), prior)

    def test_real_revalidation_requires_fresh_reviewer_and_original_coverage(self):
        fixed = self.root / 'mock-first-round'; fixed.mkdir()
        config = fixed / 'money-config.json'; ledger = fixed / 'money-ledger.jsonl'
        evidence = fixed / 'pricing.json'; renewal = fixed / 'renewal.json'
        files = [fixed / name for name in ('native.hpp', 'entry.py')]
        for path in files:
            path.write_bytes(b'mock source')
        reviewed = [{'path': str(path), 'sha256': budget.digest(path.read_bytes())} for path in files]
        review = {'kind': 'hermes_money_budget_review', 'state': 'completed', 'allBlockingFindingsClosed': True,
                  'requestId': 'review-1', 'sessionId': 'mock-real', 'reviewedFiles': reviewed}
        old = fixed / 'original-review.json'; old.write_bytes(budget.encoded(review))
        pricing = {'inputMicrosPerMillion': 2000000, 'outputMicrosPerMillion': 8000000, 'contextWindowTokens': 1048576,
                   'outputIncludesReasoning': True, 'source': '仅测试结构', 'checkedAt': 'fixture'}
        with patch.object(budget, 'LIVE_FOLDER', fixed):
            initial = budget.prepare(config, ledger, 'mock-real', 'deepseek-flash', 'https://api.deepseek.com/chat/completions',
                                     pricing, review_receipt=old, authorization_source='模拟授权')
        self.write_rate_evidence(config, evidence)
        def renew(path=None):
            return budget.prepare_revalidation(config, initial['configSha256'], renewal, evidence, review_receipt=path)
        with self.assertRaises(PermissionError):
            renew()
        updated = fixed / 'new-review.json'
        review.update(requestId='review-2', pricingEvidenceSha256=budget.digest(evidence.read_bytes()), reviewedFiles=reviewed[:1])
        updated.write_bytes(budget.encoded(review))
        with self.assertRaises(PermissionError):
            renew(updated)
        review['reviewedFiles'] = reviewed
        updated.write_bytes(budget.encoded(review))
        receipt = renew(updated)
        active = budget.checked_live_environment(config, initial['configSha256'], 'https://api.deepseek.com/chat/completions',
            'deepseek-flash', revalidation_path=renewal, revalidation_hash=receipt['revalidationSha256'],
            cumulative_limit_micros=4000000)
        self.assertEqual(active['GANGYI_MONEY_CONFIG_SHA256'], initial['configSha256'])
        self.assertEqual(active['GANGYI_MONEY_CUMULATIVE_LIMIT_MICROS'], '4000000')
        self.assertEqual(self.server.received, [])

    def test_real_nonstream_settles_and_next_process_accumulates(self):
        self.assertEqual(self.run_driver().returncode, 0)
        self.assertEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 2)
        self.assertEqual(self.state()['spentMicros'], 50)
        self.assertEqual(self.state()['heldMicros'], 0)

    def test_real_stream_uses_same_ledger(self):
        self.assertEqual(self.run_driver('stream').returncode, 0)
        self.assertEqual(self.state()['spentMicros'], 25)

    def test_zero_http_for_insufficient_budget(self):
        self.config = self.root / 'tiny.json'; self.ledger = self.root / 'tiny.jsonl'
        self.prepared = self.prepare(limit_micros=1)
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 0)

    def test_cumulative_limit_includes_prior_spend_and_stops_compatibility_retry(self):
        self.config, self.ledger = self.root / 'scope-config.json', self.root / 'scope-ledger.jsonl'
        self.pricing.pop('textByteUpperBoundVerified')
        self.pricing.pop('promptOverheadTokens')
        self.pricing['contextWindowTokens'] = 1000
        self.prepared = self.prepare()
        self.assertEqual(self.run_driver().returncode, 0)
        self.assertEqual(self.state()['spentMicros'], 25)
        self.server.behavior = 'compatibility'
        cap = 'GANGYI_MONEY_CUMULATIVE_LIMIT_MICROS'
        frozen = self.ledger.read_bytes()
        denied = self.run_driver('stream', {cap: '1280'})
        self.assertNotEqual(denied.returncode, 0)
        self.assertEqual(len(self.server.received), 1)
        self.assertEqual(self.ledger.read_bytes(), frozen)
        held = self.run_driver('stream', {cap: '1281'})
        self.assertNotEqual(held.returncode, 0)
        self.assertEqual(json.loads(held.stdout)['errorType'], 'money_budget')
        self.assertEqual(len(self.server.received), 2)
        self.assertTrue(self.ledger.read_bytes().startswith(frozen))
        reserve = json.loads(self.ledger.read_bytes().splitlines()[-2])
        self.assertEqual(reserve['cumulativeLimitMicros'], 1281)
        self.assertEqual(self.state()['heldMicros'], 1256)

    def test_cumulative_limit_rejects_invalid_scope_before_http(self):
        cap = 'GANGYI_MONEY_CUMULATIVE_LIMIT_MICROS'
        for invalid in ('0', '-1', '1.0', '10000001', '9007199254740992'):
            result = self.run_driver(env={cap: invalid})
            self.assertNotEqual(result.returncode, 0)
        missing = self.run_driver(env={cap: '4000000', 'GANGYI_MONEY_CONFIG': '', 'GANGYI_MONEY_REQUIRED': ''})
        self.assertNotEqual(missing.returncode, 0)
        self.assertEqual(self.server.received, [])

    def test_drifted_configuration_zero_http(self):
        self.config.write_bytes(self.config.read_bytes() + b' ')
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 0)

    def test_required_missing_configuration_zero_http(self):
        result = self.run_driver(env={'GANGYI_MONEY_CONFIG': ''})
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(len(self.server.received), 0)

    def test_missing_usage_keeps_entire_reservation(self):
        self.server.behavior = 'unknown'
        self.assertEqual(self.run_driver().returncode, 0)
        state = self.state()
        self.assertEqual(state['spentMicros'], 0); self.assertGreater(state['heldMicros'], 4096)
        self.assertEqual(len(state['unsettledRequests']), 1)

    def test_context_bound_reserves_without_local_prompt_estimate(self):
        self.config = self.root / 'context.json'; self.ledger = self.root / 'context.jsonl'
        del self.pricing['textByteUpperBoundVerified']
        del self.pricing['promptOverheadTokens']
        self.pricing.update(contextWindowTokens=1048576, inputMicrosPerMillion=2000000, outputMicrosPerMillion=8000000)
        self.prepared = self.prepare()
        self.server.behavior = 'unknown'
        self.assertEqual(self.run_driver().returncode, 0)
        self.assertEqual(self.state()['heldMicros'], 2098176)
        self.assertEqual(len(self.server.received), 1)

    def test_compatibility_retry_counts_both_http(self):
        self.server.behavior = 'compatibility'
        self.assertEqual(self.run_driver('stream').returncode, 0)
        self.assertEqual(len(self.server.received), 2)
        self.assertEqual(self.state()['requestReservations'], 2)
        self.assertGreater(self.state()['heldMicros'], 8192)

    def test_uncertain_network_stops_retry_and_later_calls(self):
        self.server.behavior = 'disconnect'
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 1)
        self.assertTrue(self.state()['blocked'])
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 1)

    def test_anomalous_usage_blocks(self):
        self.server.behavior = 'oversize'
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertTrue(self.state()['blocked'])
        self.assertGreater(self.state()['heldMicros'], 0)

    def test_mismatched_usage_total_is_not_refunded(self):
        self.server.behavior = 'bad-total'
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertGreater(self.state()['heldMicros'], 0)
        self.assertTrue(self.state()['blocked'])

    def test_response_model_mismatch_blocks(self):
        self.server.behavior = 'wrong-model'
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertTrue(self.state()['blocked'])

    def test_truncated_ledger_zero_http(self):
        self.ledger.write_bytes(self.ledger.read_bytes()[:-1])
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 0)

    def test_damaged_ledger_zero_http(self):
        self.ledger.write_bytes(self.ledger.read_bytes().replace(b'10000000', b'10000001'))
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 0)

    def test_crash_after_reserve_keeps_amount(self):
        self.assertEqual(self.run_driver('reserve').returncode, 0)
        held = self.state()['heldMicros']
        self.assertGreater(held, 0)
        self.assertNotEqual(self.run_driver().returncode, 0)
        self.assertEqual(len(self.server.received), 0)
        self.assertEqual(self.state()['heldMicros'], held)

    def test_concurrent_processes_share_serialized_ledger(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
            results = list(executor.map(lambda _: self.run_driver(), range(4)))
        self.assertTrue(all(result.returncode == 0 for result in results))
        self.assertEqual(self.state()['spentMicros'], 100)
        self.assertEqual(self.state()['heldMicros'], 0)

    def test_image_and_search_are_denied_before_http(self):
        self.assertNotEqual(self.run_driver('image').returncode, 0)
        self.assertNotEqual(self.run_driver('search').returncode, 0)
        self.assertEqual(len(self.server.received), 0)

    def test_prepare_never_recreates_existing_budget(self):
        with self.assertRaises(ValueError): self.prepare()
        self.assertEqual(len(self.server.received), 0)

    def test_live_prepare_needs_review_and_auth(self):
        pricing = dict(self.pricing, contextWindowTokens=1048576)
        with self.assertRaisesRegex(ValueError, '授权出处和H1'):
            budget.prepare(self.root/'live.json', self.root/'live.jsonl', 'live', 'deepseek-flash',
                           'https://api.deepseek.com/chat/completions', pricing)
        with self.assertRaisesRegex(ValueError, '上下文上限'):
            budget.prepare(self.root/'live.json', self.root/'live.jsonl', 'live', 'deepseek-flash',
                           'https://api.deepseek.com/chat/completions', dict(pricing, contextWindowTokens=4096))

    def test_prepare_denies_remote_fixture_and_excess_amount(self):
        for endpoint, amount in [('http://example.invalid:80/chat/completions', 100),
                                 (self.endpoint+'/chat/completions', 10000001)]:
            with self.assertRaises(ValueError):
                budget.prepare(self.root/'bad.json', self.root/'bad.jsonl', 'bad', 'fixture-money', endpoint,
                               self.pricing, fixture=True, limit_micros=amount)

    def test_cli_inspect_works_from_unrelated_directory(self):
        script = Path(budget.__file__).resolve()
        result = subprocess.run([sys.executable, '-B', '-X', 'utf8', str(script), 'inspect', '--config', str(self.config)],
                                cwd=self.root, capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(json.loads(result.stdout)['spentMicros'], 0)


if __name__ == '__main__':
    unittest.main(verbosity=2)
