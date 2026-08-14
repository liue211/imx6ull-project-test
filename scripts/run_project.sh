#!/bin/sh
# 板端应用守护脚本:循环拉起 project,升级后自动拉起新版本,启动失败自动回滚
#
# 用法: run_project.sh [回滚等待秒数(默认 15)]
#
# 原理:
#   - 应用启动后会在 /tmp/.ota_boot_ok 写入"启动成功"标记(main.cpp 完成初始化时)
#   - 本脚本启动应用后等待 N 秒:进程不在 或 无标记 -> 判定启动失败
#   - 失败时读取 /home/root/.ota_state 的 rollback 字段,把 project symlink
#     切回旧版本目录,再重新拉起
#   - 应用正常退出(含升级后主动退出)会回到循环,拉起 symlink 当前指向的版本

# OTA_ROOT 可用环境变量覆盖,便于本机模拟验证(板端默认 /home/root)
OTA_ROOT="${OTA_ROOT:-/home/root}"
BOOT_OK=/tmp/.ota_boot_ok
ROLLBACK_WAIT="${1:-15}"

cd "$OTA_ROOT" || { echo "OTA_ROOT 不存在: $OTA_ROOT"; exit 1; }

while true; do
    rm -f "$BOOT_OK"

    [ -d "$OTA_ROOT/project" ] || {
        echo "project 目录不存在,5 秒后重试..."
        sleep 5
        continue
    }

    cd "$OTA_ROOT/project" || continue
    echo "----- 启动应用 (版本目录: $(readlink "$OTA_ROOT/project" 2>/dev/null || echo project)) -----"
    export QT_QPA_PLATFORM=eglfs
    export LD_LIBRARY_PATH=/usr/lib:/usr/lib/qt5.6.2/lib:$LD_LIBRARY_PATH
    ./project &
    APP_PID=$!

    # 给应用一段启动时间,等待"启动成功"标记
    sleep "$ROLLBACK_WAIT"

    if ! kill -0 "$APP_PID" 2>/dev/null || [ ! -f "$BOOT_OK" ]; then
        echo "警告: 应用未在 ${ROLLBACK_WAIT}s 内正常启动,执行回滚"
        if [ -f "$OTA_ROOT/.ota_state" ]; then
            # shellcheck disable=SC1091
            . "$OTA_ROOT/.ota_state"
            if [ -n "$rollback" ] && [ -d "$OTA_ROOT/$rollback" ]; then
                ln -sfn "$rollback" "$OTA_ROOT/project"
                echo "已回滚到 $rollback"
            else
                echo "回滚点无效($rollback),保持当前版本"
            fi
        else
            echo "无回滚记录(.ota_state 不存在)"
        fi
        # 不等待已死的进程,直接进入下一轮启动
        continue
    fi

    # 启动成功,等待应用退出(正常关闭或升级后主动退出)
    wait "$APP_PID"
    echo "应用已退出,重新拉起..."
done
