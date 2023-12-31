// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright 2022 NXP
// Licensed under the MIT License.

#ifndef _TRACE_H_
#define _TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    //
    // Defining control guids, including this is required to happen before
    // including the tmh file (if the WppRecorder API is used)
    //
#include <WppRecorder.h>

//
// Tracing GUID - {F1BAA46F-0744-4C0A-9CBA-D3C3BC0773C9}
//
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(IMXSCFW, (F1BAA46F,0744,4C0A,9CBA,D3C3BC0773C9), \
        WPP_DEFINE_BIT(IMXSCFW_TRACING_DEFAULT) \
    )

// begin_wpp config
//
// USEPREFIX (IMXSCFW_LOG_ERROR, "%!STDPREFIX! [%s @ %s %u] ERROR :", __FILE__, __func__, __LINE__);
// FUNC IMXSCFW_LOG_ERROR{LEVEL=TRACE_LEVEL_ERROR, FLAGS=IMXSCFW_TRACING_DEFAULT}(MSG, ...);
//
// USEPREFIX (IMXSCFW_LOG_LOW_MEMORY, "%!STDPREFIX! [%s @ %s %u] LOW MEMORY :", __FILE__, __func__, __LINE__);
// FUNC IMXSCFW_LOG_LOW_MEMORY{LEVEL=TRACE_LEVEL_ERROR, FLAGS=IMXSCFW_TRACING_DEFAULT}(MSG, ...);
//
// USEPREFIX (IMXSCFW_LOG_WARNING, "%!STDPREFIX! [%s @ %s %u] WARNING :", __FILE__, __func__, __LINE__);
// FUNC IMXSCFW_LOG_WARNING{LEVEL=TRACE_LEVEL_WARNING, FLAGS=IMXSCFW_TRACING_DEFAULT}(MSG, ...);
//
// USEPREFIX (IMXSCFW_LOG_INFORMATION, "%!STDPREFIX! [%s @ %s %u] INFO :", __FILE__, __func__, __LINE__);
// FUNC IMXSCFW_LOG_INFORMATION{LEVEL=TRACE_LEVEL_INFORMATION, FLAGS=IMXSCFW_TRACING_DEFAULT}(MSG, ...);
//
// USEPREFIX (IMXSCFW_LOG_TRACE, "%!STDPREFIX! [%s @ %s %u] TRACE :", __FILE__, __func__, __LINE__);
// FUNC IMXSCFW_LOG_TRACE{LEVEL=TRACE_LEVEL_VERBOSE, FLAGS=IMXSCFW_TRACING_DEFAULT}(MSG, ...);
//
// end_wpp

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // _TRACE_H_