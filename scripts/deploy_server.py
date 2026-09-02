import os, sys, paramiko
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect(os.environ['SRV_HOST'], port=int(os.environ['SRV_PORT']), username=os.environ['SRV_USER'], password=os.environ['SRV_PASS'], timeout=20)
def run(cmd, timeout=1200):
    stdin, stdout, stderr = client.exec_command(cmd, timeout=timeout, get_pty=True)
    out = stdout.read().decode('utf-8', 'replace')
    code = stdout.channel.recv_exit_status()
    return out, code
try:
    sftp = client.open_sftp()
    sftp.put(os.path.join(os.environ['TEMP'], 'gangyiAI-deploy.tar.gz'), '/tmp/gangyiAI-deploy.tar.gz')
    sftp.close()
    print("[uploaded]", flush=True)
    out, code = run("cd /www/wwwroot/gangyiAI && rm -rf * .[!.]* 2>/dev/null; tar xzf /tmp/gangyiAI-deploy.tar.gz && echo extract-ok")
    print(out.strip())
    # 服务器编译（asio 用系统库）
    out, code = run("cd /www/wwwroot/gangyiAI && rm -rf build && cmake -B build > /dev/null 2>&1 && cmake --build build -j4 2>&1 | grep -E 'error|Built target gangyiAI' | head -8", timeout=1200)
    print(out)
    print(f"build exit: {code}")
    if code == 0:
        out, _ = run("ls -la /www/wwwroot/gangyiAI/build/gangyiAI | awk '{print $5}' && echo BUILD_OK")
        print(out.strip())
        # 重启 systemd 服务
        out, _ = run("systemctl restart gangyiAI && sleep 2 && systemctl is-active gangyiAI && curl -s http://127.0.0.1:39002/health")
        print(out)
        # 验证 39002 端口
        out, _ = run("curl -s -o /dev/null -w '39002 via nginx: HTTP %{http_code}\\n' http://127.0.0.1:39002/")
        print(out)
finally:
    client.close()
