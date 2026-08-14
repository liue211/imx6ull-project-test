#!/bin/bash
# 制作 OTA 升级包:组装发布目录 → tar.gz + 生成 version.json
#
# 用法: make_ota_pkg.sh <版本号> <构建产物目录> [输出目录]
# 示例:
#   # ARM 板端包(Ubuntu 交叉编译后)
#   ./make_ota_pkg.sh 1.1.0 ../build-arm
#   # Windows x86 演示包(在本机 Git Bash 下验证流程)
#   ./make_ota_pkg.sh 1.1.0 ../x86
#
# 产物:
#   <输出目录>/project_<版本号>.tar.gz     升级包(exe + myMusic/myVideo/opencv_src)
#   <输出目录>/version.json                {"version","url","md5"}
set -e

VERSION="${1:?用法: make_ota_pkg.sh <版本号> <构建产物目录> [输出目录]}"
SRC_DIR="${2:?缺少构建产物目录参数}"
OUT_DIR="${3:-$(cd "$(dirname "$0")" && pwd)/ota_server}"

REPO="$(cd "$(dirname "$0")/.." && pwd)"
PKG_NAME="project_${VERSION}.tar.gz"

# 1. 组装发布目录:可执行文件 + 运行时资源目录
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

if [ -d "$SRC_DIR" ]; then
    cp -v "$SRC_DIR/project" "$STAGE/" 2>/dev/null \
        || cp -v "$SRC_DIR/project.exe" "$STAGE/" 2>/dev/null \
        || { echo "错误: $SRC_DIR 下找不到 project/project.exe"; exit 1; }
else
    echo "错误: 构建产物目录不存在: $SRC_DIR"
    exit 1
fi
cp -rv "$REPO/project/myMusic" "$REPO/project/myVideo" \
      "$REPO/project/opencv_src" "$STAGE/" 2>/dev/null || true

# 2. 打包 + 校验和
mkdir -p "$OUT_DIR"
tar czf "$OUT_DIR/$PKG_NAME" -C "$STAGE" .
MD5="$(md5sum "$OUT_DIR/$PKG_NAME" | awk '{print $1}')"

# 3. 生成 version.json
cat > "$OUT_DIR/version.json" <<EOF
{"version":"$VERSION","url":"$PKG_NAME","md5":"$MD5"}
EOF

echo ""
echo "OTA 升级包已生成:"
echo "  包:       $OUT_DIR/$PKG_NAME ($(du -h "$OUT_DIR/$PKG_NAME" | awk '{print $1}'))"
echo "  md5:      $MD5"
echo "  版本文件: $OUT_DIR/version.json"
echo ""
echo "发布: 把 ota_server 目录放到 HTTP 服务器根目录,例如:"
echo "  cd $(dirname "$0")/ota_server && python -m http.server 8000"
