# 【逐行批注】 执行当前 Python 语句："""可恢复的同协议对照入口；私有用例仅从外部清单读取，结果逐例落盘。"""。
"""可恢复的同协议对照入口；私有用例仅从外部清单读取，结果逐例落盘。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import argparse。
import argparse
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys

# 【逐行批注】 导入后续逻辑需要的模块或符号：from evaluation_guard import (BudgetStateError, budget_copy, file_hash, isolate…。
from evaluation_guard import (BudgetStateError, budget_copy, file_hash, isolated_output,
                              # 【逐行批注】 执行当前 Python 语句：persist_budget, require_live_authorization)。
                              persist_budget, require_live_authorization)
# 【逐行批注】 导入后续逻辑需要的模块或符号：from evaluation_guard import using_decision。
from evaluation_guard import using_decision
# 【逐行批注】 导入后续逻辑需要的模块或符号：from teaching_eval_provider import call_api。
from teaching_eval_provider import call_api
# 【逐行批注】 导入后续逻辑需要的模块或符号：from evaluation_core_fixture import run as run_core。
from evaluation_core_fixture import run as run_core


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 parser，更新脚本状态。
    parser = argparse.ArgumentParser(description=__doc__)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('manifest', help，更新脚本状态。
    parser.add_argument('manifest', help='外部私有JSON清单')
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--output', required，更新脚本状态。
    parser.add_argument('--output', required=True, help='新的隔离结果目录')
    # 【逐行批注】 计算右侧结果并赋给 args，更新脚本状态。
    args = parser.parse_args()
    # 【逐行批注】 计算右侧结果并赋给 manifest，更新脚本状态。
    manifest = json.loads(Path(args.manifest).read_text(encoding='utf-8-sig'))
    # 【逐行批注】 在受控上下文中执行资源操作：with using_decision(manifest.get('decisionFile') or manifest.get('shared', {}).…。
    with using_decision(manifest.get('decisionFile') or manifest.get('shared', {}).get('decisionFile')):
        # 【逐行批注】 结束当前函数并返回结果：return run_comparison(manifest, args.output, args.manifest)。
        return run_comparison(manifest, args.output, args.manifest)


# 【逐行批注】 定义函数 run_comparison，封装可复用的处理步骤。
def run_comparison(manifest, output_dir, manifest_path):
    # 【逐行批注】 计算右侧结果并赋给 mode，更新脚本状态。
    mode = manifest.get('mode', 'preflight')
    # 【逐行批注】 判断条件 mode == 'live':，成立时执行对应分支。
    if mode == 'live':
        # 【逐行批注】 调用 require_live_authorization 完成当前脚本或测试步骤：require_live_authorization()。
        require_live_authorization()
    # 【逐行批注】 判断条件 mode not in ('preflight', 'fixture', 'live'):，成立时执行对应分支。
    if mode not in ('preflight', 'fixture', 'live'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('未知运行模式')。
        raise ValueError('未知运行模式')
    # 【逐行批注】 计算右侧结果并赋给 output，更新脚本状态。
    output = isolated_output(output_dir)
    # 【逐行批注】 计算右侧结果并赋给 output.mkdir(parents，更新脚本状态。
    output.mkdir(parents=True, exist_ok=False)
    # 【逐行批注】 计算右侧结果并赋给 cases，更新脚本状态。
    cases = json.loads(Path(manifest['casesFile']).read_text(encoding='utf-8-sig')) if manifest.get('casesFile') else []
    # 【逐行批注】 判断条件 manifest.get('caseIds'):，成立时执行对应分支。
    if manifest.get('caseIds'):
        # 【逐行批注】 计算右侧结果并赋给 selected，更新脚本状态。
        selected = set(manifest['caseIds'])
        # 【逐行批注】 计算右侧结果并赋给 cases，更新脚本状态。
        cases = [case for case in cases if case['id'] in selected]
        # 【逐行批注】 判断条件 {case['id'] for case in cases} != selected:，成立时执行对应分支。
        if {case['id'] for case in cases} != selected:
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('所选案例在材料中不存在。')。
            raise ValueError('所选案例在材料中不存在。')
    # 固定偏好文件先复制一次，避免两版执行之间用户修改原文件造成不可比。
    # 【逐行批注】 计算右侧结果并赋给 shared，更新脚本状态。
    shared = dict(manifest.get('shared', {}))
    # 【逐行批注】 计算右侧结果并赋给 cumulative_budget，更新脚本状态。
    cumulative_budget = budget_copy(output/'budget-current.json', shared.get('budgetFile'))
    # 【逐行批注】 计算右侧结果并赋给 shared['budgetFile']，更新脚本状态。
    shared['budgetFile'] = str(cumulative_budget)
    # 【逐行批注】 判断条件 shared.get('preferencesFile'):，成立时执行对应分支。
    if shared.get('preferencesFile'):
        # 【逐行批注】 计算右侧结果并赋给 frozen_preferences，更新脚本状态。
        frozen_preferences = output/'preferences-frozen.json'
        # 【逐行批注】 调用 Path 完成当前脚本或测试步骤：frozen_preferences.write_bytes(Path(shared['preferencesFile']).read_b…。
        frozen_preferences.write_bytes(Path(shared['preferencesFile']).read_bytes())
        # 【逐行批注】 计算右侧结果并赋给 shared['preferencesFile']，更新脚本状态。
        shared['preferencesFile'] = str(frozen_preferences)
    # 【逐行批注】 计算右侧结果并赋给 material_hashes，更新脚本状态。
    material_hashes = {}
    # 【逐行批注】 遍历数据并重复执行循环体：for case in cases:。
    for case in cases:
        # 【逐行批注】 判断条件 case.get('libraryDatabase'):，成立时执行对应分支。
        if case.get('libraryDatabase'):
            # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
            source = Path(case['libraryDatabase'])
            # 【逐行批注】 遍历数据并重复执行循环体：for path in (source, Path(str(source)+'-wal')):。
            for path in (source, Path(str(source)+'-wal')):
                # 【逐行批注】 计算右侧结果并赋给 material_hashes[str(path.resolve())]，更新脚本状态。
                material_hashes[str(path.resolve())] = file_hash(path) if path.exists() else None
    # 【逐行批注】 计算右侧结果并赋给 results，更新脚本状态。
    results = []
    # 【逐行批注】 定义函数 save，封装可复用的处理步骤。
    def save():
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = {'manifestSha256': file_hash(manifest_path), 'mode': mode, 'results': results,
                 # 【逐行批注】 执行当前 Python 语句：'sourceMaterialHashes': material_hashes,。
                 'sourceMaterialHashes': material_hashes,
                 # 【逐行批注】 执行当前 Python 语句：'paidModelRequests': 0 if mode != 'live' else None,。
                 'paidModelRequests': 0 if mode != 'live' else None,
                 # 【逐行批注】 执行当前 Python 语句：'acceptanceComplete': False,。
                 'acceptanceComplete': False,
                 # 【逐行批注】 执行当前 Python 语句：'note': '零调用/模拟结果仅证明入口、协议和行为，不能证明真实Token、速度或教学质量。'}。
                 'note': '零调用/模拟结果仅证明入口、协议和行为，不能证明真实Token、速度或教学质量。'}
        # 【逐行批注】 计算右侧结果并赋给 temporary，更新脚本状态。
        temporary = output/'summary.json.tmp'
        # 【逐行批注】 计算右侧结果并赋给 temporary.write_text(json.dumps(value, ensur…，更新脚本状态。
        temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding='utf-8')
        # 【逐行批注】 执行当前 Python 语句：temporary.replace(output/'summary.json')。
        temporary.replace(output/'summary.json')
    # 【逐行批注】 调用 save 完成当前脚本或测试步骤：save()。
    save()
    # 【逐行批注】 遍历数据并重复执行循环体：for entry in manifest['entries']:。
    for entry in manifest['entries']:
        # 【逐行批注】 判断条件 entry.get('variant') not in ('baseline', 'optimized', 'candidate…，成立时执行对应分支。
        if entry.get('variant') not in ('baseline', 'optimized', 'candidate'):
            # 【逐行批注】 在错误情形抛出异常：raise ValueError('入口variant无效。')。
            raise ValueError('入口variant无效。')
        # 【逐行批注】 遍历数据并重复执行循环体：for field in ('preferencesFile', 'profileName', 'frozenConfiguration', 'mo…。
        for field in ('preferencesFile', 'profileName', 'frozenConfiguration', 'model'):
            # 【逐行批注】 判断条件 field in entry and (field not in shared or entry[field] != manif…，成立时执行对应分支。
            if field in entry and (field not in shared or entry[field] != manifest.get('shared', {}).get(field)):
                # 【逐行批注】 在错误情形抛出异常：raise ValueError('两版必须共用shared中的冻结配置：'+field)。
                raise ValueError('两版必须共用shared中的冻结配置：'+field)
        # 【逐行批注】 计算右侧结果并赋给 config，更新脚本状态。
        config = dict(entry, **shared, mode=mode, outputDir=str(output))
        # 【逐行批注】 遍历数据并重复执行循环体：for case in cases:。
        for case in cases:
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 判断条件 any((file_hash(path) if Path(path).exists() else None) != value …，成立时执行对应分支。
                if any((file_hash(path) if Path(path).exists() else None) != value for path, value in material_hashes.items()):
                    # 【逐行批注】 在错误情形抛出异常：raise ValueError('资料快照在对照过程中发生变化，停止比较。')。
                    raise ValueError('资料快照在对照过程中发生变化，停止比较。')
                # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
                value = call_api('', {'config': config}, {'vars': {'case': case}})
                # 预算先于任何结果解析保存；缺回执时不能沿用上一例的旧预算继续发送。
                # 【逐行批注】 计算右侧结果并赋给 budget_status，更新脚本状态。
                budget_status = value.get('budgetStatus')
                # 【逐行批注】 判断条件 not isinstance(budget_status, dict):，成立时执行对应分支。
                if not isinstance(budget_status, dict):
                    # 【逐行批注】 在错误情形抛出异常：raise BudgetStateError('入口没有返回可确认的预算回执；停止后续请求。')。
                    raise BudgetStateError('入口没有返回可确认的预算回执；停止后续请求。')
                # 【逐行批注】 调用 persist_budget 完成当前脚本或测试步骤：persist_budget(cumulative_budget, budget_status)。
                persist_budget(cumulative_budget, budget_status)
                # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
                result = json.loads(value['output']) if 'output' in value else value
                # 【逐行批注】 执行当前 Python 语句：results.append({'caseId': case['id'], 'variant': entry['variant'], 'result': result})。
                results.append({'caseId': case['id'], 'variant': entry['variant'], 'result': result})
                # 【逐行批注】 判断条件 mode != 'preflight' and result.get('complete') is not True:，成立时执行对应分支。
                if mode != 'preflight' and result.get('complete') is not True:
                    # 【逐行批注】 调用 save 完成当前脚本或测试步骤：save()。
                    save()
                    # 【逐行批注】 结束当前函数并返回结果：return 1。
                    return 1
            # 【逐行批注】 捕获并处理异常：except Exception as error:。
            except Exception as error:
                # 【逐行批注】 调用 type、str 完成当前脚本或测试步骤：results.append({'caseId': case['id'], 'variant': entry['variant'], 'e…。
                results.append({'caseId': case['id'], 'variant': entry['variant'], 'error': type(error).__name__+': '+str(error)})
                # 无法证明安全收尾的异常一律停止，禁止下一案例或下一版本发送。
                # 【逐行批注】 调用 save 完成当前脚本或测试步骤：save()。
                save()
                # 【逐行批注】 结束当前函数并返回结果：return 1。
                return 1
            # 【逐行批注】 调用 save 完成当前脚本或测试步骤：save()。
            save()
        # 【逐行批注】 判断条件 entry.get('productExecutable'):，成立时执行对应分支。
        if entry.get('productExecutable'):
            # 【逐行批注】 判断条件 mode != 'fixture':，成立时执行对应分支。
            if mode != 'fixture':
                # 【逐行批注】 执行当前 Python 语句：results.append({'variant': entry['variant'], 'coreFlows': {'skipped': True, 'reason': '完整…。
                results.append({'variant': entry['variant'], 'coreFlows': {'skipped': True, 'reason': '完整产品三流程只允许本机模拟服务模式。'}})
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 调用 run_core 完成当前脚本或测试步骤：results.append({'variant': entry['variant'], 'coreFlows': run_core(en…。
                results.append({'variant': entry['variant'], 'coreFlows': run_core(entry['productExecutable'], output/(entry['variant']+'-core'), entry.get('sourceCommit'))})
            # 【逐行批注】 调用 save 完成当前脚本或测试步骤：save()。
            save()
    # 【逐行批注】 输出当前运行结果：print(json.dumps({'summary': str(output/'summary.json'), 'paidModelRequest…。
    print(json.dumps({'summary': str(output/'summary.json'), 'paidModelRequests': 0 if mode != 'live' else None}, ensure_ascii=False))
    # 【逐行批注】 计算右侧结果并赋给 passed，更新脚本状态。
    passed = all('error' not in row and 'error' not in row.get('result', {}) and
                 # 【逐行批注】 执行当前 Python 语句：(mode == 'preflight' or row.get('result', row.get('coreFlows', {})).get('complete') is Tr…。
                 (mode == 'preflight' or row.get('result', row.get('coreFlows', {})).get('complete') is True)
                 # 【逐行批注】 遍历数据并重复执行循环体：for row in results)。
                 for row in results)
    # 【逐行批注】 结束当前函数并返回结果：return 0 if passed else 1。
    return 0 if passed else 1


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 判断条件 hasattr(sys.stdout, 'reconfigure'):，成立时执行对应分支。
    if hasattr(sys.stdout, 'reconfigure'):
        # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
        sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：sys.exit(main())。
    sys.exit(main())
