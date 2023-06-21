/* Copyright (c) Microsoft Corporation.
 * Copyright 2022 NXP
   Licensed under the MIT License. */

#pragma once

#include "GcKmdQxpLvdsTransmitter.h"

#include "GcKmdBaseDisplay.h"

extern "C" {

#include <svc/scfw.h>

#include "linux/platform_device.h"
#include "clk/clk_imx8qxp.h"
#include "dpu/dpu-prv.h"
#include "dpu/dpu-crtc.h"
#include "dpu/dpu-plane.h"
#include "video/imx8-prefetch.h"

}

class GcKmImx8qxpDisplay : public GcKmBaseDisplay
{
public:

    GcKmImx8qxpDisplay()
    {
        m_FrontBufferSegmentOffset.QuadPart = 0L;
        m_OldFbPhysicalAddress = 0L;
        m_InterruptData = {};
        m_dpu_pdev = {};
        m_dpu = nullptr;
        RtlZeroMemory(m_client_devices, sizeof(m_client_devices));
        m_pTransmitter = &m_LvdsTransmitter;
        m_crtc = {};
        m_crtc_state = {};
        m_aux_crtc = {};
        m_crtc.aux_dpu_crtc = &m_aux_crtc;
        m_crtc.base.state = &m_crtc_state.imx_crtc_state.base;
        m_old_dmode = {};
        m_old_plane_state = {};
        m_old_fb = {};
        m_old_plane_state.fb = &m_old_fb;
        m_irqsteer_pdev = {};
        m_clk_tree = nullptr;
        m_ipc_id = {};
    }

    virtual NTSTATUS HwStart(DXGKRNL_INTERFACE* pDxgkInterface) override;

    virtual NTSTATUS HwStop(
        DXGK_DISPLAY_INFORMATION   *pFwDisplayInfo) override;

    virtual void HwSetPowerState(
        UINT SourcePhysicalAddress,
        UINT TileMode) override;

    virtual NTSTATUS SetVidPnSourceAddress(
        IN_CONST_PDXGKARG_SETVIDPNSOURCEADDRESS pSetVidPnSourceAddress) override;

    virtual NTSTATUS HwCommitVidPn(
        const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode,
        const D3DKMDT_VIDPN_TARGET_MODE* pTargetMode,
        IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn) override;

    virtual BOOLEAN InterruptRoutine(
        UINT    MessageNumber) override;

    virtual NTSTATUS ControlInterrupt(
        IN_CONST_DXGK_INTERRUPT_TYPE    InterruptType,
        IN_BOOLEAN  EnableInterrupt) override;

    virtual NTSTATUS SetVidPnSourceAddressWithMultiPlaneOverlay3(
        IN_OUT_PDXGKARG_SETVIDPNSOURCEADDRESSWITHMULTIPLANEOVERLAY3 pSetVidPnSourceAddressWithMpo3) override;

private:

    void SetupFramebuffer(
        const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode,
        UINT FrameBufferPhysicalAddress,
        UINT64 TileMode);

    void SetupPlaneState(
        const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode,
        const D3DKMDT_VIDPN_TARGET_MODE* pTargetMode);

    NTSTATUS SetupHwCommit(
        const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode,
        const D3DKMDT_VIDPN_TARGET_MODE* pTargetMode,
        struct videomode* vm,
        UINT FrameBufferPhysicalAddress,
        UINT TileMode);

    NTSTATUS HwAtomicCommit(struct videomode* vm);

    void HandleVsyncInterrupt(UINT Disp);

    NTSTATUS GetI2CResource(
        DXGKRNL_INTERFACE* pDxgkInterface,
        ULONG I2CId,
        LARGE_INTEGER* pI2CConnectionId);

    QxpLvdsTransmitter m_LvdsTransmitter;

    DXGKARGCB_NOTIFY_INTERRUPT_DATA m_InterruptData;

    PHYSICAL_ADDRESS    m_FrontBufferSegmentOffset;

    ULONG m_OldFbPhysicalAddress;

    static const INT PRG_CNT = 9;
    struct platform_device m_prg_pdev[PRG_CNT];

    static const INT DPRC_CNT = 6;
    struct platform_device m_dprc_pdev[DPRC_CNT];

    struct platform_device m_dpu_pdev;
    struct dpu_soc *m_dpu;

    static const INT CLIENT_DEVICE_CNT = 3;
    struct platform_device m_client_devices[CLIENT_DEVICE_CNT];

    struct dpu_crtc m_crtc;
    struct dpu_crtc_state m_crtc_state;

    // unused on QXP; added here to keep Linux drv flow consistent
    struct dpu_crtc m_aux_crtc;

    struct drm_display_mode m_old_dmode;
    struct drm_plane_state m_old_plane_state;
    struct drm_framebuffer m_old_fb;

    struct platform_device m_irqsteer_pdev;

    struct imx8qxp_clk_device *m_clk_tree;

    sc_ipc_id_struct_t m_ipc_id;
};

