"""在独立数据库核验真实资料任务，密钥只放入验收子进程环境。"""
import ctypes
from ctypes import wintypes
from pathlib import Path
import os
import subprocess
import sys
from teaching_eval_provider import configured_environment


class Credential(ctypes.Structure):
    _fields_ = [('Flags', wintypes.DWORD), ('Type', wintypes.DWORD), ('TargetName', wintypes.LPWSTR),
                ('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME), ('BlobSize', wintypes.DWORD),
                ('Blob', ctypes.POINTER(ctypes.c_ubyte)), ('Persist', wintypes.DWORD), ('Count', wintypes.DWORD),
                ('Attributes', ctypes.c_void_p), ('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]


if __name__ == '__main__':
    root = Path(__file__).resolve().parents[1]
    folder = (root / 'verification' / sys.argv[1]).resolve()
    if not folder.is_relative_to(root / 'verification'):
        raise ValueError('验收目录必须在本项目 verification 内')
    env = configured_environment()
    api = ctypes.WinDLL('advapi32', use_last_error=True)
    api.CredReadW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.POINTER(ctypes.POINTER(Credential))]
    api.CredFree.argtypes = [ctypes.c_void_p]
    pointer = ctypes.POINTER(Credential)()
    if not api.CredReadW('GangyiAI/BochaKey', 1, 0, ctypes.byref(pointer)):
        raise RuntimeError('未取得现有博查凭据，未调用服务')
    try:
        value = pointer.contents
        env['BOCHA_API_KEY'] = ctypes.string_at(value.Blob, value.BlobSize).decode('utf-16-le')
    finally:
        api.CredFree(pointer)
    env['RESOURCE_SEARCH_CACHE_DIR'] = str(folder / 'cache')
    build = root / ('build-library-release' if len(sys.argv) > 2 and sys.argv[2] == 'release' else 'build-library')
    env['CURL_CA_BUNDLE'] = str(build / 'curl-ca-bundle.crt')
    with (folder / 'run.log').open('w', encoding='utf-8') as log:
        result = subprocess.run([str(build / 'library-probe.exe'), str(folder / 'case.json'),
                                 str(folder / 'isolation.db'), str(folder / 'result.json')],
                                env=env, cwd=build, stdout=log, stderr=log, timeout=900,
                                creationflags=subprocess.CREATE_NO_WINDOW)
    print('资料验收入口退出码：', result.returncode)
    sys.exit(result.returncode)
