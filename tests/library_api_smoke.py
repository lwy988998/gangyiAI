# 【逐行批注】 执行当前 Python 语句："""资料接口的隔离验收：跨窗口冲突、历史快照、离线读取和重启保持，不调用模型。"""。
"""资料接口的隔离验收：跨窗口冲突、历史快照、离线读取和重启保持，不调用模型。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import closing。
from contextlib import closing
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import socket。
import socket
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sqlite3。
import sqlite3
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.error import HTTPError。
from urllib.error import HTTPError
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.parse import urlencode。
from urllib.parse import urlencode
# 【逐行批注】 导入后续逻辑需要的模块或符号：from urllib.request import Request, urlopen。
from urllib.request import Request, urlopen


# 【逐行批注】 计算右侧结果并赋给 executable，更新脚本状态。
executable = Path(sys.argv[1]).resolve()
# 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-library-api-') as folder:。
with tempfile.TemporaryDirectory(prefix='gangyi-library-api-') as folder:
    # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
    root = Path(folder)
    # 【逐行批注】 计算右侧结果并赋给 database，更新脚本状态。
    database = root / 'library.db'
    # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as listener:。
    with socket.socket() as listener:
        # 【逐行批注】 执行当前 Python 语句：listener.bind(('127.0.0.1', 0))。
        listener.bind(('127.0.0.1', 0))
        # 【逐行批注】 计算右侧结果并赋给 port，更新脚本状态。
        port = listener.getsockname()[1]
    # 【逐行批注】 计算右侧结果并赋给 base，更新脚本状态。
    base = f'http://127.0.0.1:{port}'
    # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
    env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(database),
               # 【逐行批注】 计算右侧结果并赋给 AI_API_KEY，更新脚本状态。
               AI_API_KEY='', BOCHA_API_KEY='', AI_BASE_URL='http://127.0.0.1:9/v1',
               # 【逐行批注】 计算右侧结果并赋给 LOCAL_CONTROL_TOKEN，更新脚本状态。
               LOCAL_CONTROL_TOKEN='library-api-check', AI_MODEL='未配置模型')
    # 【逐行批注】 定义函数 api，封装可复用的处理步骤。
    def api(query='', value=None):
        # 【逐行批注】 计算右侧结果并赋给 request，更新脚本状态。
        request = Request(base+'/api/my-ai/library'+query,
                          # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
                          data=None if value is None else json.dumps(value).encode(),
                          # 【逐行批注】 计算右侧结果并赋给 headers，更新脚本状态。
                          headers={'Content-Type': 'application/json'})
        # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(request, timeout=10) as response:。
        with urlopen(request, timeout=10) as response:
            # 【逐行批注】 结束当前函数并返回结果：return json.load(response)。
            return json.load(response)
    # 【逐行批注】 定义函数 start，封装可复用的处理步骤。
    def start():
        # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
        process = subprocess.Popen([str(executable)], cwd=executable.parent, env=env,
                                   # 【逐行批注】 计算右侧结果并赋给 stdout，更新脚本状态。
                                   stdout=log, stderr=log, creationflags=subprocess.CREATE_NO_WINDOW)
        # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(100):。
        for _ in range(100):
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 调用 api 完成当前脚本或测试步骤：api()。
                api()
                # 【逐行批注】 结束当前函数并返回结果：return process。
                return process
            # 【逐行批注】 捕获并处理异常：except OSError:。
            except OSError:
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                time.sleep(.1)
        # 【逐行批注】 执行当前 Python 语句：process.kill()。
        process.kill()
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('资料接口启动失败')。
        raise RuntimeError('资料接口启动失败')
    # 【逐行批注】 定义函数 stop，封装可复用的处理步骤。
    def stop(process):
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 在受控上下文中执行资源操作：with urlopen(Request(base+'/internal/shutdown', data=b'', headers={'X-Gangyi-Co…。
            with urlopen(Request(base+'/internal/shutdown', data=b'', headers={'X-Gangyi-Control-Token':'library-api-check'}), timeout=5):
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
            process.wait(timeout=10)
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
            if process.poll() is None:
                # 【逐行批注】 执行当前 Python 语句：process.kill()。
                process.kill()
                # 【逐行批注】 执行当前 Python 语句：process.wait()。
                process.wait()
    # 【逐行批注】 在受控上下文中执行资源操作：with (root/'service.log').open('w', encoding='utf-8') as log:。
    with (root/'service.log').open('w', encoding='utf-8') as log:
        # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
        process = start()
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api()['items'] == []。
            assert api()['items'] == []
            # 【逐行批注】 计算右侧结果并赋给 source，更新脚本状态。
            source = api(value={'url':'https://example.org/Study?Version=A', 'title':'隔离原文'})
            # 【逐行批注】 计算右侧结果并赋给 query，更新脚本状态。
            query = '?'+urlencode({'id':source['id']})
            # 【逐行批注】 计算右侧结果并赋给 api(value，更新脚本状态。
            api(value={'id':source['id'], 'version':source['version'], 'favorite':True})
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 api(value，更新脚本状态。
                api(value={'id':source['id'], 'version':source['version'], 'favorite':False})
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('旧窗口不应覆盖新选择')。
                raise AssertionError('旧窗口不应覆盖新选择')
            # 【逐行批注】 捕获并处理异常：except HTTPError as error:。
            except HTTPError as error:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert error.code in (400,409)。
                assert error.code in (400,409)
            # 【逐行批注】 计算右侧结果并赋给 detail，更新脚本状态。
            detail = api(query)
            # 【逐行批注】 计算右侧结果并赋给 api(value，更新脚本状态。
            api(value={'id':source['id'], 'version':detail['version'], 'state':'excluded'})
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('?filter=favorites')['items'][0]['id'] == source['id']。
            assert api('?filter=favorites')['items'][0]['id'] == source['id']
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('?filter=excluded')['items'][0]['choice']['manual'] is True。
            assert api('?filter=excluded')['items'][0]['choice']['manual'] is True
            # 两版快照模拟已持久化的升级前课堂，接口不能随新原文改写旧依据。
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 遍历数据并重复执行循环体：for id,text in [('old-snapshot','升级前的固定原文'), ('new-snapshot','更新后的原文')]:。
                for id,text in [('old-snapshot','升级前的固定原文'), ('new-snapshot','更新后的原文')]:
                    # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                    payload={'sourceId':source['id'], 'checkedAt':'2026-10-14T00:00:00Z',
                             # 【逐行批注】 执行当前 Python 语句：'publishedAt':None, 'segments':[{'id':'s1','text':text}]}。
                             'publishedAt':None, 'segments':[{'id':'s1','text':text}]}
                    # 【逐行批注】 调用 VALUES 完成当前脚本或测试步骤：db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)',(id,'library-sn…。
                    db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)',(id,'library-snapshot',json.dumps(payload),'2026-10-14',1))
                # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                payload = json.loads(db.execute('SELECT payload FROM MyAIRecord WHERE id=?',(source['id'],)).fetchone()[0])
                # 【逐行批注】 计算右侧结果并赋给 payload.update(snapshotId，更新脚本状态。
                payload.update(snapshotId='new-snapshot',readStatus='read')
                # 【逐行批注】 计算右侧结果并赋给 db.execute('UPDATE MyAIRecord SET payload，更新脚本状态。
                db.execute('UPDATE MyAIRecord SET payload=? WHERE id=?',(json.dumps(payload),source['id']))
                # 【逐行批注】 执行当前 Python 语句：db.commit()。
                db.commit()
            # 【逐行批注】 计算右侧结果并赋给 old，更新脚本状态。
            old = api('?'+urlencode({'id':source['id'],'snapshotId':'old-snapshot'}))
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert old['snapshot']['segments'][0]['text'] == '升级前的固定原文'。
            assert old['snapshot']['segments'][0]['text'] == '升级前的固定原文'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert old['snapshot']['publishedAt'] is None。
            assert old['snapshot']['publishedAt'] is None
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api(query)['snapshot']['segments'][0]['text'] == '更新后的原文'。
            assert api(query)['snapshot']['segments'][0]['text'] == '更新后的原文'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api('?'+urlencode({'search':'更新后的原文'}))['items'][0]['id'] == source['id']。
            assert api('?'+urlencode({'search':'更新后的原文'}))['items'][0]['id'] == source['id']
            # 【逐行批注】 计算右侧结果并赋给 usage，更新脚本状态。
            usage = api('/usage')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert usage['searchRequests'] == usage['fetchRequests'] == 0 and usage['tokens…。
            assert usage['searchRequests'] == usage['fetchRequests'] == 0 and usage['tokens'] is None
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 调用 stop 完成当前脚本或测试步骤：stop(process)。
            stop(process)
        # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
        process = start()
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 计算右侧结果并赋给 restored，更新脚本状态。
            restored = api(query)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert restored['favorite'] is True and restored['choice']['state'] == 'exclude…。
            assert restored['favorite'] is True and restored['choice']['state'] == 'excluded'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert restored['snapshot']['segments'][0]['text'] == '更新后的原文'。
            assert restored['snapshot']['segments'][0]['text'] == '更新后的原文'
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
            with closing(sqlite3.connect(database)) as db:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind IN ('pra…。
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind IN ('practice','quiz','review')").fetchone()[0] == 0
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute("SELECT count(*) FROM MyAIRecord WHERE kind='usage'").fetchon…。
                assert db.execute("SELECT count(*) FROM MyAIRecord WHERE kind='usage'").fetchone()[0] == 0
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 调用 stop 完成当前脚本或测试步骤：stop(process)。
            stop(process)
# 【逐行批注】 输出当前运行结果：print('资料接口冲突、排除、旧快照、离线读取、重启及零学习证据检查通过')。
print('资料接口冲突、排除、旧快照、离线读取、重启及零学习证据检查通过')
