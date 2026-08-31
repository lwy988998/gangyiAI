# 钢一定制AI

AILINES 校园版的 C++ 服务骨架，使用 Crow 提供 HTTP 服务，目标部署平台为 Debian 12。

## 构建

需要 CMake 3.20+、支持 C++17 的 g++、SQLite3 和 libcurl 开发库。首次配置时，CMake FetchContent 会下载 Crow 和 nlohmann/json。

```bash
cmake -B build
cmake --build build
```

## 运行

默认监听 `0.0.0.0:39002`，可通过 `PORT` 环境变量修改端口：

```bash
PORT=39002 ./build/gangyiAI
```

访问 `http://localhost:39002/` 查看 JSON 状态，访问 `/health` 查看健康状态。
