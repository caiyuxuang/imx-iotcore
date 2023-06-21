/******************************************************************************
 *
 * Copyright (C) 2016-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 *
 * Copyright 2022 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */

#include "precomp.h"
#include "GcKmdLogging.h"
#include "GcKmdSecDsiTransmitter.tmh"
#include "GcKmdSecDsiTransmitter.h"
#include "getresrc.h"

extern "C" {
#include "boot/dts/freescale/board.h"
#include "mipi_dsi/adv7511.h"
#include <drm/drm_fourcc.h>
#include <edidtst.h>
}

#define printk(x, ...) DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, x, __VA_ARGS__)

/* #define SECDSI_TRANSMITTER_DEBUG */
#ifdef SECDSI_TRANSMITTER_DEBUG
    #define printk_debug printk
#else
    #define printk_debug
#endif

static UCHAR *sec_dsi_default_edid = (UCHAR *)edid_mon_1920_1080_60;

/*number of entries that are queried from registry */
#define NUM_REG_ENTRIES 3

/* Use local default_edid defined above instead of registry edid */
static const bool secdsi_transmitter_override_registry_edid = FALSE;

NTSTATUS SecDsiTransmitter::GetEdid(PVOID Data, ULONG Length, UINT8 Block, UINT8 Segment)
{
    if (Data == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (secdsi_transmitter_override_registry_edid) {
        RtlCopyMemory(m_CachedEdid, sec_dsi_default_edid, EDID_SIZE);
    }
    RtlCopyMemory(Data, m_CachedEdid, min(Length, EDID_SIZE));

    return STATUS_SUCCESS;
}

CHAR* SecDsiTransmitter::GetPrintableDispInterface(void)
{
    return "mipi-dsi";
}

void SecDsiTransmitter::GetChildDescriptor(DXGK_CHILD_DESCRIPTOR* pDescriptor)
{
    pDescriptor->ChildDeviceType = TypeVideoOutput;
    pDescriptor->ChildCapabilities.HpdAwareness = HpdAwarenessAlwaysConnected;
    pDescriptor->ChildCapabilities.Type.VideoOutput.InterfaceTechnology = D3DKMDT_VOT_LVDS;
    pDescriptor->ChildCapabilities.Type.VideoOutput.MonitorOrientationAwareness = D3DKMDT_MOA_NONE;
    pDescriptor->ChildCapabilities.Type.VideoOutput.SupportsSdtvModes = FALSE;
    /* child device is not an ACPI device */
    pDescriptor->AcpiUid = 0;
    pDescriptor->ChildUid = MIPI_DSI_CHILD_UID;
}

RTL_QUERY_REGISTRY_ROUTINE SecDsiTransmitter_EDIDQueryRoutine;

_Use_decl_annotations_
NTSTATUS SecDsiTransmitter_EDIDQueryRoutine(PWSTR ValueName, ULONG ValueType, PVOID ValueData,
    ULONG ValueLength, PVOID Context, PVOID EntryContext)
{
    if ((ValueType != REG_BINARY) || (ValueLength != EDID_SIZE) || (EntryContext == NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

    /* Validate first 8 bytes of EDID header */
    BOOLEAN check_ok = TRUE;
    for (UCHAR i = 0; i < 8; i++) {
        if (*(((UCHAR *)ValueData)+i) != *(sec_dsi_default_edid + i)) {
            check_ok = FALSE;
            break;
        }
    }
    if (!check_ok) {
        return STATUS_UNSUCCESSFUL;
    }

    RtlCopyMemory(EntryContext, ValueData, EDID_SIZE);
    return STATUS_SUCCESS;
}

NTSTATUS SecDsiTransmitter::GetRegistryParams(DXGKRNL_INTERFACE* pDxgkInterface)
{
    NTSTATUS status;
    DXGK_DEVICE_INFO device_info;
    /* query_table must be number of items + 1 (last item must be zero) */
    RTL_QUERY_REGISTRY_TABLE query_table[NUM_REG_ENTRIES + 1];
    USHORT len;
    PUCHAR reg_path;
    ULONG tmp_num_lanes;
    ULONG tmp_channel_id;

    status = pDxgkInterface->DxgkCbGetDeviceInformation(pDxgkInterface->DeviceHandle, &device_info);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    /* Convert registry path from unicode string to wide string and ensure zero termination (__GFP_ZERO in kmalloc) */
    len = device_info.DeviceRegistryPath.Length;
    reg_path = (PUCHAR)kmalloc(len + sizeof(WCHAR), __GFP_ZERO);
    if (reg_path == NULL) {
        return STATUS_NO_MEMORY;
    }
    RtlCopyMemory(reg_path, device_info.DeviceRegistryPath.Buffer, len);

    /* Initialize query table - read 1 items from registry, last item in query_table is zeroed */
    RtlZeroMemory(query_table, sizeof(query_table));
    query_table[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_TYPECHECK;
    query_table[0].Name = L"Display0NumLanes";
    query_table[0].EntryContext = &tmp_num_lanes;
    query_table[0].DefaultType = (REG_DWORD << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;
    query_table[1].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_TYPECHECK;
    query_table[1].Name = L"Display0ChannelId";
    query_table[1].EntryContext = &tmp_channel_id;
    query_table[1].DefaultType = (REG_DWORD << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;
    query_table[2].Flags = 0;
    query_table[2].Name = L"Display0EDID";
    query_table[2].EntryContext = &m_CachedEdid;
    query_table[2].DefaultType = REG_BINARY;
    query_table[2].QueryRoutine = SecDsiTransmitter_EDIDQueryRoutine;
    query_table[2].DefaultData = NULL;
    query_table[2].DefaultLength = 0;
    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, (PCWSTR)reg_path, query_table, NULL, NULL);
    if (!NT_SUCCESS(status)) {
        /* Fall back with default values */
        tmp_num_lanes = 4;
        tmp_channel_id = 0;
        RtlCopyMemory(m_CachedEdid, sec_dsi_default_edid, EDID_SIZE);
        printk("SecDsi display: ERROR reading registry parameters failed.\n");
    }
    kfree(reg_path);
    if ((tmp_num_lanes != 2) && (tmp_num_lanes != 4)) {
        m_num_lanes = 4;
        status = STATUS_INVALID_PARAMETER;
    } else {
        m_num_lanes = (UCHAR)tmp_num_lanes;
    }
    if (tmp_channel_id > 3) {
        m_channel_id = 0;
        status = STATUS_INVALID_PARAMETER;
    } else {
        m_channel_id = (UCHAR)tmp_channel_id;
    }

    return status;
}

static property mn_adv_properties[] = {
    { "compatible", 1, "adi,adv7535" },
    { "" /* mark end of the list */ }
};

NTSTATUS SecDsiTransmitter::Start(DXGKRNL_INTERFACE* pDxgkInterface)
{
    NTSTATUS status;

    status = GetRegistryParams(pDxgkInterface);
    if (!NT_SUCCESS(status)) {
        printk("SecDsi display: WARNING parameters not valid. Switching to default settings: interface=%s num_lanes=%d channel_id=%d.\n",
            GetPrintableDispInterface(), m_num_lanes, m_channel_id);
    }
    printk_debug("SecDsi display: interface=%s num_lanes=%d channel_id=%d.\n",
        GetPrintableDispInterface(), m_num_lanes, m_channel_id);

    dsi_pdev.name = "mipi_dsi_dev";
    dsi_pdev.plat_name = "mn";
    board_init(&dsi_pdev);
    if (imx_sec_dsim_probe(&dsi_pdev) != 0) {
        imx_sec_dsim_remove(&dsi_pdev);
        printk("SecDsi display: ERROR imx_sec_dsim_probe failed.\n");
        return STATUS_INTERNAL_ERROR;
    }

    if (imx_sec_dsim_bind(&dsi_pdev.dev) != 0) {
        imx_sec_dsim_unbind(&dsi_pdev.dev);
        imx_sec_dsim_remove(&dsi_pdev);
        printk("SecDsi display: ERROR imx_sec_dsim_bind failed.\n");
        return STATUS_INTERNAL_ERROR;
    }

    /* MIPI-HDMI converter initialization is optional */
    do {
        /* All i2c_main, i2c_edid, i2c_cec, i2c_packet devices refer to MIPI-HDMI converter extension board adv7535.
           There are four memory blocks with separate bus address.
           m_i2c_main is the main device with allocated driver data. others are auxiliary - only for i2c regmap storage */
        m_i2c_main.is_initialized = 0;
        /* i2c_main device, ACPI i2c index 0 (first i2c device) */
        status = GetI2CresourceNum(pDxgkInterface, 0, &m_i2c_main.connection_id);
        if (!NT_SUCCESS(status)) {
            break;
        }
        /* i2c_cec device, ACPI i2c index 1 (second i2c device) */
        status = GetI2CresourceNum(pDxgkInterface, 1, &m_i2c_cec.connection_id);
        if (!NT_SUCCESS(status)) {
            break;
        }
        /* i2c_edid device, ACPI i2c index 2 (third i2c device) */
        status = GetI2CresourceNum(pDxgkInterface, 2, &m_i2c_edid.connection_id);
        if (!NT_SUCCESS(status)) {
            break;
        }

        m_i2c_main.dev.of_node.properties = (property *)&mn_adv_properties;
        m_i2c_main.dev.parent = &dsi_pdev.dev;
        if (adv7511_probe(&m_i2c_main, &m_i2c_edid, &m_i2c_cec, m_num_lanes, m_channel_id) != 0) {
            adv7511_remove(&m_i2c_main);
            break;
        }
        if (adv7511_bridge_attach(&m_i2c_main) != 0) {
            adv7511_bridge_detach(&m_i2c_main);
            adv7511_remove(&m_i2c_main);
            break;
        }
        m_i2c_main.is_initialized = 1;
        printk_debug("SecDsi display: adv7535 mipi-hdmi converter initialized.\n");
    } while (0);
    if (!m_i2c_main.is_initialized) {
        //TODO: Panel driver should be initialized. Do just test attachment of mipi driver.
        //      These taskas should be done inside a panel driver.
        test_pdev.name = "mipi_dsi_adv7535"; /* test name */
        test_pdev.plat_name = "mn";
        board_init(&test_pdev);
        test_pdev.dev.parent = &dsi_pdev.dev;
        if (sec_mipi_dsim_test_attach_dsi(&test_pdev, m_num_lanes, m_channel_id) != 0) {
            sec_mipi_dsim_test_detach_dsi(&test_pdev);
        }
    }

    /* Start clocks for mipi-dsi */
    if (imx_sec_dsim_runtime_resume(&dsi_pdev.dev) != 0) {
        imx_sec_dsim_unbind(&dsi_pdev.dev);
        imx_sec_dsim_remove(&dsi_pdev);
        printk("SecDsi display: ERROR imx_sec_dsim_runtime_resume failed.\n");
        return STATUS_INTERNAL_ERROR;
    }
    return STATUS_SUCCESS;
}

NTSTATUS SecDsiTransmitter::Stop()
{
    /* To full stop the hardware, if (m_i2c_main.is_initialized), call: adv7511_bridge_disable(&m_i2c_main); */
    /* To full stop the hardware, call: sec_mipi_dsim_bridge_disable(&dsi_pdev); */
    /* imx_sec_dsim_encoder_helper_disable just assert mipi reset - call after sec_mipi_dsim_bridge_disable */
    /* To full stop the hardware, call: imx_sec_dsim_encoder_helper_disable(&dsi_pdev); */
    /* To full stop the hardware, call: imx_sec_dsim_runtime_suspend(&dsi_pdev.dev); */

    if (!m_i2c_main.is_initialized) {
        //TODO: Panel driver should be de-initialized. Do just test detachment of mipi driver.
        sec_mipi_dsim_test_detach_dsi(&test_pdev);
    }

    if (m_i2c_main.is_initialized) {
        adv7511_bridge_detach(&m_i2c_main);
        adv7511_remove(&m_i2c_main);
    }

    imx_sec_dsim_unbind(&dsi_pdev.dev);
    imx_sec_dsim_remove(&dsi_pdev);

    return STATUS_SUCCESS;
}

int SecDsiTransmitter::GetCachedEdid(PVOID *pEdid)
{
    *pEdid = m_CachedEdid;
    return sizeof(m_CachedEdid);
}

NTSTATUS SecDsiTransmitter::GetHotPlugDetectStatus(UINT8* status)
{
    /* Display is always connected */
    *status = 1;
    return STATUS_SUCCESS;
}

NTSTATUS SecDsiTransmitter::GetI2CresourceNum(DXGKRNL_INTERFACE* pDxgkInterface, ULONG i2c_index, LARGE_INTEGER *i2c_connection_id)
{
    NTSTATUS status;
    WCHAR   buff[512];

    status = GetReslist(pDxgkInterface, (PWCHAR)&buff, sizeof(buff));
    if (!NT_SUCCESS(status)) {
        printk("SecDsi display: Error getting resource list\n");
        return status;
    }

    status = ParseReslist((PCM_RESOURCE_LIST)&buff, CmResourceTypeConnection, i2c_connection_id, NULL, i2c_index);
    if (!NT_SUCCESS(status)) {
        printk("SecDsi display: Error parsing resource list\n");
        return status;
    }

    printk_debug("SecDsi display: I2C resource found with connection id: 0x%llx\n", i2c_connection_id->QuadPart);
    return STATUS_SUCCESS;
}
