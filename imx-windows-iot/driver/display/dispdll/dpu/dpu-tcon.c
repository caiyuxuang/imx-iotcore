/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2020, 2022 NXP
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <linux/io.h>
#include <linux/media-bus-format.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <video/dpu.h>
#include <video/imx8-pc.h>
#include "dpu-prv.h"

#define SSQCNTS			0
#define SSQCYCLE		0x8
#define SWRESET			0xC
#define TCON_CTRL		0x10
#define BYPASS			BIT(3)
#define RSDSINVCTRL		0x14
#define MAPBIT3_0		0x18
#define MAPBIT7_4		0x1C
#define MAPBIT11_8		0x20
#define MAPBIT15_12		0x24
#define MAPBIT19_16		0x28
#define MAPBIT23_20		0x2C
#define MAPBIT27_24		0x30
#define MAPBIT31_28		0x34
#define MAPBIT34_32		0x38
#define MAPBIT3_0_DUAL		0x3C
#define MAPBIT7_4_DUAL		0x40
#define MAPBIT11_8_DUAL		0x44
#define MAPBIT15_12_DUAL	0x48
#define MAPBIT19_16_DUAL	0x4C
#define MAPBIT23_20_DUAL	0x50
#define MAPBIT27_24_DUAL	0x54
#define MAPBIT31_28_DUAL	0x58
#define MAPBIT34_32_DUAL	0x5C
#define SPGPOSON(n)		(0x60 + (n) * 16)
#define X(n)			(((n) & 0x7FFF) << 16)
#define Y(n)			((n) & 0x7FFF)
#define SPGMASKON(n)		(0x64 + (n) * 16)
#define SPGPOSOFF(n)		(0x68 + (n) * 16)
#define SPGMASKOFF(n)		(0x6C + (n) * 16)
#define SMXSIGS(n)		(0x120 + (n) * 8)
#define SMXFCTTABLE(n)		(0x124 + (n) * 8)
#define RESET_OVER_UNFERFLOW	0x180
#define DUAL_DEBUG		0x184

struct dpu_tcon {
	char __iomem *base;
	struct mutex mutex;
	int id;
	bool inuse;
	struct dpu_soc *dpu;
	struct pc *pc;
};

static inline u32 dpu_tcon_read(struct dpu_tcon *tcon, unsigned int offset)
{
	return readl(tcon->base + offset);
}

static inline void dpu_tcon_write(struct dpu_tcon *tcon,
				  unsigned int offset, u32 value)
{
	writel(value, tcon->base + offset);
}

int tcon_set_fmt(struct dpu_tcon *tcon, u32 bus_format)
{
	switch (bus_format) {
	case MEDIA_BUS_FMT_RGB888_1X24:
		dpu_tcon_write(tcon, MAPBIT3_0,   0x19181716);
		dpu_tcon_write(tcon, MAPBIT7_4,   0x1d1c1b1a);
		dpu_tcon_write(tcon, MAPBIT11_8,  0x0f0e0d0c);
		dpu_tcon_write(tcon, MAPBIT15_12, 0x13121110);
		dpu_tcon_write(tcon, MAPBIT19_16, 0x05040302);
		dpu_tcon_write(tcon, MAPBIT23_20, 0x09080706);
		break;
	case MEDIA_BUS_FMT_RGB101010_1X30:
	case MEDIA_BUS_FMT_RGB888_1X30_PADLO:
	case MEDIA_BUS_FMT_RGB666_1X30_PADLO:
	case MEDIA_BUS_FMT_RGB565_1X30_PADLO:
		dpu_tcon_write(tcon, MAPBIT3_0,   0x17161514);
		dpu_tcon_write(tcon, MAPBIT7_4,   0x1b1a1918);
		dpu_tcon_write(tcon, MAPBIT11_8,  0x0b0a1d1c);
		dpu_tcon_write(tcon, MAPBIT15_12, 0x0f0e0d0c);
		dpu_tcon_write(tcon, MAPBIT19_16, 0x13121110);
		dpu_tcon_write(tcon, MAPBIT23_20, 0x03020100);
		dpu_tcon_write(tcon, MAPBIT27_24, 0x07060504);
		dpu_tcon_write(tcon, MAPBIT31_28, 0x00000908);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/* This function is used to workaround TKT320590 which is related to DPR/PRG. */
void tcon_set_operation_mode(struct dpu_tcon *tcon)
{
	u32 val;

	val = dpu_tcon_read(tcon, TCON_CTRL);
	val &= ~BYPASS;
	dpu_tcon_write(tcon, TCON_CTRL, val);
}

void tcon_cfg_videomode(struct dpu_tcon *tcon,
			struct drm_display_mode *m, bool side_by_side)
{
	u32 val;
	int hdisplay, hsync_start, hsync_end;
	int vdisplay, vsync_start, vsync_end;
	int y;

	hdisplay = m->hdisplay;
	vdisplay = m->vdisplay;
	hsync_start = m->hsync_start;
	vsync_start = m->vsync_start;
	hsync_end = m->hsync_end;
	vsync_end = m->vsync_end;

	if (side_by_side) {
		hdisplay /= 2;
		hsync_start /= 2;
		hsync_end /= 2;
	}

	/*
	 * TKT320590:
	 * Turn TCON into operation mode later after the first dumb frame is
	 * generated by DPU.  This makes DPR/PRG be able to evade the frame.
	 */
	val = dpu_tcon_read(tcon, TCON_CTRL);
	val |= BYPASS;
	dpu_tcon_write(tcon, TCON_CTRL, val);

	/* dsp_control[0]: hsync */
	dpu_tcon_write(tcon, SPGPOSON(0), X(hsync_start));
	dpu_tcon_write(tcon, SPGMASKON(0), 0xffff);

	dpu_tcon_write(tcon, SPGPOSOFF(0), X(hsync_end));
	dpu_tcon_write(tcon, SPGMASKOFF(0), 0xffff);

	dpu_tcon_write(tcon, SMXSIGS(0), 0x2);
	dpu_tcon_write(tcon, SMXFCTTABLE(0), 0x1);

	/* dsp_control[1]: vsync */
	dpu_tcon_write(tcon, SPGPOSON(1), X(hsync_start) | Y(vsync_start - 1));
	dpu_tcon_write(tcon, SPGMASKON(1), 0x0);

	dpu_tcon_write(tcon, SPGPOSOFF(1), X(hsync_start) | Y(vsync_end - 1));
	dpu_tcon_write(tcon, SPGMASKOFF(1), 0x0);

	dpu_tcon_write(tcon, SMXSIGS(1), 0x3);
	dpu_tcon_write(tcon, SMXFCTTABLE(1), 0x1);

	/* dsp_control[2]: data enable */
	/* horizontal */
	dpu_tcon_write(tcon, SPGPOSON(2), 0x0);
	dpu_tcon_write(tcon, SPGMASKON(2), 0xffff);

	dpu_tcon_write(tcon, SPGPOSOFF(2), X(hdisplay));
	dpu_tcon_write(tcon, SPGMASKOFF(2), 0xffff);

	/* vertical */
	dpu_tcon_write(tcon, SPGPOSON(3), 0x0);
	dpu_tcon_write(tcon, SPGMASKON(3), 0x7fff0000);

	dpu_tcon_write(tcon, SPGPOSOFF(3), Y(vdisplay));
	dpu_tcon_write(tcon, SPGMASKOFF(3), 0x7fff0000);

	dpu_tcon_write(tcon, SMXSIGS(2), 0x2c);
	dpu_tcon_write(tcon, SMXFCTTABLE(2), 0x8);

	/* dsp_control[3]: kachuck */
	y = vdisplay + 1;
	/*
	 * If sync mode fixup is present, the kachuck signal from slave tcon
	 * should be one line later than the one from master tcon.
	 */
	if (side_by_side && tcon_is_slave(tcon))
		y++;

	dpu_tcon_write(tcon, SPGPOSON(4), X(0x0) | Y(y));
	dpu_tcon_write(tcon, SPGMASKON(4), 0x0);

	dpu_tcon_write(tcon, SPGPOSOFF(4), X(0x20) | Y(y));
	dpu_tcon_write(tcon, SPGMASKOFF(4), 0x0);

	dpu_tcon_write(tcon, SMXSIGS(3), 0x6);
	dpu_tcon_write(tcon, SMXFCTTABLE(3), 0x2);
}

bool tcon_is_master(struct dpu_tcon *tcon)
{
	const struct dpu_data *data = tcon->dpu->data;

	return tcon->id == (int)data->master_stream_id;
}

bool tcon_is_slave(struct dpu_tcon *tcon)
{
	return !tcon_is_master(tcon);
}

void tcon_configure_pc(struct dpu_tcon *tcon, unsigned int di,
			unsigned int frame_width, u32 mode, u32 format)
{
	if (!tcon || !tcon->pc) {
		WARN_ON(1);
		return;
	}

	/* pc is unused on qxp */
	/* pc_configure(tcon->pc, di, frame_width, mode, format); */
}

void tcon_enable_pc(struct dpu_tcon* tcon)
{
	if (!tcon || !tcon->pc) {
		WARN_ON(1);
		return;
	}

	/* pc is unused on qxp */
	/* pc_enable(tcon->pc); */
}

void tcon_disable_pc(struct dpu_tcon *tcon)
{
	if (!tcon || !tcon->pc) {
		WARN_ON(1);
		return;
	}

	/* pc is unused on qxp */
	/* pc_disable(tcon->pc); */
}

struct dpu_tcon *dpu_tcon_get(struct dpu_soc *dpu, int id)
{
	struct dpu_tcon *tcon;
	int i;

	for (i = 0; i < ARRAY_SIZE(tcon_ids); i++)
		if ((int)tcon_ids[i] == id)
			break;

	if (i == ARRAY_SIZE(tcon_ids))
		return ERR_PTR(-EINVAL);

	tcon = dpu->tcon_priv[i];

	mutex_lock(&tcon->mutex);

	if (tcon->inuse) {
		mutex_unlock(&tcon->mutex);
		return ERR_PTR(-EBUSY);
	}

	tcon->inuse = true;

	mutex_unlock(&tcon->mutex);

	return tcon;
}

void dpu_tcon_put(struct dpu_tcon *tcon)
{
	mutex_lock(&tcon->mutex);

	tcon->inuse = false;

	mutex_unlock(&tcon->mutex);
}

struct dpu_tcon *dpu_aux_tcon_peek(struct dpu_tcon *tcon)
{
	return tcon->dpu->tcon_priv[tcon->id ^ 1];
}

void _dpu_tcon_init(struct dpu_soc *dpu, unsigned int id)
{
}

int dpu_tcon_init(struct dpu_soc *dpu, unsigned int id,
			unsigned long unused, unsigned long base)
{
	struct dpu_tcon *tcon;

	tcon = devm_kzalloc(dpu->dev, sizeof(*tcon), GFP_KERNEL);
	if (!tcon)
		return -ENOMEM;

	dpu->tcon_priv[id] = tcon;

	tcon->base = devm_ioremap(dpu->dev, base, SZ_512);
	if (!tcon->base)
		return -ENOMEM;

	tcon->dpu = dpu;
	mutex_init(&tcon->mutex);

	return 0;
}

int dpu_tcon_deinit(struct dpu_soc *dpu, unsigned int id)
{
	struct dpu_tcon *tcon = dpu->tcon_priv[id];
	if (!tcon)
		return -ENOMEM;

	if (tcon->base)
		devm_iounmap(dpu->dev, tcon->base, SZ_512);

	kfree(tcon);

	return 0;
}

void tcon_get_pc(struct dpu_tcon *tcon, void *data)
{
	if (!tcon) {
		WARN_ON(1);
		return;
	}

	tcon->pc = data;
}