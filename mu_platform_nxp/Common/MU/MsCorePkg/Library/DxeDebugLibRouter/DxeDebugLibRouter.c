/** @file
  Debug library that routes debug to either ReportStatusCode or Serial output
  based on what protocols/tag GUIDs are installed

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>

#include <Guid/StatusCodeDataTypeId.h>
#include <Guid/StatusCodeDataTypeDebug.h>

#include "DxeDebugLibRouter.h"

static volatile BOOLEAN       mIsRscAvailable = FALSE;
static volatile BOOLEAN       mIsBSAvailable = TRUE;
EFI_EVENT                     mReportStatusCodeRegisterEvent;
EFI_EVENT                     mExitBootServicesRegisterEvent;
static EFI_BOOT_SERVICES     *mBS = NULL;

/**
  Report Status Code Callback Handler

  @param[in] Event      Event that signalled the callback.
  @param[in] Context    Pointer to an optional event contxt.

  @retval None.

**/
VOID
EFIAPI
ReportStatusCodeHandlerCallback(
    IN  EFI_EVENT    Event,
    IN  VOID    *Context
) {
    mIsRscAvailable = TRUE;
    mBS->CloseEvent (mReportStatusCodeRegisterEvent);
    return;
}


/**
  Exit Boot Services Callback Handler

  @param[in] Event      Event that signalled the callback.
  @param[in] Context    Pointer to an optional event contxt.

  @retval None.

**/
VOID
EFIAPI
ExitBootServicesHandlerCallback(
    IN  EFI_EVENT    Event,
    IN  VOID    *Context
) {
    EFI_STATUS Status;
    mIsBSAvailable = FALSE;
    if(mExitBootServicesRegisterEvent != NULL) {
      Status = mBS->CloseEvent(mExitBootServicesRegisterEvent);
      ASSERT_EFI_ERROR (Status);
    }
    return;
}


/**
* The constructor sets up the callback routine for ReportStatusCode if
* it is not already installed
*
* @param  ImageHandle   The firmware allocated handle for the EFI image.
* @param  SystemTable   A pointer to the EFI System Table.
*
* @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.
*
**/
EFI_STATUS
EFIAPI
DxeDebugLibConstructor(
    IN      EFI_HANDLE                ImageHandle,
    IN      EFI_SYSTEM_TABLE          *SystemTable
) {
    EFI_STATUS                 Status;
    VOID*                      *mRscHandlerProtocol;
    VOID*                      Registration;


    //
    // Get BootServices Table
    //
    mBS = SystemTable->BootServices;

    //
    // Check if Report Status Code handler is installed
    //
    Status = mBS->LocateProtocol(&gMsSerialStatusCodeHandlerDxeProtocolGuid,
                                 NULL,
                                 (VOID**)&mRscHandlerProtocol);

    //
    // Report Status Code may become available later
    //
    if(EFI_ERROR(Status)){
      Status = mBS->CreateEvent(EVT_NOTIFY_SIGNAL,
                                TPL_NOTIFY,
                                ReportStatusCodeHandlerCallback,
                                NULL,
                                &mReportStatusCodeRegisterEvent);

        //
        // If tag GUID is installed switch over to Report Status Code
        //
        if (!EFI_ERROR(Status)) {
            Status = mBS->RegisterProtocolNotify(&gMsSerialStatusCodeHandlerDxeProtocolGuid,
                                                 mReportStatusCodeRegisterEvent,
                                                 &Registration);
        }
    }
    else {
      mIsRscAvailable = TRUE;
    }

    //
    // Setup callback in case Exit BS is called and other callbacks needs to print
    // This should be set to a level higher than Status Code Handler callback
    //
    Status = mBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_NOTIFY,
                                ExitBootServicesHandlerCallback,
                                NULL,
                                &gEfiEventExitBootServicesGuid,
                                &mExitBootServicesRegisterEvent);
    ASSERT_EFI_ERROR (Status);

    return EFI_SUCCESS;
}

/**
* Destructor for Debug Router Lib. Unregisters EBS callback to prevent
* function calls on unloaded library
*
* @param  ImageHandle   The firmware allocated handle for the EFI image.
* @param  SystemTable   A pointer to the EFI System Table.
*
* @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.
*
**/
EFI_STATUS
EFIAPI
DxeDebugLibDestructor(
    IN      EFI_HANDLE                ImageHandle,
    IN      EFI_SYSTEM_TABLE          *SystemTable
) {
  EFI_STATUS Status;
  if(mExitBootServicesRegisterEvent != NULL) {
    Status = mBS->CloseEvent(mExitBootServicesRegisterEvent);
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

/**
  Prints a debug message to the debug output device if the specified error level is enabled.

  If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function
  GetDebugPrintErrorLevel (), then print the message specified by Format and the
  associated variable argument list to the debug output device.

  If Format is NULL, then ASSERT().

  @param  ErrorLevel  The error level of the debug message.
  @param  Format      The format string for the debug message to print.
  @param  ...         The variable argument list whose contents are accessed
                      based on the format string specified by Format.

**/
VOID
EFIAPI
DebugPrint (
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  ...
) {
  VA_LIST       Marker;
  DEBUG_PRINT   *DebugPrintFunction;

  if (mIsBSAvailable && mIsRscAvailable) {
    DebugPrintFunction = ReportStatusCodeDebugPrint;
  }
  else {
    DebugPrintFunction = SerialDebugPrint;
  }

  //Forward the DebugPrint to the correct function
  VA_START (Marker, Format);
  (*DebugPrintFunction)(ErrorLevel,Format,Marker);
  VA_END (Marker);
}


/**
If any bit in ErrorLevel is also set in DebugPrintErrorLevelLib function
GetDebugPrintErrorLevel (), then print the message specified by Format and the
associated variable argument list to the debug output device.

If Format is NULL, then ASSERT().

If the length of the message string specificed by Format is larger than the maximum allowable
record length, then directly return and not print it.

@param  ErrorLevel    The error level of the debug message.
@param  Format        Format string for the debug message to print.
@param  VaListMarker  VA_LIST marker for the variable argument list.

**/
VOID
EFIAPI
DebugVPrint(
  IN  UINTN        ErrorLevel,
  IN  CONST CHAR8  *Format,
  VA_LIST          VaListMarker
) {
  DEBUG_PRINT   *DebugPrintFunction;

  if (mIsBSAvailable && mIsRscAvailable) {
    DebugPrintFunction = ReportStatusCodeDebugPrint;
  }
  else {
    DebugPrintFunction = SerialDebugPrint;
  }

  //Forward the DebugPrint to the correct function
  (*DebugPrintFunction)(ErrorLevel,Format,VaListMarker);
}
// END

/**
  Prints an assert message containing a filename, line number, and description.
  This may be followed by a breakpoint or a dead loop.

  Print a message of the form "ASSERT <FileName>(<LineNumber>): <Description>\n"
  to the debug output device.  If DEBUG_PROPERTY_ASSERT_BREAKPOINT_ENABLED bit of
  PcdDebugPropertyMask is set then CpuBreakpoint() is called. Otherwise, if
  DEBUG_PROPERTY_ASSERT_DEADLOOP_ENABLED bit of PcdDebugPropertyMask is set then
  CpuDeadLoop() is called.  If neither of these bits are set, then this function
  returns immediately after the message is printed to the debug output device.
  DebugAssert() must actively prevent recursion.  If DebugAssert() is called while
  processing another DebugAssert(), then DebugAssert() must return immediately.

  If FileName is NULL, then a <FileName> string of "(NULL) Filename" is printed.
  If Description is NULL, then a <Description> string of "(NULL) Description" is printed.

  @param  FileName     The pointer to the name of the source file that generated the assert condition.
  @param  LineNumber   The line number in the source file that generated the assert condition
  @param  Description  The pointer to the description of the assert condition.

**/
VOID
EFIAPI
DebugAssert (
  IN CONST CHAR8  *FileName,
  IN UINTN        LineNumber,
  IN CONST CHAR8  *Description
  )
{
  DEBUG_ASSERT   *DebugAssertFunction;

  if (mIsBSAvailable && mIsRscAvailable) {
    DebugAssertFunction = ReportStatusCodeDebugAssert;
  }
  else {
    DebugAssertFunction = SerialDebugAssert;
  }

  //Forward the DebugAssert to the correct function
  (*DebugAssertFunction)(FileName,LineNumber,Description);
}


/**
  Fills a target buffer with PcdDebugClearMemoryValue, and returns the target buffer.

  This function fills Length bytes of Buffer with the value specified by
  PcdDebugClearMemoryValue, and returns Buffer.

  If Buffer is NULL, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param   Buffer  The pointer to the target buffer to be filled with PcdDebugClearMemoryValue.
  @param   Length  The number of bytes in Buffer to fill with zeros PcdDebugClearMemoryValue.

  @return  Buffer  The pointer to the target buffer filled with PcdDebugClearMemoryValue.

**/
VOID *
EFIAPI
DebugClearMemory (
  OUT VOID  *Buffer,
  IN UINTN  Length
  )
{
  //
  // If Buffer is NULL, then ASSERT().
  //
  ASSERT (Buffer != NULL);

  //
  // SetMem() checks for the the ASSERT() condition on Length and returns Buffer
  //
  return SetMem (Buffer, Length, PcdGet8(PcdDebugClearMemoryValue));
}


/**
  Returns TRUE if ASSERT() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise, FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugAssertEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED) != 0);
}


/**
  Returns TRUE if DEBUG() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise, FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_PRINT_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugPrintEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_PRINT_ENABLED) != 0);
}


/**
  Returns TRUE if DEBUG_CODE() macros are enabled.

  This function returns TRUE if the DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise, FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_DEBUG_CODE_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugCodeEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_DEBUG_CODE_ENABLED) != 0);
}


/**
  Returns TRUE if DEBUG_CLEAR_MEMORY() macro is enabled.

  This function returns TRUE if the DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of
  PcdDebugPropertyMask is set.  Otherwise, FALSE is returned.

  @retval  TRUE    The DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of PcdDebugPropertyMask is set.
  @retval  FALSE   The DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED bit of PcdDebugPropertyMask is clear.

**/
BOOLEAN
EFIAPI
DebugClearMemoryEnabled (
  VOID
  )
{
  return (BOOLEAN) ((PcdGet8(PcdDebugPropertyMask) & DEBUG_PROPERTY_CLEAR_MEMORY_ENABLED) != 0);
}

/**
  Returns TRUE if any one of the bit is set both in ErrorLevel and PcdFixedDebugPrintErrorLevel.

  This function compares the bit mask of ErrorLevel and PcdFixedDebugPrintErrorLevel.

  @retval  TRUE    Current ErrorLevel is supported.
  @retval  FALSE   Current ErrorLevel is not supported.

**/
BOOLEAN
EFIAPI
DebugPrintLevelEnabled (
  IN  CONST UINTN        ErrorLevel
  )
{
  return (BOOLEAN) ((ErrorLevel & PcdGet32(PcdFixedDebugPrintErrorLevel)) != 0);
}