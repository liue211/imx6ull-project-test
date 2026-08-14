#!/bin/sh
# 板端 OTA 应用脚本:解压升级包 → 记录回滚点 → 切换 symlink
#
# 用法: apply_update.sh <新版本目录名> <升级包路径> <当前版本号>
# 由 Qt 的 OtaClient 调用;目录约定: /home/root/project -> project_vX.Y.Z(symlink)
#
# 目录布局:
#   /home/root/project            -> symlink,指向当前版本
#   /home/root/project_v1.0.0/    旧版本(保留用于回滚)
#   /home/root/.ota_state         回滚点记录(rollback=<版本目录>)
set -e

# OTA_ROOT 可用环境变量覆盖,便于本机模拟验证(板端默认 /home/root)
OTA_ROOT="${OTA_ROOT:-/home/root}"
VERSION_DIR="$1"
PACKAGE="$2"
CUR_VERSION="$3"

if [ -z "$VERSION_DIR" ] || [ -z "$PACKAGE" ]; then
    echo "用法: apply_update.sh <版本目录名> <包路径> <当前版本号>"
    exit 1
fi
[ -f "$PACKAGE" ] || { echo "升级包不存在: $PACKAGE"; exit 1; }

cd "$OTA_ROOT"

# 1. 记录回滚点:当前 project 指向的版本目录
if [ -L project ]; then
    OLD="$(readlink project)"
else
    # 首次升级:project 还是部署时的真实目录,先改名保留为旧版本
    OLD="project_v$CUR_VERSION"
    mv project "$OLD"
fi
echo "rollback=$OLD" > .ota_state
echo "回滚点: $OLD"

# 2. 解压到新版本目录(先清空,避免残留旧文件)
rm -rf "$VERSION_DIR"
mkdir -p "$VERSION_DIR"
tar xzf "$PACKAGE" -C "$VERSION_DIR"
rm -rf "$VERSION_DIR/ota_cache"   # 下载缓存不随版本走

# 3. 切换 symlink(ln -sfn 是原子替换,中途断电不会出现半成品)
ln -sfn "$VERSION_DIR" project
echo "已切换到 $VERSION_DIR"
