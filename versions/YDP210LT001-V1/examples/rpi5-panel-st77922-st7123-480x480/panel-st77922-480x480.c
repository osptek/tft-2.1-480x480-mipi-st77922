/*
 * ST77922 480x480 1-Lane MIPI-DSI Panel Driver for Raspberry Pi 5
 * Adapted from CNflysky's panel-rpi-dsi-display.c style
 *
 * This example is from the open-source sharing by engineers of Yuying Optoelectronics (鱼鹰光电)
 * on Github.com/osptek. Welcome to provide improvement suggestions.
 *
 * 本例程来源于鱼鹰光电的工程师的开源分享 Github.com/osptek，欢迎提出改进意见
 */

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <video/mipi_display.h>

struct power_on_timing {
	unsigned long post_reset;
	unsigned long reset_low;
	unsigned long after_reset;
	unsigned long slpout;
};

struct st77922_desc {
	const struct drm_display_mode *mode;
	unsigned int lanes;
	unsigned long flags;
	enum mipi_dsi_pixel_format format;
	int (*init_sequence)(struct mipi_dsi_device *dsi);
	const struct power_on_timing *pwr_timing;
	bool do_sw_reset;
};

struct st77922 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	const struct st77922_desc *desc;
	struct gpio_desc *reset;
	enum drm_panel_orientation orientation;
};

static inline struct st77922 *to_st77922(struct drm_panel *panel)
{
	return container_of(panel, struct st77922, panel);
}

/* ==================== 你的初始化序列 ==================== */
static int st77922_480x480_init_sequence(struct mipi_dsi_device *dsi)
{
	struct mipi_dsi_multi_context ctx = { .dsi = dsi };

	mipi_dsi_dcs_write_seq_multi(&ctx, 0xF1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x60, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x65, 0x80);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x79, 0x06, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x7B, 0x00, 0x08, 0x08);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x80, 0x55, 0x62, 0x2F, 0x17, 0xF0, 0x52, 0x70, 0xD2,
				     0x52, 0x62, 0xEA, 0xCF);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x81, 0x26, 0x52, 0x72, 0x27);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x84, 0x92, 0x25);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x87, 0x10, 0x10, 0x58, 0x00, 0x02, 0x3A);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x88, 0x00, 0x00, 0x2C, 0x10, 0x04, 0x00, 0x00, 0x00,
				     0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x06);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x89, 0x00, 0x00, 0x00, 0x0F);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x8A, 0x13, 0x00, 0x2C, 0x00, 0x00, 0x2C, 0x10, 0x10,
				     0x00, 0x3E, 0x19);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x8B, 0x15, 0xB1, 0xB1, 0x44, 0x96, 0x2C, 0x10, 0x97,
				     0x8E, 0x20);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x8C, 0x1D, 0xB1, 0xB1, 0x44, 0x96, 0x2C, 0x10, 0x50,
				     0x0F, 0x01, 0xC5, 0x12, 0x09);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x8D, 0x0C, 0x17);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x8E, 0x33, 0x01, 0x0C, 0x13, 0x01, 0x01);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xB3, 0x00, 0xF0);

	mipi_dsi_dcs_write_seq_multi(&ctx, 0xF1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x71, 0xD3);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x66, 0x02, 0x3F);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xBE, 0x30, 0x00, 0xED);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x70, 0x11, 0x9C, 0x11, 0xE0, 0xE0, 0x00, 0x08, 0x78,
				     0x10, 0x00, 0x00, 0x1A);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x90, 0x04, 0x44, 0x55, 0x7A, 0x00, 0x40, 0x41, 0x65,
				     0x65);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x91, 0x04, 0x44, 0x55, 0x7B, 0x00, 0x40, 0x40, 0x65,
				     0x65);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x92, 0x04, 0x44, 0x55, 0x7C, 0x00, 0x00, 0x3F, 0x05,
				     0x65, 0x65);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x93, 0x04, 0x43, 0x55, 0x7D, 0x00, 0x00, 0x3E, 0x05,
				     0x65, 0x65);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x95, 0x9A, 0x1A, 0x00, 0x00, 0xFF);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x96, 0x44, 0x53, 0x07, 0x16, 0x20, 0x20, 0x05, 0x04,
				     0x65, 0x65, 0x00, 0x44);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x97, 0x44, 0x53, 0x25, 0x34, 0x20, 0x21, 0x03, 0x02,
				     0x65, 0x65, 0x00, 0x44);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xBA, 0x55, 0x65, 0x65, 0x65, 0x65);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x9A, 0x40, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x9B, 0x40, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x9C, 0x40, 0x12, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00,
				     0x10, 0x12, 0x00, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x9D, 0x80, 0x51, 0x00, 0x00, 0x00, 0x80, 0x1E, 0x01);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x9E, 0x51, 0x00, 0x00, 0x00, 0x80, 0x1E, 0x01);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xB4, 0x11, 0x15, 0x08, 0x0A, 0x03, 0x01, 0x12, 0x16,
				     0x1E, 0x1E, 0x1E, 0x1E);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xB5, 0x17, 0x13, 0x0B, 0x09, 0x00, 0x02, 0x14, 0x10,
				     0x1E, 0x1E, 0x1E, 0x1E);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xB6, 0x99, 0x99, 0x00, 0x03, 0xFF, 0x00, 0xFF);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x86, 0xCC, 0x04, 0xB1, 0x02, 0x58, 0x12, 0x58, 0x0E,
				     0x13, 0x01, 0xA5, 0x00, 0xA5, 0xA5);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xB7, 0x00, 0x0E, 0x0E, 0x0E, 0x07, 0x07, 0x37, 0x04,
				     0x04, 0x4C, 0x08, 0x14, 0x14, 0x2F, 0x35, 0x0F);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xB8, 0x00, 0x0E, 0x0E, 0x0E, 0x06, 0x06, 0x36, 0x03,
				     0x03, 0x4C, 0x08, 0x14, 0x14, 0x2E, 0x34, 0x0F);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xB9, 0x23, 0x23);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xBF, 0x0F, 0x13, 0x13, 0x09, 0x09, 0x09);

	mipi_dsi_dcs_write_seq_multi(&ctx, 0xF2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x73, 0x04, 0xDA, 0x12, 0x5A, 0x39);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x77, 0x6B, 0x5B, 0xFD, 0xC3, 0xC5);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x7A, 0x15, 0x27);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x7B, 0x04, 0x57);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x7E, 0x01, 0x0E);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xBF, 0x36);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xE3, 0x40, 0x40);

	mipi_dsi_dcs_write_seq_multi(&ctx, 0xF0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0xD0, 0x02);
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x21);

	/* Sleep Out */
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x11);
	msleep(120);

	/* Display On */
	mipi_dsi_dcs_write_seq_multi(&ctx, 0x29);

	return ctx.accum_err;
}
/* ======================================================= */

static int st77922_prepare(struct drm_panel *panel)
{
	struct st77922 *st77922 = to_st77922(panel);
	struct mipi_dsi_multi_context ctx = { .dsi = st77922->dsi };

	if (st77922->reset) {
		gpiod_set_value_cansleep(st77922->reset, 1);
		msleep(st77922->desc->pwr_timing->post_reset);
		gpiod_set_value_cansleep(st77922->reset, 0);
		msleep(st77922->desc->pwr_timing->reset_low);
		gpiod_set_value_cansleep(st77922->reset, 1);
		msleep(st77922->desc->pwr_timing->after_reset);
	}

	if (st77922->desc->do_sw_reset) {
		mipi_dsi_dcs_soft_reset_multi(&ctx);
		msleep(st77922->desc->pwr_timing->after_reset);
	}

	if (st77922->desc->init_sequence) {
		int ret = st77922->desc->init_sequence(st77922->dsi);
		if (ret)
			return ret;
	}

	mipi_dsi_dcs_exit_sleep_mode_multi(&ctx);
	msleep(st77922->desc->pwr_timing->slpout);

	return ctx.accum_err;
}

static int st77922_enable(struct drm_panel *panel)
{
	struct mipi_dsi_multi_context ctx = { .dsi = to_mipi_dsi_device(panel->dev) };
	mipi_dsi_dcs_set_display_on_multi(&ctx);
	return ctx.accum_err;
}

static int st77922_disable(struct drm_panel *panel)
{
	struct mipi_dsi_multi_context ctx = { .dsi = to_mipi_dsi_device(panel->dev) };
	mipi_dsi_dcs_set_display_off_multi(&ctx);
	return ctx.accum_err;
}

static int st77922_unprepare(struct drm_panel *panel)
{
	struct st77922 *st77922 = to_st77922(panel);
	struct mipi_dsi_multi_context ctx = { .dsi = st77922->dsi };

	mipi_dsi_dcs_enter_sleep_mode_multi(&ctx);
	if (st77922->reset)
		gpiod_set_value_cansleep(st77922->reset, 0);

	return ctx.accum_err;
}

static int st77922_get_modes(struct drm_panel *panel, struct drm_connector *connector)
{
	struct st77922 *st77922 = to_st77922(panel);
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, st77922->desc->mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);
	drm_mode_probed_add(connector, mode);

	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;

	drm_connector_set_orientation_from_panel(connector, panel);
	return 1;
}

static enum drm_panel_orientation st77922_get_orientation(struct drm_panel *panel)
{
	return to_st77922(panel)->orientation;
}

static const struct drm_panel_funcs st77922_funcs = {
	.prepare = st77922_prepare,
	.enable = st77922_enable,
	.disable = st77922_disable,
	.unprepare = st77922_unprepare,
	.get_modes = st77922_get_modes,
	.get_orientation = st77922_get_orientation,
};

static const struct drm_display_mode st77922_mode = {
	.clock = 18300,		/* 18.3 MHz */

	.hdisplay = 480,
	.hsync_start = 480 + 10,		/* hfront_porch = 10 */
	.hsync_end = 480 + 10 + 2,		/* hsync_pulse = 2 */
	.htotal = 480 + 10 + 2 + 10,		/* hback_porch = 10 */

	.vdisplay = 480,
	.vsync_start = 480 + 120,		/* vfront_porch = 120 */
	.vsync_end = 480 + 120 + 2,		/* vsync_pulse = 2 */
	.vtotal = 480 + 120 + 2 + 6,		/* vback_porch = 6 */

	.width_mm = 53,
	.height_mm = 53,

	.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static const struct power_on_timing st77922_pwr_timing = {
	.post_reset = 20,
	.reset_low = 20,
	.after_reset = 120,
	.slpout = 120,
};

static const struct st77922_desc st77922_desc = {
	.mode = &st77922_mode,
	.lanes = 1,
	.flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST | MIPI_DSI_MODE_LPM,
	.format = MIPI_DSI_FMT_RGB888,
	.init_sequence = st77922_480x480_init_sequence,
	.pwr_timing = &st77922_pwr_timing,
	.do_sw_reset = true,
};

static int st77922_probe(struct mipi_dsi_device *dsi)
{
	struct st77922 *st77922;
	const struct st77922_desc *desc;
	int ret;

	st77922 = devm_kzalloc(&dsi->dev, sizeof(*st77922), GFP_KERNEL);
	if (!st77922)
		return -ENOMEM;

	desc = of_device_get_match_data(&dsi->dev);
	dsi->mode_flags = desc->flags;
	dsi->format = desc->format;
	dsi->lanes = desc->lanes;

	st77922->panel.prepare_prev_first = true;
	st77922->reset = devm_gpiod_get_optional(&dsi->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(st77922->reset)) {
		dev_err(&dsi->dev, "Failed to get reset GPIO\n");
		return PTR_ERR(st77922->reset);
	}

	ret = of_drm_get_panel_orientation(dsi->dev.of_node, &st77922->orientation);
	if (ret < 0)
		st77922->orientation = DRM_MODE_PANEL_ORIENTATION_NORMAL;

	drm_panel_init(&st77922->panel, &dsi->dev, &st77922_funcs, DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_of_backlight(&st77922->panel);
	if (ret)
		return ret;

	drm_panel_add(&st77922->panel);

	mipi_dsi_set_drvdata(dsi, st77922);
	st77922->dsi = dsi;
	st77922->desc = desc;

	ret = mipi_dsi_attach(dsi);
	if (ret)
		drm_panel_remove(&st77922->panel);

	return ret;
}

static void st77922_remove(struct mipi_dsi_device *dsi)
{
	struct st77922 *st77922 = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&st77922->panel);
}

static const struct of_device_id st77922_of_match[] = {
	{ .compatible = "sitronix,st77922-480x480", .data = &st77922_desc },
	{ }
};
MODULE_DEVICE_TABLE(of, st77922_of_match);

static struct mipi_dsi_driver st77922_driver = {
	.probe = st77922_probe,
	.remove = st77922_remove,
	.driver = {
		.name = "panel-st77922-480x480",
		.of_match_table = st77922_of_match,
	},
};
module_mipi_dsi_driver(st77922_driver);

MODULE_AUTHOR("Adapted from CNflysky");
MODULE_DESCRIPTION("ST77922 480x480 2-Lane MIPI-DSI Panel Driver");
MODULE_LICENSE("GPL");
