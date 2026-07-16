# 2.1" 480×480 TFT MIPI module (ST77922) — documentation & samples

**简体中文：** [`README.md`](README.md)

---

> This repository provides **sample projects** and reference material for the 2.1-inch TFT module, aimed at evaluation and integration. Samples cover bring-up, esp-lvgl-port demos for LVGL8 / LVGL9, and a tear-free (TE) solution.

## Product overview

| Item | Description |
|:--|:--|
| Module | 2.1-inch **TFT**, **480×480** resolution |
| Interface | **MIPI DSI** |
| Driver IC | **ST77922** |
| Spec ID | **`2.1-tft-480x480-mipi-st77922`** is the common product designation in documentation |

---

## Repository layout

### Top-level

| Path | Contents |
|:--|:--|
| `docs/` | Panel init, adapter board and datasheet material |
| `examples/` | Sample projects |

### `examples/` layout

| Location | Description (internal package folder) |
|:--|:--|
| `examples/` root | Bring-up and esp-lvgl-port (LVGL8 / LVGL9) samples |
| `with-te/` | Tear-free samples (**屏幕防撕裂代码**) |

### Sample project paths

#### Bring-up and esp-lvgl-port

| Description | Path |
|:--|:--|
| ST77922 MIPI bring-up | `examples/esp32p4-2.1-tft-480x480-mipi-st77922-bringup/` |
| esp-lvgl-port (LVGL8) | `examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V8/` |
| esp-lvgl-port (LVGL9) | `examples/P4-IDF_ST77922-MIPI_ESP-LVGL-PORT_V9/` |

#### Tear-free samples (`with-te/`)

| Description | Path |
|:--|:--|
| Tear-free LVGL common demo | `examples/with-te/p4-idf_st77922-mipi_lvgl_common_demo/` |
