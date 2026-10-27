"""完全模拟 gh，验证已有发布补包、草稿发布顺序及失败不提升latest。"""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('release_publisher', Path(__file__).parents[1]/'scripts/publish_release.py')
publisher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(publisher)


class FakeGitHub:
    def __init__(self, exists=True, error=None, upload_fails=False, verify_bad=False, no_digest=False):
        self.exists, self.error, self.upload_fails = exists, error, upload_fails
        self.verify_bad, self.no_digest = verify_bad, no_digest
        self.calls, self.assets, self.data = [], {}, {}
        self.draft, self.latest, self.prerelease = not exists, exists, False

    def __call__(self, arguments):
        self.calls.append(arguments)
        code, content, error = 0, b'', b''
        if arguments[0] == 'api':
            if '/releases/assets/' in arguments[1]:
                content = self.data[int(arguments[1].rsplit('/', 1)[1])]
            elif self.error:
                code, error = 1, self.error
            elif not self.exists:
                code, error = 1, b'gh: Not Found (HTTP 404)'
            else:
                assets = [dict(item) for item in self.assets.values()]
                if self.verify_bad and assets:
                    assets[0]['size'] += 1
                content = json.dumps({'assets': assets, 'draft': self.draft, 'prerelease': self.prerelease}).encode()
        elif arguments[:2] == ['release', 'create']:
            assert '--draft' in arguments and '--latest' not in arguments
            self.exists, self.draft = True, True
        elif arguments[:2] == ['release', 'upload']:
            path = Path(arguments[3])
            # 模拟 gh 的真实 clobber 次序：先删除同名旧文件，即使上传随后失败。
            self.assets.pop(path.name, None)
            if self.upload_fails:
                code = 1
            else:
                data = path.read_bytes()
                identifier = len(self.data)+1
                item = {'name': path.name, 'id': identifier, 'size': len(data)}
                if not self.no_digest:
                    item['digest'] = 'sha256:'+hashlib.sha256(data).hexdigest()
                self.assets[path.name], self.data[identifier] = item, data
        elif arguments[:2] == ['release', 'edit']:
            if '--draft=true' in arguments:
                self.draft, self.latest = True, False
            else:
                assert len(self.assets) == 3 and '--draft=false' in arguments and '--prerelease=false' in arguments
                self.draft, self.latest = False, True
                self.prerelease = False
        else:
            raise AssertionError('未允许的外部操作：'+repr(arguments))
        return subprocess.CompletedProcess(arguments, code, content, error)


class ReleasePublishTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='gangyi-release-fixture-')
        self.root = Path(self.temporary.name)
        self.tag = 'v5.6.7'
        rows = []
        for suffix in ('exe', 'zip'):
            name = f'gangyiAI-learning-preview1-{self.tag}-x64.{suffix}'
            path = self.root/name; path.write_bytes(('synthetic-'+suffix).encode())
            rows.append(hashlib.sha256(path.read_bytes()).hexdigest()+' *'+name)
        (self.root/'SHA256SUMS.txt').write_text('\n'.join(rows)+'\n', encoding='utf-8')
        self.notes = self.root/'notes.md'; self.notes.write_text('合成发布说明', encoding='utf-8')

    def tearDown(self):
        self.temporary.cleanup()

    def run_publish(self, fake):
        return publisher.publish('fixture/repository', self.tag, self.root, self.notes, fake)

    def test_existing_release_receives_missing_assets_before_latest(self):
        fake = FakeGitHub(); result = self.run_publish(fake)
        self.assertEqual(len(result['verifiedAssets']), 3)
        self.assertFalse(any(args[:2] == ['release', 'create'] for args in fake.calls))
        self.assertEqual(fake.calls[-1][:2], ['release', 'edit'])
        # 再次运行应仅校验，不能反复删掉并重传已匹配附件。
        fake.calls.clear(); self.run_publish(fake)
        self.assertFalse(any(args[:2] == ['release', 'upload'] for args in fake.calls))

    def test_missing_release_is_draft_until_all_assets_verified(self):
        fake = FakeGitHub(exists=False); self.run_publish(fake)
        self.assertEqual(fake.calls[1][:2], ['release', 'create'])
        self.assertIn('--draft', fake.calls[1])
        self.assertIn('--draft=false', fake.calls[-1])

    def test_existing_prerelease_becomes_verified_regular_release(self):
        fake = FakeGitHub(); fake.prerelease = True
        self.run_publish(fake)
        self.assertFalse(fake.prerelease)
        self.assertTrue(fake.latest)

    def test_partial_existing_release_is_repaired(self):
        fake = FakeGitHub(); self.run_publish(fake)
        fake.assets.pop(next(iter(fake.assets))); fake.calls.clear()
        self.run_publish(fake)
        self.assertEqual(sum(args[:2] == ['release', 'upload'] for args in fake.calls), 1)

    def test_authentication_error_does_not_create_release(self):
        fake = FakeGitHub(error=b'gh: Bad credentials (HTTP 401)')
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertEqual(len(fake.calls), 1)

    def test_upload_failure_never_sets_latest(self):
        fake = FakeGitHub(upload_fails=True)
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertFalse(any('--latest' in args for args in fake.calls))

    def test_remote_verification_failure_never_sets_latest(self):
        fake = FakeGitHub(verify_bad=True)
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertFalse(any('--latest' in args for args in fake.calls))

    def test_public_asset_clobber_failure_stays_draft_and_retry_repairs(self):
        fake = FakeGitHub(); self.run_publish(fake)
        name = next(iter(fake.assets)); fake.assets[name]['digest'] = 'sha256:'+'0'*64
        fake.upload_fails = True; fake.calls.clear()
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertNotIn(name, fake.assets)
        self.assertTrue(fake.draft)
        self.assertFalse(fake.latest)
        self.assertFalse(any('--latest' in args for args in fake.calls))
        fake.upload_fails = False; self.run_publish(fake)
        self.assertFalse(fake.draft)
        self.assertTrue(fake.latest)

    def test_missing_remote_digest_downloads_and_hashes_content(self):
        fake = FakeGitHub(no_digest=True); self.run_publish(fake)
        self.assertEqual(sum(args[0] == 'api' and '/assets/' in args[1] for args in fake.calls), 3)

    def test_bad_local_checksum_stops_before_external_calls(self):
        next(self.root.glob('*.exe')).write_bytes(b'changed')
        fake = FakeGitHub()
        with self.assertRaises(ValueError): self.run_publish(fake)
        self.assertEqual(fake.calls, [])

    def test_path_escape_manifest_stops_before_external_calls(self):
        (self.root/'SHA256SUMS.txt').write_text('0'*64+' *../private.db\n', encoding='utf-8')
        fake = FakeGitHub()
        with self.assertRaises(ValueError): self.run_publish(fake)
        self.assertEqual(fake.calls, [])


if __name__ == '__main__':
    unittest.main()
