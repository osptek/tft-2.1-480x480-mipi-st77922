#include "bsp_display.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "esp_lvgl_port.h"

#include "esp_lcd_st77922.h"
#include "esp_lcd_touch_st7123.h"

/* ---------------- 屏幕参数 ---------------- */
#define EXAMPLE_LCD_H_RES               BSP_LCD_H_RES
#define EXAMPLE_LCD_V_RES               BSP_LCD_V_RES

#if LV_COLOR_DEPTH == 16
#define MIPI_DPI_PX_FORMAT              (LCD_COLOR_PIXEL_FORMAT_RGB565)
#define BSP_LCD_COLOR_DEPTH             (16)
#define LV_COLOR_FORMAT                 LV_COLOR_FORMAT_RGB565
#elif LV_COLOR_DEPTH == 24
#define MIPI_DPI_PX_FORMAT              (LCD_COLOR_PIXEL_FORMAT_RGB888)
#define BSP_LCD_COLOR_DEPTH             (24)
#define LV_COLOR_FORMAT                 LV_COLOR_FORMAT_RGB888
#endif

/* MIPI DSI PHY 需要 2.5V 供电，这里使用芯片内部 LDO_VO3 通道 */
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN       (3)
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV (2500)

/* 背光 / 复位（本屏无独立背光和复位脚，均为 -1） */
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL   (1)
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL  (!EXAMPLE_LCD_BK_LIGHT_ON_LEVEL)
#define EXAMPLE_PIN_NUM_BK_LIGHT        (-1)
#define EXAMPLE_PIN_NUM_LCD_RST         (-1)

/* ---------------- 触摸参数 ---------------- */
#define EXAMPLE_TOUCH_I2C_NUM           (0)
#define EXAMPLE_TOUCH_I2C_CLK_HZ        (400000)
#define EXAMPLE_TOUCH_I2C_SCL           (GPIO_NUM_8)
#define EXAMPLE_TOUCH_I2C_SDA           (GPIO_NUM_7)

static const char *TAG = "bsp_display";

/* LCD 面板与触摸句柄 */
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL 显示与输入设备 */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

static void example_bsp_enable_dsi_phy_power(void)
{
    /* 打开 MIPI DSI PHY 电源，使其从"无电"进入"关机"待机状态 */
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = EXAMPLE_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
}

static void example_bsp_init_lcd_backlight(void)
{
#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif
}

static void example_bsp_set_lcd_backlight(uint32_t level)
{
#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, level);
#endif
}

/* ST77922 厂商初始化命令序列（来自屏厂 2.1 寸 MIPI 参考资料） */
static const st77922_lcd_init_cmd_t lcd_init_cmds[] = {
    //  {cmd, { data }, data_size, delay_ms}
    {0xF1, (uint8_t []){0x00}, 1, 0},
    {0x60, (uint8_t []){0x00, 0x00, 0x00}, 3, 0},
    {0x65, (uint8_t []){0x80}, 1, 0},
    {0x79, (uint8_t []){0x06, 0x00}, 2, 0},
    {0x7B, (uint8_t []){0x00, 0x08, 0x08}, 3, 0},
    {0x80, (uint8_t []){0x55, 0x62, 0x2F, 0x17, 0xF0, 0x52, 0x70, 0xD2, 0x52, 0x62, 0xEA, 0xCF}, 12, 0},
    {0x81, (uint8_t []){0x26, 0x52, 0x72, 0x27}, 4, 0},
    {0x84, (uint8_t []){0x92, 0x25}, 2, 0},
    {0x87, (uint8_t []){0x10, 0x10, 0x58, 0x00, 0x02, 0x3A}, 6, 0},
    {0x88, (uint8_t []){0x00, 0x00, 0x2C, 0x10, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x06}, 15, 0},
    {0x89, (uint8_t []){0x00, 0x00, 0x00, 0x0F}, 4, 0},
    {0x8A, (uint8_t []){0x13, 0x00, 0x2C, 0x00, 0x00, 0x2C, 0x10, 0x10, 0x00, 0x3E, 0x19}, 11, 0},
    {0x8B, (uint8_t []){0x15, 0xB1, 0xB1, 0x44, 0x96, 0x2C, 0x10, 0x97, 0x8E, 0x20}, 10, 0},
    {0x8C, (uint8_t []){0x1D, 0xB1, 0xB1, 0x44, 0x96, 0x2C, 0x10, 0x50, 0x0F, 0x01, 0xC5, 0x12, 0x09}, 13, 0},
    {0x8D, (uint8_t []){0x0C, 0x17}, 2, 0},
    {0x8E, (uint8_t []){0x33, 0x01, 0x0C, 0x13, 0x01, 0x01}, 6, 0},
    {0xB3, (uint8_t []){0x00, 0xF0}, 2, 0},

    {0xF1, (uint8_t []){0x00}, 1, 0},
    {0x71, (uint8_t []){0xD3}, 1, 0},
    {0x66, (uint8_t []){0x02, 0x3F}, 2, 0},
    {0xBE, (uint8_t []){0x30, 0x00, 0xED}, 3, 0},
    {0x70, (uint8_t []){0x11, 0x9C, 0x11, 0xE0, 0xE0, 0x00, 0x08, 0x78, 0x10, 0x00, 0x00, 0x1A}, 12, 0},
    {0x90, (uint8_t []){0x04, 0x44, 0x55, 0x7A, 0x00, 0x40, 0x41, 0x65, 0x65}, 9, 0},
    {0x91, (uint8_t []){0x04, 0x44, 0x55, 0x7B, 0x00, 0x40, 0x40, 0x65, 0x65}, 9, 0},
    {0x92, (uint8_t []){0x04, 0x44, 0x55, 0x7C, 0x00, 0x00, 0x3F, 0x05, 0x65, 0x65}, 10, 0},
    {0x93, (uint8_t []){0x04, 0x43, 0x55, 0x7D, 0x00, 0x00, 0x3E, 0x05, 0x65, 0x65}, 10, 0},
    {0x94, (uint8_t []){0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 6, 0},
    {0x95, (uint8_t []){0x9A, 0x1A, 0x00, 0x00, 0xFF}, 5, 0},
    {0x96, (uint8_t []){0x44, 0x53, 0x07, 0x16, 0x20, 0x20, 0x05, 0x04, 0x65, 0x65, 0x00, 0x44}, 12, 0},
    {0x97, (uint8_t []){0x44, 0x53, 0x25, 0x34, 0x20, 0x21, 0x03, 0x02, 0x65, 0x65, 0x00, 0x44}, 12, 0},
    {0xBA, (uint8_t []){0x55, 0x65, 0x65, 0x65, 0x65}, 5, 0},
    {0x9A, (uint8_t []){0x40, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00}, 7, 0},
    {0x9B, (uint8_t []){0x40, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00}, 7, 0},
    {0x9C, (uint8_t []){0x40, 0x12, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x10, 0x12, 0x00, 0x00, 0x00}, 13, 0},
    {0x9D, (uint8_t []){0x80, 0x51, 0x00, 0x00, 0x00, 0x80, 0x1E, 0x01}, 8, 0},
    {0x9E, (uint8_t []){0x51, 0x00, 0x00, 0x00, 0x80, 0x1E, 0x01}, 7, 0},
    {0xB4, (uint8_t []){0x11, 0x15, 0x08, 0x0A, 0x03, 0x01, 0x12, 0x16, 0x1E, 0x1E, 0x1E, 0x1E}, 12, 0},
    {0xB5, (uint8_t []){0x17, 0x13, 0x0B, 0x09, 0x00, 0x02, 0x14, 0x10, 0x1E, 0x1E, 0x1E, 0x1E}, 12, 0},
    {0xB6, (uint8_t []){0x99, 0x99, 0x00, 0x03, 0xFF, 0x00, 0xFF}, 7, 0},
    {0x86, (uint8_t []){0xCC, 0x04, 0xB1, 0x02, 0x58, 0x12, 0x58, 0x0E, 0x13, 0x01, 0xA5, 0x00, 0xA5, 0xA5}, 14, 0},
    {0xB7, (uint8_t []){0x00, 0x0E, 0x0E, 0x0E, 0x07, 0x07, 0x37, 0x04, 0x04, 0x4C, 0x08, 0x14, 0x14, 0x2F, 0x35, 0x0F}, 16, 0},
    {0xB8, (uint8_t []){0x00, 0x0E, 0x0E, 0x0E, 0x06, 0x06, 0x36, 0x03, 0x03, 0x4C, 0x08, 0x14, 0x14, 0x2E, 0x34, 0x0F}, 16, 0},
    {0xB9, (uint8_t []){0x23, 0x23}, 2, 0},
    {0xBF, (uint8_t []){0x0F, 0x13, 0x13, 0x09, 0x09, 0x09}, 6, 0},

    {0xF2, (uint8_t []){0x00}, 1, 0},
    {0x73, (uint8_t []){0x04, 0xDA, 0x12, 0x5A, 0x39}, 5, 0},
    {0x77, (uint8_t []){0x6B, 0x5B, 0xFD, 0xC3, 0xC5}, 5, 0},
    {0x7A, (uint8_t []){0x15, 0x27}, 2, 0},
    {0x7B, (uint8_t []){0x04, 0x57}, 2, 0},
    {0x7E, (uint8_t []){0x01, 0x0E}, 2, 0},
    {0xBF, (uint8_t []){0x36}, 1, 0},
    {0xE3, (uint8_t []){0x40, 0x40}, 2, 0},

    {0xF0, (uint8_t []){0x00}, 1, 0},
    {0xD0, (uint8_t []){0x02}, 1, 0},
    {0x21, (uint8_t []){0x00}, 0, 0},
    {0x11, (uint8_t []){0x00}, 0, 120},
    {0x29, (uint8_t []){0x00}, 0, 0},
};

static esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    example_bsp_enable_dsi_phy_power();
    example_bsp_init_lcd_backlight();
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);

    /* 1. 创建 MIPI DSI 总线（同时初始化 DSI PHY） */
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = 1,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = 500,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus), err, TAG, "create MIPI DSI bus failed");

    /* 2. 通过 DBI 接口发送 LCD 命令/参数 */
    ESP_LOGI(TAG, "Install MIPI DSI LCD control panel");
    esp_lcd_dbi_io_config_t dbi_config = ST77922_MIPI_PANEL_IO_DBI_CONFIG();
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &io_handle), err, TAG, "create panel IO failed");

    /* 3. 配置 DPI（像素流）接口 */
    esp_lcd_dpi_panel_config_t dpi_config = {
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = 15,
        .virtual_channel = 0,
        .pixel_format = MIPI_DPI_PX_FORMAT,
        .num_fbs = 1,
        .video_timing = {
            .h_size = EXAMPLE_LCD_H_RES,
            .v_size = EXAMPLE_LCD_V_RES,
            .hsync_back_porch = 10,
            .hsync_pulse_width = 2,
            .hsync_front_porch = 10,
            .vsync_back_porch = 6,
            .vsync_pulse_width = 2,
            .vsync_front_porch = 120,
        },
        .flags.use_dma2d = true,
    };

    /* 4. 创建 ST77922 控制面板 */
    st77922_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(st77922_lcd_init_cmd_t),
        .flags.use_mipi_interface = 1,
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BSP_LCD_COLOR_DEPTH,
        .vendor_config = &vendor_config,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st77922(io_handle, &panel_config, &lcd_panel), err, TAG, "create ST77922 panel failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_reset(lcd_panel), err, TAG, "reset panel failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_init(lcd_panel), err, TAG, "init panel failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_disp_on_off(lcd_panel, true), err, TAG, "turn on display failed");

    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    return ret;

err:
    if (lcd_panel) {
        esp_lcd_panel_del(lcd_panel);
    }
    return ret;
}

static esp_err_t app_touch_init(void)
{
    /* 初始化 I2C 主机（新版 i2c_master 驱动） */
    i2c_master_bus_handle_t i2c_bus = NULL;
    const i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = EXAMPLE_TOUCH_I2C_NUM,
        .sda_io_num = EXAMPLE_TOUCH_I2C_SDA,
        .scl_io_num = EXAMPLE_TOUCH_I2C_SCL,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &i2c_bus), TAG, "I2C bus create failed");

    /* 初始化 ST7123 触摸控制器 */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_ST7123_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &tp_io_handle), TAG, "create touch panel IO failed");
    return esp_lcd_touch_new_i2c_st7123(tp_io_handle, &tp_cfg, &touch_handle);
}

static esp_err_t app_lvgl_init(void)
{
    /* 初始化 LVGL port（内部创建 LVGL 任务与 tick） */
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* 添加显示屏 */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = lcd_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = true,
            .sw_rotate = true,
            .swap_bytes = false,
            .full_refresh = false,
            .direct_mode = false,
        },
    };

    const lvgl_port_display_dsi_cfg_t dpi_cfg = {
        .flags = {
#if CONFIG_BSP_DISPLAY_LVGL_AVOID_TEAR
            .avoid_tearing = true,
#else
            .avoid_tearing = false,
#endif
        },
    };

    lvgl_disp = lvgl_port_add_disp_dsi(&disp_cfg, &dpi_cfg);

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handle,
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

esp_err_t bsp_display_init(void)
{
    ESP_RETURN_ON_ERROR(app_lcd_init(), TAG, "LCD init failed");
    ESP_RETURN_ON_ERROR(app_touch_init(), TAG, "touch init failed");
    ESP_RETURN_ON_ERROR(app_lvgl_init(), TAG, "LVGL init failed");
    return ESP_OK;
}
