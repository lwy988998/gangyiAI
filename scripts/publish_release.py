"""补齐并校验发布附件后再公开发布，已有发布也执行同一修复路径。"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def run_gh(arguments):
    return subprocess.run(['gh', *arguments], capture_output=True, check=False)


def checked(runner, arguments):
    result = runner(arguments)
    if result.returncode:
        raise RuntimeError('GitHub发布操作失败：'+arguments[0])
    return result.stdout


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
    endpoint = f'repos/{repository}/releases/tags/{tag}'
    response = runner(['api', endpoint])
    if response.returncode:
        # 鉴权、限流及网络失败不能误当成不存在，只有明确404才创建草稿。
        if b'HTTP 404' not in response.stderr:
            raise RuntimeError('无法读取已有发布，未创建或更新')
        checked(runner, ['release', 'create', tag, '--repo', repository, '--verify-tag', '--draft',
                         '--title', '钢一定制AI '+tag, '--notes-file', str(notes)])
        release = {'assets': [], 'draft': True}
    else:
        release = json.loads(response.stdout)

    def matching(asset, path, digest):
        if asset.get('size') != path.stat().st_size:
            return False
        remote_digest = asset.get('digest')
        if remote_digest:
            return remote_digest == 'sha256:'+digest
        identifier = asset.get('id')
        if not isinstance(identifier, int):
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
    # clobber 会先删旧附件；修补公开版本前转成草稿，失败时不能留下不完整的公开 latest。
    if pending and not release.get('draft', False):
        checked(runner, ['release', 'edit', tag, '--repo', repository, '--draft=true'])
    # 同内容附件不重复上传；包在前、校验清单在后，缺包或上传中断时可重跑补齐。
    for path in pending:
        checked(runner, ['release', 'upload', tag, str(path), '--repo', repository, '--clobber'])
    uploaded = json.loads(checked(runner, ['api', endpoint]))
    for name, (path, digest) in assets.items():
        found = [item for item in uploaded.get('assets', []) if item.get('name') == name]
        if len(found) != 1 or not matching(found[0], path, digest):
            raise RuntimeError('发布附件尚未完整校验：'+name)
    checked(runner, ['release', 'edit', tag, '--repo', repository, '--verify-tag', '--draft=false', '--prerelease=false',
                     '--title', '钢一定制AI '+tag, '--notes-file', str(notes), '--latest'])
    return {'tag': tag, 'verifiedAssets': list(assets)}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--artifacts', required=True)
    parser.add_argument('--notes', required=True)
    args = parser.parse_args()
    print(json.dumps(publish(args.repository, args.tag, args.artifacts, args.notes), ensure_ascii=False))
