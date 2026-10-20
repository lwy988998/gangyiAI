"""仅在临时GitHub runner验证试用版安装升级、虚构档案及产品备份恢复机制。"""
import argparse
import ctypes
from ctypes import wintypes
from contextlib import closing
import http.client
import json
import os
from pathlib import Path
import socket
import sqlite3
import subprocess
import tempfile
import time
import winreg


PROFILE = 'GangyiAI-Library-Preview'
TARGET = PROFILE + '/APIKey'
SECRET = 'fictional-ci-upgrade-only'


class Credential(ctypes.Structure):
    _fields_ = [('Flags', wintypes.DWORD), ('Type', wintypes.DWORD), ('TargetName', wintypes.LPWSTR),
                ('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME),
                ('CredentialBlobSize', wintypes.DWORD), ('CredentialBlob', ctypes.POINTER(ctypes.c_ubyte)),
                ('Persist', wintypes.DWORD), ('AttributeCount', wintypes.DWORD), ('Attributes', ctypes.c_void_p),
                ('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]


def credentials():
    api = ctypes.WinDLL('Advapi32', use_last_error=True)
    api.CredReadW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.POINTER(ctypes.POINTER(Credential))]
    api.CredReadW.restype = wintypes.BOOL
    api.CredWriteW.argtypes = [ctypes.POINTER(Credential), wintypes.DWORD]
    api.CredWriteW.restype = wintypes.BOOL
    api.CredDeleteW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD]
    api.CredFree.argtypes = [ctypes.c_void_p]
    return api


def read_fictional_credential(api):
    pointer = ctypes.POINTER(Credential)()
    assert api.CredReadW(TARGET, 1, 0, ctypes.byref(pointer)), '虚构凭据丢失'
    try:
        item = pointer.contents
        return ctypes.string_at(item.CredentialBlob, item.CredentialBlobSize).decode('utf-16-le')
    finally:
        api.CredFree(pointer)


def run(*args):
    subprocess.run([str(arg) for arg in args], check=True, timeout=180,
                   creationflags=subprocess.CREATE_NO_WINDOW)


def executable_version(path):
    command = "(Get-Item -LiteralPath '"+str(path).replace("'", "''")+"').VersionInfo.ProductVersion.Trim()"
    return subprocess.check_output(['pwsh','-NoProfile','-Command',command], creationflags=subprocess.CREATE_NO_WINDOW).decode().strip()


def initialize(service, database):
    """从被安装版本真实启动服务，执行其自身迁移；地址无密钥且仅为本机失效端点。"""
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        port = sock.getsockname()[1]
    token = 'fictional-upgrade-control'
    env = dict(os.environ, HOST='127.0.0.1', PORT=str(port), DATABASE_PATH=str(database),
               AI_BASE_URL='http://127.0.0.1:9/v1', AI_API_KEY='', AI_MODEL='fictional-upgrade',
               BOCHA_API_KEY='', LOCAL_CONTROL_TOKEN=token)
    with tempfile.TemporaryFile() as log:
        process = subprocess.Popen([str(service)], cwd=service.parent, env=env, stdout=log, stderr=log,
                                   creationflags=subprocess.CREATE_NO_WINDOW)
        try:
            for _ in range(150):
                try:
                    connection = http.client.HTTPConnection('127.0.0.1', port, timeout=1)
                    connection.request('GET', '/health')
                    response = connection.getresponse()
                    payload = response.read()
                    connection.close()
                    if response.status == 200 and json.loads(payload)['status'] == 'healthy':
                        break
                except (OSError, ValueError):
                    pass
                assert process.poll() is None, '隔离服务提前退出'
                time.sleep(.1)
            else:
                raise AssertionError('隔离服务未就绪')
            connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
            connection.request('POST', '/internal/shutdown', headers={'X-Gangyi-Control-Token': token})
            response = connection.getresponse()
            assert response.status == 200
            response.read()
            connection.close()
            assert process.wait(timeout=10) == 0
        finally:
            if process.poll() is None:
                process.kill()
                process.wait(timeout=10)


def records(database):
    with closing(sqlite3.connect(database)) as db:
        assert db.execute('PRAGMA integrity_check').fetchone()[0] == 'ok'
        return (db.execute("SELECT * FROM Course WHERE id='ci-upgrade-course'").fetchall(),
                db.execute("SELECT id,kind,payload,version FROM MyAIRecord WHERE id LIKE 'ci-upgrade-%' ORDER BY id").fetchall())


def main():
    if os.environ.get('GITHUB_ACTIONS') != 'true':
        raise RuntimeError('安装升级只允许临时GitHub runner')
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', required=True)
    parser.add_argument('--candidate', required=True)
    parser.add_argument('--backup-tool', required=True)
    parser.add_argument('--version', default='5.6.4')
    args = parser.parse_args()
    api = credentials()
    registry = 'Software\\' + PROFILE
    data_root = Path(os.environ['LOCALAPPDATA'])/PROFILE
    database = data_root/'data/gangyiAI.db'
    if database.exists():
        raise RuntimeError('拒绝覆盖已有档案')
    try:
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry):
            raise RuntimeError('拒绝覆盖已有档案设置')
    except FileNotFoundError:
        pass
    existing = ctypes.POINTER(Credential)()
    if api.CredReadW(TARGET, 1, 0, ctypes.byref(existing)):
        api.CredFree(existing)
        raise RuntimeError('拒绝覆盖已有凭据')
    database.parent.mkdir(parents=True, exist_ok=True)
    installed = False
    with tempfile.TemporaryDirectory(prefix='gangyi-ci-upgrade-') as temporary:
        root = Path(temporary)
        install = root/'installed'
        setup_args = ['/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/SP-', '/NOICONS', '/DIR='+str(install)]
        try:
            run(Path(args.baseline).resolve(), *setup_args)
            installed = True
            assert executable_version(install/'gangyiAI.exe') == '5.6.3'
            initialize(install/'gangyiAI.exe', database)
            with closing(sqlite3.connect(database)) as db, db:
                db.execute("INSERT INTO Course(id,goal,mode,title,createdAt,updatedAt) VALUES('ci-upgrade-course','公开合成目标','deep','升级课程','2026-10-19','2026-10-19')")
                for record_id, kind, payload in [('source','library-source',{'snapshotId':'ci-upgrade-snapshot'}),
                        ('snapshot','library-snapshot',{'segments':[{'id':'p1-1','page':1,'text':'公开合成原文'}]}),
                        ('choice','library-choice',{'manual':True,'state':'excluded'})]:
                    db.execute('INSERT INTO MyAIRecord VALUES(?,?,?,?,?)', ('ci-upgrade-'+record_id,kind,json.dumps(payload,ensure_ascii=False),'2026-10-19',3))
            expected = records(database)
            with winreg.CreateKey(winreg.HKEY_CURRENT_USER, registry) as key:
                winreg.SetValueEx(key, 'AIModel', 0, winreg.REG_SZ, 'fictional-upgrade-model')
                winreg.SetValueEx(key, 'AIBaseUrl', 0, winreg.REG_SZ, 'http://127.0.0.1:9/v1')
            encoded = SECRET.encode('utf-16-le')
            blob = (ctypes.c_ubyte*len(encoded)).from_buffer_copy(encoded)
            credential = Credential(Type=1, TargetName=TARGET, CredentialBlobSize=len(encoded), CredentialBlob=blob,
                                    Persist=2, UserName='fictional-ci')
            assert api.CredWriteW(ctypes.byref(credential), 0)
            backup = root/'before-upgrade.db'
            run(args.backup_tool, '--backup', database, backup)
            assert records(backup) == expected
            run(Path(args.candidate).resolve(), *setup_args)
            for name in ('gangyiAI.exe', 'gangyiAI-launcher.exe'):
                version = executable_version(install/name)
                assert version == args.version, (name, version)
            run(install/'gangyiAI-launcher.exe', '--self-test')
            initialize(install/'gangyiAI.exe', database)
            assert records(database) == expected, '升级丢失课程、原文或手动选择'
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, registry) as key:
                assert winreg.QueryValueEx(key, 'AIModel')[0] == 'fictional-upgrade-model'
                assert winreg.QueryValueEx(key, 'AIBaseUrl')[0] == 'http://127.0.0.1:9/v1'
            assert read_fictional_credential(api) == SECRET
            with closing(sqlite3.connect(database)) as db, db:
                db.execute("UPDATE Course SET title='恢复前修改' WHERE id='ci-upgrade-course'")
            changed = records(database)
            rollback = root/'rollback.db'
            run(args.backup_tool, '--restore', backup, database, rollback)
            assert records(database) == expected and records(rollback) == changed
            run(install/'unins000.exe', '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART')
            installed = False
            for _ in range(100):
                if not install.exists():
                    break
                time.sleep(.1)
            assert not install.exists(), '卸载目录未清除'
            assert records(database) == expected, '卸载不得删除课程数据库'
            pointer = ctypes.POINTER(Credential)()
            if api.CredReadW(TARGET, 1, 0, ctypes.byref(pointer)):
                api.CredFree(pointer)
                raise AssertionError('卸载未清除虚构凭据')
            print(json.dumps({'passed':True,'baseline':'5.6.3','candidate':args.version,'profile':PROFILE,
                              'upgradePreservesDataSettingsCredentials':True,'productBackupRestoreAndRollback':True,
                              'uninstallPreservesDatabase':True,'paidModelRequests':0}, ensure_ascii=False))
        finally:
            if installed and (install/'unins000.exe').exists():
                run(install/'unins000.exe', '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART')
            api.CredDeleteW(TARGET, 1, 0)


if __name__ == '__main__':
    main()
