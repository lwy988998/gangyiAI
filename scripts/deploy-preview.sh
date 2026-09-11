#!/bin/bash
# 部署脚本：仅更新预览服务 gangyiAI-preview
set -e

REMOTE_USER="root"
REMOTE_HOST="103.236.84.172"
REMOTE_PORT="22366"
REMOTE_DIR="/www/wwwroot/gangyiAI-v020"
SERVICE_NAME="gangyiAI-preview.service"
BACKUP_DIR="/root/gangyiai-backups"

echo "=== 预览服务部署开始 ==="
echo "目标: ${REMOTE_HOST}:${REMOTE_PORT}"
echo "服务: ${SERVICE_NAME}"
echo ""

# 1. 备份当前二进制
echo "[1/6] 备份现有二进制..."
ssh -p ${REMOTE_PORT} ${REMOTE_USER}@${REMOTE_HOST} <<'REMOTE_BACKUP'
set -e
BACKUP_DIR="/root/gangyiai-backups"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BACKUP_PATH="${BACKUP_DIR}/preview_${TIMESTAMP}"
mkdir -p "${BACKUP_PATH}"
cd /www/wwwroot/gangyiAI-v020
if [ -f build-real/gangyiAI.exe ]; then
  cp build-real/gangyiAI.exe "${BACKUP_PATH}/"
  cp build-real/gangyiAI-launcher.exe "${BACKUP_PATH}/" 2>/dev/null || true
  echo "备份已保存到 ${BACKUP_PATH}"
else
  echo "警告: 未找到现有二进制"
fi
REMOTE_BACKUP

# 2. 停止预览服务
echo "[2/6] 停止预览服务..."
ssh -p ${REMOTE_PORT} ${REMOTE_USER}@${REMOTE_HOST} "systemctl stop ${SERVICE_NAME}"
sleep 2

# 3. 构建本地 Release
echo "[3/6] 本地构建 Release..."
cd "$(dirname "$0")"
if [ ! -d "build-real" ]; then
  cmake -B build-real -DCMAKE_BUILD_TYPE=Release
fi
cmake --build build-real --config Release --target gangyiAI gangyiAI-launcher

# 4. 上传新二进制
echo "[4/6] 上传新二进制..."
scp -P ${REMOTE_PORT} \
  build-real/gangyiAI.exe \
  build-real/gangyiAI-launcher.exe \
  ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_DIR}/build-real/

# 5. 启动预览服务
echo "[5/6] 启动预览服务..."
ssh -p ${REMOTE_PORT} ${REMOTE_USER}@${REMOTE_HOST} "systemctl start ${SERVICE_NAME}"
sleep 3

# 6. 验证服务状态
echo "[6/6] 验证服务状态..."
ssh -p ${REMOTE_PORT} ${REMOTE_USER}@${REMOTE_HOST} <<'REMOTE_CHECK'
set -e
STATUS=$(systemctl is-active gangyiAI-preview.service)
if [ "$STATUS" = "active" ]; then
  echo "✓ 预览服务已启动"
  PID=$(systemctl show -p MainPID --value gangyiAI-preview.service)
  echo "  PID: ${PID}"
  systemctl show -p ActiveEnterTimestamp --value gangyiAI-preview.service | head -1
  sleep 1
  if curl -sf http://127.0.0.1:39003/health > /dev/null; then
    echo "✓ 健康检查通过"
  else
    echo "✗ 健康检查失败"
    exit 1
  fi
else
  echo "✗ 服务启动失败"
  journalctl -u gangyiAI-preview.service -n 20 --no-pager
  exit 1
fi
REMOTE_CHECK

echo ""
echo "=== 部署完成 ==="
echo "预览地址: http://103.236.84.172:39003"
echo ""
echo "回滚命令（如需要）："
echo "  ssh -p ${REMOTE_PORT} ${REMOTE_USER}@${REMOTE_HOST}"
echo "  systemctl stop ${SERVICE_NAME}"
echo "  LAST_BACKUP=\$(ls -t ${BACKUP_DIR}/preview_* | head -1)"
echo "  cp \${LAST_BACKUP}/gangyiAI.exe ${REMOTE_DIR}/build-real/"
echo "  cp \${LAST_BACKUP}/gangyiAI-launcher.exe ${REMOTE_DIR}/build-real/"
echo "  systemctl start ${SERVICE_NAME}"
