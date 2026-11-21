"""导出仅含产品源码的快照，开发测试及验证记录保留在开发仓库。"""

import argparse
import hashlib
import json
from pathlib import Path
import zipfile


SOURCE_DIRECTORIES = ('src', 'include', 'public', 'cmake', 'prompts', 'schemas', 'resources', 'assets', 'installer')
ROOT_FILES = ('CMakeLists.txt', 'README.md', 'PRODUCT.md', 'DESIGN.md', 'LICENSE', 'LICENSE.txt')
BUILD_SCRIPTS = ('build_windows.ps1', 'setup_library_dependencies.ps1', 'generate_brand_assets.ps1', 'export_source.py')
EXCLUDED_DIRECTORIES = {'tests', 'test', '__tests__', '__pycache__', 'verification', 'fixtures', 'node_modules', '.git', 'ui-previews'}
EXCLUDED_SUFFIXES = {'.exe', '.dll', '.pdb', '.obj', '.o', '.a', '.lib', '.pyc', '.db', '.sqlite', '.sqlite3', '.log', '.zip', '.7z'}


def permitted(path):
    """目录白名单之后再检查每个路径，防止测试入口或本地数据混入。"""
    if any(part.lower() in EXCLUDED_DIRECTORIES for part in path.parts):
        return False
    name = path.name.lower()
    if name.startswith(('.env', 'test_', 'test-', 'mock_', 'fixture_')) or name in {'settings.json', 'home-recommendations.json'}:
        return False
    if any(marker in name for marker in ('_tests.', '-tests.', '_smoke.', '-smoke.')):
        return False
    return path.suffix.lower() not in EXCLUDED_SUFFIXES


def export_source(root, output):
    root = root.resolve()
    output = output.resolve()
    if output.is_relative_to(root):
        raise ValueError('源码快照必须写到开发仓库外，避免混入后续交付。')
    if output.exists():
        raise FileExistsError('目标文件已存在，请使用新的快照名称。')
    paths = [root / name for name in ROOT_FILES if (root / name).is_file()]
    for directory in SOURCE_DIRECTORIES:
        if (root / directory).is_dir():
            paths.extend(path for path in (root / directory).rglob('*') if path.is_file())
    paths.extend(root / 'scripts' / name for name in BUILD_SCRIPTS if (root / 'scripts' / name).is_file())
    output.parent.mkdir(parents=True, exist_ok=True)
    manifest = []
    # 使用排他创建，保留旧交付；不递归删除或覆盖任何已有材料。
    with zipfile.ZipFile(output, 'x', compression=zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(paths):
            relative = path.relative_to(root)
            if not permitted(relative):
                continue
            if not path.resolve().is_relative_to(root):
                raise ValueError(f'不导出指向仓库外的链接：{relative.as_posix()}')
            data = path.read_bytes()
            archive.writestr(relative.as_posix(), data)
            manifest.append({'path': relative.as_posix(), 'sha256': hashlib.sha256(data).hexdigest(), 'bytes': len(data)})
        archive.writestr('源码说明.md', '# 产品源码快照\n\n这是当前开发状态的源码快照，不表示全部vNext功能或最终验收已完成。\n\n'
                         '本包不包含测试代码、测试程序、构建产物、验证日志、数据库或本机密钥。开发验证保留在开发仓库。\n\n'
                         '保留产品代码中的中文注释、前端资源、提示词、Schema和第三方前端库许可。C++构建依赖按scripts中的依赖准备脚本安装，未捆绑本机第三方二进制目录。\n\n'
                         '使用CMake构建时指定 -DBUILD_TESTING=OFF；也可使用原Windows构建脚本。正式发布、真实模型教学质量和WebView2验收以最终交付说明为准。\n')
        archive.writestr('source-manifest.json', json.dumps({'kind': 'development_source_snapshot', 'files': manifest}, ensure_ascii=False, indent=2))
    # 对成品再次逐项核对，不能仅凭排除规则宣称“干净源码”。
    with zipfile.ZipFile(output) as archive:
        for name in archive.namelist():
            if not permitted(Path(name)):
                raise ValueError(f'源码包含禁止项：{name}')
        if archive.testzip() is not None:
            raise ValueError('源码包完整性检查失败。')
    return {'path': str(output), 'sourceFiles': len(manifest), 'sha256': hashlib.sha256(output.read_bytes()).hexdigest(), 'forbiddenEntries': 0}


def main():
    parser = argparse.ArgumentParser(description='导出不含测试和程序产物的产品源码快照')
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(export_source(args.root, args.output), ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
