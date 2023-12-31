/* Copyright (c) Microsoft Corporation. All rights reserved.
   Copyright 2022 NXP
   Licensed under the MIT License.

Module Name:

    Trace.h

Abstract:

    Header file for the debug tracing related function defintions and macros.

Environment:

    Kernel mode

*/

#ifndef _TRACE_H__
#define _TRACE_H__

#include "Wpprecorder.h"

extern "C"
{

// Define the tracing flags.
// Tracing GUID - 742b0972-565d-4f4f-81c7-f86b61933ebd

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        imxlpi2cdrvTraceGuid, (742b0972,565d,4f4f,81c7,f86b61933ebd), \
                                                                            \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                              \
        WPP_DEFINE_BIT(TRACE_DRIVER)                                   \
        WPP_DEFINE_BIT(TRACE_DEVICE)                                   \
        WPP_DEFINE_BIT(TRACE_CTRLR)                                    \
        WPP_DEFINE_BIT(TRACE_PBC)                                  \
        WPP_DEFINE_BIT(TRACE_READ)                                    \
        WPP_DEFINE_BIT(TRACE_WRITE)                                   \
        WPP_DEFINE_BIT(TRACE_INTRPT)                                  \
        )

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

}
//
// WPP orders static parameters before dynamic parameters. To support the Trace function
// defined below which sets FLAGS=MYDRIVER_ALL_INFO, a custom macro must be defined to
// reorder the arguments to what the .tpl configuration file expects.
//
#define WPP_RECORDER_FLAGS_LEVEL_ARGS(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_ARGS(lvl, flags)
#define WPP_RECORDER_FLAGS_LEVEL_FILTER(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_FILTER(lvl, flags)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAGS=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
//

#endif // of _TRACE_H__