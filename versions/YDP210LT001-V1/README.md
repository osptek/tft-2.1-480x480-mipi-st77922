<p align="left"><img alt="OSPTEK" src="./images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 2.1″ TFT 480×480（ST77922 · MIPI）</h1>

<p align="center"><b>圆形 TFT / IPS 模组 · MIPI · ST77922 · 电容触摸</b></p>

<p align="center"><a href="./README_EN.md">English</a> | 简体中文 · <a href="../../README.md">规格族索引</a></p>

<p align="center">
  <img alt="Size: 2.1 inch" src="https://img.shields.io/badge/Size-2.1%22-3498DB?style=flat-square" />
  <img alt="Resolution: 480x480" src="https://img.shields.io/badge/Resolution-480%C3%97480-8E44AD?style=flat-square" />
  <img alt="Interface: MIPI" src="https://img.shields.io/badge/Interface-MIPI-27AE60?style=flat-square" />
  <img alt="Driver: ST77922" src="https://img.shields.io/badge/Driver-ST77922-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 2.1 寸 480×480 TFT MIPI 模组（ST77922）宣传图" src="./images/product.png" width="640" /></p>

## 目录

- [产品简介](#产品简介)
- [规格参数](#规格参数)
- [示例工程](#示例工程)
- [仓库结构](#仓库结构)
- [相关资料](#相关资料)
- [购买链接](#购买链接)
- [技术支持](#技术支持)

---

## 产品简介

OSPTEK **2.1 寸 480×480 TFT（IPS）** 是一款 **MIPI** 接口圆形彩色显示模组，显示驱动为 **ST77922**，触摸驱动为 **ST7123**。适合穿戴表盘、圆形仪表与小型圆形 HMI 等场景。

规格标识（仓库名）：`tft-2.1-480x480-mipi-st77922`

当前模组版本：**YDP210LT001-V1**。电气与外形细节以 [`docs/YDP210LT001-V1.pdf`](./docs/YDP210LT001-V1.pdf) 为准。

## 规格参数

| 项目 | 规格 |
| ---- | ---- |
| 尺寸 | 2.1 英寸 |
| 类型 | TFT / IPS（彩色，圆形） |
| 分辨率 | 480×480 |
| 接口 | MIPI |
| 驱动 IC | ST77922 |
| 触摸驱动 | ST7123 |

> 完整外形尺寸、FPC 定义、供电与时序以产品规格书 / 驱动手册为准。

## 示例工程

| 说明 | 路径 |
| ---- | ---- |
| ESP32-P4 · ST77922 MIPI bring-up（点屏） | [`examples/esp32p4-tft-2.1-480x480-mipi-st77922-bringup/`](./examples/esp32p4-tft-2.1-480x480-mipi-st77922-bringup/) |
| ESP32-P4 · ST77922 MIPI + esp-lvgl-port / LVGL8 | [`examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V8/`](./examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V8/) |
| ESP32-P4 · ST77922 MIPI + esp-lvgl-port / LVGL9 | [`examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V9/`](./examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V9/) |
| ESP32-P4 v3.2 · ESP-IDF 6.1 · ST77922 MIPI + LVGL 9 | [`examples/esp32p4-idf6_st77922-mipi_esp-lvgl-port_lvgl9/`](./examples/esp32p4-idf6_st77922-mipi_esp-lvgl-port_lvgl9/) |
| ESP32-P4 · LVGL + TE 防撕裂 | [`examples/with-te/p4-idf_st77922-mipi_lvgl_common_demo/`](./examples/with-te/p4-idf_st77922-mipi_lvgl_common_demo/) |
| ESP32-P4 v3.2 · ESP-IDF 6.1 · LVGL + TE 防撕裂 | [`examples/with-te/esp32p4-idf6_st77922-mipi_lvgl_common_demo/`](./examples/with-te/esp32p4-idf6_st77922-mipi_lvgl_common_demo/) |
| Raspberry Pi 5 · ST77922 480×480 面板驱动 / DT overlay（仅显示） | [`examples/rpi5-panel-st77922-480x480/`](./examples/rpi5-panel-st77922-480x480/) |
| Raspberry Pi 5 · ST77922 显示 + ST7123 触摸 / DT overlay | [`examples/rpi5-panel-st77922-st7123-480x480/`](./examples/rpi5-panel-st77922-st7123-480x480/) |

## 仓库结构

```text
tft-2.1-480x480-mipi-st77922/                                # 仓库根（导航见 ../../README.md）
└── versions/
    └── YDP210LT001-V1/                                # 本料号完整资料
        ├── README.md
        ├── README_EN.md
        ├── images/
        ├── docs/
        └── examples/
```

## 相关资料

### 本产品资料

| 资料 | 链接 |
| ---- | ---- |
| 产品规格书（YDP210LT001-V1） | [`docs/YDP210LT001-V1.pdf`](./docs/YDP210LT001-V1.pdf) |
| 3D 图纸（YDP210LT001-V1） | [`docs/YDP210LT001-V1.dwg`](./docs/YDP210LT001-V1.dwg) |
| 驱动 IC 数据手册（ST77922） | [`docs/ST77922_SPEC_V0.1.pdf`](./docs/ST77922_SPEC_V0.1.pdf) |
| 初始化序列（Gamma 2.2） | [`docs/ST77922_IVO213_480RGBx480_Smart8120_20250327_Gamma2.2.txt`](./docs/ST77922_IVO213_480RGBx480_Smart8120_20250327_Gamma2.2.txt) |
| 2.1 寸屏幕转接板（V1.0） | [`docs/2.1寸屏幕转接板V1.0.pdf`](./docs/2.1%E5%AF%B8%E5%B1%8F%E5%B9%95%E8%BD%AC%E6%8E%A5%E6%9D%BFV1.0.pdf) |
| 2.1 寸屏幕转接板原理图 | [`docs/2.1寸屏幕转接板原理图.png`](./docs/2.1%E5%AF%B8%E5%B1%8F%E5%B9%95%E8%BD%AC%E6%8E%A5%E6%9D%BF%E5%8E%9F%E7%90%86%E5%9B%BE.png) |

### 示例工程

- [ESP32-P4 ST77922 MIPI bring-up](./examples/esp32p4-tft-2.1-480x480-mipi-st77922-bringup/)
- [ESP32-P4 ST77922 MIPI + LVGL8](./examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V8/)
- [ESP32-P4 ST77922 MIPI + LVGL9](./examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V9/)
- [ESP32-P4 v3.2 · ESP-IDF 6.1 · ST77922 MIPI + LVGL 9](./examples/esp32p4-idf6_st77922-mipi_esp-lvgl-port_lvgl9/)
- [ESP32-P4 LVGL + TE](./examples/with-te/p4-idf_st77922-mipi_lvgl_common_demo/)
- [ESP32-P4 v3.2 · ESP-IDF 6.1 · LVGL + TE](./examples/with-te/esp32p4-idf6_st77922-mipi_lvgl_common_demo/)
- [Raspberry Pi 5 ST77922 面板（仅显示）](./examples/rpi5-panel-st77922-480x480/)
- [Raspberry Pi 5 ST77922 显示 + ST7123 触摸](./examples/rpi5-panel-st77922-st7123-480x480/)

## 购买链接

<p align="center">
  <a href="https://shop110742373.taobao.com/"><img alt="淘宝官方店铺" src="https://img.shields.io/badge/淘宝-官方店铺-FF6A00?style=for-the-badge" /></a>
  &nbsp;&nbsp;
  <a href="https://www.aliexpress.com/store/1105701619"><img alt="速卖通官方店铺" src="https://img.shields.io/badge/速卖通-官方店铺-FF6A00?style=for-the-badge" /></a>
</p>

**国内（淘宝）**

- 店铺：[鱼鹰光电工厂店](https://shop110742373.taobao.com/)

**海外（AliExpress）**

- 店铺：[OSPTEK Official Store](https://www.aliexpress.com/store/1105701619)

## 技术支持

- 技术支持 / 产品咨询：<luyu@osptek.com>
- QQ 技术交流群：**985881096**
- 公司官网：<https://osptek.com/>
- 有任何问题，都可以在本仓库 Issues 中提问

---

<p align="center"><sub>© 2026 OSPTEK 鱼鹰光电 · 本仓库资料采用 CC BY 4.0 许可</sub></p>
