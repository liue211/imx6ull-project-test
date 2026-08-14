#!/bin/bash
# 把编译好的工程部署到 SD 卡的 rootfs 分区(Ubuntu 下运行)
#
# 用法:
#   sudo ./deploy_sd.sh <sd设备> <rootfs分区> [OpenCV安装目录]
# 示例:
#   sudo ./deploy_sd.sh /dev/sdb /dev/sdb2
#   sudo ./deploy_sd.sh /dev/sdb /dev/sdb2 /home/liu/linux/opencv-3.4.1/install
#
# 先执行 lsblk 确认设备名,选错会写错盘!
set -e

if [ "$(id -u)" -ne 0 ]; then
    echo "请用 sudo 运行: sudo ./deploy_sd.sh /dev/sdX /dev/sdX2"
    exit 1
fi

DEV="${1:?用法: sudo ./deploy_sd.sh /dev/sdX /dev/sdX2 [OpenCV安装目录]}"
PART="${2:?缺少 rootfs 分区参数,例如 /dev/sdb2}"
OPENCV_INSTALL="${3:-}"

case "$DEV" in
    /dev/sd[a-z]|/dev/mmcblk[0-9]) ;;
    *) echo "设备名看起来不对: $DEV"; exit 1 ;;
esac
[ -b "$PART" ] || { echo "分区不存在: $PART"; exit 1; }

# 安全校验:拒绝把当前系统的磁盘当成 SD 卡
if grep -qs "^$DEV" /proc/mounts; then
    for m in / /boot /home; do
        if mountpoint -q "$m" 2>/dev/null && grep -qs "^$DEV" /proc/mounts; then
            echo "错误: $DEV 看起来是当前系统的磁盘,拒绝操作"
            exit 1
        fi
    done
fi
if mount | grep -q "^$PART "; then
    echo "错误: $PART 已挂载,请先 umount 再运行"
    exit 1
fi

SRC_DIR="$(cd "$(dirname "$0")" && pwd)"
[ -f "$SRC_DIR/build-arm/project" ] || {
    echo "错误: 找不到 $SRC_DIR/build-arm/project,请先运行 ./build_arm.sh"
    exit 1
}

echo "----- 将部署到以下设备 -----"
lsblk -o NAME,SIZE,MODEL "$DEV"
FSTYPE="$(blkid -o value -s TYPE "$PART" 2>/dev/null || true)"
echo "rootfs 分区: $PART (类型: ${FSTYPE:-未知})"
read -r -p "确认把工程部署到 $PART ? (输入 yes 继续): " ans
[ "$ans" = "yes" ] || { echo "已取消"; exit 1; }

MNT="$(mktemp -d)"
cleanup() {
    umount "$MNT" 2>/dev/null || true
    rmdir "$MNT" 2>/dev/null || true
}
trap cleanup EXIT

mount "$PART" "$MNT"

mkdir -p "$MNT/home/root/project"
cp -v "$SRC_DIR/build-arm/project" "$MNT/home/root/project/"
cp -rv "$SRC_DIR/project/myMusic" \
       "$SRC_DIR/project/myVideo" \
       "$SRC_DIR/project/opencv_src" \
       "$MNT/home/root/project/"

# OTA 升级脚本(板端 /home/root/ 下,run_project.sh 守护启动 + apply_update.sh 升级切换)
cp -v "$SRC_DIR/scripts/run_project.sh" \
      "$SRC_DIR/scripts/apply_update.sh" \
      "$MNT/home/root/"

if [ -n "$OPENCV_INSTALL" ] && [ -d "$OPENCV_INSTALL/lib" ]; then
    echo "---- 拷贝 OpenCV 动态库到板子系统 /usr/lib ----"
    cp -v "$OPENCV_INSTALL"/lib/libopencv_*.so* "$MNT/usr/lib/"
elif [ -n "$OPENCV_INSTALL" ]; then
    echo "警告: 给的 OpenCV 目录不存在,跳过拷贝: $OPENCV_INSTALL"
fi

sync
umount "$MNT"
trap - EXIT
rmdir "$MNT" 2>/dev/null || true

echo ""
echo "部署完成。板子端运行(推荐用守护脚本,支持 OTA 升级回滚):"
echo "  chmod +x /home/root/run_project.sh && /home/root/run_project.sh"
echo ""
echo "或手动启动:"
echo "  cd /home/root/project"
echo "  export QT_QPA_PLATFORM=eglfs"
echo "  export LD_LIBRARY_PATH=/usr/lib:/usr/lib/qt5.6.2/lib:\$LD_LIBRARY_PATH"
echo "  ./project"
