#!/bin/bash
# imx6ull Qt 工程交叉编译脚本(Ubuntu 下运行)
#
# 用法:
#   ./build_arm.sh                                   # 自动查找 qmake / OpenCV
#   ./build_arm.sh <ARM-Qt-qmake> <OpenCV-install目录>
#
# 也可以用环境变量 QMAKE / ARM_OPENCV_DIR 指定。
# 本脚本由 Windows 同步过来后,若报 \r 错误:
#   sed -i 's/\r$//' build_arm.sh deploy_sd.sh
set -e

QMAKE="${1:-$QMAKE}"
OPENCV_DIR="${2:-$ARM_OPENCV_DIR}"

# ---------- 1. 定位 ARM Qt 的 qmake ----------
if [ -z "$QMAKE" ]; then
    for p in \
        /usr/local/arm/qt5.6.2/5.6.2/arm-linux-gnueabihf/bin/qmake \
        /usr/local/arm/qt5.6.2/5.6.2/gcc_64/bin/qmake \
        /usr/local/arm/qt5.5.1/5.5.1/arm-linux-gnueabihf/bin/qmake; do
        if [ -x "$p" ]; then QMAKE="$p"; break; fi
    done
fi
if [ -z "$QMAKE" ]; then
    QMAKE="$(find /usr/local/arm /opt -maxdepth 7 -type f -name qmake 2>/dev/null | head -n 1 || true)"
fi
if [ -z "$QMAKE" ] || [ ! -x "$QMAKE" ]; then
    echo "错误: 找不到 ARM 版 qmake。"
    echo "请手动指定: ./build_arm.sh /你的路径/.../bin/qmake"
    exit 1
fi
echo "==> qmake: $QMAKE"

# 确认这个 Qt 是 ARM 版(核心库必须是 ARM 架构)
QT_PREFIX="$("$QMAKE" -query QT_INSTALL_PREFIX 2>/dev/null || true)"
QT_CORE="$QT_PREFIX/lib/libQt5Core.so"
if [ -f "$QT_CORE" ]; then
    if file "$QT_CORE" | grep -q "ARM"; then
        echo "==> Qt: ARM 版 OK ($QT_PREFIX)"
    else
        echo "错误: $QT_CORE 不是 ARM 库(如果是 gcc_64,那是 x86_64 Qt,不能用于板子)"
        exit 1
    fi
else
    echo "警告: 没找到 $QT_CORE,先继续,编译时可能会报错"
fi

# 检查工程用到的两个 Qt 模块
for m in Charts Multimedia; do
    if ls "$QT_PREFIX"/lib/libQt5${m}* >/dev/null 2>&1; then
        echo "==> Qt$m: 有"
    else
        echo "警告: 缺少 Qt$m(缺 Charts 时 qmake 会报 Unknown module: charts)"
    fi
done

# ---------- 2. 定位 ARM OpenCV ----------
if [ -z "$OPENCV_DIR" ]; then
    for p in \
        /home/coucou/linux/opencv-3.4.1/install \
        /home/liu/linux/opencv-3.4.1/install \
        /usr/local/arm/opencv-3.4.1/install; do
        if [ -d "$p/lib" ]; then OPENCV_DIR="$p"; break; fi
    done
fi
if [ -z "$OPENCV_DIR" ] || [ ! -d "$OPENCV_DIR/lib" ]; then
    echo "错误: 找不到 ARM OpenCV。"
    echo "请手动指定: ./build_arm.sh <qmake路径> /你的路径/opencv-3.4.1/install"
    exit 1
fi
echo "==> OpenCV: $OPENCV_DIR"
if [ -f "$OPENCV_DIR/lib/libopencv_core.so" ]; then
    if file "$OPENCV_DIR/lib/libopencv_core.so" | grep -q "ARM"; then
        echo "==> OpenCV: ARM 版 OK"
    else
        echo "警告: libopencv_core.so 不是 ARM 库,请检查路径"
    fi
else
    echo "警告: $OPENCV_DIR/lib/libopencv_core.so 不存在"
fi

# ---------- 3. 编译 ----------
mkdir -p build-arm
cd build-arm
"$QMAKE" ARM_OPENCV_DIR="$OPENCV_DIR" ../project/project.pro
make -j"$(nproc 2>/dev/null || echo 2)"

echo ""
if file project | grep -q "ARM"; then
    echo "编译成功: $(pwd)/project (ARM 可执行文件)"
    echo "下一步: sudo ./deploy_sd.sh /dev/sdX /dev/sdX2 [可选:OpenCV安装目录]"
else
    echo "错误: 编译产物不是 ARM 可执行文件"
    exit 1
fi
