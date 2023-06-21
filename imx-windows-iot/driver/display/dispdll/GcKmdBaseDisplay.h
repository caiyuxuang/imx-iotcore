/* Copyright (c) Microsoft Corporation.
 * Copyright 2022 NXP
   Licensed under the MIT License. */

#pragma once

#include "GcKmdHdmiTransmitter.h"

struct GcKmdFrameBuffer
{
    void* m_Address{ nullptr };
    size_t m_Size{ 0 };
};

class GcKmBaseDisplay : public GcKmDisplay
{
public:

    GcKmBaseDisplay()
    {
        m_LocalVidMemPhysicalBase = {};
        m_NativeMonitorMode = {};
        m_bNativeMonitorModeSet = false;
        m_pDxgkInterface = {};
        m_PreviousPostDisplayInfo = {};
        m_FbPhysicalAddr = {};
        m_FbSize = 0;
        m_pTransmitter = nullptr;
    }

    virtual ~GcKmBaseDisplay() {}

    virtual NTSTATUS Start(
        DXGKRNL_INTERFACE  *pDxgkInterface,
        UINT                LocalVidMemPhysicalBase,
        ULONG              *pNumberOfVideoPresentSources,
        ULONG              *pNumberOfChildren);

    virtual NTSTATUS HwStart(DXGKRNL_INTERFACE* pDxgkInterface) = 0;

    virtual NTSTATUS Stop();

    virtual NTSTATUS HwStop(
        DXGK_DISPLAY_INFORMATION   *pFwDisplayInfo) = 0;

    virtual NTSTATUS QueryAdapterInfo(
        const DXGKARG_QUERYADAPTERINFO *pQueryAdapterInfo);

    virtual NTSTATUS QueryChildRelations(
        INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
        IN_ULONG                        ChildRelationsSize);

    virtual NTSTATUS QueryChildStatus(
        IN_PDXGK_CHILD_STATUS   ChildStatus,
        IN_BOOLEAN              NonDestructiveOnly);

    virtual NTSTATUS QueryDeviceDescriptor(
        IN_ULONG                        ChildUid,
        INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor);

    virtual NTSTATUS SetPowerState(
        IN_ULONG                DeviceUid,
        IN_DEVICE_POWER_STATE   DevicePowerState,
        IN_POWER_ACTION         ActionType);

    virtual void HwSetPowerState(
        UINT SourcePhysicalAddress,
        UINT TileMode) = 0;

    virtual NTSTATUS SetPointerPosition(
        IN_CONST_PDXGKARG_SETPOINTERPOSITION    pSetPointerPosition);

    virtual NTSTATUS SetPointerShape(
        IN_CONST_PDXGKARG_SETPOINTERSHAPE   pSetPointerShape);

    virtual NTSTATUS IsSupportedVidPn(
        INOUT_PDXGKARG_ISSUPPORTEDVIDPN pIsSupportedVidPn);

    virtual NTSTATUS RecommendFunctionalVidPn(
        IN_CONST_PDXGKARG_RECOMMENDFUNCTIONALVIDPN_CONST    pRecommendFunctionalVidPn);

    virtual NTSTATUS EnumVidPnCofuncModality(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality);

    virtual NTSTATUS SetVidPnSourceVisibility(
        IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY  pSetVidPnSourceVisibility);

    virtual NTSTATUS CommitVidPn(
        IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn);

    virtual NTSTATUS HwCommitVidPn(
        const D3DKMDT_VIDPN_SOURCE_MODE* pSourceMode,
        const D3DKMDT_VIDPN_TARGET_MODE* pTargetMode,
        IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn) = 0;

    virtual NTSTATUS UpdateActiveVidPnPresentPath(
        IN_CONST_PDXGKARG_UPDATEACTIVEVIDPNPRESENTPATH_CONST    pUpdateActiveVidPnPresentPath);

    virtual NTSTATUS RecommendMonitorModes(
        IN_CONST_PDXGKARG_RECOMMENDMONITORMODES_CONST   pRecommendMonitorMode);

    virtual NTSTATUS GetScanLine(
        INOUT_PDXGKARG_GETSCANLINE  pGetScanLine);

    virtual NTSTATUS QueryVidPnHWCapability(
        INOUT_PDXGKARG_QUERYVIDPNHWCAPABILITY   pVidPnHWCaps);

    virtual NTSTATUS PresentDisplayOnly(
        IN_CONST_PDXGKARG_PRESENT_DISPLAYONLY  pPresentDisplayOnly);

    virtual NTSTATUS StopDeviceAndReleasePostDisplayOwnership(
        _In_ D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId,
        _Out_ PDXGK_DISPLAY_INFORMATION     pDisplayInfo);

    virtual NTSTATUS UpdateMonitorLinkInfo(
        INOUT_PDXGKARG_UPDATEMONITORLINKINFO    pUpdateMonitorLinkInfo);

private:

    NTSTATUS ProcessVidPnPaths(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        D3DKMDT_HVIDPNTOPOLOGY hVidPnTopology,
        DXGK_VIDPNTOPOLOGY_INTERFACE const* pTopologyInterface);

    NTSTATUS ProcessSourceModeSet(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath);

    NTSTATUS ProcessTargetModeSet(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath);

    NTSTATUS ProcessVidPnPathAttributes(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath,
        D3DKMDT_HVIDPNTOPOLOGY hVidPnTopology,
        DXGK_VIDPNTOPOLOGY_INTERFACE const* pTopologyInterface);

    NTSTATUS SourceModeSetNeedsUpdate(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath,
        BOOLEAN* bNeedsUpdate);

    NTSTATUS SourceHasPinnedMode(
        D3DKMDT_HVIDPN hVidPn,
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath,
        BOOLEAN* bHasPinnedMode);

    NTSTATUS TargetHasPinnedMode(
        D3DKMDT_HVIDPN hVidPn,
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath,
        BOOLEAN* bHasPinnedMode);

    NTSTATUS TargetModeSetNeedsUpdate(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath,
        BOOLEAN* bNeedsUpdate);

    BOOLEAN ScalingNeedsUpdate(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath);

    BOOLEAN RotationNeedsUpdate(
        IN_CONST_PDXGKARG_ENUMVIDPNCOFUNCMODALITY_CONST pEnumCofuncModality,
        const D3DKMDT_VIDPN_PRESENT_PATH* pVidPnPath);

    NTSTATUS AddNewSourceModeSet(
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        D3DKMDT_HVIDPN hVidPn,
        D3DDDI_VIDEO_PRESENT_SOURCE_ID VidPnSourceId);

    NTSTATUS AddNewTargetModeSet(
        const DXGK_VIDPN_INTERFACE* pVidPnInterface,
        D3DKMDT_HVIDPN hVidPn,
        D3DKMDT_VIDEO_PRESENT_TARGET_MODE_ID VidPnTargeId);

    NTSTATUS AddNewSourceModeInfo(
        D3DKMDT_HVIDPNSOURCEMODESET hSourceModeSet,
        const DXGK_VIDPNSOURCEMODESET_INTERFACE* pSourceModeSetInterface);

    NTSTATUS AddNewTargetModeInfo(
        D3DKMDT_HVIDPNTARGETMODESET hTargetModeSet,
        const DXGK_VIDPNTARGETMODESET_INTERFACE* pTargetModeSetInterface);

    void SetSourceModeInfoToNative(D3DKMDT_VIDPN_SOURCE_MODE* pModeInfo);

    void SetSourceModeInfoToPreviousPostDisplayInfo(D3DKMDT_VIDPN_SOURCE_MODE* pModeInfo);

    void SetTargetModeInfoToNative(D3DKMDT_VIDPN_TARGET_MODE* pModeInfo);

    void SetTargetModeInfoToPreviousPostDisplayInfo(D3DKMDT_VIDPN_TARGET_MODE* pModeInfo);

    NTSTATUS SavePreviousPostDisplayInfo(DXGKRNL_INTERFACE* pDxgkInterface);

    NTSTATUS MapFrameBuffer(GcKmdFrameBuffer* pFrameBuffer, PHYSICAL_ADDRESS PhysicAddress, ULONG Size);

    void UnmapFrameBuffer(GcKmdFrameBuffer* pFrameBuffer);

    NTSTATUS GetFramebufferInfo(DXGKRNL_INTERFACE* pDxgkInterface, PHYSICAL_ADDRESS *pAddress, ULONG *pSize);

    D3DKMDT_MONITOR_SOURCE_MODE m_NativeMonitorMode;
    BOOLEAN m_bNativeMonitorModeSet;

    GcKmdFrameBuffer m_FrameBuffer;

protected:

    UINT    m_LocalVidMemPhysicalBase;
    DXGKRNL_INTERFACE  *m_pDxgkInterface;
    DXGK_DISPLAY_INFORMATION m_PreviousPostDisplayInfo;
    PHYSICAL_ADDRESS m_FbPhysicalAddr;
    ULONG m_FbSize;

    const UINT CHILD_COUNT = 1;
    const UINT VIDEO_PRESENT_SOURCES_COUNT = 1;

    BaseTransmitter *m_pTransmitter;

    const GUID GC_KMD_DISPLAY_LOGGING =
        { 0xa7bf27a0, 0x7401, 0x4733, { 0x9f, 0xed,  0xfd,  0xb5,  0x10,  0x67,  0xfe,  0xcc } };
};

