# 【逐行批注】 执行当前 Python 语句："""仅在临时GitHub runner验证试用版安装升级、虚构档案及产品备份恢复机制。"""。
"""仅在临时GitHub runner验证试用版安装升级、虚构档案及产品备份恢复机制。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import argparse。
import argparse
# 【逐行批注】 导入后续逻辑需要的模块或符号：import ctypes。
import ctypes
# 【逐行批注】 导入后续逻辑需要的模块或符号：from ctypes import wintypes。
from ctypes import wintypes
# 【逐行批注】 导入后续逻辑需要的模块或符号：from contextlib import closing。
from contextlib import closing
# 【逐行批注】 导入后续逻辑需要的模块或符号：import http.client。
import http.client
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
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
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile
# 【逐行批注】 导入后续逻辑需要的模块或符号：import time。
import time
# 【逐行批注】 导入后续逻辑需要的模块或符号：import winreg。
import winreg


# 【逐行批注】 计算右侧结果并赋给 PROFILE，更新脚本状态。
PROFILE = 'GangyiAI-Library-Preview'
# 【逐行批注】 计算右侧结果并赋给 TARGET，更新脚本状态。
TARGET = PROFILE + '/APIKey'
# 【逐行批注】 计算右侧结果并赋给 SECRET，更新脚本状态。
SECRET = 'fictional-ci-upgrade-only'


# 【逐行批注】 定义类 Credential，组织相关数据与行为。
class Credential(ctypes.Structure):
    # 【逐行批注】 计算右侧结果并赋给 _fields_，更新脚本状态。
    _fields_ = [('Flags', wintypes.DWORD), ('Type', wintypes.DWORD), ('TargetName', wintypes.LPWSTR),
                # 【逐行批注】 执行当前 Python 语句：('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME),。
                ('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME),
                # 【逐行批注】 执行当前 Python 语句：('CredentialBlobSize', wintypes.DWORD), ('CredentialBlob', ctypes.POINTER(ctypes.c_ubyte)…。
                ('CredentialBlobSize', wintypes.DWORD), ('CredentialBlob', ctypes.POINTER(ctypes.c_ubyte)),
                # 【逐行批注】 执行当前 Python 语句：('Persist', wintypes.DWORD), ('AttributeCount', wintypes.DWORD), ('Attributes', ctypes.c_…。
                ('Persist', wintypes.DWORD), ('AttributeCount', wintypes.DWORD), ('Attributes', ctypes.c_void_p),
                # 【逐行批注】 执行当前 Python 语句：('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]。
                ('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]


# 【逐行批注】 定义函数 credentials，封装可复用的处理步骤。
def credentials():
    # 【逐行批注】 计算右侧结果并赋给 api，更新脚本状态。
    api = ctypes.WinDLL('Advapi32', use_last_error=True)
    # 【逐行批注】 计算右侧结果并赋给 api.CredReadW.argtypes，更新脚本状态。
    api.CredReadW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.POINTER(ctypes.POINTER(Credential))]
    # 【逐行批注】 计算右侧结果并赋给 api.CredReadW.restype，更新脚本状态。
    api.CredReadW.restype = wintypes.BOOL
    # 【逐行批注】 计算右侧结果并赋给 api.CredWriteW.argtypes，更新脚本状态。
    api.CredWriteW.argtypes = [ctypes.POINTER(Credential), wintypes.DWORD]
    # 【逐行批注】 计算右侧结果并赋给 api.CredWriteW.restype，更新脚本状态。
    api.CredWriteW.restype = wintypes.BOOL
    # 【逐行批注】 计算右侧结果并赋给 api.CredDeleteW.argtypes，更新脚本状态。
    api.CredDeleteW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD]
    # 【逐行批注】 计算右侧结果并赋给 api.CredFree.argtypes，更新脚本状态。
    api.CredFree.argtypes = [ctypes.c_void_p]
    # 【逐行批注】 结束当前函数并返回结果：return api。
    return api


# 【逐行批注】 定义函数 read_fictional_credential，封装可复用的处理步骤。
def read_fictional_credential(api):
    # 【逐行批注】 计算右侧结果并赋给 pointer，更新脚本状态。
    pointer = ctypes.POINTER(Credential)()
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api.CredReadW(TARGET, 1, 0, ctypes.byref(pointer)), '虚构凭据丢失'。
    assert api.CredReadW(TARGET, 1, 0, ctypes.byref(pointer)), '虚构凭据丢失'
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 item，更新脚本状态。
        item = pointer.contents
        # 【逐行批注】 结束当前函数并返回结果：return ctypes.string_at(item.CredentialBlob, item.CredentialBlobSize)…。
        return ctypes.string_at(item.CredentialBlob, item.CredentialBlobSize).decode('utf-16-le')
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：api.CredFree(pointer)。
        api.CredFree(pointer)


# 【逐行批注】 定义函数 run，封装可复用的处理步骤。
def run(*args):
    # 【逐行批注】 计算右侧结果并赋给 subprocess.run([str(arg) for arg in args], c…，更新脚本状态。
    subprocess.run([str(arg) for arg in args], check=True, timeout=180,
                   # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                   creationflags=subprocess.CREATE_NO_WINDOW)


# 【逐行批注】 定义函数 executable_version，封装可复用的处理步骤。
def executable_version(path):
    # 【逐行批注】 计算右侧结果并赋给 command，更新脚本状态。
    command = "(Get-Item -LiteralPath '"+str(path).replace("'", "''")+"').VersionInfo.ProductVersion.Trim()"
    # 【逐行批注】 结束当前函数并返回结果：return subprocess.check_output(['pwsh','-NoProfile','-Command',comman…。
    return subprocess.check_output(['pwsh','-NoProfile','-Command',command], creationflags=subprocess.CREATE_NO_WINDOW).decode().strip()


# 【逐行批注】 定义函数 initialize，封装可复用的处理步骤。
def initialize(service, database):
    # 【逐行批注】 执行当前 Python 语句："""从被安装版本真实启动服务，执行其自身迁移；地址无密钥且仅为本机失效端点。"""。
    """从被安装版本真实启动服务，执行其自身迁移；地址无密钥且仅为本机失效端点。"""
    # 【逐行批注】 在受控上下文中执行资源操作：with socket.socket() as sock:。
    with socket.socket() as sock:
        # 【逐行批注】 执行当前 Python 语句：sock.bind(('127.0.0.1', 0))。
        sock.bind(('127.0.0.1', 0))
        # 【逐行批注】 计算右侧结果并赋给 port，更新脚本状态。
        port = sock.getsockname()[1]
    # 【逐行批注】 计算右侧结果并赋给 token，更新脚本状态。
    token = 'fictional-upgrade-control'
    # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
    env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(database),
               # 【逐行批注】 计算右侧结果并赋给 AI_BASE_URL，更新脚本状态。
               AI_BASE_URL='http://127.0.0.1:9/v1', AI_API_KEY='', AI_MODEL='fictional-upgrade',
               # 【逐行批注】 计算右侧结果并赋给 BOCHA_API_KEY，更新脚本状态。
               BOCHA_API_KEY='', LOCAL_CONTROL_TOKEN=token)
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryFile() as log:。
    with tempfile.TemporaryFile() as log:
        # 【逐行批注】 计算右侧结果并赋给 process，更新脚本状态。
        process = subprocess.Popen([str(service)], cwd=service.parent, env=env, stdout=log, stderr=log,
                                   # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                                   creationflags=subprocess.CREATE_NO_WINDOW)
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(150):。
            for _ in range(150):
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
                    connection = http.client.HTTPConnection('127.0.0.1', port, timeout=1)
                    # 【逐行批注】 执行当前 Python 语句：connection.request('GET', '/health')。
                    connection.request('GET', '/health')
                    # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
                    response = connection.getresponse()
                    # 【逐行批注】 计算右侧结果并赋给 payload，更新脚本状态。
                    payload = response.read()
                    # 【逐行批注】 执行当前 Python 语句：connection.close()。
                    connection.close()
                    # 【逐行批注】 判断条件 response.status == 200 and json.loads(payload)['status'] == 'hea…，成立时执行对应分支。
                    if response.status == 200 and json.loads(payload)['status'] == 'healthy':
                        # 【逐行批注】 执行当前 Python 语句：break。
                        break
                # 【逐行批注】 捕获并处理异常：except (OSError, ValueError):。
                except (OSError, ValueError):
                    # 【逐行批注】 执行当前 Python 语句：pass。
                    pass
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert process.poll() is None, '隔离服务提前退出'。
                assert process.poll() is None, '隔离服务提前退出'
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                time.sleep(.1)
            # 【逐行批注】 执行前一条件不成立时的分支。
            else:
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('隔离服务未就绪')。
                raise AssertionError('隔离服务未就绪')
            # 【逐行批注】 计算右侧结果并赋给 connection，更新脚本状态。
            connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
            # 【逐行批注】 计算右侧结果并赋给 connection.request('POST', '/internal/shutdo…，更新脚本状态。
            connection.request('POST', '/internal/shutdown', headers={'X-Gangyi-Control-Token': token})
            # 【逐行批注】 计算右侧结果并赋给 response，更新脚本状态。
            response = connection.getresponse()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert response.status == 200。
            assert response.status == 200
            # 【逐行批注】 执行当前 Python 语句：response.read()。
            response.read()
            # 【逐行批注】 执行当前 Python 语句：connection.close()。
            connection.close()
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert process.wait(timeout=10) == 0。
            assert process.wait(timeout=10) == 0
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 判断条件 process.poll() is None:，成立时执行对应分支。
            if process.poll() is None:
                # 【逐行批注】 执行当前 Python 语句：process.kill()。
                process.kill()
                # 【逐行批注】 计算右侧结果并赋给 process.wait(timeout，更新脚本状态。
                process.wait(timeout=10)


# 【逐行批注】 定义函数 records，封装可复用的处理步骤。
def records(database):
    # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db:。
    with closing(sqlite3.connect(database)) as db:
        # 【逐行批注】 校验关键条件，测试失败时立即报错：assert db.execute('PRAGMA integrity_check').fetchone()[0] == 'ok'。
        assert db.execute('PRAGMA integrity_check').fetchone()[0] == 'ok'
        # 【逐行批注】 结束当前函数并返回结果：return (db.execute("SELECT * FROM Course WHERE id='ci-upgrade-course'…。
        return (db.execute("SELECT * FROM Course WHERE id='ci-upgrade-course'").fetchall(),
                # 【逐行批注】 执行当前 Python 语句：db.execute("SELECT id,kind,payload,version FROM MyAIRecord WHERE id LIKE 'ci-upgrade-%' O…。
                db.execute("SELECT id,kind,payload,version FROM MyAIRecord WHERE id LIKE 'ci-upgrade-%' ORDER BY id").fetchall())


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 判断条件 os.environ.get('GITHUB_ACTIONS') != 'true':，成立时执行对应分支。
    if os.environ.get('GITHUB_ACTIONS') != 'true':
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('安装升级只允许临时GitHub runner')。
        raise RuntimeError('安装升级只允许临时GitHub runner')
    # 【逐行批注】 计算右侧结果并赋给 parser，更新脚本状态。
    parser = argparse.ArgumentParser()
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--baseline', required，更新脚本状态。
    parser.add_argument('--baseline', required=True)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--candidate', required，更新脚本状态。
    parser.add_argument('--candidate', required=True)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--backup-tool', required，更新脚本状态。
    parser.add_argument('--backup-tool', required=True)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument('--version', default，更新脚本状态。
    parser.add_argument('--version', default='5.6.4')
    # 【逐行批注】 计算右侧结果并赋给 args，更新脚本状态。
    args = parser.parse_args()
    # 【逐行批注】 计算右侧结果并赋给 api，更新脚本状态。
    api = credentials()
    # 【逐行批注】 计算右侧结果并赋给 registry，更新脚本状态。
    registry = 'Software\\' + PROFILE
    # 【逐行批注】 计算右侧结果并赋给 data_root，更新脚本状态。
    data_root = Path(os.environ['LOCALAPPDATA'])/PROFILE
    # 【逐行批注】 计算右侧结果并赋给 database，更新脚本状态。
    database = data_root/'data/gangyiAI.db'
    # 【逐行批注】 判断条件 database.exists():，成立时执行对应分支。
    if database.exists():
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('拒绝覆盖已有档案')。
        raise RuntimeError('拒绝覆盖已有档案')
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 在受控上下文中执行资源操作：with winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry):。
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry):
            # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('拒绝覆盖已有档案设置')。
            raise RuntimeError('拒绝覆盖已有档案设置')
    # 【逐行批注】 捕获并处理异常：except FileNotFoundError:。
    except FileNotFoundError:
        # 【逐行批注】 执行当前 Python 语句：pass。
        pass
    # 【逐行批注】 计算右侧结果并赋给 existing，更新脚本状态。
    existing = ctypes.POINTER(Credential)()
    # 【逐行批注】 判断条件 api.CredReadW(TARGET, 1, 0, ctypes.byref(existing)):，成立时执行对应分支。
    if api.CredReadW(TARGET, 1, 0, ctypes.byref(existing)):
        # 【逐行批注】 执行当前 Python 语句：api.CredFree(existing)。
        api.CredFree(existing)
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('拒绝覆盖已有凭据')。
        raise RuntimeError('拒绝覆盖已有凭据')
    # 【逐行批注】 计算右侧结果并赋给 database.parent.mkdir(parents，更新脚本状态。
    database.parent.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 计算右侧结果并赋给 installed，更新脚本状态。
    installed = False
    # 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-ci-upgrade-') as temporary:。
    with tempfile.TemporaryDirectory(prefix='gangyi-ci-upgrade-') as temporary:
        # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
        root = Path(temporary)
        # 【逐行批注】 计算右侧结果并赋给 install，更新脚本状态。
        install = root/'installed'
        # 【逐行批注】 计算右侧结果并赋给 setup_args，更新脚本状态。
        setup_args = ['/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/SP-', '/NOICONS', '/DIR='+str(install)]
        # 【逐行批注】 开始可能抛出异常的保护代码块。
        try:
            # 【逐行批注】 调用 run、Path 完成当前脚本或测试步骤：run(Path(args.baseline).resolve(), *setup_args)。
            run(Path(args.baseline).resolve(), *setup_args)
            # 【逐行批注】 计算右侧结果并赋给 installed，更新脚本状态。
            installed = True
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert executable_version(install/'gangyiAI.exe') == '5.6.3'。
            assert executable_version(install/'gangyiAI.exe') == '5.6.3'
            # 【逐行批注】 调用 initialize 完成当前脚本或测试步骤：initialize(install/'gangyiAI.exe', database)。
            initialize(install/'gangyiAI.exe', database)
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db, db:。
            with closing(sqlite3.connect(database)) as db, db:
                # 【逐行批注】 调用 Course、VALUES 完成当前脚本或测试步骤：db.execute("INSERT INTO Course(id,goal,mode,title,createdAt,updatedAt…。
                db.execute("INSERT INTO Course(id,goal,mode,title,createdAt,updatedAt) VALUES('ci-upgrade-course','公开合成目标','deep','升级课程','2026-10-19','2026-10-19')")
                # 【逐行批注】 遍历数据并重复执行循环体：for record_id, kind, payload in [('source','library-source',{'snapshotId':…。
                for record_id, kind, payload in [('source','library-source',{'snapshotId':'ci-upgrade-snapshot'}),
                        # 【逐行批注】 执行当前 Python 语句：('snapshot','library-snapshot',{'segments':[{'id':'p1-1','page':1,'text':'公开合成原文'}]}),。
                        ('snapshot','library-snapshot',{'segments':[{'id':'p1-1','page':1,'text':'公开合成原文'}]}),
                        # 【逐行批注】 执行当前 Python 语句：('choice','library-choice',{'manual':True,'state':'excluded'})]:。
                        ('choice','library-choice',{'manual':True,'state':'excluded'})]:
                    # 【逐行批注】 计算右侧结果并赋给 db.execute('INSERT INTO MyAIRecord VALUES(?,…，更新脚本状态。
                    db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)', ('ci-upgrade-'+record_id,kind,json.dumps(payload,ensure_ascii=False),'2026-10-19',3))
            # 【逐行批注】 计算右侧结果并赋给 expected，更新脚本状态。
            expected = records(database)
            # 【逐行批注】 在受控上下文中执行资源操作：with winreg.CreateKey(winreg.HKEY_CURRENT_USER, registry) as key:。
            with winreg.CreateKey(winreg.HKEY_CURRENT_USER, registry) as key:
                # 【逐行批注】 执行当前 Python 语句：winreg.SetValueEx(key, 'AIModel', 0, winreg.REG_SZ, 'fictional-upgrade-model')。
                winreg.SetValueEx(key, 'AIModel', 0, winreg.REG_SZ, 'fictional-upgrade-model')
                # 【逐行批注】 执行当前 Python 语句：winreg.SetValueEx(key, 'AIBaseUrl', 0, winreg.REG_SZ, 'http://127.0.0.1:9/v1')。
                winreg.SetValueEx(key, 'AIBaseUrl', 0, winreg.REG_SZ, 'http://127.0.0.1:9/v1')
            # 【逐行批注】 计算右侧结果并赋给 encoded，更新脚本状态。
            encoded = SECRET.encode('utf-16-le')
            # 【逐行批注】 计算右侧结果并赋给 blob，更新脚本状态。
            blob = (ctypes.c_ubyte*len(encoded)).from_buffer_copy(encoded)
            # 【逐行批注】 计算右侧结果并赋给 credential，更新脚本状态。
            credential = Credential(Type=1, TargetName=TARGET, CredentialBlobSize=len(encoded), CredentialBlob=blob,
                                    # 【逐行批注】 计算右侧结果并赋给 Persist，更新脚本状态。
                                    Persist=2, UserName='fictional-ci')
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert api.CredWriteW(ctypes.byref(credential), 0)。
            assert api.CredWriteW(ctypes.byref(credential), 0)
            # 【逐行批注】 计算右侧结果并赋给 backup，更新脚本状态。
            backup = root/'before-upgrade.db'
            # 【逐行批注】 调用 run 完成当前脚本或测试步骤：run(args.backup_tool, '--backup', database, backup)。
            run(args.backup_tool, '--backup', database, backup)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert records(backup) == expected。
            assert records(backup) == expected
            # 【逐行批注】 调用 run、Path 完成当前脚本或测试步骤：run(Path(args.candidate).resolve(), *setup_args)。
            run(Path(args.candidate).resolve(), *setup_args)
            # 【逐行批注】 遍历数据并重复执行循环体：for name in ('gangyiAI.exe', 'gangyiAI-launcher.exe'):。
            for name in ('gangyiAI.exe', 'gangyiAI-launcher.exe'):
                # 【逐行批注】 计算右侧结果并赋给 version，更新脚本状态。
                version = executable_version(install/name)
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert version == args.version, (name, version)。
                assert version == args.version, (name, version)
            # 【逐行批注】 调用 run 完成当前脚本或测试步骤：run(install/'gangyiAI-launcher.exe', '--self-test')。
            run(install/'gangyiAI-launcher.exe', '--self-test')
            # 【逐行批注】 调用 initialize 完成当前脚本或测试步骤：initialize(install/'gangyiAI.exe', database)。
            initialize(install/'gangyiAI.exe', database)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert records(database) == expected, '升级丢失课程、原文或手动选择'。
            assert records(database) == expected, '升级丢失课程、原文或手动选择'
            # 【逐行批注】 在受控上下文中执行资源操作：with winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry) as key:。
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry) as key:
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert winreg.QueryValueEx(key, 'AIModel')[0] == 'fictional-upgrade-model'。
                assert winreg.QueryValueEx(key, 'AIModel')[0] == 'fictional-upgrade-model'
                # 【逐行批注】 校验关键条件，测试失败时立即报错：assert winreg.QueryValueEx(key, 'AIBaseUrl')[0] == 'http://127.0.0.1:9/v1'。
                assert winreg.QueryValueEx(key, 'AIBaseUrl')[0] == 'http://127.0.0.1:9/v1'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert read_fictional_credential(api) == SECRET。
            assert read_fictional_credential(api) == SECRET
            # 【逐行批注】 在受控上下文中执行资源操作：with closing(sqlite3.connect(database)) as db, db:。
            with closing(sqlite3.connect(database)) as db, db:
                # 【逐行批注】 计算右侧结果并赋给 db.execute("UPDATE Course SET title，更新脚本状态。
                db.execute("UPDATE Course SET title='恢复前修改' WHERE id='ci-upgrade-course'")
            # 【逐行批注】 计算右侧结果并赋给 changed，更新脚本状态。
            changed = records(database)
            # 【逐行批注】 计算右侧结果并赋给 rollback，更新脚本状态。
            rollback = root/'rollback.db'
            # 【逐行批注】 调用 run 完成当前脚本或测试步骤：run(args.backup_tool, '--restore', backup, database, rollback)。
            run(args.backup_tool, '--restore', backup, database, rollback)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert records(database) == expected and records(rollback) == changed。
            assert records(database) == expected and records(rollback) == changed
            # 【逐行批注】 调用 run 完成当前脚本或测试步骤：run(install/'unins000.exe', '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NOR…。
            run(install/'unins000.exe', '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART')
            # 【逐行批注】 计算右侧结果并赋给 installed，更新脚本状态。
            installed = False
            # 【逐行批注】 遍历数据并重复执行循环体：for _ in range(100):。
            for _ in range(100):
                # 【逐行批注】 判断条件 not install.exists():，成立时执行对应分支。
                if not install.exists():
                    # 【逐行批注】 执行当前 Python 语句：break。
                    break
                # 【逐行批注】 执行当前 Python 语句：time.sleep(.1)。
                time.sleep(.1)
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert not install.exists(), '卸载目录未清除'。
            assert not install.exists(), '卸载目录未清除'
            # 【逐行批注】 校验关键条件，测试失败时立即报错：assert records(database) == expected, '卸载不得删除课程数据库'。
            assert records(database) == expected, '卸载不得删除课程数据库'
            # 【逐行批注】 计算右侧结果并赋给 pointer，更新脚本状态。
            pointer = ctypes.POINTER(Credential)()
            # 【逐行批注】 判断条件 api.CredReadW(TARGET, 1, 0, ctypes.byref(pointer)):，成立时执行对应分支。
            if api.CredReadW(TARGET, 1, 0, ctypes.byref(pointer)):
                # 【逐行批注】 执行当前 Python 语句：api.CredFree(pointer)。
                api.CredFree(pointer)
                # 【逐行批注】 在错误情形抛出异常：raise AssertionError('卸载未清除虚构凭据')。
                raise AssertionError('卸载未清除虚构凭据')
            # 【逐行批注】 输出当前运行结果：print(json.dumps({'passed':True,'baseline':'5.6.3','candidate':args.versio…。
            print(json.dumps({'passed':True,'baseline':'5.6.3','candidate':args.version,'profile':PROFILE,
                              # 【逐行批注】 执行当前 Python 语句：'upgradePreservesDataSettingsCredentials':True,'productBackupRestoreAndRollback':True,。
                              'upgradePreservesDataSettingsCredentials':True,'productBackupRestoreAndRollback':True,
                              # 【逐行批注】 计算右侧结果并赋给 'uninstallPreservesDatabase':True,'paidModel…，更新脚本状态。
                              'uninstallPreservesDatabase':True,'paidModelRequests':0}, ensure_ascii=False))
        # 【逐行批注】 执行当前 Python 语句：finally:。
        finally:
            # 【逐行批注】 判断条件 installed and (install/'unins000.exe').exists():，成立时执行对应分支。
            if installed and (install/'unins000.exe').exists():
                # 【逐行批注】 调用 run 完成当前脚本或测试步骤：run(install/'unins000.exe', '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NOR…。
                run(install/'unins000.exe', '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART')
            # 【逐行批注】 执行当前 Python 语句：api.CredDeleteW(TARGET, 1, 0)。
            api.CredDeleteW(TARGET, 1, 0)


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()
