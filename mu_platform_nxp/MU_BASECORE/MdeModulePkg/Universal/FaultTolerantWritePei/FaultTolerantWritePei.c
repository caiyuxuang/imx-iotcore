/** @file
  This driver installs gEdkiiFaultTolerantWriteGuid PPI to inform
  the check for FTW last write data has been done.

Copyright (c) 2013 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Guid/SystemNvDataGuid.h>
#include <Guid/FaultTolerantWrite.h>
#include <Guid/VariableFlashInfo.h>   // MU_CHANGE - TCBZ3479 - Add Variable Flash Information HOB
#include <Library/PeiServicesLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/SafeIntLib.h>       // MU_CHANGE - TCBZ3479 - Add Variable Flash Information HOB

EFI_PEI_PPI_DESCRIPTOR     mPpiListVariable = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEdkiiFaultTolerantWriteGuid,
  NULL
};

/**
  Get the last Write Header pointer.
  The last write header is the header whose 'complete' state hasn't been set.
  After all, this header may be a EMPTY header entry for next Allocate.


  @param FtwWorkSpaceHeader Pointer of the working block header
  @param FtwWorkSpaceSize   Size of the work space
  @param FtwWriteHeader     Pointer to retrieve the last write header

  @retval  EFI_SUCCESS      Get the last write record successfully
  @retval  EFI_ABORTED      The FTW work space is damaged

**/
EFI_STATUS
FtwGetLastWriteHeader (
  IN EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER  *FtwWorkSpaceHeader,
  IN UINTN                                    FtwWorkSpaceSize,
  OUT EFI_FAULT_TOLERANT_WRITE_HEADER         **FtwWriteHeader
  )
{
  UINTN                           Offset;
  EFI_FAULT_TOLERANT_WRITE_HEADER *FtwHeader;

  *FtwWriteHeader = NULL;
  FtwHeader       = (EFI_FAULT_TOLERANT_WRITE_HEADER *) (FtwWorkSpaceHeader + 1);
  Offset          = sizeof (EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER);

  while (FtwHeader->Complete == FTW_VALID_STATE) {
    Offset += FTW_WRITE_TOTAL_SIZE (FtwHeader->NumberOfWrites, FtwHeader->PrivateDataSize);
    //
    // If Offset exceed the FTW work space boudary, return error.
    //
    if (Offset >= FtwWorkSpaceSize) {
      *FtwWriteHeader = FtwHeader;
      return EFI_ABORTED;
    }

    FtwHeader = (EFI_FAULT_TOLERANT_WRITE_HEADER *) ((UINT8 *) FtwWorkSpaceHeader + Offset);
  }
  //
  // Last write header is found
  //
  *FtwWriteHeader = FtwHeader;

  return EFI_SUCCESS;
}

/**
  Get the last Write Record pointer. The last write Record is the Record
  whose DestinationCompleted state hasn't been set. After all, this Record
  may be a EMPTY record entry for next write.


  @param FtwWriteHeader  Pointer to the write record header
  @param FtwWriteRecord  Pointer to retrieve the last write record

  @retval EFI_SUCCESS        Get the last write record successfully
  @retval EFI_ABORTED        The FTW work space is damaged

**/
EFI_STATUS
FtwGetLastWriteRecord (
  IN EFI_FAULT_TOLERANT_WRITE_HEADER          *FtwWriteHeader,
  OUT EFI_FAULT_TOLERANT_WRITE_RECORD         **FtwWriteRecord
  )
{
  UINTN                           Index;
  EFI_FAULT_TOLERANT_WRITE_RECORD *FtwRecord;

  *FtwWriteRecord = NULL;
  FtwRecord       = (EFI_FAULT_TOLERANT_WRITE_RECORD *) (FtwWriteHeader + 1);

  //
  // Try to find the last write record "that has not completed"
  //
  for (Index = 0; Index < FtwWriteHeader->NumberOfWrites; Index += 1) {
    if (FtwRecord->DestinationComplete != FTW_VALID_STATE) {
      //
      // The last write record is found
      //
      *FtwWriteRecord = FtwRecord;
      return EFI_SUCCESS;
    }

    FtwRecord++;

    if (FtwWriteHeader->PrivateDataSize != 0) {
      FtwRecord = (EFI_FAULT_TOLERANT_WRITE_RECORD *) ((UINTN) FtwRecord + (UINTN) FtwWriteHeader->PrivateDataSize);
    }
  }
  //
  //  if Index == NumberOfWrites, then
  //  the last record has been written successfully,
  //  but the Header->Complete Flag has not been set.
  //  also return the last record.
  //
  if (Index == FtwWriteHeader->NumberOfWrites) {
    *FtwWriteRecord = (EFI_FAULT_TOLERANT_WRITE_RECORD *) ((UINTN) FtwRecord - FTW_RECORD_SIZE (FtwWriteHeader->PrivateDataSize));
    return EFI_SUCCESS;
  }

  return EFI_ABORTED;
}

/**
  Check to see if it is a valid work space.


  @param WorkingHeader   Pointer of working block header
  @param WorkingLength   Working block length

  @retval TRUE          The work space is valid.
  @retval FALSE         The work space is invalid.

**/
BOOLEAN
IsValidWorkSpace (
  IN EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER    *WorkingHeader,
  IN UINTN                                      WorkingLength
  )
{
  UINT8 Data;

  if (WorkingHeader == NULL) {
    return FALSE;
  }

  if ((WorkingHeader->WorkingBlockValid != FTW_VALID_STATE) || (WorkingHeader->WorkingBlockInvalid == FTW_VALID_STATE)) {
    DEBUG ((EFI_D_ERROR, "FtwPei: Work block header valid bit check error\n"));
    return FALSE;
  }

  if (WorkingHeader->WriteQueueSize != (WorkingLength - sizeof (EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER))) {
    DEBUG ((EFI_D_ERROR, "FtwPei: Work block header WriteQueueSize check error\n"));
    return FALSE;
  }

  //
  // Check signature with gEdkiiWorkingBlockSignatureGuid
  //
  if (!CompareGuid (&gEdkiiWorkingBlockSignatureGuid, &WorkingHeader->Signature)) {
    DEBUG ((EFI_D_ERROR, "FtwPei: Work block header signature check error, it should be gEdkiiWorkingBlockSignatureGuid\n"));
    //
    // To be compatible with old signature gEfiSystemNvDataFvGuid.
    //
    if (!CompareGuid (&gEfiSystemNvDataFvGuid, &WorkingHeader->Signature)) {
      return FALSE;
    } else {
      Data = *(UINT8 *) (WorkingHeader + 1);
      if (Data != 0xff) {
        DEBUG ((EFI_D_ERROR, "FtwPei: Old format FTW structure can't be handled\n"));
        ASSERT (FALSE);
        return FALSE;
      }
    }
  }

  return TRUE;

}

// MU_CHANGE - START - TCBZ3479 - Add Variable Flash Information HOB
/**
  Get the HOB that contains variable flash information.

  @param[out] VariableFlashInfo   Pointer to a pointer to set to the variable flash information structure.

  @retval EFI_SUCCESS             Variable flash information was found successfully.
  @retval EFI_INVALID_PARAMETER   The VariableFlashInfo pointer given is NULL.
  @retval EFI_NOT_FOUND           Variable flash information could not be found.

**/
EFI_STATUS
GetVariableFlashInfo (
  OUT VARIABLE_FLASH_INFO       **VariableFlashInfo
  )
{
  EFI_HOB_GUID_TYPE             *GuidHob;

  if (VariableFlashInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  GuidHob = GetFirstGuidHob (&gVariableFlashInfoHobGuid);
  if (GuidHob == NULL) {
    return EFI_NOT_FOUND;
  }

  *VariableFlashInfo = GET_GUID_HOB_DATA (GuidHob);

  //
  // Assert if more than one variable flash information HOB is present.
  //
  DEBUG_CODE (
    if ((GetNextGuidHob (&gVariableFlashInfoHobGuid, GET_NEXT_HOB (GuidHob)) != NULL)) {
      DEBUG ((DEBUG_ERROR, "ERROR: Found two variable flash information HOBs\n"));
      ASSERT (FALSE);
    }
  );

  return EFI_SUCCESS;
}
// MU_CHANGE - END - TCBZ3479 - Add Variable Flash Information HOB

/**
  Main entry for Fault Tolerant Write PEIM.

  @param[in]  FileHandle              Handle of the file being invoked.
  @param[in]  PeiServices             Pointer to PEI Services table.

  @retval EFI_SUCCESS  If the interface could be successfully installed
  @retval Others       Returned from PeiServicesInstallPpi()

**/
EFI_STATUS
EFIAPI
PeimFaultTolerantWriteInitialize (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                                Status;
  EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER   *FtwWorkingBlockHeader;
  EFI_FAULT_TOLERANT_WRITE_HEADER           *FtwLastWriteHeader;
  EFI_FAULT_TOLERANT_WRITE_RECORD           *FtwLastWriteRecord;
  VARIABLE_FLASH_INFO                       *VariableFlashInfo;     // MU_CHANGE - TCBZ3479 - Add Variable Flash Information HOB
  EFI_PHYSICAL_ADDRESS                      WorkSpaceAddress;
  UINTN                                     WorkSpaceLength;
  EFI_PHYSICAL_ADDRESS                      SpareAreaAddress;
  UINTN                                     SpareAreaLength;
  EFI_PHYSICAL_ADDRESS                      WorkSpaceInSpareArea;
  FAULT_TOLERANT_WRITE_LAST_WRITE_DATA      FtwLastWrite;

  FtwWorkingBlockHeader = NULL;
  FtwLastWriteHeader = NULL;
  FtwLastWriteRecord = NULL;

// MU_CHANGE - START - TCBZ3479 - Add Variable Flash Information HOB
  SpareAreaAddress  = 0;
  SpareAreaLength  = 0;
  WorkSpaceAddress = 0;
  WorkSpaceLength = 0;

  Status = GetVariableFlashInfo (&VariableFlashInfo);
  if (!EFI_ERROR (Status)) {
    // This driver currently assumes the size will be UINT32 so only accept
    // that for now.
    Status = SafeUint64ToUintn (VariableFlashInfo->FtwSpareLength, &SpareAreaLength);
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      SpareAreaLength = 0;
    }
    Status = SafeUint64ToUintn (VariableFlashInfo->FtwWorkingLength, &WorkSpaceLength);
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      WorkSpaceLength = 0;
    }
    SpareAreaAddress = VariableFlashInfo->FtwSpareBaseAddress;
    WorkSpaceAddress = VariableFlashInfo->FtwWorkingBaseAddress;
  }

  if (SpareAreaLength == 0) {
    SpareAreaLength  = (UINTN) PcdGet32 (PcdFlashNvStorageFtwSpareSize);
  }
  if (WorkSpaceLength == 0) {
    WorkSpaceLength  = (UINTN) PcdGet32 (PcdFlashNvStorageFtwWorkingSize);
  }
  if (SpareAreaAddress == 0) {
    SpareAreaAddress = (EFI_PHYSICAL_ADDRESS) PcdGet64 (PcdFlashNvStorageFtwSpareBase64);
    if (SpareAreaAddress == 0) {
      SpareAreaAddress = (EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdFlashNvStorageFtwSpareBase);
    }
  }
  if (WorkSpaceAddress == 0) {
    WorkSpaceAddress = (EFI_PHYSICAL_ADDRESS) PcdGet64 (PcdFlashNvStorageFtwWorkingBase64);
    if (WorkSpaceAddress == 0) {
      WorkSpaceAddress = (EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdFlashNvStorageFtwWorkingBase);
    }
  }
// MU_CHANGE - END - TCBZ3479 - Add Variable Flash Information HOB

  //
  // The address of FTW working base and spare base must not be 0.
  //
  ASSERT ((WorkSpaceAddress != 0) && (SpareAreaAddress != 0));

  FtwWorkingBlockHeader = (EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER *) (UINTN) WorkSpaceAddress;
  if (IsValidWorkSpace (FtwWorkingBlockHeader, WorkSpaceLength)) {
    Status = FtwGetLastWriteHeader (
               FtwWorkingBlockHeader,
               WorkSpaceLength,
               &FtwLastWriteHeader
               );
    if (!EFI_ERROR (Status)) {
      Status = FtwGetLastWriteRecord (
                 FtwLastWriteHeader,
                 &FtwLastWriteRecord
                 );
    }

    if (!EFI_ERROR (Status)) {
      ASSERT (FtwLastWriteRecord != NULL);
      if ((FtwLastWriteRecord->SpareComplete == FTW_VALID_STATE) && (FtwLastWriteRecord->DestinationComplete != FTW_VALID_STATE)) {
        //
        // If FTW last write was still in progress with SpareComplete set and DestinationComplete not set.
        // It means the target buffer has been backed up in spare block, then target block has been erased,
        // but the target buffer has not been writen in target block from spare block, we need to build
        // FAULT_TOLERANT_WRITE_LAST_WRITE_DATA GUID hob to hold the FTW last write data.
        //
        FtwLastWrite.TargetAddress = (EFI_PHYSICAL_ADDRESS) (UINTN) ((INT64) SpareAreaAddress + FtwLastWriteRecord->RelativeOffset);
        FtwLastWrite.SpareAddress = SpareAreaAddress;
        FtwLastWrite.Length = SpareAreaLength;
        DEBUG ((
          EFI_D_INFO,
          "FtwPei last write data: TargetAddress - 0x%x SpareAddress - 0x%x Length - 0x%x\n",
          (UINTN) FtwLastWrite.TargetAddress,
          (UINTN) FtwLastWrite.SpareAddress,
          (UINTN) FtwLastWrite.Length));
        BuildGuidDataHob (&gEdkiiFaultTolerantWriteGuid, (VOID *) &FtwLastWrite, sizeof (FAULT_TOLERANT_WRITE_LAST_WRITE_DATA));
      }
    }
  } else {
    FtwWorkingBlockHeader = NULL;
    //
    // If the working block workspace is not valid, try to find workspace in the spare block.
    //
    WorkSpaceInSpareArea = SpareAreaAddress + SpareAreaLength - WorkSpaceLength;
    while (WorkSpaceInSpareArea >= SpareAreaAddress) {
      if (CompareGuid (&gEdkiiWorkingBlockSignatureGuid, (EFI_GUID *) (UINTN) WorkSpaceInSpareArea)) {
        //
        // Found the workspace.
        //
        DEBUG ((EFI_D_INFO, "FtwPei: workspace in spare block is at 0x%x.\n", (UINTN) WorkSpaceInSpareArea));
        FtwWorkingBlockHeader = (EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER *) (UINTN) WorkSpaceInSpareArea;
        break;
      }
      WorkSpaceInSpareArea = WorkSpaceInSpareArea - sizeof (EFI_GUID);
    }
    if ((FtwWorkingBlockHeader != NULL) && IsValidWorkSpace (FtwWorkingBlockHeader, WorkSpaceLength)) {
      //
      // It was workspace self reclaim, build FAULT_TOLERANT_WRITE_LAST_WRITE_DATA GUID hob for it.
      //
      FtwLastWrite.TargetAddress = WorkSpaceAddress - (WorkSpaceInSpareArea - SpareAreaAddress);
      FtwLastWrite.SpareAddress = SpareAreaAddress;
      FtwLastWrite.Length = SpareAreaLength;
      DEBUG ((
        EFI_D_INFO,
        "FtwPei last write data: TargetAddress - 0x%x SpareAddress - 0x%x Length - 0x%x\n",
        (UINTN) FtwLastWrite.TargetAddress,
        (UINTN) FtwLastWrite.SpareAddress,
        (UINTN) FtwLastWrite.Length));
      BuildGuidDataHob (&gEdkiiFaultTolerantWriteGuid, (VOID *) &FtwLastWrite, sizeof (FAULT_TOLERANT_WRITE_LAST_WRITE_DATA));
    } else {
      //
      // Both are invalid.
      //
      DEBUG ((EFI_D_ERROR, "FtwPei: Both working and spare block are invalid.\n"));
    }
  }

  //
  // Install gEdkiiFaultTolerantWriteGuid PPI to inform the check for FTW last write data has been done.
  //
  return PeiServicesInstallPpi (&mPpiListVariable);
}
