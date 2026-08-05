# imx6ull-p

基于正点原子 imx6ull 的嵌入式 Qt 模块化项目的**逐模块复刻工程**。

> 原则:不复制原文件,参考原始实现逐模块重写,学习嵌入式 Linux Qt 开发。
> 架构、进度与构建说明见 [项目总结.md](项目总结.md)。

## 功能模块

- 轮播图:定时自动切换 + 左右按钮 + 圆点指示
- OpenCV 工具集:预处理 / 滤波 / 边缘检测 / 噪声 / 分割 / 变换 / 特征
- 摄像头人脸识别:VideoCapture + Haar 级联 + 拍照
- imx6ull 板级设备:LED / 蜂鸣器 / AP3216C 曲线 / ICM20608 六轴
- 音频播放器 / 视频播放器:扫描 `myMusic`、`myVideo` 目录播放
- MQTT 客户端:自研 MQTT 3.1.1 实现,发布/订阅

自动化测试:`tests/`(QtTest,12 个用例,支持 offscreen 无头运行)。

## 目录结构

```
project/
├── project.pro       # 主工程文件(x86/arm 库路径切换,后续步骤加入)
├── main.cpp          # 入口
├── mainwidget.*      # 主框架:左列表 + 右堆栈 + 工厂(步骤 2)
├── form/             # UI 页面层(后续步骤)
├── hardware/         # 硬件层(后续步骤)
├── images/  style/   # 资源(后续步骤)
└── myMusic/ myVideo/ # 媒体资源(后续步骤)

tests/
└── tst_mainwidget.cpp  # 主框架行为测试(QtTest,可无头运行)
```

## 本机 x86 构建(Windows 临时方案)

步骤 1 仅依赖 Qt Widgets,可用 Anaconda 自带的 Qt 5.15.2 + TDM-GCC 在本机编译验证:

```powershell
qmake ..\project\project.pro   # 在 x86/ 目录中执行
mingw32-make
.\project.exe
```

> 注意:后续步骤需要 OpenCV 3.4.x、Qt5Mqtt 等,仍按计划在 WSL/Linux 环境进行。
