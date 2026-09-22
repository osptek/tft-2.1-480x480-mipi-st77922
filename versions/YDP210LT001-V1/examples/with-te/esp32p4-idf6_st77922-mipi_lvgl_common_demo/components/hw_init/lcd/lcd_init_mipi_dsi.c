/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "hw_init.h"

#if CONFIG_EXAMPLE_LCD_INTERFACE_MIPI_DSI

#include "esp_ldo_regulator.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st77922.h"
#include "driver/gpio.h"

static const char *TAG = "hw_lcd_init";

#define HW_LDO_MIPI_CHAN                        (3)
#define HW_LDO_MIPI_VOLTAGE_MV                  (2500)
#define HW_LCD_BIT_PER_PIXEL                    (16)
#define HW_MIPI_DPI_PX_FORMAT                   (LCD_COLOR_FMT_RGB565)
#define HW_PIN_NUM_LCD_RST                      (-1)

static esp_lcd_dsi_bus_handle_t s_mipi_dsi_bus;
static esp_lcd_panel_io_handle_t s_mipi_dbi_io;
static esp_lcd_panel_handle_t s_panel_handle;

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

static void lcd_ldo_power_on(void)
{
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = HW_LDO_MIPI_CHAN,
        .voltage_mv = HW_LDO_MIPI_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
}

esp_err_t hw_lcd_init(esp_lcd_panel_handle_t *panel_handle, esp_lcd_panel_io_handle_t *io_handle, esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode, esp_lv_adapter_rotation_t rotation)
{
    lcd_ldo_power_on();

    ESP_LOGD(TAG, "Install LCD driver");
    esp_lcd_dsi_bus_config_t bus_config = {                    \
        .bus_id = 0,                                           \
        .num_data_lanes = 1,                                   \
        .phy_clk_src = 0,                                      \
        .lane_bit_rate_mbps = 500,                             \
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &s_mipi_dsi_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_dbi_io_config_t dbi_config = ST77922_MIPI_PANEL_IO_DBI_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(s_mipi_dsi_bus, &dbi_config, &s_mipi_dbi_io));

    ESP_LOGI(TAG, "Install LCD driver of st77922");
    esp_lcd_dpi_panel_config_t dpi_config = {                 \
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,          \
        .dpi_clock_freq_mhz = 15,                             \
        .virtual_channel = 0,                                 \
        .in_color_format = HW_MIPI_DPI_PX_FORMAT,             \
        .num_fbs = 1,                                         \
        .video_timing = {                                     \
            .h_size = HW_LCD_H_RES,                           \
            .v_size = HW_LCD_V_RES,                           \
            .hsync_back_porch = 10,                           \
            .hsync_pulse_width = 2,                           \
            .hsync_front_porch = 10,                          \
            .vsync_back_porch = 6,                            \
            .vsync_pulse_width = 2,                           \
            .vsync_front_porch = 120,                         \
        },                                                    \
    };

    dpi_config.num_fbs = esp_lv_adapter_get_required_frame_buffer_count(tear_avoid_mode, rotation);
    st77922_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,      // Uncomment these line if use custom initialization commands
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(st77922_lcd_init_cmd_t),
        .flags.use_mipi_interface = 1,
        .mipi_config = {
            .dsi_bus = s_mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = HW_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = HW_LCD_BIT_PER_PIXEL,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st77922(s_mipi_dbi_io, &panel_config, &s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));

    *panel_handle = s_panel_handle;
    if (io_handle) {
        *io_handle = s_mipi_dbi_io;
    }

    return ESP_OK;
}

#endif
