"""可恢复的同协议对照入口；私有用例仅从外部清单读取，结果逐例落盘。"""
import argparse
import json
from pathlib import Path
import sys

from evaluation_guard import (BudgetStateError, budget_copy, file_hash, isolated_output,
                              persist_budget, require_live_authorization)
from evaluation_guard import using_decision
from teaching_eval_provider import call_api
from evaluation_core_fixture import run as run_core


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('manifest', help='外部私有JSON清单')
    parser.add_argument('--output', required=True, help='新的隔离结果目录')
    args = parser.parse_args()
    manifest = json.loads(Path(args.manifest).read_text(encoding='utf-8-sig'))
    with using_decision(manifest.get('decisionFile') or manifest.get('shared', {}).get('decisionFile')):
        return run_comparison(manifest, args.output, args.manifest)


def run_comparison(manifest, output_dir, manifest_path):
    mode = manifest.get('mode', 'preflight')
    if mode == 'live':
        require_live_authorization()
    if mode not in ('preflight', 'fixture', 'live'):
        raise ValueError('未知运行模式')
    output = isolated_output(output_dir)
    output.mkdir(parents=True, exist_ok=False)
    cases = json.loads(Path(manifest['casesFile']).read_text(encoding='utf-8-sig')) if manifest.get('casesFile') else []
    if manifest.get('caseIds'):
        selected = set(manifest['caseIds'])
        cases = [case for case in cases if case['id'] in selected]
        if {case['id'] for case in cases} != selected:
            raise ValueError('所选案例在材料中不存在。')
    # 固定偏好文件先复制一次，避免两版执行之间用户修改原文件造成不可比。
    shared = dict(manifest.get('shared', {}))
    cumulative_budget = budget_copy(output/'budget-current.json', shared.get('budgetFile'))
    shared['budgetFile'] = str(cumulative_budget)
    if shared.get('preferencesFile'):
        frozen_preferences = output/'preferences-frozen.json'
        frozen_preferences.write_bytes(Path(shared['preferencesFile']).read_bytes())
        shared['preferencesFile'] = str(frozen_preferences)
    material_hashes = {}
    for case in cases:
        if case.get('libraryDatabase'):
            source = Path(case['libraryDatabase'])
            for path in (source, Path(str(source)+'-wal')):
                material_hashes[str(path.resolve())] = file_hash(path) if path.exists() else None
    results = []
    def save():
        value = {'manifestSha256': file_hash(manifest_path), 'mode': mode, 'results': results,
                 'sourceMaterialHashes': material_hashes,
                 'paidModelRequests': 0 if mode != 'live' else None,
                 'acceptanceComplete': False,
                 'note': '零调用/模拟结果仅证明入口、协议和行为，不能证明真实Token、速度或教学质量。'}
        temporary = output/'summary.json.tmp'
        temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding='utf-8')
        temporary.replace(output/'summary.json')
    save()
    for entry in manifest['entries']:
        if entry.get('variant') not in ('baseline', 'optimized', 'candidate'):
            raise ValueError('入口variant无效。')
        for field in ('preferencesFile', 'profileName', 'frozenConfiguration', 'model'):
            if field in entry and (field not in shared or entry[field] != manifest.get('shared', {}).get(field)):
                raise ValueError('两版必须共用shared中的冻结配置：'+field)
        config = dict(entry, **shared, mode=mode, outputDir=str(output))
        for case in cases:
            try:
                if any((file_hash(path) if Path(path).exists() else None) != value for path, value in material_hashes.items()):
                    raise ValueError('资料快照在对照过程中发生变化，停止比较。')
                value = call_api('', {'config': config}, {'vars': {'case': case}})
                # 预算先于任何结果解析保存；缺回执时不能沿用上一例的旧预算继续发送。
                budget_status = value.get('budgetStatus')
                if not isinstance(budget_status, dict):
                    raise BudgetStateError('入口没有返回可确认的预算回执；停止后续请求。')
                persist_budget(cumulative_budget, budget_status)
                result = json.loads(value['output']) if 'output' in value else value
                results.append({'caseId': case['id'], 'variant': entry['variant'], 'result': result})
            except Exception as error:
                results.append({'caseId': case['id'], 'variant': entry['variant'], 'error': type(error).__name__+': '+str(error)})
                # 无法证明安全收尾的异常一律停止，禁止下一案例或下一版本发送。
                save()
                return 1
            save()
        if entry.get('productExecutable'):
            if mode != 'fixture':
                results.append({'variant': entry['variant'], 'coreFlows': {'skipped': True, 'reason': '完整产品三流程只允许本机模拟服务模式。'}})
            else:
                results.append({'variant': entry['variant'], 'coreFlows': run_core(entry['productExecutable'], output/(entry['variant']+'-core'), entry.get('sourceCommit'))})
            save()
    print(json.dumps({'summary': str(output/'summary.json'), 'paidModelRequests': 0 if mode != 'live' else None}, ensure_ascii=False))
    passed = all('error' not in row and 'error' not in row.get('result', {}) and
                 (mode == 'preflight' or row.get('result', row.get('coreFlows', {})).get('complete') is True)
                 for row in results)
    return 0 if passed else 1


if __name__ == '__main__':
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8')
    sys.exit(main())
