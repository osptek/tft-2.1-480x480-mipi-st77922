#include "esp_err.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"

#include "bsp_display.h"

void app_main(void)
{
    /* 屏幕 + 触摸 + LVGL 初始化（全部封装在 bsp_display 中） */
    ESP_ERROR_CHECK(bsp_display_init());

    /* 显示 LVGL 示例界面（操作 LVGL 前后需加锁/解锁） */
    lvgl_port_lock(0);
    lv_demo_widgets();
    lvgl_port_unlock();
}
