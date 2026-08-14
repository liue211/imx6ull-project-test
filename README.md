# imx6ull-p

基于正点原子 imx6ull 的嵌入式 Qt 模块化项目。

> 架构、进度与构建说明见 [项目总结.md](项目总结.md)。

## 功能模块

- 轮播图:定时自动切换 + 左右按钮 + 圆点指示
- OpenCV 工具集:预处理 / 滤波 / 边缘检测 / 噪声 / 分割 / 变换 / 特征
- 摄像头人脸识别:VideoCapture + Haar 级联 + 拍照
- imx6ull 板级设备:LED / 蜂鸣器 / AP3216C 曲线 / ICM20608 六轴
- 音频播放器 / 视频播放器:扫描 `myMusic`、`myVideo` 目录播放
- MQTT 客户端:自研 MQTT 3.1.1 实现,发布/订阅
- OTA 升级:版本检查 / 下载 / MD5 校验 / symlink 切换 / 失败自动回滚

自动化测试:`tests/`(QtTest,36 个用例,支持 offscreen 无头运行)。

## 目录结构

```
project/
├── project.pro       # 主工程文件(x86/arm 库路径切换 + APP_VERSION)
├── main.cpp          # 入口(启动标记 + OTA 后台检查)
├── mainwidget.*      # 主框架:顶部导航 + 页面堆栈 + 工厂
├── form/             # UI 页面层(8 个页面)
├── hardware/         # 硬件访问层(sysfs / 字符设备,路径可注入)
├── ota/              # OTA 客户端(版本检查/下载/校验)
├── lib/              # 自研 MQTT 3.1.1
├── images/  style/   # 资源(qrc 编译进 exe)
└── myMusic/ myVideo/ opencv_src/  # 运行时资源目录

scripts/
├── apply_update.sh   # 板端升级切换(回滚点记录 + symlink)
├── run_project.sh    # 守护启动 + 升级回滚看门狗
├── make_ota_pkg.sh   # 制作升级包 + version.json
└── ota_server/       # 升级服务器目录

tests/
├── tst_mainwidget    # 主框架行为测试(8 页面)
├── tst_hardware      # 硬件层路径注入测试
└── tst_ota           # OTA 纯函数测试(版本比较/JSON/MD5)
```

## 本机 x86 构建(Windows 临时方案)

步骤 1 仅依赖 Qt Widgets,可用 Anaconda 自带的 Qt 5.15.2 + TDM-GCC 在本机编译验证:

```powershell
qmake ..\project\project.pro   # 在 x86/ 目录中执行
mingw32-make
.\project.exe
```

> 注意:后续步骤需要 OpenCV 3.4.x、Qt5Mqtt 等,仍按计划在 WSL/Linux 环境进行。

## Ubuntu 交叉编译 + SD 卡部署(imx6ull)

工程根目录已提供两个脚本:

```bash
# 1) 交叉编译(自动查找 ARM Qt 和 ARM OpenCV,找不到就按提示传路径)
./build_arm.sh
# 或: ./build_arm.sh /路径/ARM-Qt/bin/qmake /路径/opencv-3.4.1/install

# 2) 部署到 SD 卡 rootfs 分区(先 lsblk 确认设备号!)
sudo ./deploy_sd.sh /dev/sdb /dev/sdb2
# 如果板子系统里没有 OpenCV,再传 OpenCV 安装目录让它一起拷:
sudo ./deploy_sd.sh /dev/sdb /dev/sdb2 /路径/opencv-3.4.1/install
```

板子 SD 启动后:

```bash
# 推荐:守护启动(支持 OTA 升级后自动拉起新版本 / 失败回滚)
chmod +x /home/root/run_project.sh
/home/root/run_project.sh &

# 或手动启动
cd /home/root/project
export QT_QPA_PLATFORM=eglfs
export LD_LIBRARY_PATH=/usr/lib:/usr/lib/qt5.6.2/lib:$LD_LIBRARY_PATH
./project
```

OTA 升级发布流程:

```bash
# 1) 打升级包(版本号需与 project.pro 的 APP_VERSION 一致,生成 version.json)
./scripts/make_ota_pkg.sh 1.1.0 ../build-arm

# 2) 在 scripts/ota_server/ 目录起 HTTP 服务器,板子 OTA 页面填电脑地址即可升级
cd scripts/ota_server && python -m http.server 8000
```

> 从 Windows 拷过去的脚本如果报 `\r` 错误,先执行 `sed -i 's/\r$//' build_arm.sh deploy_sd.sh`。
