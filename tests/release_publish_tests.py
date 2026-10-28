"""完全模拟 gh，验证已有发布补包、草稿发布顺序及失败不提升latest。"""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from urllib.parse import parse_qs, urlsplit

spec = importlib.util.spec_from_file_location('release_publisher', Path(__file__).parents[1]/'scripts/publish_release.py')
publisher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(publisher)


class FakeGitHub:
    def __init__(self, exists=True, error=None, upload_fails=False, verify_bad=False, no_digest=False,
                 by_tag_404=False, list_error=None, fail_create_response=False, omit_upload=False):
        self.exists, self.error, self.upload_fails = exists, error, upload_fails
        self.verify_bad, self.no_digest = verify_bad, no_digest
        self.calls, self.assets, self.data = [], {}, {}
        self.draft, self.latest, self.prerelease = not exists, exists, False
        self.identifier, self.tag = 17, 'v5.6.7'
        self.by_tag_404, self.list_error = by_tag_404, list_error
        self.fail_create_response, self.omit_upload = fail_create_response, omit_upload
        self.extra_releases, self.pages, self.operations = [], None, []

    def snapshot(self, identifier=None, tag=None, draft=None):
        assets = [dict(item) for item in self.assets.values()]
        if self.verify_bad and assets:
            assets[0]['size'] += 1
        identifier = self.identifier if identifier is None else identifier
        return {'id': identifier, 'tag_name': self.tag if tag is None else tag,
                'assets': assets, 'draft': self.draft if draft is None else draft, 'prerelease': self.prerelease,
                'upload_url': f'https://uploads.github.com/repos/fixture/repository/releases/{identifier}/assets{{?name,label}}'}

    def __call__(self, arguments):
        self.calls.append(arguments)
        code, content, error = 0, b'', b''
        assert arguments[0] == 'api', '发布流程只能经REST绑定稳定ID，不能使用按tag解析的gh release命令'
        endpoint = arguments[1]
        method = arguments[arguments.index('--method')+1] if '--method' in arguments else 'GET'
        upload = endpoint.startswith('https://uploads.github.com/')
        body = None
        if '--input' in arguments and not upload:
            body = json.loads(Path(arguments[arguments.index('--input')+1]).read_text(encoding='utf-8'))
        self.operations.append({'method': method, 'endpoint': endpoint, 'body': body})
        if method == 'GET' and '/releases/tags/' in endpoint:
            if self.error:
                code, error = 1, self.error
            elif not self.exists or self.by_tag_404 or self.draft:
                code, error = 1, b'gh: Not Found (HTTP 404)'
            else:
                content = json.dumps(self.snapshot()).encode()
        elif method == 'GET' and '/releases?' in endpoint:
            if self.list_error:
                code, error = 1, self.list_error
            else:
                page = int(parse_qs(urlsplit(endpoint).query)['page'][0])
                releases = ([self.snapshot()] if self.exists else []) + self.extra_releases
                if self.pages is not None:
                    releases = self.pages[page-1] if page <= len(self.pages) else []
                else:
                    releases = releases[(page-1)*100:page*100]
                content = json.dumps(releases).encode()
        elif method == 'GET' and '/git/ref/tags/' in endpoint:
            content = json.dumps({'ref': 'refs/tags/'+self.tag, 'object': {'type': 'commit', 'sha': 'a'*40}}).encode()
        elif method == 'POST' and endpoint == 'repos/fixture/repository/releases':
            assert not self.exists and body['draft'] is True and body['make_latest'] == 'false'
            assert body['tag_name'] == self.tag
            self.exists, self.draft = True, True
            self.identifier = 31
            if self.fail_create_response:
                self.fail_create_response = False
                code, error = 1, b'gh: Bad Gateway (HTTP 502)'
            else:
                content = json.dumps(self.snapshot()).encode()
        elif '/releases/assets/' in endpoint:
            identifier = int(endpoint.rsplit('/', 1)[1])
            if method == 'GET':
                assert 'Accept: application/octet-stream' in arguments
                content = self.data[identifier]
            elif method == 'DELETE':
                assert self.draft, '先转草稿，再删除该发布的损坏附件'
                for name in list(self.assets):
                    if self.assets[name]['id'] == identifier:
                        del self.assets[name]
            else:
                raise AssertionError('无效附件操作')
        elif method == 'POST' and upload:
            assert urlsplit(endpoint).path == f'/repos/fixture/repository/releases/{self.identifier}/assets'
            assert self.draft and 'Content-Type: application/octet-stream' in arguments
            path = Path(arguments[arguments.index('--input')+1])
            assert parse_qs(urlsplit(endpoint).query)['name'] == [path.name]
            if self.upload_fails:
                code = 1
            else:
                data = path.read_bytes()
                identifier = len(self.data)+1
                item = {'name': path.name, 'id': identifier, 'size': len(data)}
                if not self.no_digest:
                    item['digest'] = 'sha256:'+hashlib.sha256(data).hexdigest()
                if not self.omit_upload:
                    self.assets[path.name], self.data[identifier] = item, data
                content = json.dumps(item).encode()
        elif endpoint == f'repos/fixture/repository/releases/{self.identifier}':
            assert self.exists
            if method == 'GET':
                pass
            elif method == 'PATCH' and body.get('draft') is True:
                self.draft, self.latest = True, False
            elif method == 'PATCH':
                assert len(self.assets) == 3 and body['draft'] is False and body['prerelease'] is False
                assert body['make_latest'] == 'true'
                self.draft, self.latest = False, True
                self.prerelease = False
            else:
                raise AssertionError('无效发布ID操作')
            content = json.dumps(self.snapshot()).encode()
        else:
            raise AssertionError('未允许的外部操作：'+repr(arguments))
        return subprocess.CompletedProcess(arguments, code, content, error)

    def creates(self):
        return [item for item in self.operations if item['method'] == 'POST' and item['endpoint'] == 'repos/fixture/repository/releases']

    def uploads(self):
        return [item for item in self.operations if item['endpoint'].startswith('https://uploads.github.com/')]

    def publications(self):
        return [item for item in self.operations if item['method'] == 'PATCH' and item['body'].get('draft') is False]


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
        self.assertEqual(result['releaseId'], fake.identifier)
        self.assertFalse(fake.creates())
        self.assertEqual(fake.operations[-1]['method'], 'PATCH')
        # 再次运行应仅校验，不能反复删掉并重传已匹配附件。
        fake.calls.clear(); fake.operations.clear(); self.run_publish(fake)
        self.assertFalse(fake.uploads())
        self.assertFalse(any(item['method'] == 'DELETE' for item in fake.operations))

    def test_missing_release_is_draft_until_all_assets_verified(self):
        fake = FakeGitHub(exists=False); self.run_publish(fake)
        self.assertEqual(len(fake.creates()), 1)
        self.assertIs(fake.creates()[0]['body']['draft'], True)
        self.assertIs(fake.publications()[-1]['body']['draft'], False)
        self.assertEqual(fake.identifier, 31)
        self.assertTrue(any(item['method'] == 'GET' and item['endpoint'].endswith('/releases/31') for item in fake.operations))
        self.assertEqual(sum('/releases/tags/' in item['endpoint'] for item in fake.operations), 1)

    def test_existing_prerelease_becomes_verified_regular_release(self):
        fake = FakeGitHub(); fake.prerelease = True
        self.run_publish(fake)
        self.assertFalse(fake.prerelease)
        self.assertTrue(fake.latest)

    def test_partial_existing_release_is_repaired(self):
        fake = FakeGitHub(); self.run_publish(fake)
        fake.assets.pop(next(iter(fake.assets))); fake.calls.clear(); fake.operations.clear()
        self.run_publish(fake)
        self.assertEqual(len(fake.uploads()), 1)

    def test_authentication_error_does_not_create_release(self):
        fake = FakeGitHub(error=b'gh: Bad credentials (HTTP 401)')
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertEqual(len(fake.calls), 1)

    def test_upload_failure_never_sets_latest(self):
        fake = FakeGitHub(upload_fails=True)
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertFalse(fake.publications())

    def test_remote_verification_failure_never_sets_latest(self):
        fake = FakeGitHub(verify_bad=True)
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertFalse(fake.publications())

    def test_public_asset_clobber_failure_stays_draft_and_retry_repairs(self):
        fake = FakeGitHub(); self.run_publish(fake)
        name = next(iter(fake.assets)); fake.assets[name]['digest'] = 'sha256:'+'0'*64
        fake.upload_fails = True; fake.calls.clear(); fake.operations.clear()
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertNotIn(name, fake.assets)
        self.assertTrue(fake.draft)
        self.assertFalse(fake.latest)
        self.assertFalse(fake.publications())
        fake.upload_fails = False; self.run_publish(fake)
        self.assertFalse(fake.draft)
        self.assertTrue(fake.latest)

    def test_missing_remote_digest_downloads_and_hashes_content(self):
        fake = FakeGitHub(no_digest=True); self.run_publish(fake)
        self.assertEqual(sum(args[0] == 'api' and '/assets/' in args[1] for args in fake.calls), 3)

    def test_by_tag_404_reuses_existing_draft_without_creating(self):
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        result = self.run_publish(fake)
        self.assertEqual(result['releaseId'], 17)
        self.assertFalse(fake.creates())
        self.assertEqual(len(fake.uploads()), 3)
        self.assertTrue(any('/releases?per_page=100&page=1' in args[1] for args in fake.calls))
        self.assertTrue(all('/releases/17/assets?' in item['endpoint'] for item in fake.uploads()))
        # 已上传完整附件的草稿再次执行时，只校验原ID，不重新创建或上传。
        fake.draft, fake.latest = True, False
        fake.calls.clear(); fake.operations.clear()
        self.run_publish(fake)
        self.assertFalse(fake.creates())
        self.assertFalse(fake.uploads())

    def test_existing_draft_is_found_after_full_page(self):
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        first = [fake.snapshot(identifier=1000+i, tag=f'v0.0.{i}', draft=False) for i in range(100)]
        fake.pages = [first, [fake.snapshot()]]
        self.run_publish(fake)
        self.assertFalse(fake.creates())
        self.assertTrue(any('page=2' in args[1] for args in fake.calls))

    def test_duplicate_drafts_are_rejected_before_any_mutation(self):
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        fake.extra_releases = [fake.snapshot(identifier=99, draft=True)]
        with self.assertRaisesRegex(RuntimeError, '多个发布记录.*17,99'):
            self.run_publish(fake)
        self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    def test_published_release_with_same_tag_draft_is_rejected_before_mutation(self):
        fake = FakeGitHub()
        fake.extra_releases = [fake.snapshot(identifier=99, draft=True)]
        with self.assertRaisesRegex(RuntimeError, '多个发布记录.*17,99'):
            self.run_publish(fake)
        self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))
        self.assertFalse(fake.creates())
        self.assertFalse(fake.uploads())

    def test_successful_by_tag_with_failed_listing_never_mutates(self):
        for error in (b'gh: Forbidden (HTTP 403)', b'gh: rate limited (HTTP 429)', b'network timeout'):
            with self.subTest(error=error):
                fake = FakeGitHub(list_error=error)
                with self.assertRaises(RuntimeError): self.run_publish(fake)
                self.assertEqual(len(fake.calls), 2)
                self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    def test_successful_by_tag_requires_same_unique_id_in_listing(self):
        for page in ([], [FakeGitHub().snapshot(identifier=99)]):
            with self.subTest(page=page):
                fake = FakeGitHub(); fake.pages = [page]
                with self.assertRaisesRegex(RuntimeError, '发布身份不一致'):
                    self.run_publish(fake)
                self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    def test_duplicate_draft_on_later_page_is_not_silently_chosen(self):
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        fake.pages = [[fake.snapshot()] + [fake.snapshot(identifier=1000+i, tag=f'v0.0.{i}') for i in range(99)],
                      [fake.snapshot(identifier=99, draft=True)]]
        with self.assertRaisesRegex(RuntimeError, '多个发布记录'):
            self.run_publish(fake)
        self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    def test_auth_rate_limit_and_transport_failure_do_not_trigger_draft_search(self):
        for error in (b'gh: Forbidden (HTTP 403)', b'gh: rate limited (HTTP 429)', b'network timeout',
                      b'gh: Forbidden (HTTP 403); context: previous HTTP 404'):
            with self.subTest(error=error):
                fake = FakeGitHub(error=error)
                with self.assertRaises(RuntimeError): self.run_publish(fake)
                self.assertEqual(len(fake.calls), 1)

    def test_failed_draft_listing_does_not_create_release(self):
        for error in (b'gh: Not Found (HTTP 404)', b'gh: Bad credentials (HTTP 401)', b'gh: rate limited (HTTP 429)'):
            with self.subTest(error=error):
                fake = FakeGitHub(exists=False, list_error=error)
                with self.assertRaises(RuntimeError): self.run_publish(fake)
                self.assertFalse(fake.creates())
                self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    def test_successful_create_with_lost_response_is_reused_on_retry(self):
        fake = FakeGitHub(exists=False, fail_create_response=True)
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertTrue(fake.exists and fake.draft)
        self.assertEqual(len(fake.creates()), 1)
        self.run_publish(fake)
        self.assertEqual(len(fake.creates()), 1)
        self.assertFalse(fake.draft)

    def test_missing_uploaded_asset_never_publishes(self):
        fake = FakeGitHub(omit_upload=True)
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        self.assertTrue(fake.draft)
        self.assertFalse(fake.latest)
        self.assertFalse(fake.publications())

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
