# 【逐行批注】 执行当前 Python 语句："""完全模拟 gh，验证已有发布补包、草稿发布顺序及失败不提升latest。"""。
"""完全模拟 gh，验证已有发布补包、草稿发布顺序及失败不提升latest。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import hashlib。
import hashlib
# 【逐行批注】 导入后续逻辑需要的模块或符号：import importlib.util。
import importlib.util
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import unittest。
import unittest
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import parse_qs, urlsplit。
from urllib.parse import parse_qs, urlsplit

# 【逐行批注】 计算右侧结果并赋给 spec，更新脚本状态。
spec = importlib.util.spec_from_file_location('release_publisher', Path(__file__).parents[1]/'scripts/publish_release.py')
# 【逐行批注】 计算右侧结果并赋给 publisher，更新脚本状态。
publisher = importlib.util.module_from_spec(spec)
# 【逐行批注】 执行当前 Python 语句：spec.loader.exec_module(publisher)。
spec.loader.exec_module(publisher)


# 【逐行批注】 定义类 FakeGitHub，组织相关数据与行为。
class FakeGitHub:
    # 【逐行批注】 定义函数 __init__，封装可复用的处理步骤。
    def __init__(self, exists=True, error=None, upload_fails=False, verify_bad=False, no_digest=False,
                 # 【逐行批注】 计算右侧结果并赋给 by_tag_404，更新脚本状态。
                 by_tag_404=False, list_error=None, fail_create_response=False, omit_upload=False):
        # 【逐行批注】 计算右侧结果并赋给 self.exists, self.error, self.upload_fails，更新脚本状态。
        self.exists, self.error, self.upload_fails = exists, error, upload_fails
        # 【逐行批注】 计算右侧结果并赋给 self.verify_bad, self.no_digest，更新脚本状态。
        self.verify_bad, self.no_digest = verify_bad, no_digest
        # 【逐行批注】 计算右侧结果并赋给 self.calls, self.assets, self.data，更新脚本状态。
        self.calls, self.assets, self.data = [], {}, {}
        # 【逐行批注】 计算右侧结果并赋给 self.draft, self.latest, self.prerelease，更新脚本状态。
        self.draft, self.latest, self.prerelease = not exists, exists, False
        # 【逐行批注】 计算右侧结果并赋给 self.identifier, self.tag，更新脚本状态。
        self.identifier, self.tag = 17, 'v5.6.7'
        # 【逐行批注】 计算右侧结果并赋给 self.by_tag_404, self.list_error，更新脚本状态。
        self.by_tag_404, self.list_error = by_tag_404, list_error
        # 【逐行批注】 计算右侧结果并赋给 self.fail_create_response, self.omit_upload，更新脚本状态。
        self.fail_create_response, self.omit_upload = fail_create_response, omit_upload
        # 【逐行批注】 计算右侧结果并赋给 self.extra_releases, self.pages, self.operat…，更新脚本状态。
        self.extra_releases, self.pages, self.operations = [], None, []

    # 【逐行批注】 定义函数 snapshot，封装可复用的处理步骤。
    def snapshot(self, identifier=None, tag=None, draft=None):
        # 【逐行批注】 计算右侧结果并赋给 assets，更新脚本状态。
        assets = [dict(item) for item in self.assets.values()]
        # 【逐行批注】 判断条件 self.verify_bad and assets:，成立时执行对应分支。
        if self.verify_bad and assets:
            # 【逐行批注】 计算右侧结果并赋给 assets[0]['size'] +，更新脚本状态。
            assets[0]['size'] += 1
        # 【逐行批注】 计算右侧结果并赋给 identifier，更新脚本状态。
        identifier = self.identifier if identifier is None else identifier
        # 【逐行批注】 结束当前函数并返回结果：return {'id': identifier, 'tag_name': self.tag if tag is None else ta…。
        return {'id': identifier, 'tag_name': self.tag if tag is None else tag,
                # 【逐行批注】 执行当前 Python 语句：'assets': assets, 'draft': self.draft if draft is None else draft, 'prerelease': self.pre…。
                'assets': assets, 'draft': self.draft if draft is None else draft, 'prerelease': self.prerelease,
                # 【逐行批注】 执行当前 Python 语句：'upload_url': f'https://uploads.github.com/repos/fixture/repository/releases/{identifier}…。
                'upload_url': f'https://uploads.github.com/repos/fixture/repository/releases/{identifier}/assets{{?name,label}}'}

    # 【逐行批注】 定义函数 __call__，封装可复用的处理步骤。
    def __call__(self, arguments):
        # 【逐行批注】 执行当前 Python 语句：self.calls.append(arguments)。
        self.calls.append(arguments)
        # 【逐行批注】 计算右侧结果并赋给 code, content, error，更新脚本状态。
        code, content, error = 0, b'', b''
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert arguments[0] == 'api', '发布流程只能经REST绑定稳定ID，不能使用按tag解析的gh release命令'。
        assert arguments[0] == 'api', '发布流程只能经REST绑定稳定ID，不能使用按tag解析的gh release命令'
        # 【逐行批注】 计算右侧结果并赋给 endpoint，更新脚本状态。
        endpoint = arguments[1]
        # 【逐行批注】 计算右侧结果并赋给 method，更新脚本状态。
        method = arguments[arguments.index('--method')+1] if '--method' in arguments else 'GET'
        # 【逐行批注】 计算右侧结果并赋给 upload，更新脚本状态。
        upload = endpoint.startswith('https://uploads.github.com/')
        # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
        body = None
        # 【逐行批注】 判断条件 '--input' in arguments and not upload:，成立时执行对应分支。
        if '--input' in arguments and not upload:
            # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
            body = json.loads(Path(arguments[arguments.index('--input')+1]).read_text(encoding='utf-8'))
        # 【逐行批注】 执行当前 Python 语句：self.operations.append({'method': method, 'endpoint': endpoint, 'body': body})。
        self.operations.append({'method': method, 'endpoint': endpoint, 'body': body})
        # 【逐行批注】 判断条件 method == 'GET' and '/releases/tags/' in endpoint:，成立时执行对应分支。
        if method == 'GET' and '/releases/tags/' in endpoint:
            # 【逐行批注】 判断条件 self.error:，成立时执行对应分支。
            if self.error:
                # 【逐行批注】 计算右侧结果并赋给 code, error，更新脚本状态。
                code, error = 1, self.error
            # 【逐行批注】 当前一条件不成立时继续判断：not self.exists or self.by_tag_404 or self.draft:。
            elif not self.exists or self.by_tag_404 or self.draft:
                # 【逐行批注】 计算右侧结果并赋给 code, error，更新脚本状态。
                code, error = 1, b'gh: Not Found (HTTP 404)'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
                content = json.dumps(self.snapshot()).encode()
        # 【逐行批注】 当前一条件不成立时继续判断：method == 'GET' and '/releases?' in endpoint:。
        elif method == 'GET' and '/releases?' in endpoint:
            # 【逐行批注】 判断条件 self.list_error:，成立时执行对应分支。
            if self.list_error:
                # 【逐行批注】 计算右侧结果并赋给 code, error，更新脚本状态。
                code, error = 1, self.list_error
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 page，更新脚本状态。
                page = int(parse_qs(urlsplit(endpoint).query)['page'][0])
                # 【逐行批注】 计算右侧结果并赋给 releases，更新脚本状态。
                releases = ([self.snapshot()] if self.exists else []) + self.extra_releases
                # 【逐行批注】 判断条件 self.pages is not None:，成立时执行对应分支。
                if self.pages is not None:
                    # 【逐行批注】 计算右侧结果并赋给 releases，更新脚本状态。
                    releases = self.pages[page-1] if page <= len(self.pages) else []
                # 【逐行批注】 执行前一条件不成立时的分支。
                else:
                    # 【逐行批注】 计算右侧结果并赋给 releases，更新脚本状态。
                    releases = releases[(page-1)*100:page*100]
                # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
                content = json.dumps(releases).encode()
        # 【逐行批注】 当前一条件不成立时继续判断：method == 'GET' and '/git/ref/tags/' in endpoint:。
        elif method == 'GET' and '/git/ref/tags/' in endpoint:
            # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
            content = json.dumps({'ref': 'refs/tags/'+self.tag, 'object': {'type': 'commit', 'sha': 'a'*40}}).encode()
        # 【逐行批注】 当前一条件不成立时继续判断：method == 'POST' and endpoint == 'repos/fixture/repository/relea…。
        elif method == 'POST' and endpoint == 'repos/fixture/repository/releases':
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not self.exists and body['draft'] is True and body['make_latest'] == 'fa…。
            assert not self.exists and body['draft'] is True and body['make_latest'] == 'false'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert body['tag_name'] == self.tag。
            assert body['tag_name'] == self.tag
            # 【逐行批注】 计算右侧结果并赋给 self.exists, self.draft，更新脚本状态。
            self.exists, self.draft = True, True
            # 【逐行批注】 计算右侧结果并赋给 self.identifier，更新脚本状态。
            self.identifier = 31
            # 【逐行批注】 判断条件 self.fail_create_response:，成立时执行对应分支。
            if self.fail_create_response:
                # 【逐行批注】 计算右侧结果并赋给 self.fail_create_response，更新脚本状态。
                self.fail_create_response = False
                # 【逐行批注】 计算右侧结果并赋给 code, error，更新脚本状态。
                code, error = 1, b'gh: Bad Gateway (HTTP 502)'
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
                content = json.dumps(self.snapshot()).encode()
        # 【逐行批注】 当前一条件不成立时继续判断：'/releases/assets/' in endpoint:。
        elif '/releases/assets/' in endpoint:
            # 【逐行批注】 计算右侧结果并赋给 identifier，更新脚本状态。
            identifier = int(endpoint.rsplit('/', 1)[1])
            # 【逐行批注】 判断条件 method == 'GET':，成立时执行对应分支。
            if method == 'GET':
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert 'Accept: application/octet-stream' in arguments。
                assert 'Accept: application/octet-stream' in arguments
                # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
                content = self.data[identifier]
            # 【逐行批注】 当前一条件不成立时继续判断：method == 'DELETE':。
            elif method == 'DELETE':
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert self.draft, '先转草稿，再删除该发布的损坏附件'。
                assert self.draft, '先转草稿，再删除该发布的损坏附件'
                # 【逐行批注】 遍历数据并重复执行循环体：for name in list(self.assets):。
                for name in list(self.assets):
                    # 【逐行批注】 判断条件 self.assets[name]['id'] == identifier:，成立时执行对应分支。
                    if self.assets[name]['id'] == identifier:
                        # 【逐行批注】 执行当前 Python 语句：del self.assets[name]。
                        del self.assets[name]
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('无效附件操作')。
                raise AssertionError('无效附件操作')
        # 【逐行批注】 当前一条件不成立时继续判断：method == 'POST' and upload:。
        elif method == 'POST' and upload:
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert urlsplit(endpoint).path == f'/repos/fixture/repository/releases/{self.id…。
            assert urlsplit(endpoint).path == f'/repos/fixture/repository/releases/{self.identifier}/assets'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert self.draft and 'Content-Type: application/octet-stream' in arguments。
            assert self.draft and 'Content-Type: application/octet-stream' in arguments
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = Path(arguments[arguments.index('--input')+1])
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parse_qs(urlsplit(endpoint).query)['name'] == [path.name]。
            assert parse_qs(urlsplit(endpoint).query)['name'] == [path.name]
            # 【逐行批注】 判断条件 self.upload_fails:，成立时执行对应分支。
            if self.upload_fails:
                # 【逐行批注】 计算右侧结果并赋给 code，更新脚本状态。
                code = 1
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                data = path.read_bytes()
                # 【逐行批注】 计算右侧结果并赋给 identifier，更新脚本状态。
                identifier = len(self.data)+1
                # 【逐行批注】 计算右侧结果并赋给 item，更新脚本状态。
                item = {'name': path.name, 'id': identifier, 'size': len(data)}
                # 【逐行批注】 判断条件 not self.no_digest:，成立时执行对应分支。
                if not self.no_digest:
                    # 【逐行批注】 计算右侧结果并赋给 item['digest']，更新脚本状态。
                    item['digest'] = 'sha256:'+hashlib.sha256(data).hexdigest()
                # 【逐行批注】 判断条件 not self.omit_upload:，成立时执行对应分支。
                if not self.omit_upload:
                    # 【逐行批注】 计算右侧结果并赋给 self.assets[path.name], self.data[identifier]，更新脚本状态。
                    self.assets[path.name], self.data[identifier] = item, data
                # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
                content = json.dumps(item).encode()
        # 【逐行批注】 当前一条件不成立时继续判断：endpoint == f'repos/fixture/repository/releases/{self.identifier…。
        elif endpoint == f'repos/fixture/repository/releases/{self.identifier}':
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert self.exists。
            assert self.exists
            # 【逐行批注】 判断条件 method == 'GET':，成立时执行对应分支。
            if method == 'GET':
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 当前一条件不成立时继续判断：method == 'PATCH' and body.get('draft') is True:。
            elif method == 'PATCH' and body.get('draft') is True:
                # 【逐行批注】 计算右侧结果并赋给 self.draft, self.latest，更新脚本状态。
                self.draft, self.latest = True, False
            # 【逐行批注】 当前一条件不成立时继续判断：method == 'PATCH':。
            elif method == 'PATCH':
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert len(self.assets) == 3 and body['draft'] is False and body['prerelease'] …。
                assert len(self.assets) == 3 and body['draft'] is False and body['prerelease'] is False
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert body['make_latest'] == 'true'。
                assert body['make_latest'] == 'true'
                # 【逐行批注】 计算右侧结果并赋给 self.draft, self.latest，更新脚本状态。
                self.draft, self.latest = False, True
                # 【逐行批注】 计算右侧结果并赋给 self.prerelease，更新脚本状态。
                self.prerelease = False
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('无效发布ID操作')。
                raise AssertionError('无效发布ID操作')
            # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
            content = json.dumps(self.snapshot()).encode()
        # 【逐行批注】 执行前一条件不成立时的分支。
        else:
            # 【逐行批注】 在错误情形抛出异常：raise AssertionError('未允许的外部操作：'+repr(arguments))。
            raise AssertionError('未允许的外部操作：'+repr(arguments))
        # 【逐行批注】 结束当前函数并返回结果：return subprocess.CompletedProcess(arguments, code, content, error)。
        return subprocess.CompletedProcess(arguments, code, content, error)

    # 【逐行批注】 定义函数 creates，封装可复用的处理步骤。
    def creates(self):
        # 【逐行批注】 结束当前函数并返回结果：return [item for item in self.operations if item['method'] == 'POST' …。
        return [item for item in self.operations if item['method'] == 'POST' and item['endpoint'] == 'repos/fixture/repository/releases']

    # 【逐行批注】 定义函数 uploads，封装可复用的处理步骤。
    def uploads(self):
        # 【逐行批注】 结束当前函数并返回结果：return [item for item in self.operations if item['endpoint'].startswi…。
        return [item for item in self.operations if item['endpoint'].startswith('https://uploads.github.com/')]

    # 【逐行批注】 定义函数 publications，封装可复用的处理步骤。
    def publications(self):
        # 【逐行批注】 结束当前函数并返回结果：return [item for item in self.operations if item['method'] == 'PATCH'…。
        return [item for item in self.operations if item['method'] == 'PATCH' and item['body'].get('draft') is False]


# 【逐行批注】 定义类 ReleasePublishTests，组织相关数据与行为。
class ReleasePublishTests(unittest.TestCase):
    # 【逐行批注】 定义函数 setUp，封装可复用的处理步骤。
    def setUp(self):
        # 【逐行批注】 计算右侧结果并赋给 self.temporary，更新脚本状态。
        self.temporary = tempfile.TemporaryDirectory(prefix='gangyi-release-fixture-')
        # 【逐行批注】 计算右侧结果并赋给 self.root，更新脚本状态。
        self.root = Path(self.temporary.name)
        # 【逐行批注】 计算右侧结果并赋给 self.tag，更新脚本状态。
        self.tag = 'v5.6.7'
        # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
        rows = []
        # 【逐行批注】 遍历数据并重复执行循环体：for suffix in ('exe', 'zip'):。
        for suffix in ('exe', 'zip'):
            # 【逐行批注】 计算右侧结果并赋给 name，更新脚本状态。
            name = f'gangyiAI-learning-preview1-{self.tag}-x64.{suffix}'
            # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
            path = self.root/name; path.write_bytes(('synthetic-'+suffix).encode())
            # 【逐行批注】 执行当前 Python 语句：rows.append(hashlib.sha256(path.read_bytes()).hexdigest()+' *'+name)。
            rows.append(hashlib.sha256(path.read_bytes()).hexdigest()+' *'+name)
        # 【逐行批注】 计算右侧结果并赋给 (self.root/'SHA256SUMS.txt').write_text('\n'…，更新脚本状态。
        (self.root/'SHA256SUMS.txt').write_text('\n'.join(rows)+'\n', encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 self.notes，更新脚本状态。
        self.notes = self.root/'notes.md'; self.notes.write_text('合成发布说明', encoding='utf-8')

    # 【逐行批注】 定义函数 tearDown，封装可复用的处理步骤。
    def tearDown(self):
        # 【逐行批注】 执行当前 Python 语句：self.temporary.cleanup()。
        self.temporary.cleanup()

    # 【逐行批注】 定义函数 run_publish，封装可复用的处理步骤。
    def run_publish(self, fake):
        # 【逐行批注】 结束当前函数并返回结果：return publisher.publish('fixture/repository', self.tag, self.root, s…。
        return publisher.publish('fixture/repository', self.tag, self.root, self.notes, fake)

    # 【逐行批注】 定义函数 test_existing_release_receives_missing_assets_before_latest，封装可复用的处理步骤。
    def test_existing_release_receives_missing_assets_before_latest(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(); result = self.run_publish(fake)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(result['verifiedAssets']), 3)。
        self.assertEqual(len(result['verifiedAssets']), 3)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['releaseId'], fake.identifier)。
        self.assertEqual(result['releaseId'], fake.identifier)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.creates())。
        self.assertFalse(fake.creates())
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(fake.operations[-1]['method'], 'PATCH')。
        self.assertEqual(fake.operations[-1]['method'], 'PATCH')
        # 再次运行应仅校验，不能反复删掉并重传已匹配附件。
        # 【逐行批注】 执行当前 Python 语句：fake.calls.clear(); fake.operations.clear(); self.run_publish(fake)。
        fake.calls.clear(); fake.operations.clear(); self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.uploads())。
        self.assertFalse(fake.uploads())
        # 【逐行批注】 调用 any 完成当前脚本或测试步骤：self.assertFalse(any(item['method'] == 'DELETE' for item in fake.oper…。
        self.assertFalse(any(item['method'] == 'DELETE' for item in fake.operations))

    # 【逐行批注】 定义函数 test_missing_release_is_draft_until_all_assets_verified，封装可复用的处理步骤。
    def test_missing_release_is_draft_until_all_assets_verified(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(exists=False); self.run_publish(fake)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.creates()), 1)。
        self.assertEqual(len(fake.creates()), 1)
        # 【逐行批注】 执行当前 Python 语句：self.assertIs(fake.creates()[0]['body']['draft'], True)。
        self.assertIs(fake.creates()[0]['body']['draft'], True)
        # 【逐行批注】 执行当前 Python 语句：self.assertIs(fake.publications()[-1]['body']['draft'], False)。
        self.assertIs(fake.publications()[-1]['body']['draft'], False)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(fake.identifier, 31)。
        self.assertEqual(fake.identifier, 31)
        # 【逐行批注】 调用 any 完成当前脚本或测试步骤：self.assertTrue(any(item['method'] == 'GET' and item['endpoint'].ends…。
        self.assertTrue(any(item['method'] == 'GET' and item['endpoint'].endswith('/releases/31') for item in fake.operations))
        # 【逐行批注】 调用 sum 完成当前脚本或测试步骤：self.assertEqual(sum('/releases/tags/' in item['endpoint'] for item i…。
        self.assertEqual(sum('/releases/tags/' in item['endpoint'] for item in fake.operations), 1)

    # 【逐行批注】 定义函数 test_existing_prerelease_becomes_verified_regular_release，封装可复用的处理步骤。
    def test_existing_prerelease_becomes_verified_regular_release(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(); fake.prerelease = True
        # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
        self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.prerelease)。
        self.assertFalse(fake.prerelease)
        # 【逐行批注】 执行当前 Python 语句：self.assertTrue(fake.latest)。
        self.assertTrue(fake.latest)

    # 【逐行批注】 定义函数 test_partial_existing_release_is_repaired，封装可复用的处理步骤。
    def test_partial_existing_release_is_repaired(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(); self.run_publish(fake)
        # 【逐行批注】 调用 next、iter 完成当前脚本或测试步骤：fake.assets.pop(next(iter(fake.assets))); fake.calls.clear(); fake.op…。
        fake.assets.pop(next(iter(fake.assets))); fake.calls.clear(); fake.operations.clear()
        # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
        self.run_publish(fake)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.uploads()), 1)。
        self.assertEqual(len(fake.uploads()), 1)

    # 【逐行批注】 定义函数 test_authentication_error_does_not_create_release，封装可复用的处理步骤。
    def test_authentication_error_does_not_create_release(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(error=b'gh: Bad credentials (HTTP 401)')
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.calls), 1)。
        self.assertEqual(len(fake.calls), 1)

    # 【逐行批注】 定义函数 test_upload_failure_never_sets_latest，封装可复用的处理步骤。
    def test_upload_failure_never_sets_latest(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(upload_fails=True)
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.publications())。
        self.assertFalse(fake.publications())

    # 【逐行批注】 定义函数 test_remote_verification_failure_never_sets_latest，封装可复用的处理步骤。
    def test_remote_verification_failure_never_sets_latest(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(verify_bad=True)
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.publications())。
        self.assertFalse(fake.publications())

    # 【逐行批注】 定义函数 test_public_asset_clobber_failure_stays_draft_and_retry_repairs，封装可复用的处理步骤。
    def test_public_asset_clobber_failure_stays_draft_and_retry_repairs(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(); self.run_publish(fake)
        # 【逐行批注】 计算右侧结果并赋给 name，更新脚本状态。
        name = next(iter(fake.assets)); fake.assets[name]['digest'] = 'sha256:'+'0'*64
        # 【逐行批注】 计算右侧结果并赋给 fake.upload_fails，更新脚本状态。
        fake.upload_fails = True; fake.calls.clear(); fake.operations.clear()
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertNotIn(name, fake.assets)。
        self.assertNotIn(name, fake.assets)
        # 【逐行批注】 执行当前 Python 语句：self.assertTrue(fake.draft)。
        self.assertTrue(fake.draft)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.latest)。
        self.assertFalse(fake.latest)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.publications())。
        self.assertFalse(fake.publications())
        # 【逐行批注】 计算右侧结果并赋给 fake.upload_fails，更新脚本状态。
        fake.upload_fails = False; self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.draft)。
        self.assertFalse(fake.draft)
        # 【逐行批注】 执行当前 Python 语句：self.assertTrue(fake.latest)。
        self.assertTrue(fake.latest)

    # 【逐行批注】 定义函数 test_missing_remote_digest_downloads_and_hashes_content，封装可复用的处理步骤。
    def test_missing_remote_digest_downloads_and_hashes_content(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(no_digest=True); self.run_publish(fake)
        # 【逐行批注】 调用 sum 完成当前脚本或测试步骤：self.assertEqual(sum(args[0] == 'api' and '/assets/' in args[1] for a…。
        self.assertEqual(sum(args[0] == 'api' and '/assets/' in args[1] for args in fake.calls), 3)

    # 【逐行批注】 定义函数 test_by_tag_404_reuses_existing_draft_without_creating，封装可复用的处理步骤。
    def test_by_tag_404_reuses_existing_draft_without_creating(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(result['releaseId'], 17)。
        self.assertEqual(result['releaseId'], 17)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.creates())。
        self.assertFalse(fake.creates())
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.uploads()), 3)。
        self.assertEqual(len(fake.uploads()), 3)
        # 【逐行批注】 计算右侧结果并赋给 self.assertTrue(any('/releases?per_page，更新脚本状态。
        self.assertTrue(any('/releases?per_page=100&page=1' in args[1] for args in fake.calls))
        # 【逐行批注】 调用 all 完成当前脚本或测试步骤：self.assertTrue(all('/releases/17/assets?' in item['endpoint'] for it…。
        self.assertTrue(all('/releases/17/assets?' in item['endpoint'] for item in fake.uploads()))
        # 已上传完整附件的草稿再次执行时，只校验原ID，不重新创建或上传。
        # 【逐行批注】 计算右侧结果并赋给 fake.draft, fake.latest，更新脚本状态。
        fake.draft, fake.latest = True, False
        # 【逐行批注】 执行当前 Python 语句：fake.calls.clear(); fake.operations.clear()。
        fake.calls.clear(); fake.operations.clear()
        # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
        self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.creates())。
        self.assertFalse(fake.creates())
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.uploads())。
        self.assertFalse(fake.uploads())

    # 【逐行批注】 定义函数 test_existing_draft_is_found_after_full_page，封装可复用的处理步骤。
    def test_existing_draft_is_found_after_full_page(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        # 【逐行批注】 计算右侧结果并赋给 first，更新脚本状态。
        first = [fake.snapshot(identifier=1000+i, tag=f'v0.0.{i}', draft=False) for i in range(100)]
        # 【逐行批注】 计算右侧结果并赋给 fake.pages，更新脚本状态。
        fake.pages = [first, [fake.snapshot()]]
        # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
        self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.creates())。
        self.assertFalse(fake.creates())
        # 【逐行批注】 计算右侧结果并赋给 self.assertTrue(any('page，更新脚本状态。
        self.assertTrue(any('page=2' in args[1] for args in fake.calls))

    # 【逐行批注】 定义函数 test_duplicate_drafts_are_rejected_before_any_mutation，封装可复用的处理步骤。
    def test_duplicate_drafts_are_rejected_before_any_mutation(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        # 【逐行批注】 计算右侧结果并赋给 fake.extra_releases，更新脚本状态。
        fake.extra_releases = [fake.snapshot(identifier=99, draft=True)]
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaisesRegex(RuntimeError, '多个发布记录.*17,99'):。
        with self.assertRaisesRegex(RuntimeError, '多个发布记录.*17,99'):
            # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
            self.run_publish(fake)
        # 【逐行批注】 调用 all 完成当前脚本或测试步骤：self.assertTrue(all(item['method'] == 'GET' for item in fake.operatio…。
        self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    # 【逐行批注】 定义函数 test_published_release_with_same_tag_draft_is_rejected_before_mutation，封装可复用的处理步骤。
    def test_published_release_with_same_tag_draft_is_rejected_before_mutation(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub()
        # 【逐行批注】 计算右侧结果并赋给 fake.extra_releases，更新脚本状态。
        fake.extra_releases = [fake.snapshot(identifier=99, draft=True)]
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaisesRegex(RuntimeError, '多个发布记录.*17,99'):。
        with self.assertRaisesRegex(RuntimeError, '多个发布记录.*17,99'):
            # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
            self.run_publish(fake)
        # 【逐行批注】 调用 all 完成当前脚本或测试步骤：self.assertTrue(all(item['method'] == 'GET' for item in fake.operatio…。
        self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.creates())。
        self.assertFalse(fake.creates())
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.uploads())。
        self.assertFalse(fake.uploads())

    # 【逐行批注】 定义函数 test_successful_by_tag_with_failed_listing_never_mutates，封装可复用的处理步骤。
    def test_successful_by_tag_with_failed_listing_never_mutates(self):
        # 【逐行批注】 遍历数据并重复执行循环体：for error in (b'gh: Forbidden (HTTP 403)', b'gh: rate limited (HTTP 429)',…。
        for error in (b'gh: Forbidden (HTTP 403)', b'gh: rate limited (HTTP 429)', b'network timeout'):
            # 【逐行批注】 在受控上下文中执行资源操作：with self.subTest(error=error):。
            with self.subTest(error=error):
                # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
                fake = FakeGitHub(list_error=error)
                # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
                with self.assertRaises(RuntimeError): self.run_publish(fake)
                # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.calls), 2)。
                self.assertEqual(len(fake.calls), 2)
                # 【逐行批注】 调用 all 完成当前脚本或测试步骤：self.assertTrue(all(item['method'] == 'GET' for item in fake.operatio…。
                self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    # 【逐行批注】 定义函数 test_successful_by_tag_requires_same_unique_id_in_listing，封装可复用的处理步骤。
    def test_successful_by_tag_requires_same_unique_id_in_listing(self):
        # 【逐行批注】 遍历数据并重复执行循环体：for page in ([], [FakeGitHub().snapshot(identifier=99)]):。
        for page in ([], [FakeGitHub().snapshot(identifier=99)]):
            # 【逐行批注】 在受控上下文中执行资源操作：with self.subTest(page=page):。
            with self.subTest(page=page):
                # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
                fake = FakeGitHub(); fake.pages = [page]
                # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaisesRegex(RuntimeError, '发布身份不一致'):。
                with self.assertRaisesRegex(RuntimeError, '发布身份不一致'):
                    # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
                    self.run_publish(fake)
                # 【逐行批注】 调用 all 完成当前脚本或测试步骤：self.assertTrue(all(item['method'] == 'GET' for item in fake.operatio…。
                self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    # 【逐行批注】 定义函数 test_duplicate_draft_on_later_page_is_not_silently_chosen，封装可复用的处理步骤。
    def test_duplicate_draft_on_later_page_is_not_silently_chosen(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(by_tag_404=True); fake.draft = True
        # 【逐行批注】 计算右侧结果并赋给 fake.pages，更新脚本状态。
        fake.pages = [[fake.snapshot()] + [fake.snapshot(identifier=1000+i, tag=f'v0.0.{i}') for i in range(99)],
                      # 【逐行批注】 计算右侧结果并赋给 [fake.snapshot(identifier，更新脚本状态。
                      [fake.snapshot(identifier=99, draft=True)]]
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaisesRegex(RuntimeError, '多个发布记录'):。
        with self.assertRaisesRegex(RuntimeError, '多个发布记录'):
            # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
            self.run_publish(fake)
        # 【逐行批注】 调用 all 完成当前脚本或测试步骤：self.assertTrue(all(item['method'] == 'GET' for item in fake.operatio…。
        self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    # 【逐行批注】 定义函数 test_auth_rate_limit_and_transport_failure_do_not_trigger_draft_search，封装可复用的处理步骤。
    def test_auth_rate_limit_and_transport_failure_do_not_trigger_draft_search(self):
        # 【逐行批注】 遍历数据并重复执行循环体：for error in (b'gh: Forbidden (HTTP 403)', b'gh: rate limited (HTTP 429)',…。
        for error in (b'gh: Forbidden (HTTP 403)', b'gh: rate limited (HTTP 429)', b'network timeout',
                      # 【逐行批注】 调用 Forbidden 完成当前脚本或测试步骤：b'gh: Forbidden (HTTP 403); context: previous HTTP 404'):。
                      b'gh: Forbidden (HTTP 403); context: previous HTTP 404'):
            # 【逐行批注】 在受控上下文中执行资源操作：with self.subTest(error=error):。
            with self.subTest(error=error):
                # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
                fake = FakeGitHub(error=error)
                # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
                with self.assertRaises(RuntimeError): self.run_publish(fake)
                # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.calls), 1)。
                self.assertEqual(len(fake.calls), 1)

    # 【逐行批注】 定义函数 test_failed_draft_listing_does_not_create_release，封装可复用的处理步骤。
    def test_failed_draft_listing_does_not_create_release(self):
        # 【逐行批注】 遍历数据并重复执行循环体：for error in (b'gh: Not Found (HTTP 404)', b'gh: Bad credentials (HTTP 401…。
        for error in (b'gh: Not Found (HTTP 404)', b'gh: Bad credentials (HTTP 401)', b'gh: rate limited (HTTP 429)'):
            # 【逐行批注】 在受控上下文中执行资源操作：with self.subTest(error=error):。
            with self.subTest(error=error):
                # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
                fake = FakeGitHub(exists=False, list_error=error)
                # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
                with self.assertRaises(RuntimeError): self.run_publish(fake)
                # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.creates())。
                self.assertFalse(fake.creates())
                # 【逐行批注】 调用 all 完成当前脚本或测试步骤：self.assertTrue(all(item['method'] == 'GET' for item in fake.operatio…。
                self.assertTrue(all(item['method'] == 'GET' for item in fake.operations))

    # 【逐行批注】 定义函数 test_successful_create_with_lost_response_is_reused_on_retry，封装可复用的处理步骤。
    def test_successful_create_with_lost_response_is_reused_on_retry(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(exists=False, fail_create_response=True)
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertTrue(fake.exists and fake.draft)。
        self.assertTrue(fake.exists and fake.draft)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.creates()), 1)。
        self.assertEqual(len(fake.creates()), 1)
        # 【逐行批注】 执行当前 Python 语句：self.run_publish(fake)。
        self.run_publish(fake)
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：self.assertEqual(len(fake.creates()), 1)。
        self.assertEqual(len(fake.creates()), 1)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.draft)。
        self.assertFalse(fake.draft)

    # 【逐行批注】 定义函数 test_missing_uploaded_asset_never_publishes，封装可复用的处理步骤。
    def test_missing_uploaded_asset_never_publishes(self):
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub(omit_upload=True)
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(RuntimeError): self.run_publish(fake)。
        with self.assertRaises(RuntimeError): self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertTrue(fake.draft)。
        self.assertTrue(fake.draft)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.latest)。
        self.assertFalse(fake.latest)
        # 【逐行批注】 执行当前 Python 语句：self.assertFalse(fake.publications())。
        self.assertFalse(fake.publications())

    # 【逐行批注】 定义函数 test_bad_local_checksum_stops_before_external_calls，封装可复用的处理步骤。
    def test_bad_local_checksum_stops_before_external_calls(self):
        # 【逐行批注】 调用 next 完成当前脚本或测试步骤：next(self.root.glob('*.exe')).write_bytes(b'changed')。
        next(self.root.glob('*.exe')).write_bytes(b'changed')
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub()
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(ValueError): self.run_publish(fake)。
        with self.assertRaises(ValueError): self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(fake.calls, [])。
        self.assertEqual(fake.calls, [])

    # 【逐行批注】 定义函数 test_path_escape_manifest_stops_before_external_calls，封装可复用的处理步骤。
    def test_path_escape_manifest_stops_before_external_calls(self):
        # 【逐行批注】 计算右侧结果并赋给 (self.root/'SHA256SUMS.txt').write_text('0'*…，更新脚本状态。
        (self.root/'SHA256SUMS.txt').write_text('0'*64+' *../private.db\n', encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 fake，更新脚本状态。
        fake = FakeGitHub()
        # 【逐行批注】 在受控上下文中执行资源操作：with self.assertRaises(ValueError): self.run_publish(fake)。
        with self.assertRaises(ValueError): self.run_publish(fake)
        # 【逐行批注】 执行当前 Python 语句：self.assertEqual(fake.calls, [])。
        self.assertEqual(fake.calls, [])


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 执行当前 Python 语句：unittest.main()。
    unittest.main()
