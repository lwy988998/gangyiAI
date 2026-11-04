# 【逐行批注】 执行当前 Python 语句："""客观协议检查与固定候选评分要求；教学语义结论另在对照报告中逐轮审阅。"""。
"""客观协议检查与固定候选评分要求；教学语义结论另在对照报告中逐轮审阅。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json


# 【逐行批注】 定义函数 get_assert，封装可复用的处理步骤。
def get_assert(output, context):
    # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
    result = json.loads(output) if isinstance(output, str) else output
    # 【逐行批注】 计算右侧结果并赋给 case，更新脚本状态。
    case = context['vars']['case']
    # 【逐行批注】 判断条件 any(turn['task'].get('failure',{}).get('kind')=='eval_budget' fo…，成立时执行对应分支。
    if any(turn['task'].get('failure',{}).get('kind')=='eval_budget' for turn in result['turns']):
        # 【逐行批注】 结束当前函数并返回结果：return {'pass':False,'score':0,'reason':'验收Token预算不足，模型请求已停止；案例未完成，不作…。
        return {'pass':False,'score':0,'reason':'验收Token预算不足，模型请求已停止；案例未完成，不作为教学效果结论'}
    # 【逐行批注】 计算右侧结果并赋给 failures，更新脚本状态。
    failures = []
    # 【逐行批注】 计算右侧结果并赋给 interactions，更新脚本状态。
    interactions = result['interactions']
    # 【逐行批注】 计算右侧结果并赋给 actual_inputs，更新脚本状态。
    actual_inputs = {r['id']:r['payload'] for r in interactions if r['kind']=='chat-user'}
    # 【逐行批注】 计算右侧结果并赋给 evaluations，更新脚本状态。
    evaluations = [r['payload'] for r in interactions if r['kind'] == 'practice']
    # 【逐行批注】 遍历数据并重复执行循环体：for turn in result['turns']:。
    for turn in result['turns']:
        # 【逐行批注】 计算右侧结果并赋给 event, task，更新脚本状态。
        event, task = turn['event'], turn['task']
        # 【逐行批注】 判断条件 task['status'] in ('failed', 'superseded'):，成立时执行对应分支。
        if task['status'] in ('failed', 'superseded'):
            # 【逐行批注】 执行当前 Python 语句：failures.append('教学请求失败：'+task.get('error', task['status']))。
            failures.append('教学请求失败：'+task.get('error', task['status']))
        # 【逐行批注】 遍历数据并重复执行循环体：for inp in turn['currentInputs']:。
        for inp in turn['currentInputs']:
            # 【逐行批注】 计算右侧结果并赋给 inp，更新脚本状态。
            inp = dict(actual_inputs.get(inp['interactionId'], {}), **inp)
            # 【逐行批注】 判断条件 event['type'] == 'teaching_request' and any(e['interactionId'] =…，成立时执行对应分支。
            if event['type'] == 'teaching_request' and any(e['interactionId'] == inp['interactionId'] for e in evaluations):
                # 【逐行批注】 执行当前 Python 语句：failures.append('教学请求被当成作答评分')。
                failures.append('教学请求被当成作答评分')
            # 【逐行批注】 判断条件 event['requestId'] == 'eval-turn-0' and inp.get('helpMode', 'aft…，成立时执行对应分支。
            if event['requestId'] == 'eval-turn-0' and inp.get('helpMode', 'after_hint' if inp.get('assisted') else 'independent') != 'independent':
                # 【逐行批注】 执行当前 Python 语句：failures.append('普通讲解被误记为本题帮助')。
                failures.append('普通讲解被误记为本题帮助')
        # 【逐行批注】 判断条件 event.get('requestType') in ('hint','worked_example','rephrase'):，成立时执行对应分支。
        if event.get('requestType') in ('hint','worked_example','rephrase'):
            # 【逐行批注】 计算右侧结果并赋给 targets，更新脚本状态。
            targets = {actual_inputs[inp['interactionId']].get('questionId') for inp in turn['currentInputs']
                       # 【逐行批注】 判断条件 inp['interactionId'] in actual_inputs}，成立时执行对应分支。
                       if inp['interactionId'] in actual_inputs}
            # 【逐行批注】 遍历数据并重复执行循环体：for action in task.get('completedActions',{}).values():。
            for action in task.get('completedActions',{}).values():
                # 【逐行批注】 计算右侧结果并赋给 decoded，更新脚本状态。
                decoded=json.loads(action['encoded'])
                # 【逐行批注】 计算右侧结果并赋给 section，更新脚本状态。
                section=decoded.get('args',{}).get('section',{})
                # 【逐行批注】 判断条件 decoded['tool']=='append_section' and section.get('helpType') in…，成立时执行对应分支。
                if decoded['tool']=='append_section' and section.get('helpType') in ('hint','worked_example'):
                    # 【逐行批注】 判断条件 section.get('targetQuestionId') not in targets:，成立时执行对应分支。
                    if section.get('targetQuestionId') not in targets:
                        # 【逐行批注】 执行当前 Python 语句：failures.append('教学请求的帮助给了另一道题')。
                        failures.append('教学请求的帮助给了另一道题')
                    # 【逐行批注】 判断条件 event.get('requestType')=='hint' and section.get('helpType')=='w…，成立时执行对应分支。
                    if event.get('requestType')=='hint' and section.get('helpType')=='worked_example':
                        # 【逐行批注】 执行当前 Python 语句：failures.append('只请求小提示却提供完整示范')。
                        failures.append('只请求小提示却提供完整示范')
    # 【逐行批注】 遍历数据并重复执行循环体：for grade in case.get('grading', []):。
    for grade in case.get('grading', []):
        # 【逐行批注】 计算右侧结果并赋给 text，更新脚本状态。
        text = grade['response']
        # 【逐行批注】 计算右侧结果并赋给 matches，更新脚本状态。
        matches = [e for e in evaluations if e.get('response') == text]
        # 【逐行批注】 判断条件 grade.get('followup'):，成立时执行对应分支。
        if grade.get('followup'):
            # 【逐行批注】 判断条件 matches:，成立时执行对应分支。
            if matches:
                # 【逐行批注】 执行当前 Python 语句：failures.append('纯追问被评分')。
                failures.append('纯追问被评分')
        # 【逐行批注】 当前一条件不成立时继续判断：not matches:。
        elif not matches:
            # 【逐行批注】 执行当前 Python 语句：failures.append('缺少预编回答的评价：'+text)。
            failures.append('缺少预编回答的评价：'+text)
        # 【逐行批注】 当前一条件不成立时继续判断：any(e.get('score') is not None and not grade['minScore'] <= e['s…。
        elif any(e.get('score') is not None and not grade['minScore'] <= e['score'] <= grade['maxScore'] for e in matches):
            # 【逐行批注】 执行当前 Python 语句：failures.append('评分偏离预编参考范围：'+text)。
            failures.append('评分偏离预编参考范围：'+text)
    # 【逐行批注】 遍历数据并重复执行循环体：for turn in result['turns']:。
    for turn in result['turns']:
        # 【逐行批注】 判断条件 turn['event'].get('sectionId') == 'question-1':，成立时执行对应分支。
        if turn['event'].get('sectionId') == 'question-1':
            # 【逐行批注】 遍历数据并重复执行循环体：for inp in turn['currentInputs']:。
            for inp in turn['currentInputs']:
                # 【逐行批注】 计算右侧结果并赋给 inp，更新脚本状态。
                inp = dict(actual_inputs.get(inp['interactionId'], {}), **inp)
                # 【逐行批注】 判断条件 inp.get('assisted') or inp.get('helpHistory'):，成立时执行对应分支。
                if inp.get('assisted') or inp.get('helpHistory'):
                    # 【逐行批注】 执行当前 Python 语句：failures.append('另一题的帮助影响了独立题的判断')。
                    failures.append('另一题的帮助影响了独立题的判断')
    # 【逐行批注】 判断条件 case.get('seedSecond') is False and result['variant'] == 'optimi…，成立时执行对应分支。
    if case.get('seedSecond') is False and result['variant'] == 'optimized':
        # 【逐行批注】 计算右侧结果并赋给 original，更新脚本状态。
        original = result['lesson']['sections'][1]['questionId']
        # 【逐行批注】 判断条件 not any(s.get('rePracticeOf') == original for s in result['lesso…，成立时执行对应分支。
        if not any(s.get('rePracticeOf') == original for s in result['lesson']['sections']):
            # 【逐行批注】 执行当前 Python 语句：failures.append('复练新题缺少原问题关联')。
            failures.append('复练新题缺少原问题关联')
    # 【逐行批注】 判断条件 len(result['turns']) != len(case['turns']):，成立时执行对应分支。
    if len(result['turns']) != len(case['turns']):
        # 【逐行批注】 执行当前 Python 语句：failures.append('多轮案例没有完整走完')。
        failures.append('多轮案例没有完整走完')
    # 【逐行批注】 结束当前函数并返回结果：return {'pass': not failures, 'score': max(0,1-len(failures)/max(1, l…。
    return {'pass': not failures, 'score': max(0,1-len(failures)/max(1, len(case['turns'])+len(case.get('grading', [])))),
            # 【逐行批注】 执行当前 Python 语句：'reason': '；'.join(failures) if failures else '客观协议与预编评分范围通过；教学适合度见逐轮审阅'}。
            'reason': '；'.join(failures) if failures else '客观协议与预编评分范围通过；教学适合度见逐轮审阅'}
