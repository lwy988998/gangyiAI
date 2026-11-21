"""冻结评测入口的源码与链接材料，拒绝把陈旧程序当成当前实现。"""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def source_snapshot():
    paths = {ROOT / name for name in ('CMakeLists.txt', 'tests/teaching_eval_driver.cpp',
             'scripts/build_teaching_eval.ps1', 'scripts/tutor_build_identity.py',
             'scripts/tutor_experiment.py', 'scripts/evaluation_guard.py',
             'scripts/teaching_eval_provider.py', 'scripts/tutor_benchmark.py', 'scripts/ai_money_budget.py')}
    paths.add(ROOT / 'scripts/tutor_component_scenes.py')
    for name in ('include', 'src', 'prompts', 'cmake'):
        paths.update(path for path in (ROOT / name).rglob('*') if path.is_file())
    return {path.relative_to(ROOT).as_posix(): sha256(path) for path in sorted(paths)}


def checked_build(driver):
    """同时核对源码集合、生成提示、链接对象、运行库与最终入口。"""
    driver = Path(driver).resolve()
    receipt = json.loads(Path(str(driver) + '.build.json').read_text(encoding='utf-8-sig'))
    if (receipt.get('identityVersion') != 2 or receipt.get('frozenBaseline') is not False or
            receipt.get('inputs') != source_snapshot() or receipt.get('executableSha256') != sha256(driver)):
        raise ValueError('评测入口构建凭据不匹配，请重新构建。')
    if not receipt.get('artifacts'):
        raise ValueError('缺少链接材料。')
    for item in receipt['artifacts']:
        if sha256(item['path']) != item['sha256']:
            raise ValueError('评测入口链接材料已变化。')
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['capture', 'stamp'])
    parser.add_argument('--snapshot', type=Path, required=True)
    parser.add_argument('--driver', type=Path)
    parser.add_argument('--build-dir', type=Path)
    args = parser.parse_args()
    if args.command == 'capture':
        args.snapshot.write_text(json.dumps(source_snapshot(), ensure_ascii=False), encoding='utf-8')
        return
    before = json.loads(args.snapshot.read_text(encoding='utf-8'))
    if before != source_snapshot():
        raise ValueError('构建期间源码变化，禁止生成有效凭据。')
    path = Path(str(args.driver) + '.build.json')
    receipt = json.loads(path.read_text(encoding='utf-8-sig'))
    build = args.build_dir.resolve()
    artifacts = [build / 'libsqlite3_amalgamation.a', build / 'libcurl-x64.dll']
    artifacts += list((build / 'generated').rglob('*.hpp'))
    artifacts += list((build / 'CMakeFiles/learning-agent-tests.dir/src').glob('*.obj'))
    receipt.update(identityVersion=2, inputs=before, executableSha256=sha256(args.driver),
                   artifacts=[{'path': str(p.resolve()), 'sha256': sha256(p)} for p in artifacts])
    path.write_text(json.dumps(receipt, ensure_ascii=False, indent=2), encoding='utf-8')
    checked_build(args.driver)


if __name__ == '__main__':
    main()
