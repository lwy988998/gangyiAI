"""验证导师停止会收到终态、同一窗口可继续提问，关闭订阅后 HTTP 仍可用。"""
import asyncio
import json
import sys
from pathlib import Path
from urllib.request import Request, urlopen
import websockets
from classroom_v560_fixture import start_session


async def exercise(info, api):
    def fixture(body):
        with urlopen(Request(info['fixture']+'/fixture-control',data=json.dumps(body).encode(),
                            headers={'Content-Type':'application/json'}),timeout=10) as response:
            json.load(response)

    fixture({'slow':True})
    async with websockets.connect(info['base'].replace('http:','ws:')+'/ws/ask') as connection:
        await connection.send(json.dumps({'type':'ask','question':'验证停止正在生成的回答','requestId':'mentor-stop',
                                          'courseId':info['courseId'],'lessonId':info['lessonId']}))
        events=[]
        while True:
            event=json.loads(await asyncio.wait_for(connection.recv(),10))
            events.append(event)
            if event['type']=='delta':
                await connection.send(json.dumps({'type':'stop'}))
                break
        while True:
            event=json.loads(await asyncio.wait_for(connection.recv(),5))
            events.append(event)
            if event['type'] in ('done','error'): break
        assert event['type']=='done' and event['cancelled'], '停止后必须投递已取消终态'
        stopped=api('/api/learning-agent?taskId='+event['taskId'])
        assert stopped['status']=='cancelled'
        fixture({})
        await connection.send(json.dumps({'type':'ask','question':'停止后继续提问','requestId':'mentor-after-stop',
                                          'courseId':info['courseId'],'lessonId':info['lessonId']}))
        while True:
            event=json.loads(await asyncio.wait_for(connection.recv(),10))
            if event['type'] in ('done','error'): break
        assert event['type']=='done' and not event['cancelled'], '停止后同一连接应能继续提问'
    task=event['taskId']
    # 未消费完回放就关闭连接，覆盖切页和刷新时的服务端发送竞争。
    for _ in range(35):
        async with websockets.connect(info['base'].replace('http:','ws:')+'/ws/learning-agent',close_timeout=.3) as connection:
            await connection.send(json.dumps({'taskId':task,'afterSeq':0}))
            await connection.recv()
        assert api('/health')['status']=='healthy'


def main(executable):
    with start_session(executable) as (info,api,process):
        asyncio.run(exercise(info,api))
    print('导师实时连接：停止终态、停止后继续提问、关闭回放与后续请求通过。')


if __name__=='__main__':
    sys.stdout.reconfigure(encoding='utf-8')
    main(Path(sys.argv[1]).resolve())
