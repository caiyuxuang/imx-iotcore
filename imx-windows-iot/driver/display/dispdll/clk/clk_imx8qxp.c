/*
 * Copyright 2022 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <ntddk.h>
#include <stddef.h>
#include <dt-bindings/firmware/imx/rsrc.h>
#include "clk_imx8qxp.h"
#include "clk_imx8qxp_lpcg.h"
#include "linux/dev_printk.h"
#include "linux/kernel.h"

/* Main object of the clock tree - single instance for all monitors */
static struct imx8qxp_clk_device clktree;

static struct clk *dc0_sels[5];

static void fill_dc0_sels(struct clk **sels, struct clk **clks)
{
    sels[0] = clks[IMX8QXP_CLK_DUMMY];
    sels[1] = clks[IMX8QXP_CLK_DUMMY];
    sels[2] = clks[IMX8QXP_DC0_PLL0_CLK];
    sels[3] = clks[IMX8QXP_DC0_PLL1_CLK];
    sels[4] = clks[IMX8QXP_DC0_BYPASS0_CLK];
}

struct clk *clk_get_item_imx8qxp(int index)
{
    if (index < 0 || index >= IMX8QXP_DC_CLK_END) {
        return NULL;
    }
    return clktree.clks[index];
}

/* --------------------------- High-level clock configuration functions --------------------------------*/

struct imx8qxp_clk_device *clk_init_imx8qxp()
{
    struct imx8qxp_clk_device *dev = &clktree;

    if (!dev) {
        return NULL;
    }

    dev->lpcg_reg = ioremap(IMX_LPCG_BASE, IMX_LPCG_SIZE);
    if (dev->lpcg_reg == NULL) {
        dev_err(NULL, "CLK: mmap of LPCG regs failed!\n");
        return NULL;
    }

    /* clock sources */
    dev->clks[IMX8QXP_CLK_DUMMY] = imx8q_clk_fixed("dummy", NULL, IMX_CLK_FREQ_0K);
    dev->clks[IMX8QXP_DC0_AXI_EXT_CLK] = imx8q_clk_fixed("dc_axi_ext_clk", NULL, IMX_CLK_FREQ_800M);
    dev->clks[IMX8QXP_DC0_AXI_INT_CLK] = imx8q_clk_fixed("dc_axi_int_clk", NULL, IMX_CLK_FREQ_400M);
    dev->clks[IMX8QXP_DC0_CFG_CLK] = imx8q_clk_fixed("dc_cfg_clk", NULL, IMX_CLK_FREQ_100M);

    /* LPCG */
    dev->clks[IMX8QXP_DC0_DISP0_LPCG_CLK] = imx_clk_lpcg_scu("dc0_disp0_lpcg_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x0, 0, false);
    dev->clks[IMX8QXP_DC0_DISP1_LPCG_CLK] = imx_clk_lpcg_scu("dc0_disp1_lpcg_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x0, 4, false);
    dev->clks[IMX8QXP_DC0_PRG0_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg0_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x20, 0, false);
    dev->clks[IMX8QXP_DC0_PRG0_APB_CLK] = imx_clk_lpcg_scu("dc0_prg0_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x20, 16, false);
    dev->clks[IMX8QXP_DC0_PRG1_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg1_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x24, 0, false);
    dev->clks[IMX8QXP_DC0_PRG1_APB_CLK] = imx_clk_lpcg_scu("dc0_prg1_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x24, 16, false);
    dev->clks[IMX8QXP_DC0_PRG2_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg2_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x28, 0, false);
    dev->clks[IMX8QXP_DC0_PRG2_APB_CLK] = imx_clk_lpcg_scu("dc0_prg2_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x28, 16, false);
    dev->clks[IMX8QXP_DC0_PRG3_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg3_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg+ 0x34, 0, false);
    dev->clks[IMX8QXP_DC0_PRG3_APB_CLK] = imx_clk_lpcg_scu("dc0_prg3_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x34, 16, false);
    dev->clks[IMX8QXP_DC0_PRG4_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg4_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x38, 0, false);
    dev->clks[IMX8QXP_DC0_PRG4_APB_CLK] = imx_clk_lpcg_scu("dc0_prg4_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x38, 16, false);
    dev->clks[IMX8QXP_DC0_PRG5_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg5_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x3C, 0, false);
    dev->clks[IMX8QXP_DC0_PRG5_APB_CLK] = imx_clk_lpcg_scu("dc0_prg5_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x3C, 16, false);
    dev->clks[IMX8QXP_DC0_PRG6_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg6_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x40, 0, false);
    dev->clks[IMX8QXP_DC0_PRG6_APB_CLK] = imx_clk_lpcg_scu("dc0_prg6_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x40, 16, false);
    dev->clks[IMX8QXP_DC0_PRG7_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg7_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x44, 0, false);
    dev->clks[IMX8QXP_DC0_PRG7_APB_CLK] = imx_clk_lpcg_scu("dc0_prg7_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x44, 16, false);
    dev->clks[IMX8QXP_DC0_PRG8_RTRAM_CLK] = imx_clk_lpcg_scu("dc0_prg8_rtram_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x48, 0, false);
    dev->clks[IMX8QXP_DC0_PRG8_APB_CLK] = imx_clk_lpcg_scu("dc0_prg8_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x48, 16, false);
    dev->clks[IMX8QXP_DC0_DPR0_APB_CLK] = imx_clk_lpcg_scu("dc0_dpr0_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x18, 16, false);
    dev->clks[IMX8QXP_DC0_DPR0_B_CLK] = imx_clk_lpcg_scu("dc0_dpr0_b_clk", dev->clks[IMX8QXP_DC0_AXI_EXT_CLK], dev->lpcg_reg + 0x18, 20, false);
    dev->clks[IMX8QXP_DC0_DPR1_APB_CLK] = imx_clk_lpcg_scu("dc0_dpr1_apb_clk", dev->clks[IMX8QXP_DC0_CFG_CLK], dev->lpcg_reg + 0x2C, 16, false);
    dev->clks[IMX8QXP_DC0_DPR1_B_CLK] = imx_clk_lpcg_scu("dc0_dpr1_b_clk", dev->clks[IMX8QXP_DC0_AXI_EXT_CLK], dev->lpcg_reg + 0x2C, 20, false);
    dev->clks[IMX8QXP_DC0_RTRAM0_CLK] = imx_clk_lpcg_scu("dc0_rtram0_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x1C, 0, false);
    dev->clks[IMX8QXP_DC0_RTRAM1_CLK] = imx_clk_lpcg_scu("dc0_rtram1_clk", dev->clks[IMX8QXP_DC0_AXI_INT_CLK], dev->lpcg_reg + 0x30, 0, false);

    /* Display controller SS */
    dev->clks[IMX8QXP_DC0_PLL0_CLK] = imx_clk_scu("dc0_pll0_clk", IMX_SC_R_DC_0_PLL_0, IMX_SC_PM_CLK_PLL);
    dev->clks[IMX8QXP_DC0_PLL1_CLK] = imx_clk_scu("dc0_pll1_clk", IMX_SC_R_DC_0_PLL_1, IMX_SC_PM_CLK_PLL);
    dev->clks[IMX8QXP_DC0_BYPASS0_CLK] = imx_clk_scu("dc0_bypass0_clk", IMX_SC_R_DC_0_VIDEO0, IMX_SC_PM_CLK_BYPASS);
    dev->clks[IMX8QXP_DC0_BYPASS1_CLK] = imx_clk_scu("dc0_bypass1_clk", IMX_SC_R_DC_0_VIDEO1, IMX_SC_PM_CLK_BYPASS);

    fill_dc0_sels(dc0_sels, dev->clks);

    dev->clks[IMX8QXP_DC0_DISP0_CLK] = imx_clk_scu2("dc0_disp0_clk", dc0_sels, ARRAY_SIZE(dc0_sels), IMX_SC_R_DC_0, IMX_SC_PM_CLK_MISC0);
    dev->clks[IMX8QXP_DC0_DISP1_CLK] = imx_clk_scu2("dc0_disp1_clk", dc0_sels, ARRAY_SIZE(dc0_sels), IMX_SC_R_DC_0, IMX_SC_PM_CLK_MISC1);

    /* MIPI-LVDS SS */
    dev->clks[IMX8QXP_LVDS0_PIX_CLK] = imx_clk_scu("lvds0_pixel_clk", IMX_SC_R_LVDS_0, IMX_SC_PM_CLK_MISC2);
    dev->clks[IMX8QXP_LVDS0_BYPASS_CLK] = imx_clk_scu("lvds0_bypass_clk", IMX_SC_R_LVDS_0, IMX_SC_PM_CLK_BYPASS);
    dev->clks[IMX8QXP_LVDS0_PHY_CLK] = imx_clk_scu("lvds0_phy_clk", IMX_SC_R_LVDS_0, IMX_SC_PM_CLK_MISC3);

    dev->clks[IMX8QXP_LVDS1_PIX_CLK] = imx_clk_scu("lvds1_pixel_clk", IMX_SC_R_LVDS_1, IMX_SC_PM_CLK_MISC2);
    dev->clks[IMX8QXP_LVDS1_BYPASS_CLK] = imx_clk_scu("lvds1_bypass_clk", IMX_SC_R_LVDS_1, IMX_SC_PM_CLK_BYPASS);
    dev->clks[IMX8QXP_LVDS1_PHY_CLK] = imx_clk_scu("lvds1_phy_clk", IMX_SC_R_LVDS_1, IMX_SC_PM_CLK_MISC3);

    return dev;
}

void clk_dump_clock_tree_imx8qxp(const struct imx8qxp_clk_device *dev)
{
    if (!dev) {
        return;
    }

    for (int i = 0; i < IMX8QXP_DC_CLK_END; i++) {
        imx8q_clk_dump(dev->clks[i]);
    }
}

int clk_deinit_imx8qxp(struct imx8qxp_clk_device *dev)
{
    if (!dev) {
        return -1;
    }

    for (int i = 0; i < IMX8QXP_DC_CLK_END; i++) {
        if (dev->clks[i] != NULL) {
            imx8q_clk_deinit(dev->clks[i]);
        }
    }

    if (dev->lpcg_reg) {
        iounmap(dev->lpcg_reg, IMX_LPCG_SIZE);
    }

    return 0;
}
