# 2.1 寸 480×480 TFT MIPI 模组（ST77922）资料与示例

**English：** [`README_EN.md`](README_EN.md)

---

> 本仓库提供该 2.1 寸圆形 TFT 模组的 **示例工程** 与相关资料，面向选型与集成。示例基于 esp-lvgl-port 提供 LVGL8 / LVGL9 演示、bring-up 点屏与屏幕防撕裂（TE）方案。

## 产品概要

| 项目 | 说明 |
|:--|:--|
| 模组规格 | 2.1 英寸 **TFT**，分辨率 **480×480** |
| 接口 | **MIPI DSI** |
| 驱动芯片 | **ST77922** |
| 规格标识 | 产品资料中常用 **`2.1-tft-480x480-mipi-st77922`** 表示本规格 |

---

## 仓库结构

### 顶层目录

| 路径 | 说明 |
|:--|:--|
| `docs/` | 屏幕初始化、转接板与规格书等资料 |
| `examples/` | 示例工程 |

### `examples/` 分类

| 分类 | 说明（对应内部资料目录） |
|:--|:--|
| `examples/` 根目录 | bring-up 点屏与 esp-lvgl-port（LVGL8 / LVGL9）示例 |
| `with-te/` | **屏幕防撕裂代码** |

### 示例工程路径

#### bring-up 与 esp-lvgl-port

| 说明 | 路径 |
|:--|:--|
| ST77922 MIPI bring-up（点屏） | `examples/esp32p4-2.1-tft-480x480-mipi-st77922-bringup/` |
| esp-lvgl-port（LVGL8） | `examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V8/` |
| esp-lvgl-port（LVGL9） | `examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V9/` |

#### 屏幕防撕裂代码（`with-te/`）

| 说明 | 路径 |
|:--|:--|
| 屏幕防撕裂 LVGL 通用示例 | `examples/with-te/p4-idf_st77922-mipi_lvgl_common_demo/` |
