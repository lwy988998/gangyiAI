import os, sys, paramiko
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
client = paramiko.SSHClient()
client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
client.connect(os.environ['SRV_HOST'], port=int(os.environ['SRV_PORT']), username=os.environ['SRV_USER'], password=os.environ['SRV_PASS'], timeout=20)
def run(cmd, timeout=900):
    stdin, stdout, stderr = client.exec_command(cmd, timeout=timeout, get_pty=True)
    out = stdout.read().decode('utf-8', 'replace')
    return out
try:
    sftp = client.open_sftp()
    sftp.put(os.path.join(os.environ['TEMP'], 'gangyiAI-live.tar.gz'), '/tmp/gangyiAI-live.tar.gz')
    sftp.close()
    print("[uploaded]", flush=True)
    cmd = "cd /www/wwwroot/gangyiAI && rm -rf * .[!.]* 2>/dev/null; tar xzf /tmp/gangyiAI-live.tar.gz"
    out = run(cmd)
    print(out)
    # 服务器访问极算云测试
    out = run("curl -s -o /dev/null -w 'jisuanyun via proxy: HTTP %{http_code} in %{time_total}s\\n' --max-time 8 -x http://127.0.0.1:7897 https://xkj.jisuanyun.vip/v1/models 2>&1")
    print(out)
    # 追加 ai_live_test target 到 CMakeLists
    cmake_add = "\nadd_executable(ai_live_test src/ai_live_test.cpp src/ai_client.cpp)\ntarget_include_directories(ai_live_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)\ntarget_link_libraries(ai_live_test PRIVATE nlohmann_json::nlohmann_json CURL::libcurl)\n"
    sftp = client.open_sftp()
    with sftp.open('/www/wwwroot/gangyiAI/CMakeLists.txt', 'a') as f:
        f.write(cmake_add)
    sftp.close()
    out = run("cd /www/wwwroot/gangyiAI && cmake -B build > /dev/null 2>&1 && cmake --build build --target ai_live_test -j4 2>&1 | grep -E 'error|Built target' | head -6")
    print(out)
finally:
    client.close()
