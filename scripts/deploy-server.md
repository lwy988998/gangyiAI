# 钢一定制AI 部署说明（服务器）

## 环境依赖（Debian 12）
```bash
apt-get install -y build-essential cmake libsqlite3-dev libcurl4-openssl-dev libasio-dev nlohmann-json3-dev
```

## 构建
```bash
cd /www/wwwroot/gangyiAI
rm -rf build
cmake -B build
cmake --build build -j4
```

## systemd 服务（端口 39002）
```bash
systemctl daemon-reload
systemctl enable gangyiAI
systemctl start gangyiAI
systemctl status gangyiAI
```

## 环境变量
| 变量 | 默认值 | 说明 |
|---|---|---|
| PORT | 39002 | 监听端口 |
| DATABASE_PATH | data/gangyiAI.db | SQLite 路径 |
| AI_BASE_URL | (待配置) | AI API 地址 |
| AI_API_KEY | (待配置) | AI 密钥 |
| AI_MODEL | (待配置) | 模型名 |

## 验证
```bash
curl http://127.0.0.1:39002/health
# => {"status":"healthy"}
```
