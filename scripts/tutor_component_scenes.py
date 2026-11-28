"""组件候选材料的结构审核及原生能力预检；不发送模型请求。"""
from __future__ import annotations

import argparse
from collections import Counter
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate_scene_plan(case):
    """旧集允许无计划；新计划不能把静态图、名称或合成状态冒充真实交互。"""
    if 'componentPlan' not in case:
        return None
    plan = case['componentPlan']
    require(isinstance(plan, dict), '组件计划须为对象')
    require(plan.get('mode') in {'interactive', 'static', 'text_fallback', 'none'}, '组件计划类型无效')
    require(isinstance(plan.get('reason'), str) and plan['reason'].strip(), '组件计划须说明适配边界')
    fixtures = plan.get('fixtures')
    static_types = plan.get('staticTypes')
    require(isinstance(fixtures, list) and len(fixtures) <= 8, '合成组件须为至多8项的列表')
    require(isinstance(static_types, list) and all(isinstance(x, str) for x in static_types) and
            len(set(static_types)) == len(static_types) and set(static_types) <= {'forces', 'flowchart', 'timeline', 'function'},
            '静态类型须为已有受控图示')
    require(plan.get('evidenceSource') == 'synthetic_fixture', '候选材料须明确标记为合成夹具')
    ids, types = set(), []
    for item in fixtures:
        require(isinstance(item, dict) and set(item) == {'id', 'config', 'states'}, '合成组件只接受身份、配置和状态')
        component_id = item['id']
        require(isinstance(component_id, str) and 0 < len(component_id.encode('utf-8')) <= 100 and component_id not in ids,
                '组件身份为空、过长或重复')
        ids.add(component_id)
        config = item['config']
        require(isinstance(config, dict) and isinstance(config.get('type'), str) and config['type'].strip(), '组件配置须含类型')
        require(not {'id', 'title', 'status'} & set(config), '组件配置不得伪造媒体身份或失败状态')
        types.append(config['type'])
        states = item['states']
        require(isinstance(states, list) and 1 <= len(states) <= 20 and all(isinstance(x, dict) for x in states),
                '合成组件须含1至20份状态')
        for state in states:
            require(type(state.get('schemaVersion')) is int and state['schemaVersion'] == 1 and
                    type(state.get('contentVersion')) is int and state['contentVersion'] == 1,
                    '候选状态须绑定当前协议和内容版本1')
    require(len(case['componentTypes']) == len(set(case['componentTypes'])) and set(types) == set(case['componentTypes']),
            '组件标签须与实际配置类型一致')
    if plan['mode'] == 'interactive':
        require(bool(fixtures) and not static_types, '交互计划须含配置状态，不能混入静态图示')
    else:
        require(not fixtures and not case['componentTypes'], '非交互计划不能声明交互组件')
        require(bool(static_types) == (plan['mode'] == 'static'), '只有静态计划可列出静态图示')
    return plan


def component_event(plan):
    """状态只作为合成输入送入主控；所有观察结论仍由原生代码重算。"""
    fixtures = plan['fixtures']
    return {
        'deepView': 'lecture', 'contentId': 'evaluation-components',
        'mediaState': {item['id']: copy.deepcopy(item['states'][-1]) for item in fixtures},
        'componentActivities': {item['id']: {
            'schemaVersion': 1, 'contentVersion': 1, 'sessionId': 'synthetic-evaluation',
            'totalChanges': len(item['states']), 'truncated': False,
            'events': [{'seq': index + 1, 'kind': 'change', 'state': copy.deepcopy(state)}
                       for index, state in enumerate(item['states'])],
        } for item in fixtures},
    }


def inspect_fixtures(driver, cases, env):
    """调用真实 C++ 的配置、课件与轨迹校验，不在 Python 复制学科公式。"""
    payload = {'cases': [{'id': case['id'], 'componentFixtures': case.get('componentPlan', {}).get('fixtures', [])}
                         for case in cases]}
    for case, item in zip(cases, payload['cases']):
        if 'setExercise' in case:
            item['questions'] = [{'setExercise': case['setExercise']}]
    with tempfile.TemporaryDirectory(prefix='gangyi-component-preflight-') as folder:
        path = Path(folder) / 'scenes.json'
        path.write_text(json.dumps(payload, ensure_ascii=False, allow_nan=False), encoding='utf-8')
        process = subprocess.run([str(driver), '--inspect-component-fixtures', str(path)], env=env,
                                 capture_output=True, encoding='utf-8', timeout=60)
    require(process.returncode in (0, 1) and bool(process.stdout), '原生组件预检未生成完整记录')
    from tutor_benchmark import parse_json
    result = parse_json(process.stdout)
    require(result.get('httpRequests') == 0 and type(result.get('valid')) is bool and
            len(result.get('cases', [])) == len(cases) and
            [item.get('caseId') for item in result['cases']] == [case['id'] for case in cases] and
            result['valid'] == all(item.get('valid') is True for item in result['cases']) and
            (process.returncode == 0) == result['valid'], '原生组件预检结果不完整或状态不符')
    for case, row in zip(cases, result['cases']):
        if 'setExercise' in case and row['valid']:
            require(row.get('checkedQuestions') == 1, '原生驱动未核对集合题，请重新构建。')
    return result


def audit(dataset, driver):
    """审核绑定当前构建；标签、合成计算和真实学习证据分别报告。"""
    import tutor_benchmark as benchmark
    from tutor_build_identity import checked_build, sha256
    from tutor_experiment import clean_environment
    coverage = benchmark.validate_dataset(dataset)
    receipt = checked_build(driver)
    process = subprocess.run([str(driver), '--interactive-capabilities'], env=clean_environment(),
                             capture_output=True, encoding='utf-8', timeout=30, check=True)
    registry = benchmark.parse_json(process.stdout)
    registered = {item['type'] for item in registry['components']}
    checked = inspect_fixtures(driver, dataset['cases'], clean_environment())
    rows = []
    for case, inspected in zip(dataset['cases'], checked['cases']):
        plan = case.get('componentPlan')
        unknown = sorted(set(case['componentTypes']) - registered)
        states = sum(len(item['states']) for item in (plan or {}).get('fixtures', []))
        rows.append({'caseId': case['id'], 'mode': plan['mode'] if plan else 'legacy_unclassified',
                     'interactiveTypes': case['componentTypes'], 'unregisteredTypes': unknown,
                     'fixtureValidation': inspected, 'syntheticStates': states,
                     'priorUserMessages': sum(turn['role'] == 'user' for turn in case['turns']),
                     'studentActivityVerified': False, 'realDialogueVerified': False,
                     'realMaterialVerified': False, 'finalAcceptanceEligible': False})
    valid = checked['valid'] and not any(row['unregisteredTypes'] for row in rows) and all('componentPlan' in c for c in dataset['cases'])
    return {'schemaVersion': 1, 'kind': 'component_scene_audit', 'datasetId': dataset['datasetId'],
            'datasetHash': benchmark.digest(dataset), 'driverSha256': receipt['executableSha256'],
            'buildReceiptSha256': sha256(str(driver) + '.build.json'), 'registry': registry,
            'coverage': coverage, 'modeCounts': dict(Counter(row['mode'] for row in rows)),
            'fixtureStates': sum(row['syntheticStates'] for row in rows), 'structurallyValid': valid,
            'realStudentActivities': 0, 'realContinuousDialogues': 0, 'verifiedMaterials': 0,
            'finalAcceptanceEligible': False, 'httpRequests': 0, 'paidModelRequests': 0, 'cases': rows}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dataset', type=Path, required=True)
    parser.add_argument('--driver', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try:
        import tutor_benchmark as benchmark
        report = audit(benchmark.load_json(args.dataset), args.driver.resolve())
        benchmark.write_json(args.output, report)
        print(json.dumps({'saved': str(args.output), 'structurallyValid': report['structurallyValid'],
                          'modeCounts': report['modeCounts'], 'paidModelRequests': 0}, ensure_ascii=False))
        return 0 if report['structurallyValid'] else 1
    except (ValueError, OSError, TypeError, KeyError, subprocess.SubprocessError) as error:
        print('组件材料审核未完成：' + str(error), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
