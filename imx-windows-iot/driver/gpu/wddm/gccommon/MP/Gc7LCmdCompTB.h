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

#pragma once

#include "Gc7LCmdBufPatch.h"

//
// GC_7L_CMD_COMPUTE_TRANSPARENTBLT_N : N is the ROP value
//

const unsigned int  GC_7L_CMD_COMPUTE_TRANSPARENTBLT_1_2[] =
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
    0x080102a4, 0x00000000,
    0x08010420, 0x00000000,
    0x08010416, 0x00000000,
    0x08010409, 0x00000100,
    0x0801dc06, 0x80000000,
    0x0801021f, 0x00000000,
    0x08010424, 0x0000001d,
    0x0801040a, 0xfe08c000,     // INST_GPUVA
    0x08015580, 0x00000002,
    0x0801021a, 0x00000001,
    0x08010425, 0x0000001c,
    0x08010403, 0x00000004,
    0x08010402, 0x00001f01,
    0x08010404, 0x00000000,
    0x08010228, 0x00000000,
    0x08010412, 0x00000000,
    0x0804dc00, 0x00011000,     // DST_STORAGE_IMAGE
    0x00000100, 0x00100010,
    0x321019d2, 0xdeadbeef,
    0x08015b00, 0x003c2a00,
    0x08015b80, 0x00000002,
    0x08015c00, 0x00000000,
    0x08015c80, 0x00000000,
    0x08015d00, 0x00000000,
    0x08015700, 0xfe086000,     // SRC_SAMPLED_IMAGE
    0x08015780, 0x00000000,
    0x08015312, 0x20000000,
    0x0801dc04, 0xfe089000,     // UNIFORM_GPUVA
    0x08010240, 0x03000003,
    0x08010249, 0x00000000,
    0x08010247, 0x00000020,
    0x08030253, 0x0000000f,
    0x0000000f, 0x00000000,
    0x08010e27, 0x0000001f,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e03, 0x00000c24,
    0x08010e03, 0x00000010,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08030250, 0x00000000,     // DISPATCH_WG
    0x00000000, 0x00000000,
    0x0801024b, 0x00000000,
    0x0801024d, 0x00000000,
    0x0801024f, 0x00000000,
    0x0803dc08, 0x00000001,     // DISPATCH_WG2
    0x00000001, 0x00000001,
    0x08010248, 0xbadabeeb,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e03, 0x00000c67,
    0x08010e03, 0x00000010,
    0x0801022c, 0x0000001f,
    0x08015311, 0x00000000,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701
};


GC_7L_PATCH_ENTRY  GC_7L_CMD_TRANSPARENTBLT_PATCH_TABLE[]
{
    {  37, GC7L_PATCH_INST_GPUVA        },
    {  55, GC7L_PATCH_DST_STORAGE_IMAGE },
    {  71, GC7L_PATCH_SRC_SAMPLED_IMAGE },
    {  77, GC7L_PATCH_UNIFORM_GPUVA     },
    { 103, GC7L_PATCH_DISPATCH_WG       },
    { 113, GC7L_PATCH_DISPATCH_WG2      }
};


const unsigned int GC_7L_TRANSPARENTBLT_SHADER_ALPHA[] =
{
    0x02001032, 0x00201c04, 0xa0000450, 0x00000007,
    0x020010b1, 0x00200c04, 0x81540040, 0x76ffdff8,
    0x0200158f, 0x2aa00c04, 0xa00000c0, 0x7800000f,
    0x04001009, 0x00000004, 0x40000000, 0x002a8008,
    0x00000156, 0x2a800804, 0x50000040, 0x00000407,
    0x04001032, 0x00201c04, 0xa0000650, 0x00000007,
    0x040010b1, 0x15600c04, 0x81fe0040, 0x76ffdff8,
    0x0400158f, 0x3fe00c04, 0xa00000c0, 0x7800000f,
    0x00000156, 0x3fc00804, 0x50000040, 0x00000e87,
    0x06001032, 0x00001c04, 0x20000c50, 0x00000007,
    0x01811032, 0x00001c04, 0x20000850, 0x00000007,
    0x01821032, 0x00001c04, 0x20001050, 0x00000007,
    0x0183102d, 0x15200804, 0x80000000, 0x00000000,
    0x01831001, 0x15003804, 0x00000000, 0x707e0008,
    0x06001002, 0x10003804, 0x008000c0, 0x003a8008,
    0x01811024, 0x00000005, 0x01550040, 0x00000028,
    0x06011024, 0x00000005, 0x01ff0040, 0x00154028,
    0x06001003, 0x20001804, 0x01aa00c0, 0x00000000,
    0x0781102f, 0x3fa00f20, 0x80010040, 0x00000007,
    0x07821032, 0x00201c04, 0xa0001850, 0x00000007,
    0x06001032, 0x00001c04, 0x60000050, 0x00000007,
    0x07831009, 0x00200004, 0x80000000, 0x00390018,
    0x078111b1, 0x39201c04, 0x81c80140, 0x76ffdff8,
    0x0781158f, 0x39201c04, 0xa00000c0, 0x7800000f,
    0x078112cf, 0x39001c04, 0x415400c0, 0x7400000a,
    0x0081150f, 0x39001c04, 0x500000c0, 0x7400000f,
    0x01801001, 0x15000804, 0x40000000, 0x003f8008,
    0x00000156, 0x00001804, 0x50000040, 0x00000e87,
    0x0780083a, 0x39200c00, 0x80a90050, 0x00390038,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const unsigned int GC_7L_TRANSPARENTBLT_SHADER_NO_ALPHA[] =
{
    0x02001032, 0x00201c04, 0xa0000450, 0x00000007,
    0x020010b1, 0x00200c04, 0x81540040, 0x76ffdff8,
    0x0200158f, 0x2aa00c04, 0xa00000c0, 0x7800000f,
    0x04001009, 0x00000004, 0x40000000, 0x002a8008,
    0x00000156, 0x2a800804, 0x50000040, 0x00000407,
    0x04001032, 0x00201c04, 0xa0000650, 0x00000007,
    0x040010b1, 0x15600c04, 0x81fe0040, 0x76ffdff8,
    0x0400158f, 0x3fe00c04, 0xa00000c0, 0x7800000f,
    0x00000156, 0x3fc00804, 0x50000040, 0x00000e87,
    0x06001032, 0x00001c04, 0x20000c50, 0x00000007,
    0x01811032, 0x00001c04, 0x20000850, 0x00000007,
    0x01821032, 0x00001c04, 0x20001050, 0x00000007,
    0x0183102d, 0x15200804, 0x80000000, 0x00000000,
    0x01831001, 0x15003804, 0x00000000, 0x707e0008,
    0x06001002, 0x10003804, 0x008000c0, 0x003a8008,
    0x01811024, 0x00000005, 0x01550040, 0x00000028,
    0x06011024, 0x00000005, 0x01ff0040, 0x00154028,
    0x06001003, 0x20001804, 0x01aa00c0, 0x00000000,
    0x0781102f, 0x3fa00f20, 0x80010040, 0x00000007,
    0x07821032, 0x00201c04, 0xa0001850, 0x00000007,
    0x06001032, 0x00001c04, 0x60000050, 0x00000007,
    0x07831009, 0x00200004, 0x80000000, 0x00390018,
    0x038111b1, 0x29201c04, 0x81480140, 0x76ffdff8,
    0x0381158f, 0x29201c04, 0xa00000c0, 0x7800000f,
    0x038112cf, 0x29001c04, 0x415400c0, 0x7400000a,
    0x0081150f, 0x29001c04, 0x500000c0, 0x7400000f,
    0x01801001, 0x15000804, 0x40000000, 0x003f8008,
    0x00000156, 0x00001804, 0x50000040, 0x00000e87,
    0x0780083a, 0x39200c00, 0x80a90050, 0x00390038,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

