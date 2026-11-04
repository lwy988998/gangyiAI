# 【逐行批注】 执行当前 Python 语句："""补齐并校验发布附件后再公开发布，已有发布也执行同一修复路径。"""。
"""补齐并校验发布附件后再公开发布，已有发布也执行同一修复路径。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import argparse。
import argparse
# 【逐行批注】 导入后续逻辑需要的模块或符号：import hashlib。
import hashlib
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import re。
import re
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import urlencode。
from urllib.parse import urlencode


# 【逐行批注】 定义函数 run_gh，封装可复用的处理步骤。
def run_gh(arguments):
    # 【逐行批注】 结束当前函数并返回结果：return subprocess.run(['gh', *arguments], capture_output=True, check=…。
    return subprocess.run(['gh', *arguments], capture_output=True, check=False)


# 【逐行批注】 定义函数 checked，封装可复用的处理步骤。
def checked(runner, arguments):
    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
    result = runner(arguments)
    # 【逐行批注】 判断条件 result.returncode:，成立时执行对应分支。
    if result.returncode:
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('GitHub发布操作失败：'+arguments[0])。
        raise RuntimeError('GitHub发布操作失败：'+arguments[0])
    # 【逐行批注】 结束当前函数并返回结果：return result.stdout。
    return result.stdout


# 【逐行批注】 定义函数 api_json，封装可复用的处理步骤。
def api_json(runner, endpoint, method='GET', body=None):
    # 【逐行批注】 计算右侧结果并赋给 arguments，更新脚本状态。
    arguments = ['api', endpoint, '--method', method]
    # 【逐行批注】 判断条件 body is None:，成立时执行对应分支。
    if body is None:
        # 【逐行批注】 结束当前函数并返回结果：return json.loads(checked(runner, arguments))。
        return json.loads(checked(runner, arguments))
    # JSON正文保留真实换行，避免gh字段参数把布尔值或发布说明转成字符串。
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-release-request-') as folder:。
    with tempfile.TemporaryDirectory(prefix='gangyi-release-request-') as folder:
        # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
        path = Path(folder)/'body.json'
        # 【逐行批注】 计算右侧结果并赋给 path.write_text(json.dumps(body, ensure_ascii，更新脚本状态。
        path.write_text(json.dumps(body, ensure_ascii=False), encoding='utf-8')
        # 【逐行批注】 结束当前函数并返回结果：return json.loads(checked(runner, arguments+['--input', str(path)]))。
        return json.loads(checked(runner, arguments+['--input', str(path)]))


# 【逐行批注】 定义函数 release_identity，封装可复用的处理步骤。
def release_identity(value, tag, identifier=None):
    # 【逐行批注】 判断条件 (not isinstance(value, dict) or type(value.get('id')) is not int…，成立时执行对应分支。
    if (not isinstance(value, dict) or type(value.get('id')) is not int or value['id'] <= 0 or
            # 【逐行批注】 调用 type 完成当前脚本或测试步骤：value.get('tag_name') != tag or type(value.get('draft')) is not bool …。
            value.get('tag_name') != tag or type(value.get('draft')) is not bool or
            # 【逐行批注】 调用 isinstance 完成当前脚本或测试步骤：not isinstance(value.get('assets'), list) or。
            not isinstance(value.get('assets'), list) or
            # 【逐行批注】 执行当前 Python 语句：(identifier is not None and value['id'] != identifier)):。
            (identifier is not None and value['id'] != identifier)):
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布身份或附件清单无效，未继续修改')。
        raise RuntimeError('发布身份或附件清单无效，未继续修改')
    # 【逐行批注】 结束当前函数并返回结果：return value。
    return value


# 【逐行批注】 定义函数 find_release，封装可复用的处理步骤。
def find_release(runner, repository, tag):
    # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
    response = runner(['api', f'repos/{repository}/releases/tags/{tag}'])
    # 【逐行批注】 计算右侧结果并赋给 known_id，更新脚本状态。
    known_id = None
    # 【逐行批注】 判断条件 not response.returncode:，成立时执行对应分支。
    if not response.returncode:
        # 【逐行批注】 计算右侧结果并赋给 known_id，更新脚本状态。
        known_id = release_identity(json.loads(response.stdout), tag)['id']
    # by-tag只查询已公开发布。404还必须分页检查草稿；认证、限流或其它失败不能创建。
    # 【逐行批注】 执行前一条件不成立时的分支。
    else:
        # 【逐行批注】 计算右侧结果并赋给 statuses，更新脚本状态。
        statuses = set(re.findall(rb'\bHTTP (\d{3})\b', response.stderr))
        # 【逐行批注】 判断条件 statuses != {b'404'}:，成立时执行对应分支。
        if statuses != {b'404'}:
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('无法读取已有发布，未创建或更新')。
            raise RuntimeError('无法读取已有发布，未创建或更新')
    # 即使by-tag成功，也要排除同标签的其它草稿；两个读取入口必须指向唯一同一ID。
    # 【逐行批注】 计算右侧结果并赋给 found，更新脚本状态。
    found = {}
    # 【逐行批注】 计算右侧结果并赋给 seen，更新脚本状态。
    seen = set()
    # 【逐行批注】 遍历数据并重复执行循环体：for page in range(1, 1001):。
    for page in range(1, 1001):
        # 【逐行批注】 计算右侧结果并赋给 releases，更新脚本状态。
        releases = api_json(runner, f'repos/{repository}/releases?per_page=100&page={page}')
        # 【逐行批注】 判断条件 not isinstance(releases, list) or len(releases) > 100:，成立时执行对应分支。
        if not isinstance(releases, list) or len(releases) > 100:
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布分页结果无效，未创建或更新')。
            raise RuntimeError('发布分页结果无效，未创建或更新')
        # 【逐行批注】 遍历数据并重复执行循环体：for item in releases:。
        for item in releases:
            # 【逐行批注】 判断条件 (not isinstance(item, dict) or type(item.get('id')) is not int o…，成立时执行对应分支。
            if (not isinstance(item, dict) or type(item.get('id')) is not int or item['id'] <= 0 or
                    # 【逐行批注】 调用 isinstance 完成当前脚本或测试步骤：not isinstance(item.get('tag_name'), str) or item['id'] in seen):。
                    not isinstance(item.get('tag_name'), str) or item['id'] in seen):
                # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布分页身份变化或格式无效，未创建或更新')。
                raise RuntimeError('发布分页身份变化或格式无效，未创建或更新')
            # 【逐行批注】 执行当前 Python 语句：seen.add(item['id'])。
            seen.add(item['id'])
            # 【逐行批注】 判断条件 item['tag_name'] == tag:，成立时执行对应分支。
            if item['tag_name'] == tag:
                # 【逐行批注】 计算右侧结果并赋给 found[item['id']]，更新脚本状态。
                found[item['id']] = release_identity(item, tag)
        # 【逐行批注】 判断条件 len(releases) < 100:，成立时执行对应分支。
        if len(releases) < 100:
            # 【逐行批注】 执行当前 Python 语句：break。
            break
    # 【逐行批注】 执行前一条件不成立时的分支。
    else:
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布分页未完整读完，未创建或更新')。
        raise RuntimeError('发布分页未完整读完，未创建或更新')
    # 【逐行批注】 判断条件 len(found) > 1:，成立时执行对应分支。
    if len(found) > 1:
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('同一标签存在多个发布记录，请先核对Release ID：'+','.join(map(str, s…。
        raise RuntimeError('同一标签存在多个发布记录，请先核对Release ID：'+','.join(map(str, sorted(found))))
    # 【逐行批注】 判断条件 known_id is not None and (len(found) != 1 or known_id not in fou…，成立时执行对应分支。
    if known_id is not None and (len(found) != 1 or known_id not in found):
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('按标签与分页查询的发布身份不一致，未创建或更新')。
        raise RuntimeError('按标签与分页查询的发布身份不一致，未创建或更新')
    # 【逐行批注】 结束当前函数并返回结果：return next(iter(found.values()), None)。
    return next(iter(found.values()), None)


# 【逐行批注】 定义函数 local_assets，封装可复用的处理步骤。
def local_assets(directory, tag):
    # 【逐行批注】 计算右侧结果并赋给 directory，更新脚本状态。
    directory = Path(directory).resolve()
    # 【逐行批注】 计算右侧结果并赋给 checksum，更新脚本状态。
    checksum = directory/'SHA256SUMS.txt'
    # 【逐行批注】 计算右侧结果并赋给 expected，更新脚本状态。
    expected = {}
    # 【逐行批注】 遍历数据并重复执行循环体：for line in checksum.read_text(encoding='utf-8-sig').splitlines():。
    for line in checksum.read_text(encoding='utf-8-sig').splitlines():
        # 【逐行批注】 计算右侧结果并赋给 match，更新脚本状态。
        match = re.fullmatch(r'([a-fA-F0-9]{64})\s+\*?(.+)', line)
        # 【逐行批注】 判断条件 not match:，成立时执行对应分支。
        if not match:
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('发布校验清单格式无效')。
            raise ValueError('发布校验清单格式无效')
        # 【逐行批注】 计算右侧结果并赋给 digest, name，更新脚本状态。
        digest, name = match.groups()
        # 【逐行批注】 判断条件 Path(name).name != name or '/' in name or '\\' in name or '#' in…，成立时执行对应分支。
        if Path(name).name != name or '/' in name or '\\' in name or '#' in name or name.startswith('-'):
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('发布附件名称无效')。
            raise ValueError('发布附件名称无效')
        # 【逐行批注】 判断条件 not re.fullmatch(r'gangyiAI-(?:setup|portable|learning-preview1|…，成立时执行对应分支。
        if not re.fullmatch(r'gangyiAI-(?:setup|portable|learning-preview1|teaching-preview1)-'+re.escape(tag)+r'-x64\.(?:exe|zip)', name):
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('附件名称与版本不一致')。
            raise ValueError('附件名称与版本不一致')
        # 【逐行批注】 计算右侧结果并赋给 path，更新脚本状态。
        path = directory/name
        # 【逐行批注】 判断条件 name in expected or path.is_symlink() or not path.is_file() or h…，成立时执行对应分支。
        if name in expected or path.is_symlink() or not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != digest.lower():
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('发布附件缺失、重复或SHA256不符：'+name)。
            raise ValueError('发布附件缺失、重复或SHA256不符：'+name)
        # 【逐行批注】 计算右侧结果并赋给 expected[name]，更新脚本状态。
        expected[name] = (path, digest.lower())
    # 【逐行批注】 判断条件 not any(name.endswith('.exe') for name in expected) or not any(n…，成立时执行对应分支。
    if not any(name.endswith('.exe') for name in expected) or not any(name.endswith('.zip') for name in expected):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('发布需要完整安装包和免安装包')。
        raise ValueError('发布需要完整安装包和免安装包')
    # 【逐行批注】 计算右侧结果并赋给 expected[checksum.name]，更新脚本状态。
    expected[checksum.name] = (checksum, hashlib.sha256(checksum.read_bytes()).hexdigest())
    # 【逐行批注】 结束当前函数并返回结果：return expected。
    return expected


# 【逐行批注】 定义函数 publish，封装可复用的处理步骤。
def publish(repository, tag, directory, notes, runner=run_gh):
    # 【逐行批注】 判断条件 not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', repository)…，成立时执行对应分支。
    if not re.fullmatch(r'[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+', repository) or not re.fullmatch(r'v\d+\.\d+\.\d+', tag):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('仓库或发布标签格式无效')。
        raise ValueError('仓库或发布标签格式无效')
    # 【逐行批注】 计算右侧结果并赋给 assets，更新脚本状态。
    assets = local_assets(directory, tag)
    # 【逐行批注】 计算右侧结果并赋给 notes，更新脚本状态。
    notes = Path(notes).resolve(strict=True)
    # 【逐行批注】 计算右侧结果并赋给 release，更新脚本状态。
    release = find_release(runner, repository, tag)
    # 【逐行批注】 计算右侧结果并赋给 title, body，更新脚本状态。
    title, body = '钢一定制AI '+tag, notes.read_text(encoding='utf-8-sig')
    # 【逐行批注】 判断条件 release is None:，成立时执行对应分支。
    if release is None:
        # 保留原--verify-tag边界，REST创建发布前确认标签已存在，不能顺便创建新标签。
        # 【逐行批注】 计算右侧结果并赋给 reference，更新脚本状态。
        reference = api_json(runner, f'repos/{repository}/git/ref/tags/{tag}')
        # 【逐行批注】 判断条件 (reference.get('ref') != 'refs/tags/'+tag or，成立时执行对应分支。
        if (reference.get('ref') != 'refs/tags/'+tag or
                # 【逐行批注】 执行当前 Python 语句：not re.fullmatch(r'[0-9a-fA-F]{40}', reference.get('object', {}).get('sha', ''))):。
                not re.fullmatch(r'[0-9a-fA-F]{40}', reference.get('object', {}).get('sha', ''))):
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布标签尚未核实，未创建草稿')。
            raise RuntimeError('发布标签尚未核实，未创建草稿')
        # 【逐行批注】 计算右侧结果并赋给 release，更新脚本状态。
        release = release_identity(api_json(runner, f'repos/{repository}/releases', 'POST',
            # 【逐行批注】 执行当前 Python 语句：{'tag_name': tag, 'name': title, 'body': body, 'draft': True, 'prerelease': False, 'make_…。
            {'tag_name': tag, 'name': title, 'body': body, 'draft': True, 'prerelease': False, 'make_latest': 'false'}), tag)
        # 【逐行批注】 判断条件 release['draft'] is not True:，成立时执行对应分支。
        if release['draft'] is not True:
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('创建结果不是草稿，未继续上传')。
            raise RuntimeError('创建结果不是草稿，未继续上传')
    # 【逐行批注】 计算右侧结果并赋给 identifier，更新脚本状态。
    identifier = release['id']
    # 【逐行批注】 计算右侧结果并赋给 endpoint，更新脚本状态。
    endpoint = f'repos/{repository}/releases/{identifier}'
    # 创建及所有后续操作绑定同一个ID，不再用tag解析草稿或上传目标。
    # 【逐行批注】 计算右侧结果并赋给 release，更新脚本状态。
    release = release_identity(api_json(runner, endpoint), tag, identifier)

    # 【逐行批注】 定义函数 matching，封装可复用的处理步骤。
    def matching(asset, path, digest):
        # 【逐行批注】 判断条件 asset.get('size') != path.stat().st_size:，成立时执行对应分支。
        if asset.get('size') != path.stat().st_size:
            # 【逐行批注】 结束当前函数并返回结果：return False。
            return False
        # 【逐行批注】 计算右侧结果并赋给 remote_digest，更新脚本状态。
        remote_digest = asset.get('digest')
        # 【逐行批注】 判断条件 remote_digest:，成立时执行对应分支。
        if remote_digest:
            # 【逐行批注】 结束当前函数并返回结果：return remote_digest == 'sha256:'+digest。
            return remote_digest == 'sha256:'+digest
        # 【逐行批注】 计算右侧结果并赋给 identifier，更新脚本状态。
        identifier = asset.get('id')
        # 【逐行批注】 判断条件 type(identifier) is not int or identifier <= 0:，成立时执行对应分支。
        if type(identifier) is not int or identifier <= 0:
            # 【逐行批注】 结束当前函数并返回结果：return False。
            return False
        # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
        content = checked(runner, ['api', f'repos/{repository}/releases/assets/{identifier}',
                                   # 【逐行批注】 执行当前 Python 语句：'-H', 'Accept: application/octet-stream'])。
                                   '-H', 'Accept: application/octet-stream'])
        # 【逐行批注】 结束当前函数并返回结果：return hashlib.sha256(content).hexdigest() == digest。
        return hashlib.sha256(content).hexdigest() == digest

    # 【逐行批注】 计算右侧结果并赋给 pending，更新脚本状态。
    pending = []
    # 【逐行批注】 遍历数据并重复执行循环体：for name, (path, digest) in assets.items():。
    for name, (path, digest) in assets.items():
        # 【逐行批注】 计算右侧结果并赋给 found，更新脚本状态。
        found = [item for item in release.get('assets', []) if item.get('name') == name]
        # 【逐行批注】 判断条件 len(found) == 1 and matching(found[0], path, digest):，成立时执行对应分支。
        if len(found) == 1 and matching(found[0], path, digest):
            # 【逐行批注】 执行当前 Python 语句：continue。
            continue
        # 【逐行批注】 执行当前 Python 语句：pending.append(path)。
        pending.append(path)
    # 替换附件前先将这个ID转成草稿，上传失败也不能留下不完整的公开latest。
    # 【逐行批注】 判断条件 pending and not release.get('draft', False):，成立时执行对应分支。
    if pending and not release.get('draft', False):
        # 【逐行批注】 计算右侧结果并赋给 release，更新脚本状态。
        release = release_identity(api_json(runner, endpoint, 'PATCH', {'draft': True, 'make_latest': 'false'}), tag, identifier)
        # 【逐行批注】 判断条件 release['draft'] is not True:，成立时执行对应分支。
        if release['draft'] is not True:
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('尚未转成草稿，未替换附件')。
            raise RuntimeError('尚未转成草稿，未替换附件')
    # 【逐行批注】 计算右侧结果并赋给 upload_url，更新脚本状态。
    upload_url = release.get('upload_url', '').split('{', 1)[0]
    # 【逐行批注】 判断条件 pending and upload_url != f'https://uploads.github.com/repos/{re…，成立时执行对应分支。
    if pending and upload_url != f'https://uploads.github.com/repos/{repository}/releases/{identifier}/assets':
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布上传地址与Release ID不匹配，未上传')。
        raise RuntimeError('发布上传地址与Release ID不匹配，未上传')
    # 同内容附件不重复上传；包在前、校验清单在后，缺包或上传中断时可重跑补齐。
    # 【逐行批注】 遍历数据并重复执行循环体：for path in pending:。
    for path in pending:
        # 【逐行批注】 遍历数据并重复执行循环体：for existing in release['assets']:。
        for existing in release['assets']:
            # 【逐行批注】 判断条件 existing.get('name') == path.name:，成立时执行对应分支。
            if existing.get('name') == path.name:
                # 【逐行批注】 计算右侧结果并赋给 asset_id，更新脚本状态。
                asset_id = existing.get('id')
                # 【逐行批注】 判断条件 type(asset_id) is not int or asset_id <= 0:，成立时执行对应分支。
                if type(asset_id) is not int or asset_id <= 0:
                    # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('待替换附件缺少稳定ID，未删除')。
                    raise RuntimeError('待替换附件缺少稳定ID，未删除')
                # 【逐行批注】 调用 checked 完成当前脚本或测试步骤：checked(runner, ['api', f'repos/{repository}/releases/assets/{asset_i…。
                checked(runner, ['api', f'repos/{repository}/releases/assets/{asset_id}', '--method', 'DELETE'])
        # 【逐行批注】 计算右侧结果并赋给 uploaded_asset，更新脚本状态。
        uploaded_asset = json.loads(checked(runner, ['api', upload_url+'?'+urlencode({'name': path.name}),
            # 【逐行批注】 调用 str 完成当前脚本或测试步骤：'--method', 'POST', '-H', 'Content-Type: application/octet-stream', '…。
            '--method', 'POST', '-H', 'Content-Type: application/octet-stream', '--input', str(path)]))
        # 【逐行批注】 判断条件 uploaded_asset.get('name') != path.name or uploaded_asset.get('s…，成立时执行对应分支。
        if uploaded_asset.get('name') != path.name or uploaded_asset.get('size') != path.stat().st_size:
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('附件上传回执不匹配，未公开发布')。
            raise RuntimeError('附件上传回执不匹配，未公开发布')
    # 【逐行批注】 计算右侧结果并赋给 uploaded，更新脚本状态。
    uploaded = release_identity(api_json(runner, endpoint), tag, identifier)
    # 【逐行批注】 遍历数据并重复执行循环体：for name, (path, digest) in assets.items():。
    for name, (path, digest) in assets.items():
        # 【逐行批注】 计算右侧结果并赋给 found，更新脚本状态。
        found = [item for item in uploaded.get('assets', []) if item.get('name') == name]
        # 【逐行批注】 判断条件 len(found) != 1 or not matching(found[0], path, digest):，成立时执行对应分支。
        if len(found) != 1 or not matching(found[0], path, digest):
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布附件尚未完整校验：'+name)。
            raise RuntimeError('发布附件尚未完整校验：'+name)
    # 【逐行批注】 计算右侧结果并赋给 published，更新脚本状态。
    published = release_identity(api_json(runner, endpoint, 'PATCH',
        # 【逐行批注】 执行当前 Python 语句：{'draft': False, 'prerelease': False, 'name': title, 'body': body, 'make_latest': 'true'}…。
        {'draft': False, 'prerelease': False, 'name': title, 'body': body, 'make_latest': 'true'}), tag, identifier)
    # 【逐行批注】 判断条件 published['draft'] is not False or published.get('prerelease') i…，成立时执行对应分支。
    if published['draft'] is not False or published.get('prerelease') is not False:
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('发布状态未确认，请按Release ID核对')。
        raise RuntimeError('发布状态未确认，请按Release ID核对')
    # 【逐行批注】 结束当前函数并返回结果：return {'tag': tag, 'releaseId': identifier, 'verifiedAssets': list(a…。
    return {'tag': tag, 'releaseId': identifier, 'verifiedAssets': list(assets)}


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 计算右侧结果并赋给 parser，更新脚本状态。
    parser = argparse.ArgumentParser(description=__doc__)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--repository', required，更新脚本状态。
    parser.add_argument('--repository', required=True)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--tag', required，更新脚本状态。
    parser.add_argument('--tag', required=True)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--artifacts', required，更新脚本状态。
    parser.add_argument('--artifacts', required=True)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--notes', required，更新脚本状态。
    parser.add_argument('--notes', required=True)
    # 【逐行批注】 计算右侧结果并赋给 args，更新脚本状态。
    args = parser.parse_args()
    # 【逐行批注】 输出当前运行结果：print(json.dumps(publish(args.repository, args.tag, args.artifacts, args.n…。
    print(json.dumps(publish(args.repository, args.tag, args.artifacts, args.notes), ensure_ascii=False))
