/****************************************************************************
* Copyright (c) Microsoft Corporation.
*
*    Licensed under the MIT License.
*    Licensed under the GPL License.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/

#pragma once

const UINT  GC_7L_CMD_FILL_IMG[] =
{
    0x08010e00, 0x00000000,
    0x08010e80, 0x30000000,
    0x080101f6, 0x00000000,
    0x08010380, 0x00000001,
    0x0801028b, 0x34000001,
    0x08010221, 0x00000808,
    0x08015310, 0x00000001,
    0x08010e03, 0x00004c27,
    0x08010e03, 0x00000010,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e03, 0x00000c67,
    0x08010e03, 0x00000010,
    0x0801022c, 0x0000001f,
    0x08015311, 0x00000000,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x0801502e, 0x00000001,
    0x08015019, 0x00000180,
    0x08015006, 0xfe081000,     // DST_START_GPUVA
    0x08015000, 0xfe081000,     // SRC_START_GPUVA
    0x08015009, 0x00200820,     // DST_CONIFG : MULTI_TILING(31:31), SUPER_TILING(30:30), TILING(29:29), MSAA(28:27), STRIDE(20:0)
    0x0801500a, 0x00000000,     // DST_CONFIG_EX: TILE_MODE(27:26)
    0x08015002, 0x00200820,     // SRC_CONFIG : MULTI_TILING(31:31), SUPER_TILING(30:30), TILING(29:29), MSAA(28:27), STRIDE(20:0)
    0x08015003, 0x00000000,     // SRC_CONFIG_EX: TILE_MODE(22:21)
    0x0801503d, 0xfe0fffff,     // SRC_END_GPUVA
    0x080150cd, 0xfe0fffff,     // DST_END_GPUVA
    0x08025011, 0x000075a1,     // BLT_NEW_CLEAR_VALUE1: NEW_CLEAR_VALUE_VALUE(31:0)
    0x000075a1, 0xdeadbeef,     // BLT_NEW_CLEAR_VALUE2: NEW_CLEAR_VALUE_VALUE(31:0)
    0x08025013, 0xffffffff,
    0xffffffff, 0xdeadbeef,
    0x0801500b, 0x00000000,     // DST_ORIGIN_YX
    0x0801500c, 0x00fa0208,     // WINDOW_SIZE_YX
    0x08015018, 0x00000001,
    0x0801502b, 0x00000003,
    0x0801502e, 0x00000000,
    0x0801502e, 0x00000001,
    0x08010e20, 0x00001001,
    0x48000000, 0x00001001,
    0x0801502e, 0x00000000,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e03, 0x00000c67,
    0x08010e03, 0x00000010,
    0x0801022c, 0x0000001f,
    0x08015311, 0x00000000,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701
};

enum GC_7L_CMD_FILL_IMG_PATCH_LOCATION
{
    GC_7L_CMD_FILL_IMG_DST_START_GPUVA       = 0x09C/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_SRC_START_GPUVA       = 0x0A4/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_DST_CONFIG            = 0x0AC/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_DST_CONFIG_EX         = 0x0B4/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_SRC_CONFIG            = 0x0BC/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_SRC_CONFIG_EX         = 0x0C4/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_SRC_END_GPUVA         = 0x0CC/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_DST_END_GPUVA         = 0x0D4/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_BLT_NEW_CLEAR_VALUE1  = 0x0DC/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_BLT_NEW_CLEAR_VALUE2  = 0x0E0/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_DST_ORIGIN_YX         = 0x0FC/sizeof(UINT),
    GC_7L_CMD_FILL_IMG_WINDOW_SIZE_YX        = 0x104/sizeof(UINT),
};