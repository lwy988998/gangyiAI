# 【逐行批注】 执行当前 Python 语句："""在独立数据库核验真实资料任务，密钥只放入验收子进程环境。"""。
"""在独立数据库核验真实资料任务，密钥只放入验收子进程环境。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import ctypes。
import ctypes
# 【逐行批注】 导入后续逻辑需要的模块或符号：from ctypes import wintypes。
from ctypes import wintypes
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import os。
import os
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：from teaching_eval_provider import configured_environment。
from teaching_eval_provider import configured_environment


# 【逐行批注】 定义类 Credential，组织相关数据与行为。
class Credential(ctypes.Structure):
    # 【逐行批注】 计算右侧结果并赋给 _fields_，更新脚本状态。
    _fields_ = [('Flags', wintypes.DWORD), ('Type', wintypes.DWORD), ('TargetName', wintypes.LPWSTR),
                # 【逐行批注】 执行当前 Python 语句：('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME), ('BlobSize', wintypes.D…。
                ('Comment', wintypes.LPWSTR), ('LastWritten', wintypes.FILETIME), ('BlobSize', wintypes.DWORD),
                # 【逐行批注】 执行当前 Python 语句：('Blob', ctypes.POINTER(ctypes.c_ubyte)), ('Persist', wintypes.DWORD), ('Count', wintypes…。
                ('Blob', ctypes.POINTER(ctypes.c_ubyte)), ('Persist', wintypes.DWORD), ('Count', wintypes.DWORD),
                # 【逐行批注】 执行当前 Python 语句：('Attributes', ctypes.c_void_p), ('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.…。
                ('Attributes', ctypes.c_void_p), ('TargetAlias', wintypes.LPWSTR), ('UserName', wintypes.LPWSTR)]


# 【逐行批注】 判断条件 __name__ == '__main__':，成立时执行对应分支。
if __name__ == '__main__':
    # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
    root = Path(__file__).resolve().parents[1]
    # 【逐行批注】 计算右侧结果并赋给 folder，更新脚本状态。
    folder = (root / 'verification' / sys.argv[1]).resolve()
    # 【逐行批注】 判断条件 not folder.is_relative_to(root / 'verification'):，成立时执行对应分支。
    if not folder.is_relative_to(root / 'verification'):
        # 【逐行批注】 在错误情形抛出异常：raise ValueError('验收目录必须在本项目 verification 内')。
        raise ValueError('验收目录必须在本项目 verification 内')
    # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
    env = configured_environment()
    # 【逐行批注】 计算右侧结果并赋给 api，更新脚本状态。
    api = ctypes.WinDLL('advapi32', use_last_error=True)
    # 【逐行批注】 计算右侧结果并赋给 api.CredReadW.argtypes，更新脚本状态。
    api.CredReadW.argtypes = [wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.POINTER(ctypes.POINTER(Credential))]
    # 【逐行批注】 计算右侧结果并赋给 api.CredFree.argtypes，更新脚本状态。
    api.CredFree.argtypes = [ctypes.c_void_p]
    # 【逐行批注】 计算右侧结果并赋给 pointer，更新脚本状态。
    pointer = ctypes.POINTER(Credential)()
    # 【逐行批注】 判断条件 not api.CredReadW('GangyiAI/BochaKey', 1, 0, ctypes.byref(pointe…，成立时执行对应分支。
    if not api.CredReadW('GangyiAI/BochaKey', 1, 0, ctypes.byref(pointer)):
        # 【逐行批注】 在错误情形抛出异常：raise RuntimeError('未取得现有博查凭据，未调用服务')。
        raise RuntimeError('未取得现有博查凭据，未调用服务')
    # 【逐行批注】 开始可能抛出异常的保护代码块。
    try:
        # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
        value = pointer.contents
        # 【逐行批注】 计算右侧结果并赋给 env['BOCHA_API_KEY']，更新脚本状态。
        env['BOCHA_API_KEY'] = ctypes.string_at(value.Blob, value.BlobSize).decode('utf-16-le')
    # 【逐行批注】 执行当前 Python 语句：finally:。
    finally:
        # 【逐行批注】 执行当前 Python 语句：api.CredFree(pointer)。
        api.CredFree(pointer)
    # 【逐行批注】 计算右侧结果并赋给 env['RESOURCE_SEARCH_CACHE_DIR']，更新脚本状态。
    env['RESOURCE_SEARCH_CACHE_DIR'] = str(folder / 'cache')
    # 【逐行批注】 计算右侧结果并赋给 build，更新脚本状态。
    build = root / ('build-library-release' if len(sys.argv) > 2 and sys.argv[2] == 'release' else 'build-library')
    # 【逐行批注】 计算右侧结果并赋给 env['CURL_CA_BUNDLE']，更新脚本状态。
    env['CURL_CA_BUNDLE'] = str(build / 'curl-ca-bundle.crt')
    # 【逐行批注】 在受控上下文中执行资源操作：with (folder / 'run.log').open('w', encoding='utf-8') as log:。
    with (folder / 'run.log').open('w', encoding='utf-8') as log:
        # 【逐行批注】 计算右侧结果并赋给 result，更新脚本状态。
        result = subprocess.run([str(build / 'library-probe.exe'), str(folder / 'case.json'),
                                 # 【逐行批注】 调用 str 完成当前脚本或测试步骤：str(folder / 'isolation.db'), str(folder / 'result.json')],。
                                 str(folder / 'isolation.db'), str(folder / 'result.json')],
                                # 【逐行批注】 计算右侧结果并赋给 env，更新脚本状态。
                                env=env, cwd=build, stdout=log, stderr=log, timeout=900,
                                # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                                creationflags=subprocess.CREATE_NO_WINDOW)
    # 【逐行批注】 输出当前运行结果：print('资料验收入口退出码：', result.returncode)。
    print('资料验收入口退出码：', result.returncode)
    # 【逐行批注】 执行当前 Python 语句：sys.exit(result.returncode)。
    sys.exit(result.returncode)
