#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 屏幕分辨率 */
#define BSP_LCD_H_RES   (480)
#define BSP_LCD_V_RES   (480)

/**
 * @brief 初始化屏幕硬件（MIPI DSI + ST77922）、触摸（ST7123）以及 LVGL
 *
 * 内部依次完成：
 *   1. LCD 面板初始化（DPHY 供电、DSI 总线、DBI/DPI、ST77922 命令）
 *   2. 触摸控制器初始化（I2C + ST7123）
 *   3. LVGL port 初始化并挂载显示与触摸输入
 *
 * @return ESP_OK 成功，其他为失败
 */
esp_err_t bsp_display_init(void);

#ifdef __cplusplus
}
#endif
