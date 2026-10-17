"""资料接口的隔离验收：跨窗口冲突、历史快照、离线读取和重启保持，不调用模型。"""
import json
from contextlib import closing
import os
from pathlib import Path
import socket
import sqlite3
import subprocess
import sys
import tempfile
import time
from urllib.error import HTTPError
from urllib.parse import urlencode
from urllib.request import Request, urlopen


executable = Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='gangyi-library-api-') as folder:
    root = Path(folder)
    database = root / 'library.db'
    with socket.socket() as listener:
        listener.bind(('127.0.0.1', 0))
        port = listener.getsockname()[1]
    base = f'http://127.0.0.1:{port}'
    env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(database),
               AI_API_KEY='', BOCHA_API_KEY='', AI_BASE_URL='http://127.0.0.1:9/v1',
               LOCAL_CONTROL_TOKEN='library-api-check', AI_MODEL='未配置模型')
    def api(query='', value=None):
        request = Request(base+'/api/my-ai/library'+query,
                          data=None if value is None else json.dumps(value).encode(),
                          headers={'Content-Type': 'application/json'})
        with urlopen(request, timeout=10) as response:
            return json.load(response)
    def start():
        process = subprocess.Popen([str(executable)], cwd=executable.parent, env=env,
                                   stdout=log, stderr=log, creationflags=subprocess.CREATE_NO_WINDOW)
        for _ in range(100):
            try:
                api()
                return process
            except OSError:
                time.sleep(.1)
        process.kill()
        raise RuntimeError('资料接口启动失败')
    def stop(process):
        try:
            with urlopen(Request(base+'/internal/shutdown', data=b'', headers={'X-Gangyi-Control-Token':'library-api-check'}), timeout=5):
                pass
            process.wait(timeout=10)
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
    with (root/'service.log').open('w', encoding='utf-8') as log:
        process = start()
        try:
            assert api()['items'] == []
            source = api(value={'url':'https://example.org/Study?Version=A', 'title':'隔离原文'})
            query = '?'+urlencode({'id':source['id']})
            api(value={'id':source['id'], 'version':source['version'], 'favorite':True})
            try:
                api(value={'id':source['id'], 'version':source['version'], 'favorite':False})
                raise AssertionError('旧窗口不应覆盖新选择')
            except HTTPError as error:
                assert error.code in (400,409)
            detail = api(query)
            api(value={'id':source['id'], 'version':detail['version'], 'state':'excluded'})
            assert api('?filter=favorites')['items'][0]['id'] == source['id']
            assert api('?filter=excluded')['items'][0]['choice']['manual'] is True
            # 两版快照模拟已持久化的升级前课堂，接口不能随新原文改写旧依据。
            with closing(sqlite3.connect(database)) as db:
                for id,text in [('old-snapshot','升级前的固定原文'), ('new-snapshot','更新后的原文')]:
                    payload={'sourceId':source['id'], 'checkedAt':'2026-10-14T00:00:00Z',
                             'publishedAt':None, 'segments':[{'id':'s1','text':text}]}
                    db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)',(id,'library-snapshot',json.dumps(payload),'2026-10-14',1))
                payload = json.loads(db.execute('SELECT payload FROM MyAIRecord WHERE id=?',(source['id'],)).fetchone()[0])
                payload.update(snapshotId='new-snapshot',readStatus='read')
                db.execute('UPDATE MyAIRecord SET payload=? WHERE id=?',(json.dumps(payload),source['id']))
                db.commit()
            old = api('?'+urlencode({'id':source['id'],'snapshotId':'old-snapshot'}))
            assert old['snapshot']['segments'][0]['text'] == '升级前的固定原文'
            assert old['snapshot']['publishedAt'] is None
            assert api(query)['snapshot']['segments'][0]['text'] == '更新后的原文'
            assert api('?'+urlencode({'search':'更新后的原文'}))['items'][0]['id'] == source['id']
            usage = api('/usage')
            assert usage['searchRequests'] == usage['fetchRequests'] == 0 and usage['tokens'] is None
        finally:
            stop(process)
        process = start()
        try:
            restored = api(query)
            assert restored['favorite'] is True and restored['choice']['state'] == 'excluded'
            assert restored['snapshot']['segments'][0]['text'] == '更新后的原文'
            with closing(sqlite3.connect(database)) as db:
                assert db.execute("SELECT count(*) FROM LearningInteraction WHERE kind IN ('practice','quiz','review')").fetchone()[0] == 0
                assert db.execute("SELECT count(*) FROM MyAIRecord WHERE kind='usage'").fetchone()[0] == 0
        finally:
            stop(process)
print('资料接口冲突、排除、旧快照、离线读取、重启及零学习证据检查通过')
