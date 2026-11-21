# 【逐行批注】 执行当前 Python 语句："""验证导师停止会收到终态、同一窗口可继续提问，关闭订阅后 HTTP 仍可用。"""。
"""验证导师停止会收到终态、同一窗口可继续提问，关闭订阅后 HTTP 仍可用。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import asyncio。
import asyncio
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
import re
import sqlite3
from contextlib import closing
from urllib.error import HTTPError
from urllib.parse import urlencode
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen
# 【逐行批注】 导入后续逻辑需要的模块或符号：import websockets。
import websockets
# 【逐行批注】 导入后续逻辑需要的模块或符号：from classroom_v560_fixture import start_session。
from classroom_v560_fixture import start_session


def seed_material(info, api):
    source = api('/api/my-ai/library', {
        'url': 'https://example.org/mentor-material', 'title': '课堂资料测试',
        'courseId': info['courseId'],
    })
    source_id = source['id']
    with closing(sqlite3.connect(info['database'])) as db:
        for snapshot, segments in [
            ('snap-pinned', [{'id': 'fixed-one', 'page': 4, 'text': '旧快照中的第一段。'},
                             {'id': 'fixed-two', 'page': 5, 'text': '旧快照中的第二段。'}]),
            ('snap-latest', [{'id': 'latest-one', 'page': 2, 'text': '新版资料正文。'}]),
        ]:
            data = dict(sourceId=source_id, segments=segments, checkedAt='2026-11-18')
            db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)',
                       (snapshot, 'library-snapshot', json.dumps(data), '2026-11-18', 1))
        payload = json.loads(db.execute('SELECT payload FROM MyAIRecord WHERE id=?', (source_id,)).fetchone()[0])
        payload.update(snapshotId='snap-latest', readStatus='read')
        db.execute('UPDATE MyAIRecord SET payload=? WHERE id=?', (json.dumps(payload), source_id))
        db.commit()
    return dict(sourceId=source_id, snapshotId='snap-pinned')


async def exercise_material(info, api):
    material = seed_material(info, api)
    course = info['courseId']
    url = info['base'] + '/ask?' + urlencode(dict(material, courseId=course))
    with urlopen(url, timeout=10) as response:
        page = response.read().decode()
    conversation = re.search(r'data-conversation-id="([^"]+)"', page)[1]
    assert '查看选定原文' in page and '课堂资料测试' in page
    conversation_path = '/api/conversations/' + conversation
    home_step_id = api('/api/home/next-step').get('id')

    async def ask(connection, request_id, question):
        await connection.send(json.dumps(dict(type='ask', requestId=request_id, question=question,
                                              courseId=course, material=material)))
        deltas = []
        while True:
            try:
                event = json.loads(await asyncio.wait_for(connection.recv(), 15))
            except TimeoutError as error:
                with closing(sqlite3.connect(info['database'])) as db:
                    task = db.execute(
                        '''SELECT json_extract(payload, '$.status') FROM ClassroomActivity
                           WHERE kind='agent-task' AND json_extract(payload, '$.event.requestId')=?''',
                        (request_id,)).fetchone()
                raise AssertionError(f'资料追问 {request_id} 等待终态超时，已保存任务状态：{task}') from error
            if event['type'] in ('done', 'error'):
                return event, ''.join(deltas)
            if event['type'] == 'delta':
                deltas.append(event['text'])

    async with websockets.connect(info['base'].replace('http:', 'ws:') + '/ws/ask') as connection:
        result, answer = await ask(connection, 'material-one', '先解释第一段')
        assert result['type'] == 'done' and '旧快照中的第一段' in answer and 'snap-pinned#fixed-one' in answer, (result, answer)
        followups = 6
        questions = ['先解释第一段']
        for index in range(followups):
            question = f'接着上个问题解释，第 {index + 1} 次追问'
            result, answer = await ask(connection, f'material-follow-up-{index}', question)
            assert result['type'] == 'done' and 'snap-pinned#fixed-one' in answer, (result, answer)
            assert any(f'前次提问：{previous}' in answer for previous in questions), answer
            questions.append(question)
        result, answer = await ask(connection, 'material-excerpt', '读取后半部分')
        assert result['type'] == 'done' and '旧快照中的第二段' in answer and 'snap-pinned#fixed-two' in answer, (result, answer)
        result, answer = await ask(connection, 'material-wrong-citation', '伪造新版引用')
        assert result['type'] == 'error' and not answer, '引用另一版本的回复不能流出'
    history = api(conversation_path)['messages']
    assert sum(item['role'] == 'assistant' for item in history) == followups + 2
    assert not any('旧快照中的第一段' in item['text'] for item in api('/api/conversations/general')['messages'])
    assert api('/api/home/next-step').get('id') == home_step_id

    try:
        api('/api/ask', dict(question='伪造快照应在提交时失败', courseId=course,
                             material=dict(material, snapshotId='missing')))
        raise AssertionError('不存在的快照不能生成教学任务')
    except HTTPError as error:
        assert error.code == 409
    path = '/api/my-ai/library?' + urlencode(dict(id=material['sourceId'], courseId=course))
    source = api(path)
    api('/api/my-ai/library', dict(id=material['sourceId'], version=source['version'], state='excluded', courseId=course))
    try:
        with urlopen(url, timeout=10):
            raise AssertionError('已排除的资料不能继续使用')
    except HTTPError as error:
        assert error.code == 404 and '返回学习资料' in error.read().decode()
    assert api(conversation_path)['messages'] == history


# 【逐行批注】 定义函数 exercise，封装可复用的处理步骤。
async def exercise(info, api):
    # 【逐行批注】 定义函数 fixture，封装可复用的处理步骤。
    def fixture(body):
        # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(info['fixture']+'/fixture-control',data=json.dumps(body).e…。
        with urlopen(Request(info['fixture']+'/fixture-control',data=json.dumps(body).encode(),
                            # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                            headers={'Content-Type':'application/json'}),timeout=10) as response:
            # 【逐行批注】 执行当前 Python 语句：json.load(response)。
            json.load(response)

    # 【逐行批注】 调用 fixture 完成当前脚本或测试步骤：fixture({'slow':True})。
    fixture({'slow':True})
    # 【逐行批注】 执行当前 Python 语句：async with websockets.connect(info['base'].replace('http:','ws:')+'/ws/ask') as connectio…。
    async with websockets.connect(info['base'].replace('http:','ws:')+'/ws/ask') as connection:
        # 【逐行批注】 执行当前 Python 语句：await connection.send(json.dumps({'type':'ask','question':'验证停止正在生成的回答','requestId':'ment…。
        await connection.send(json.dumps({'type':'ask','question':'验证停止正在生成的回答','requestId':'mentor-stop',
                                          # 【逐行批注】 执行当前 Python 语句：'courseId':info['courseId'],'lessonId':info['lessonId']}))。
                                          'courseId':info['courseId'],'lessonId':info['lessonId']}))
        # 【逐行批注】 计算右侧结果并赋给 events，更新脚本状态。
        events=[]
        # 【逐行批注】 在条件成立期间重复执行：while True:。
        while True:
            # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
            event=json.loads(await asyncio.wait_for(connection.recv(),10))
            # 【逐行批注】 执行当前 Python 语句：events.append(event)。
            events.append(event)
            # 【逐行批注】 判断条件 event['type']=='delta':，成立时执行对应分支。
            if event['type']=='delta':
                # 【逐行批注】 执行当前 Python 语句：await connection.send(json.dumps({'type':'stop'}))。
                await connection.send(json.dumps({'type':'stop'}))
                # 【逐行批注】 执行当前 Python 语句：break。
                break
        # 【逐行批注】 在条件成立期间重复执行：while True:。
        while True:
            # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
            event=json.loads(await asyncio.wait_for(connection.recv(),5))
            # 【逐行批注】 执行当前 Python 语句：events.append(event)。
            events.append(event)
            # 【逐行批注】 判断条件 event['type'] in ('done','error'): break，成立时执行对应分支。
            if event['type'] in ('done','error'): break
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert event['type']=='done' and event['cancelled'], '停止后必须投递已取消终态'。
        assert event['type']=='done' and event['cancelled'], '停止后必须投递已取消终态'
        # 【逐行批注】 计算右侧结果并赋给 stopped，更新脚本状态。
        stopped=api('/api/learning-agent?taskId='+event['taskId'])
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert stopped['status']=='cancelled'。
        assert stopped['status']=='cancelled'
        # 【逐行批注】 调用 fixture 完成当前脚本或测试步骤：fixture({})。
        fixture({})
        # 【逐行批注】 执行当前 Python 语句：await connection.send(json.dumps({'type':'ask','question':'停止后继续提问','requestId':'mentor-a…。
        await connection.send(json.dumps({'type':'ask','question':'停止后继续提问','requestId':'mentor-after-stop',
                                          # 【逐行批注】 执行当前 Python 语句：'courseId':info['courseId'],'lessonId':info['lessonId']}))。
                                          'courseId':info['courseId'],'lessonId':info['lessonId']}))
        # 【逐行批注】 在条件成立期间重复执行：while True:。
        while True:
            # 【逐行批注】 计算右侧结果并赋给 event，更新脚本状态。
            event=json.loads(await asyncio.wait_for(connection.recv(),10))
            # 【逐行批注】 判断条件 event['type'] in ('done','error'): break，成立时执行对应分支。
            if event['type'] in ('done','error'): break
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert event['type']=='done' and not event['cancelled'], '停止后同一连接应能继续提问'。
        assert event['type']=='done' and not event['cancelled'], '停止后同一连接应能继续提问'
    # 【逐行批注】 计算右侧结果并赋给 task，更新脚本状态。
    task=event['taskId']
    # 未消费完回放就关闭连接，覆盖切页和刷新时的服务端发送竞争。
    # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(35):。
    for _ in range(35):
        # 【逐行批注】 计算右侧结果并赋给 async with websockets.connect(info['base'].r…，更新脚本状态。
        async with websockets.connect(info['base'].replace('http:','ws:')+'/ws/learning-agent',close_timeout=.3) as connection:
            # 【逐行批注】 执行当前 Python 语句：await connection.send(json.dumps({'taskId':task,'afterSeq':0}))。
            await connection.send(json.dumps({'taskId':task,'afterSeq':0}))
            # 【逐行批注】 执行当前 Python 语句：await connection.recv()。
            await connection.recv()
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('/health')['status']=='healthy'。
        assert api('/health')['status']=='healthy'


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main(executable):
    # 【逐行批注】 在受控上下文中执行资源操作：with start_session(executable) as (info,api,process):。
    with start_session(executable) as (info,api,process):
        # 【逐行批注】 调用 exercise 完成当前脚本或测试步骤：asyncio.run(exercise(info,api))。
        asyncio.run(exercise(info,api))
        asyncio.run(exercise_material(info,api))
    # 【逐行批注】 输出当前运行结果：print('导师实时连接：停止终态、停止后继续提问、关闭回放与后续请求通过。')。
    print('导师实时连接通过停止、重连和回放测试，资料学习通过固定快照、追问、引用校验和排除测试。')


# 【逐行批注】 判断条件 __name__=='__main__':，成立时执行对应分支。
if __name__=='__main__':
    # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
    sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main、Path 完成当前脚本或测试步骤：main(Path(sys.argv[1]).resolve())。
    main(Path(sys.argv[1]).resolve())
