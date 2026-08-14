# OTA 升级说明（升级计划·功能组三）

> 状态:已实现(2026-08-14)
> 范围:应用层 OTA(project 可执行文件 + 资源目录),不动内核/rootfs
> 架构:HTTP 检查/下载 + MD5 校验 + symlink 版本切换 + 启动看门狗回滚

## 1. 总体流程

```
[发布端]                          [板端]
make_ota_pkg.sh 打包
  → ota_server/ 起 HTTP 服务
                                    ① OTA 页面(或启动时静默)检查 version.json
                                    ② 版本比较(按段数值):服务器 > 当前 → 可升级
                                    ③ 下载 project_vX.Y.Z.tar.gz 到 ota_cache/
                                    ④ MD5 校验(与 version.json 比对)
                                    ⑤ apply_update.sh:记录回滚点 → 解压新目录
                                       → 切换 symlink → 应用重启
                                    ⑥ run_project.sh 拉起新版本;
                                       15s 内无"启动成功"标记 → 自动切回旧版本
```

## 2. 目录布局(板端 /home/root/)

```
/home/root/
├── project -> project_v1.1.0      symlink,运行时目录(旧版本文本/首次部署时为真实目录)
├── project_v1.0.0/                旧版本(升级时保留,回滚用)
├── project_v1.1.0/                新版本(解压自升级包)
├── .ota_state                     回滚点记录: rollback=project_v1.0.0
├── run_project.sh                 守护启动脚本(回滚看门狗)
└── apply_update.sh                升级切换脚本
```

首次升级时 `project/` 是真实目录,脚本会先 `mv project project_v<当前版本>`
再建立 symlink,此后 project 恒为 symlink。

## 3. 升级包规范

`version.json`(由 make_ota_pkg.sh 自动生成):

```json
{"version":"1.1.0","url":"project_1.1.0.tar.gz","md5":"<32位hex>"}
```

| 字段 | 说明 |
|---|---|
| version | 新版本号,必须高于当前版本才会触发升级 |
| url | 包文件名(相对 ota_server 目录)或完整 URL |
| md5 | 升级包 MD5;板端校验失败则删除坏包并中止,不影响当前版本 |

包内容:project 可执行文件 + myMusic/myVideo/opencv_src 资源目录。

## 4. 版本号管理

- 当前版本写在 `project/project.pro`: `DEFINES += APP_VERSION=\\\"1.0.0\\\"`
- OTA 页面显示"当前版本";升级判断 = 服务器版本 > 当前版本
- 发新版本时:改 project.pro 版本号 → 重新编译 → `make_ota_pkg.sh <新版本号> <构建目录>`

## 5. 发布与升级操作

```bash
# ① 打升级包(Ubuntu 上打 ARM 包;本机 Git Bash 可打 x86 演示包)
./scripts/make_ota_pkg.sh 1.1.0 ../build-arm

# ② 起服务器(在 ota_server 目录,电脑上)
cd scripts/ota_server && python -m http.server 8000

# ③ 板子 OTA 页面:URL 填 http://192.168.137.1:8000(电脑 ICS 网口地址)
#    → 检查更新 → 立即升级 → 自动重启
#    (或改 exe 同目录 ota.ini 的 ota/baseUrl 持久化)
```

## 6. 回滚机制

- **触发条件**:run_project.sh 启动应用后等待 15s(可传参调整),应用进程不存在
  或未写 `/tmp/.ota_boot_ok`(应用 main 初始化完成时写入) → 判定启动失败
- **动作**:读取 `.ota_state` 的 rollback 字段 → `ln -sfn` 切回旧版本目录 → 重新拉起
- **保证**:symlink 切换是原子操作;升级包先完整下载并 MD5 校验后才落盘;
  坏包/断网/下载中断都不会碰当前版本目录

## 7. 验证清单(验收标准对照)

- [ ] 手动发布 1.1.0 → 板子 OTA 页面检查到新版本 → 升级后自动重启 → 版本号变化
- [ ] 断电重启后仍运行新版本(目录持久)
- [ ] 伪造 md5 错误的包 → 提示 MD5 校验失败,当前版本不受影响
- [ ] 删掉新版本目录中的可执行文件(模拟坏版本)→ 重启后自动回滚旧版本
- [ ] 断网时检查版本 → 报"检查版本失败",应用正常运行

## 8. 代码结构

| 文件 | 作用 |
|---|---|
| `project/ota/otaclient.h/.cpp` | 版本检查/下载/MD5/调用切换脚本;静态纯函数可测 |
| `project/form/ota_upgrade.h/.cpp` | OTA 页面(第 8 个模块) |
| `scripts/apply_update.sh` | 板端:解压/回滚点记录/symlink 切换 |
| `scripts/run_project.sh` | 板端:守护启动 + 回滚看门狗 |
| `scripts/make_ota_pkg.sh` | 打包 + 生成 version.json |
| `scripts/ota_server/` | 服务器目录(version.json + 升级包 + 说明) |
| `tests/tst_ota/` | 纯函数测试(版本比较/JSON 解析/MD5) |

## 9. 已知限制与后续

- 升级包不含 Qt/OpenCV 运行库:库版本变化需另做 rootfs 级升级
- 目前无 TLS(HTTP 明文):公网部署建议加 HTTPS 或包签名(验签可用
  QSslCertificate / OpenSSL)
- 升级包较大时(OpenCV 资源)下载时间取决于带宽,进度条已展示进度
