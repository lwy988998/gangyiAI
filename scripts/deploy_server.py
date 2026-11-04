# 【逐行批注】 导入后续逻辑需要的模块或符号：import os, sys, paramiko。
import os, sys, paramiko
# 【逐行批注】 计算右侧结果并赋给 sys.stdout.reconfigure(encoding，更新脚本状态。
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
# 【逐行批注】 计算右侧结果并赋给 client，更新脚本状态。
client = paramiko.SSHClient()
# 【逐行批注】 执行当前 Python 语句：client.set_missing_host_key_policy(paramiko.AutoAddPolicy())。
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
# 【逐行批注】 计算右侧结果并赋给 client.connect(os.environ['SRV_HOST'], port，更新脚本状态。
client.connect(os.environ['SRV_HOST'], port=int(os.environ['SRV_PORT']), username=os.environ['SRV_USER'], password=os.environ['SRV_PASS'], timeout=20)
# 【逐行批注】 定义函数 run，封装可复用的处理步骤。
def run(cmd, timeout=1200):
    # 【逐行批注】 计算右侧结果并赋给 stdin, stdout, stderr，更新脚本状态。
    stdin, stdout, stderr = client.exec_command(cmd, timeout=timeout, get_pty=True)
    # 【逐行批注】 计算右侧结果并赋给 out，更新脚本状态。
    out = stdout.read().decode('utf-8', 'replace')
    # 【逐行批注】 计算右侧结果并赋给 code，更新脚本状态。
    code = stdout.channel.recv_exit_status()
    # 【逐行批注】 结束当前函数并返回结果：return out, code。
    return out, code
# 【逐行批注】 开始可能抛出异常的保护代码块。
try:
    # 【逐行批注】 计算右侧结果并赋给 sftp，更新脚本状态。
    sftp = client.open_sftp()
    # 【逐行批注】 执行当前 Python 语句：sftp.put(os.path.join(os.environ['TEMP'], 'gangyiAI-deploy.tar.gz'), '/tmp/gangyiAI-deplo…。
    sftp.put(os.path.join(os.environ['TEMP'], 'gangyiAI-deploy.tar.gz'), '/tmp/gangyiAI-deploy.tar.gz')
    # 【逐行批注】 执行当前 Python 语句：sftp.close()。
    sftp.close()
    # 【逐行批注】 输出当前运行结果：print("[uploaded]", flush=True)。
    print("[uploaded]", flush=True)
    # 【逐行批注】 计算右侧结果并赋给 out, code，更新脚本状态。
    out, code = run("cd /www/wwwroot/gangyiAI && rm -rf * .[!.]* 2>/dev/null; tar xzf /tmp/gangyiAI-deploy.tar.gz && echo extract-ok")
    # 【逐行批注】 输出当前运行结果：print(out.strip())。
    print(out.strip())
    # 服务器编译（asio 用系统库）
    # 【逐行批注】 计算右侧结果并赋给 out, code，更新脚本状态。
    out, code = run("cd /www/wwwroot/gangyiAI && rm -rf build && cmake -B build > /dev/null 2>&1 && cmake --build build -j4 2>&1 | grep -E 'error|Built target gangyiAI' | head -8", timeout=1200)
    # 【逐行批注】 输出当前运行结果：print(out)。
    print(out)
    # 【逐行批注】 输出当前运行结果：print(f"build exit: {code}")。
    print(f"build exit: {code}")
    # 【逐行批注】 判断条件 code == 0:，成立时执行对应分支。
    if code == 0:
        # 【逐行批注】 计算右侧结果并赋给 out, _，更新脚本状态。
        out, _ = run("ls -la /www/wwwroot/gangyiAI/build/gangyiAI | awk '{print $5}' && echo BUILD_OK")
        # 【逐行批注】 输出当前运行结果：print(out.strip())。
        print(out.strip())
        # 重启 systemd 服务
        # 【逐行批注】 计算右侧结果并赋给 out, _，更新脚本状态。
        out, _ = run("systemctl restart gangyiAI && sleep 2 && systemctl is-active gangyiAI && curl -s http://127.0.0.1:39002/health")
        # 【逐行批注】 输出当前运行结果：print(out)。
        print(out)
        # 验证 39002 端口
        # 【逐行批注】 计算右侧结果并赋给 out, _，更新脚本状态。
        out, _ = run("curl -s -o /dev/null -w '39002 via nginx: HTTP %{http_code}\\n' http://127.0.0.1:39002/")
        # 【逐行批注】 输出当前运行结果：print(out)。
        print(out)
# 【逐行批注】 执行当前 Python 语句：finally:。
finally:
    # 【逐行批注】 执行当前 Python 语句：client.close()。
    client.close()
