# OTA 升级服务器目录

把 `make_ota_pkg.sh` 生成的升级包(`project_<版本>.tar.gz`)和 `version.json` 放在本目录,
然后在**本目录**启动 HTTP 服务即可:

```bash
cd scripts/ota_server
python -m http.server 8000
```

板子侧的升级服务器地址填: `http://192.168.137.1:8000`
(192.168.137.1 = 电脑的 ICS 共享网口地址,见 docs/联网指南.md)

## version.json 格式

```json
{"version":"1.1.0","url":"project_1.1.0.tar.gz","md5":"<32位hex>"}
```

| 字段 | 说明 |
|---|---|
| version | 新版本号,与包的版本一致 |
| url | 升级包文件名(相对本目录)或完整 URL |
| md5 | 升级包 MD5,板端下载后校验,不一致则放弃 |

不要手写 md5 —— 用 `scripts/make_ota_pkg.sh` 生成,它会自动计算:

```bash
./make_ota_pkg.sh 1.1.0 ../build-arm     # Ubuntu 上打 ARM 包
./make_ota_pkg.sh 1.1.0 ../x86           # Windows x86 演示包
```
