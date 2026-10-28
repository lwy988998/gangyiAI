"""补齐并校验发布附件后再公开发布，已有发布也执行同一修复路径。"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile
from urllib.parse import urlencode


def run_gh(arguments):
    return subprocess.run(['gh', *arguments], capture_output=True, check=False)


def checked(runner, arguments):
    result = runner(arguments)
    if result.returncode:
        raise RuntimeError('GitHub发布操作失败：'+arguments[0])
    return result.stdout


def api_json(runner, endpoint, method='GET', body=None):
    arguments = ['api', endpoint, '--method', method]
    if body is None:
        return json.loads(checked(runner, arguments))
    # JSON正文保留真实换行，避免gh字段参数把布尔值或发布说明转成字符串。
    with tempfile.TemporaryDirectory(prefix='gangyi-release-request-') as folder:
        path = Path(folder)/'body.json'
        path.write_text(json.dumps(body, ensure_ascii=False), encoding='utf-8')
        return json.loads(checked(runner, arguments+['--input', str(path)]))


def release_identity(value, tag, identifier=None):
    if (not isinstance(value, dict) or type(value.get('id')) is not int or value['id'] <= 0 or
            value.get('tag_name') != tag or type(value.get('draft')) is not bool or
            not isinstance(value.get('assets'), list) or
            (identifier is not None and value['id'] != identifier)):
        raise RuntimeError('发布身份或附件清单无效，未继续修改')
    return value


def find_release(runner, repository, tag):
    response = runner(['api', f'repos/{repository}/releases/tags/{tag}'])
    known_id = None
    if not response.returncode:
        known_id = release_identity(json.loads(response.stdout), tag)['id']
    # by-tag只查询已公开发布。404还必须分页检查草稿；认证、限流或其它失败不能创建。
    else:
        statuses = set(re.findall(rb'\bHTTP (\d{3})\b', response.stderr))
        if statuses != {b'404'}:
            raise RuntimeError('无法读取已有发布，未创建或更新')
    # 即使by-tag成功，也要排除同标签的其它草稿；两个读取入口必须指向唯一同一ID。
    found = {}
    seen = set()
    for page in range(1, 1001):
        releases = api_json(runner, f'repos/{repository}/releases?per_page=100&page={page}')
        if not isinstance(releases, list) or len(releases) > 100:
            raise RuntimeError('发布分页结果无效，未创建或更新')
        for item in releases:
            if (not isinstance(item, dict) or type(item.get('id')) is not int or item['id'] <= 0 or
                    not isinstance(item.get('tag_name'), str) or item['id'] in seen):
                raise RuntimeError('发布分页身份变化或格式无效，未创建或更新')
            seen.add(item['id'])
            if item['tag_name'] == tag:
                found[item['id']] = release_identity(item, tag)
        if len(releases) < 100:
            break
    else:
        raise RuntimeError('发布分页未完整读完，未创建或更新')
    if len(found) > 1:
        raise RuntimeError('同一标签存在多个发布记录，请先核对Release ID：'+','.join(map(str, sorted(found))))
    if known_id is not None and (len(found) != 1 or known_id not in found):
        raise RuntimeError('按标签与分页查询的发布身份不一致，未创建或更新')
    return next(iter(found.values()), None)


def local_assets(directory, tag):
    directory = Path(directory).resolve()
    checksum = directory/'SHA256SUMS.txt'
    expected = {}
    for line in checksum.read_text(encoding='utf-8-sig').splitlines():
        match = re.fullmatch(r'([a-fA-F0-9]{64})\s+\*?(.+)', line)
        if not match:
            raise ValueError('发布校验清单格式无效')
        digest, name = match.groups()
        if Path(name).name != name or '/' in name or '\\' in name or '#' in name or name.startswith('-'):
            raise ValueError('发布附件名称无效')
        if not re.fullmatch(r'gangyiAI-(?:setup|portable|learning-preview1|teaching-preview1)-'+re.escape(tag)+r'-x64\.(?:exe|zip)', name):
            raise ValueError('附件名称与版本不一致')
        path = directory/name
        if name in expected or path.is_symlink() or not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != digest.lower():
            raise ValueError('发布附件缺失、重复或SHA256不符：'+name)
        expected[name] = (path, digest.lower())
    if not any(name.endswith('.exe') for name in expected) or not any(name.endswith('.zip') for name in expected):
        raise ValueError('发布需要完整安装包和免安装包')
    expected[checksum.name] = (checksum, hashlib.sha256(checksum.read_bytes()).hexdigest())
    return expected


def publish(repository, tag, directory, notes, runner=run_gh):
    if not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', repository) or not re.fullmatch(r'v\d+\.\d+\.\d+', tag):
        raise ValueError('仓库或发布标签格式无效')
    assets = local_assets(directory, tag)
    notes = Path(notes).resolve(strict=True)
    release = find_release(runner, repository, tag)
    title, body = '钢一定制AI '+tag, notes.read_text(encoding='utf-8-sig')
    if release is None:
        # 保留原--verify-tag边界，REST创建发布前确认标签已存在，不能顺便创建新标签。
        reference = api_json(runner, f'repos/{repository}/git/ref/tags/{tag}')
        if (reference.get('ref') != 'refs/tags/'+tag or
                not re.fullmatch(r'[0-9a-fA-F]{40}', reference.get('object', {}).get('sha', ''))):
            raise RuntimeError('发布标签尚未核实，未创建草稿')
        release = release_identity(api_json(runner, f'repos/{repository}/releases', 'POST',
            {'tag_name': tag, 'name': title, 'body': body, 'draft': True, 'prerelease': False, 'make_latest': 'false'}), tag)
        if release['draft'] is not True:
            raise RuntimeError('创建结果不是草稿，未继续上传')
    identifier = release['id']
    endpoint = f'repos/{repository}/releases/{identifier}'
    # 创建及所有后续操作绑定同一个ID，不再用tag解析草稿或上传目标。
    release = release_identity(api_json(runner, endpoint), tag, identifier)

    def matching(asset, path, digest):
        if asset.get('size') != path.stat().st_size:
            return False
        remote_digest = asset.get('digest')
        if remote_digest:
            return remote_digest == 'sha256:'+digest
        identifier = asset.get('id')
        if type(identifier) is not int or identifier <= 0:
            return False
        content = checked(runner, ['api', f'repos/{repository}/releases/assets/{identifier}',
                                   '-H', 'Accept: application/octet-stream'])
        return hashlib.sha256(content).hexdigest() == digest

    pending = []
    for name, (path, digest) in assets.items():
        found = [item for item in release.get('assets', []) if item.get('name') == name]
        if len(found) == 1 and matching(found[0], path, digest):
            continue
        pending.append(path)
    # 替换附件前先将这个ID转成草稿，上传失败也不能留下不完整的公开latest。
    if pending and not release.get('draft', False):
        release = release_identity(api_json(runner, endpoint, 'PATCH', {'draft': True, 'make_latest': 'false'}), tag, identifier)
        if release['draft'] is not True:
            raise RuntimeError('尚未转成草稿，未替换附件')
    upload_url = release.get('upload_url', '').split('{', 1)[0]
    if pending and upload_url != f'https://uploads.github.com/repos/{repository}/releases/{identifier}/assets':
        raise RuntimeError('发布上传地址与Release ID不匹配，未上传')
    # 同内容附件不重复上传；包在前、校验清单在后，缺包或上传中断时可重跑补齐。
    for path in pending:
        for existing in release['assets']:
            if existing.get('name') == path.name:
                asset_id = existing.get('id')
                if type(asset_id) is not int or asset_id <= 0:
                    raise RuntimeError('待替换附件缺少稳定ID，未删除')
                checked(runner, ['api', f'repos/{repository}/releases/assets/{asset_id}', '--method', 'DELETE'])
        uploaded_asset = json.loads(checked(runner, ['api', upload_url+'?'+urlencode({'name': path.name}),
            '--method', 'POST', '-H', 'Content-Type: application/octet-stream', '--input', str(path)]))
        if uploaded_asset.get('name') != path.name or uploaded_asset.get('size') != path.stat().st_size:
            raise RuntimeError('附件上传回执不匹配，未公开发布')
    uploaded = release_identity(api_json(runner, endpoint), tag, identifier)
    for name, (path, digest) in assets.items():
        found = [item for item in uploaded.get('assets', []) if item.get('name') == name]
        if len(found) != 1 or not matching(found[0], path, digest):
            raise RuntimeError('发布附件尚未完整校验：'+name)
    published = release_identity(api_json(runner, endpoint, 'PATCH',
        {'draft': False, 'prerelease': False, 'name': title, 'body': body, 'make_latest': 'true'}), tag, identifier)
    if published['draft'] is not False or published.get('prerelease') is not False:
        raise RuntimeError('发布状态未确认，请按Release ID核对')
    return {'tag': tag, 'releaseId': identifier, 'verifiedAssets': list(assets)}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--artifacts', required=True)
    parser.add_argument('--notes', required=True)
    args = parser.parse_args()
    print(json.dumps(publish(args.repository, args.tag, args.artifacts, args.notes), ensure_ascii=False))
