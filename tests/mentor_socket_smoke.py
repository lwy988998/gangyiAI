# 【逐行批注】 执行当前 Python 语句："""验证导师停止会收到终态、同一窗口可继续提问，关闭订阅后 HTTP 仍可用。"""。
"""验证导师停止会收到终态、同一窗口可继续提问，关闭订阅后 HTTP 仍可用。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import asyncio。
import asyncio
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
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
    # 【逐行批注】 输出当前运行结果：print('导师实时连接：停止终态、停止后继续提问、关闭回放与后续请求通过。')。
    print('导师实时连接：停止终态、停止后继续提问、关闭回放与后续请求通过。')


# 【逐行批注】 判断条件 __name__=='__main__':，成立时执行对应分支。
if __name__=='__main__':
    # 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
    sys.stdout.reconfigure(encoding='utf-8')
    # 【逐行批注】 调用 main、Path 完成当前脚本或测试步骤：main(Path(sys.argv[1]).resolve())。
    main(Path(sys.argv[1]).resolve())
