/*
 * Cadence High-Definition Multimedia Interface (HDMI) driver
 *
 * Copyright 2019-2022 NXP
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef CDNS_MHDP_IMX_H_
#define CDNS_MHDP_IMX_H_

#include <drm/bridge/cdns-mhdp.h>
#include <drm/drm_encoder.h>
#include <linux/spinlock.h>

struct imx_mhdp_device;

struct imx_hdp_clks {
	struct clk *av_pll;
	struct clk *dig_pll;
	struct clk *clk_ipg;
	struct clk *clk_core;
	struct clk *clk_pxl;
	struct clk *clk_pxl_mux;
	struct clk *clk_pxl_link;

	struct clk *lpcg_hdp;
	struct clk *lpcg_msi;
	struct clk *lpcg_pxl;
	struct clk *lpcg_vif;
	struct clk *lpcg_lis;
	struct clk *lpcg_apb;
	struct clk *lpcg_apb_csr;
	struct clk *lpcg_apb_ctrl;

	struct clk *lpcg_i2s;
	struct clk *clk_i2s_bypass;
};

struct imx_mhdp_device {
	struct cdns_mhdp_device mhdp;
	struct drm_encoder encoder;

	//struct mutex audio_mutex;
	spinlock_t audio_lock;
	bool connected;
	bool active;
	bool suspended;
	struct imx_hdp_clks clks;
	const struct firmware *fw;
	const char *firmware_name;

	int bus_type;

	struct device		*pd_mhdp_dev;
	struct device		*pd_pll0_dev;
	struct device		*pd_pll1_dev;
	struct device_link	*pd_mhdp_link;
	struct device_link	*pd_pll0_link;
	struct device_link	*pd_pll1_link;
};

void cdns_mhdp_imx_encoder_disable(struct drm_encoder *encoder);
void cdns_mhdp_imx_encoder_enable(struct drm_encoder *encoder);
int cdns_mhdp_imx_bind(struct device *dev, struct device *master,
	void *data);
void cdns_mhdp_imx_unbind(struct device *dev, struct device *master,
	void *data);
#endif /* CDNS_MHDP_IMX_H_ */
