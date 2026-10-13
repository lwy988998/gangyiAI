"""客观协议检查与固定候选评分要求；教学语义结论另在对照报告中逐轮审阅。"""
import json


def get_assert(output, context):
    result = json.loads(output) if isinstance(output, str) else output
    case = context['vars']['case']
    if any(turn['task'].get('failure',{}).get('kind')=='eval_budget' for turn in result['turns']):
        return {'pass':False,'score':0,'reason':'验收Token预算不足，模型请求已停止；案例未完成，不作为教学效果结论'}
    failures = []
    interactions = result['interactions']
    actual_inputs = {r['id']:r['payload'] for r in interactions if r['kind']=='chat-user'}
    evaluations = [r['payload'] for r in interactions if r['kind'] == 'practice']
    for turn in result['turns']:
        event, task = turn['event'], turn['task']
        if task['status'] in ('failed', 'superseded'):
            failures.append('教学请求失败：'+task.get('error', task['status']))
        for inp in turn['currentInputs']:
            inp = dict(actual_inputs.get(inp['interactionId'], {}), **inp)
            if event['type'] == 'teaching_request' and any(e['interactionId'] == inp['interactionId'] for e in evaluations):
                failures.append('教学请求被当成作答评分')
            if event['requestId'] == 'eval-turn-0' and inp.get('helpMode', 'after_hint' if inp.get('assisted') else 'independent') != 'independent':
                failures.append('普通讲解被误记为本题帮助')
        if event.get('requestType') in ('hint','worked_example','rephrase'):
            targets = {actual_inputs[inp['interactionId']].get('questionId') for inp in turn['currentInputs']
                       if inp['interactionId'] in actual_inputs}
            for action in task.get('completedActions',{}).values():
                decoded=json.loads(action['encoded'])
                section=decoded.get('args',{}).get('section',{})
                if decoded['tool']=='append_section' and section.get('helpType') in ('hint','worked_example'):
                    if section.get('targetQuestionId') not in targets:
                        failures.append('教学请求的帮助给了另一道题')
                    if event.get('requestType')=='hint' and section.get('helpType')=='worked_example':
                        failures.append('只请求小提示却提供完整示范')
    for grade in case.get('grading', []):
        text = grade['response']
        matches = [e for e in evaluations if e.get('response') == text]
        if grade.get('followup'):
            if matches:
                failures.append('纯追问被评分')
        elif not matches:
            failures.append('缺少预编回答的评价：'+text)
        elif any(e.get('score') is not None and not grade['minScore'] <= e['score'] <= grade['maxScore'] for e in matches):
            failures.append('评分偏离预编参考范围：'+text)
    for turn in result['turns']:
        if turn['event'].get('sectionId') == 'question-1':
            for inp in turn['currentInputs']:
                inp = dict(actual_inputs.get(inp['interactionId'], {}), **inp)
                if inp.get('assisted') or inp.get('helpHistory'):
                    failures.append('另一题的帮助影响了独立题的判断')
    if case.get('seedSecond') is False and result['variant'] == 'optimized':
        original = result['lesson']['sections'][1]['questionId']
        if not any(s.get('rePracticeOf') == original for s in result['lesson']['sections']):
            failures.append('复练新题缺少原问题关联')
    if len(result['turns']) != len(case['turns']):
        failures.append('多轮案例没有完整走完')
    return {'pass': not failures, 'score': max(0,1-len(failures)/max(1, len(case['turns'])+len(case.get('grading', [])))),
            'reason': '；'.join(failures) if failures else '客观协议与预编评分范围通过；教学适合度见逐轮审阅'}
