// Copyright (c) Microsoft Corporation. All rights reserved.
// Copyright 2022 NXP
// Licensed under the MIT License.

#ifndef _IMX_PEP_TRACE_H_
#define _IMX_PEP_TRACE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Defining control guids, including this is required to happen before
// including the tmh file (if the WppRecorder API is used)
//
#include <WppRecorder.h>

//
// Tracing GUID - 0B6DAB94-64F9-41E7-B4BF-4AFEED17CCDD
//
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(IMXPEP, (0B6DAB94,64F9,41E7,B4BF,4AFEED17CCDD), \
        WPP_DEFINE_BIT(IMX_TRACING_DEFAULT) \
    )

// begin_wpp config
//
// FUNC IMX_LOG_ERROR{LEVEL=TRACE_LEVEL_ERROR, FLAGS=IMX_TRACING_DEFAULT}(MSG, ...);
// USEPREFIX (IMX_LOG_ERROR, "%!STDPREFIX! [%s @ %u] ERROR :", __FILE__, __LINE__);
//
// FUNC IMX_LOG_LOW_MEMORY{LEVEL=TRACE_LEVEL_ERROR, FLAGS=IMX_TRACING_DEFAULT}(MSG, ...);
// USEPREFIX (IMX_LOG_LOW_MEMORY, "%!STDPREFIX! [%s @ %u] LOW MEMORY :", __FILE__, __LINE__);
//
// FUNC IMX_LOG_WARNING{LEVEL=TRACE_LEVEL_WARNING, FLAGS=IMX_TRACING_DEFAULT}(MSG, ...);
// USEPREFIX (IMX_LOG_WARNING, "%!STDPREFIX! [%s @ %u] WARNING :", __FILE__, __LINE__);
//
// FUNC IMX_LOG_INFORMATION{LEVEL=TRACE_LEVEL_INFORMATION, FLAGS=IMX_TRACING_DEFAULT}(MSG, ...);
// USEPREFIX (IMX_LOG_INFORMATION, "%!STDPREFIX! [%s @ %u] INFO :", __FILE__, __LINE__);
//
// FUNC IMX_LOG_TRACE{LEVEL=TRACE_LEVEL_VERBOSE, FLAGS=IMX_TRACING_DEFAULT}(MSG, ...);
// USEPREFIX (IMX_LOG_TRACE, "%!STDPREFIX! [%s @ %u] TRACE :", __FILE__, __LINE__);
//
// end_wpp

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // _IMX_PEP_TRACE_H_

