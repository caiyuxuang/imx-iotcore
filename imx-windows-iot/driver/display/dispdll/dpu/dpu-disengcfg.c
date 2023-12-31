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

#include <drm/drm_mode.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include "dpu-prv.h"

#define CLOCKCTRL		0x8
typedef enum {
	DSPCLKDIVIDE__DIV1,	/* Ext disp clk signal has pix clk freq. */
	DSPCLKDIVIDE__DIV2,	/* Ext disp clk signal has 2x the pix clk freq. */
} clkdivide_t;
#define POLARITYCTRL		0xC
#define POLHS_HIGH		BIT(0)
#define POLVS_HIGH		BIT(1)
#define POLEN_HIGH		BIT(2)
#define PIXINV_INV		BIT(3)
#define SRCSELECT		0x10
#define SIG_SELECT_MASK		0x3

struct dpu_disengcfg {
	char __iomem *base;
	struct mutex mutex;
	int id;
	bool inuse;
	struct dpu_soc *dpu;
};

static inline u32 dpu_dec_read(struct dpu_disengcfg *dec, unsigned int offset)
{
	return readl(dec->base + offset);
}

static inline void dpu_dec_write(struct dpu_disengcfg *dec,
				 unsigned int offset, u32 value)
{
	writel(value, dec->base + offset);
}

void disengcfg_sig_select(struct dpu_disengcfg *dec, dec_sig_sel_t sig_sel)
{
	u32 val;

	mutex_lock(&dec->mutex);
	val = dpu_dec_read(dec, SRCSELECT);
	val &= ~SIG_SELECT_MASK;
	val |= sig_sel;
	dpu_dec_write(dec, SRCSELECT, val);
	mutex_unlock(&dec->mutex);
}

struct dpu_disengcfg *dpu_dec_get(struct dpu_soc *dpu, int id)
{
	struct dpu_disengcfg *dec;
	int i;

	for (i = 0; i < ARRAY_SIZE(dec_ids); i++)
		if ((int)dec_ids[i] == id)
			break;

	if (i == ARRAY_SIZE(dec_ids))
		return ERR_PTR(-EINVAL);

	dec = dpu->dec_priv[i];

	mutex_lock(&dec->mutex);

	if (dec->inuse) {
		mutex_unlock(&dec->mutex);
		return ERR_PTR(-EBUSY);
	}

	dec->inuse = true;

	mutex_unlock(&dec->mutex);

	return dec;
}

void dpu_dec_put(struct dpu_disengcfg *dec)
{
	mutex_lock(&dec->mutex);

	dec->inuse = false;

	mutex_unlock(&dec->mutex);
}

struct dpu_disengcfg *dpu_aux_dec_peek(struct dpu_disengcfg *dec)
{
	return dec->dpu->dec_priv[dec->id ^ 1];
}

void _dpu_dec_init(struct dpu_soc *dpu, unsigned int id)
{
	struct dpu_disengcfg *dec;
	u32 val;
	int i;

	for (i = 0; i < ARRAY_SIZE(dec_ids); i++)
		if (ed_ids[i] == id)
			break;

	if (i == ARRAY_SIZE(dec_ids)) {
		WARN_ON(1);
		return;
	}

	dec = dpu->dec_priv[i];

	val = dpu_dec_read(dec, POLARITYCTRL);
	val &= ~POLHS_HIGH;
	val &= ~POLVS_HIGH;
	dpu_dec_write(dec, POLARITYCTRL, val);

	disengcfg_sig_select(dec, DEC_SIG_SEL_FRAMEGEN);
}

int dpu_dec_init(struct dpu_soc *dpu, unsigned int id,
			unsigned long unused, unsigned long base)
{
	struct dpu_disengcfg *dec;

	dec = devm_kzalloc(dpu->dev, sizeof(*dec), GFP_KERNEL);
	if (!dec)
		return -ENOMEM;

	dpu->dec_priv[id] = dec;

	dec->base = devm_ioremap(dpu->dev, base, SZ_16);
	if (!dec->base)
		return -ENOMEM;

	dec->dpu = dpu;
	dec->id = id;
	mutex_init(&dec->mutex);

	_dpu_dec_init(dpu, id);

	return 0;
}

int dpu_dec_deinit(struct dpu_soc *dpu, unsigned int id)
{
	struct dpu_disengcfg *dec = dpu->dec_priv[id];
	if (!dec)
		return -ENOMEM;

	if (dec->base)
		devm_iounmap(dpu->dev, dec->base, SZ_16);

	kfree(dec);

	return 0;
}
