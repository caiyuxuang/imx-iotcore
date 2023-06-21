/*
* Copyright 2022 NXP
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the distribution.
*
* * Neither the name of the copyright holder nor the
*   names of its contributors may be used to endorse or promote products
*   derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include "Lcdifv3.h"
#include "iMX8LcdHwLib.h"

/* layer encoding formats (input) */
#define BPP16_RGB565    0x4
#define BPP16_ARGB1555  0x5
#define BPP16_ARGB4444  0x6
#define BPP16_YCbCr422  0x7
#define BPP24_RGB888    0x8
#define BPP32_ARGB8888  0x9
#define BPP32_ABGR8888  0xa

/* Base addresses of LCDIF peripherals */
LCDIF_MemMapPtr DP[LCDIFMAX_DEV] = LCDIF_BASE_PTRS;

/**
  Reset LCDIF block.
**/
EFI_STATUS
Lcdifv3_Reset (
    Lcdifv3_Device Dev
    )
{
    if (Dev >= LCDIFMAX_DEV) {
        return EFI_DEVICE_ERROR;
    }

    /* LCDIF reset */
    LCDIF_CTRL_CLR_REG(DP[Dev]) = LCDIF_CTRL_SW_RESET_MASK;
    while (LCDIF_CTRL_REG(DP[Dev]) & LCDIF_CTRL_SW_RESET_MASK);

    LCDIF_CTRL_SET_REG(DP[Dev]) = LCDIF_CTRL_SW_RESET_MASK;
    while (!(LCDIF_CTRL_REG(DP[Dev]) & LCDIF_CTRL_SW_RESET_MASK));

    LCDIF_CTRL_CLR_REG(DP[Dev]) = LCDIF_CTRL_SW_RESET_MASK;
    while (LCDIF_CTRL_REG(DP[Dev]) & LCDIF_CTRL_SW_RESET_MASK);

    return EFI_SUCCESS;
}

/**
  LCDIF enable/disable.

  @param enable Enable/Disable bool flag.
**/
EFI_STATUS
Lcdifv3_Enable (
    Lcdifv3_Device Dev,
    bool Enable
    )
{
    uint32_t loop = 2000U;
    uint32_t disp, ctrl;

    if (Dev >= LCDIFMAX_DEV) {
        return EFI_DEVICE_ERROR;
    }

    disp = LCDIF_DISP_PARA_REG(DP[Dev]);
    ctrl = LCDIF_CTRLDESCL0_5_REG(DP[Dev]);

    if (Enable) {
        /* disp on */
        disp |= LCDIF_DISP_PARA_DISP_ON_MASK;
        LCDIF_DISP_PARA_REG(DP[Dev]) = disp;
        /* enable shadow load */
        ctrl |= LCDIF_CTRLDESCL0_5_SHADOW_LOAD_EN_MASK;
        LCDIF_CTRLDESCL0_5_REG(DP[Dev]) = ctrl;
        /* enable layer dma */
        ctrl |= LCDIF_CTRLDESCL0_5_EN_MASK;
        LCDIF_CTRLDESCL0_5_REG(DP[Dev]) = ctrl;
    } else {
        /* disable layer dma */
        ctrl &= ~LCDIF_CTRLDESCL0_5_EN_MASK;
        ctrl |= LCDIF_CTRLDESCL0_5_SHADOW_LOAD_EN_MASK;
        LCDIF_CTRLDESCL0_5_REG(DP[Dev]) = ctrl;
        /* dma config takes effect at the end of frame,
        so add delay to wait dma disable done before turn off disp. */
        while (--loop) {
            if (LCDIF_INT_STATUS_D0_REG(DP[Dev]) & LCDIF_INT_STATUS_D0_VS_BLANK_MASK) {
                break;
            }
            MicroSecondDelay(10);
        }
        disp &= ~LCDIF_DISP_PARA_DISP_ON_MASK;
        LCDIF_DISP_PARA_REG(DP[Dev]) = disp;
    }
    return EFI_SUCCESS;
}

/**
  Basic LCDIF init not related to the timing mode.

  @param FrameBuffer Physical address of the SW framebuffer.
**/
EFI_STATUS
Lcdifv3_Init (
    Lcdifv3_Device Dev,
    uintptr_t FrameBuffer
    )
{
    uint32_t panic;
    if ((Dev >= LCDIFMAX_DEV) || (FrameBuffer == 0)) {
        return EFI_DEVICE_ERROR;
    }

    /* the thres_low should be 1/3 FIFO, that is 511/3 = 171
     * and thres_high should be 2/3 FIFO, that is 511*2/3 = 340
     */
    panic = LCDIF_PANIC0_THRES_PANIC_THRES_LOW(171) | LCDIF_PANIC0_THRES_PANIC_THRES_HIGH(341);
    LCDIF_PANIC0_THRES_REG(DP[Dev]) = panic;
    /* Enable Panic */
    LCDIF_INT_ENABLE_D1_REG(DP[Dev]) = LCDIF_INT_ENABLE_D1_PLANE_PANIC_EN_MASK;

    /* Set address of the frame buffer */
    LCDIF_CTRLDESCL_LOW0_4_REG(DP[Dev]) = FrameBuffer;

    return EFI_SUCCESS;
}

/**
  LCDIF configuration into required timing mode.

  @param Timing Pointer to structure containing detailed timing information.
**/
EFI_STATUS
Lcdifv3_SetTimingMode (
    Lcdifv3_Device Dev,
    IMX_DISPLAY_TIMING* Timing
    )
{
    uint32_t disp_size, hsyn_para, vsyn_para, vsyn_hsyn_width, ctrldescl0_1, polarities;
    uint32_t disp_para = 0;
    uint32_t ctrldescl0_5 = 0;

    if ((Dev >= LCDIFMAX_DEV) || (Timing == NULL)) {
        return EFI_DEVICE_ERROR;
    }

    /* config display timings */
    disp_size = LCDIF_DISP_SIZE_DELTA_Y(Timing->VActive) | LCDIF_DISP_SIZE_DELTA_X(Timing->HActive);
    LCDIF_DISP_SIZE_REG(DP[Dev]) = disp_size;

    hsyn_para = LCDIF_HSYN_PARA_BP_H(Timing->HBlank - Timing->HSyncOffset - Timing->HSync) | 
                LCDIF_HSYN_PARA_FP_H(Timing->HSyncOffset);
    LCDIF_HSYN_PARA_REG(DP[Dev]) = hsyn_para;

    vsyn_para = LCDIF_VSYN_PARA_BP_V(Timing->VBlank - Timing->VSyncOffset - Timing->VSync) | 
                LCDIF_VSYN_PARA_FP_V(Timing->VSyncOffset);
    LCDIF_VSYN_PARA_REG(DP[Dev]) = vsyn_para;

    vsyn_hsyn_width = LCDIF_VSYN_HSYN_WIDTH_PW_V(Timing->VSync) 
                      | LCDIF_VSYN_HSYN_WIDTH_PW_H(Timing->HSync);
    LCDIF_VSYN_HSYN_WIDTH_REG(DP[Dev]) = vsyn_hsyn_width;

    /* config layer size */
    ctrldescl0_1 = LCDIF_CTRLDESCL0_1_HEIGHT(Timing->VActive) | LCDIF_CTRLDESCL0_1_WIDTH(Timing->HActive);
    LCDIF_CTRLDESCL0_1_REG(DP[Dev]) = ctrldescl0_1;

    /* Polarities */
    polarities = (LCDIF_CTRL_CLR_INV_HS_MASK | LCDIF_CTRL_CLR_INV_VS_MASK |
                  /* SEC MIPI DSI specific */
                  LCDIF_CTRL_CLR_INV_PXCK_MASK | LCDIF_CTRL_CLR_INV_DE_MASK);
    LCDIF_CTRL_CLR_REG(DP[Dev]) = polarities;

    /* Set output bus format */
    disp_para = LCDIF_DISP_PARA_REG(DP[Dev]);
    disp_para &= LCDIF_DISP_PARA_LINE_PATTERN(0xf);
    /* 24 bits output (LP_RGB888_OR_YUV444) */
    disp_para |= LCDIF_DISP_PARA_LINE_PATTERN(0);
    /* config display mode: default is normal mode */
    disp_para &= LCDIF_DISP_PARA_DISP_MODE(3);
    disp_para |= LCDIF_DISP_PARA_DISP_MODE(0);
    LCDIF_DISP_PARA_REG(DP[Dev]) = disp_para;

    /* Set input bus format */
    ctrldescl0_5 = LCDIF_CTRLDESCL0_5_REG(DP[Dev]);
    ctrldescl0_5 &= ~(LCDIF_CTRLDESCL0_5_BPP(0xf) | LCDIF_CTRLDESCL0_5_YUV_FORMAT(0x3));
    switch (Timing->PixelFormat) {
        case PIXEL_FORMAT_ARGB32:
            ctrldescl0_5 |= LCDIF_CTRLDESCL0_5_BPP(BPP32_ARGB8888);
            break;
        default:
            DEBUG ((DEBUG_ERROR, "Unsupported pixel format: %u\n", Timing->PixelFormat));
            return EFI_DEVICE_ERROR;
    }
    LCDIF_CTRLDESCL0_5_REG(DP[Dev]) = ctrldescl0_5;

    /* Set stride */
    LCDIF_CTRLDESCL0_3_REG(DP[Dev]) = (LCDIF_CTRLDESCL0_3_PITCH(Timing->HActive * 
                                       LcdGetBytesPerPixel(Timing->PixelFormat)) | 0x110000);

    return EFI_SUCCESS;
}

/**
  Dump LCDIF registers.
**/
EFI_STATUS
Lcdifv3_Dump (
    Lcdifv3_Device Dev
    )
{
    if (Dev >= LCDIFMAX_DEV) {
        return EFI_DEVICE_ERROR;
    }
    DEBUG ((DEBUG_INFO, "------------------------LCDIFv3------------------------\n"));
    DEBUG ((DEBUG_INFO, "LCDIF_CTRL              = 0x%08X\n", LCDIF_CTRL_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_DISP_PARA         = 0x%08X\n", LCDIF_DISP_PARA_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_DISP_SIZE         = 0x%08X\n", LCDIF_DISP_SIZE_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_HSYN_PARA         = 0x%08X\n", LCDIF_HSYN_PARA_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_VSYN_PARA         = 0x%08X\n", LCDIF_VSYN_PARA_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_VSYN_HSYN_WIDTH   = 0x%08X\n", LCDIF_VSYN_HSYN_WIDTH_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_INT_STATUS_D0     = 0x%08X\n", LCDIF_INT_STATUS_D0_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_INT_ENABLE_D0     = 0x%08X\n", LCDIF_INT_ENABLE_D0_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_INT_STATUS_D1     = 0x%08X\n", LCDIF_INT_STATUS_D1_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_INT_ENABLE_D1     = 0x%08X\n", LCDIF_INT_ENABLE_D1_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CTRLDESCL0_1      = 0x%08X\n", LCDIF_CTRLDESCL0_1_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CTRLDESCL0_3      = 0x%08X\n", LCDIF_CTRLDESCL0_3_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_TRLDESCL_LOW0_4   = 0x%08X\n", LCDIF_CTRLDESCL_LOW0_4_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CTRLDESCL_HIGH0_4 = 0x%08X\n", LCDIF_CTRLDESCL_HIGH0_4_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_TRLDESCL0_5       = 0x%08X\n", LCDIF_CTRLDESCL0_5_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CSC0_CTRL         = 0x%08X\n", LCDIF_CSC0_CTRL_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CSC0_COEF0        = 0x%08X\n", LCDIF_CSC0_COEF0_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CSC0_COEF1        = 0x%08X\n", LCDIF_CSC0_COEF1_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CSC0_COEF2        = 0x%08X\n", LCDIF_CSC0_COEF2_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CSC0_COEF3        = 0x%08X\n", LCDIF_CSC0_COEF3_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CSC0_COEF4        = 0x%08X\n", LCDIF_CSC0_COEF4_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_CSC0_COEF5        = 0x%08X\n", LCDIF_CSC0_COEF5_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "LCDIF_PANIC0_THRES      = 0x%08X\n", LCDIF_PANIC0_THRES_REG(DP[Dev])));
    DEBUG ((DEBUG_INFO, "-----------------------------------------------------\n"));
    return EFI_SUCCESS;
}
