// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright 2022 NXP
// Licensed under the MIT License.

#ifndef _IMXPOWERDEF_H_
#define _IMXPOWERDEF_H_

#include <guiddef.h>

//
// Display power components
//

enum IMX_PWRCOMPONENT_DISPLAY {
    IMX_PWRCOMPONENT_DISPLAY_3DENGINE,
    IMX_PWRCOMPONENT_DISPLAY_IPU,
    IMX_PWRCOMPONENT_DISPLAY_MONITOR,
    IMX_PWRCOMPONENT_DISPLAY_COUNT,
};

// {4EA8D720-7C7C-43FD-ADB1-709D1CF1480C}
DEFINE_GUID(
    IMX_PWRCOMPONENTGUID_DISPLAY_3DENGINE,
    0x4ea8d720, 0x7c7c, 0x43fd, 0xad, 0xb1, 0x70, 0x9d, 0x1c, 0xf1, 0x48, 0xc);

// {0C04E0BA-03EE-4A5B-8D09-6FB7F729A939}
DEFINE_GUID(
    IMX_PWRCOMPONENTGUID_DISPLAY_IPU,
    0xc04e0ba, 0x3ee, 0x4a5b, 0x8d, 0x9, 0x6f, 0xb7, 0xf7, 0x29, 0xa9, 0x39);

// {04CCB252-1A40-4FC7-BF57-40310C662308}
DEFINE_GUID(
    IMX_PWRCOMPONENTGUID_DISPLAY_MONITOR,
    0x4ccb252, 0x1a40, 0x4fc7, 0xbf, 0x57, 0x40, 0x31, 0xc, 0x66, 0x23, 0x8);

#endif // _IMXPOWERDEF_H_