/*
 * Copyright 2017-2019 NXP
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

#ifndef PHY_MIXEL_LVDS_COMBO_H_
#define PHY_MIXEL_LVDS_COMBO_H_

#include "phy.h"

void mixel_phy_combo_lvds_set_phy_speed(struct phy *phy,
					unsigned long phy_clk_rate);
void mixel_phy_combo_lvds_set_hsync_pol(struct phy *phy, bool active_high);
void mixel_phy_combo_lvds_set_vsync_pol(struct phy *phy, bool active_high);
int mixel_lvds_combo_phy_init(struct phy *phy);
int mixel_lvds_combo_phy_power_on(struct phy *phy);
int mixel_lvds_combo_phy_power_off(struct phy *phy);
int mixel_lvds_combo_phy_probe(struct platform_device *pdev);
int mixel_lvds_combo_phy_remove(struct platform_device *pdev, bool stop);

#endif	/* PHY_MIXEL_LVDS_COMBO_H_ */
