/*
* Description: Processor Devices
*
*  Copyright (c) Microsoft Corporation. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
*/


OperationRegion(FUSE,SystemMemory,0x30350400,0x900)      // 0x3035_0D00
Field(FUSE, AnyAcc, Nolock, Preserve)
{
  Offset(0x240),
  MC15, 8,          // 0x640 NET1 MAC address bytes 5
  MC14, 8,          // 0x641 NET1 MAC address bytes 4
  MC13, 8,          // 0x642 NET1 MAC address bytes 3
  MC12, 8,          // 0x643 NET1 MAC address bytes 2
  Offset(0x250),
  MC11, 8,          // 0x650 NET1 MAC address bytes 1
  MC10, 8,          // 0x651 NET1 MAC address bytes 0
  MC25, 8,          // 0x652 NET2 MAC address bytes 5
  MC24, 8,          // 0x653 NET2 MAC address bytes 4
  Offset(0x260),
  MC23, 8,          // 0x660 NET2 MAC address bytes 3
  MC22, 8,          // 0x661 NET2 MAC address bytes 2
  MC21, 8,          // 0x662 NET2 MAC address bytes 1
  MC20, 8,          // 0x663 NET2 MAC address bytes 0
}

//
// Description: This is a Processor #0 Device
//
Device (CPU0)
{
    Name (_HID, "ACPI0007")
    Name (_UID, 0x0)
    Method (_STA)
    {
        Return(0xf)
    }
}

//
// Description: This is a Processor #1 Device
//
Device (CPU1)
{
    Name (_HID, "ACPI0007")
    Name (_UID, 0x1)
    Method (_STA)
    {
        Return(0xf)
    }
}

//
// Description: This is a Processor #2 Device
//
Device (CPU2)
{
    Name (_HID, "ACPI0007")
    Name (_UID, 0x2)
    Method (_STA)
    {
        Return(0xf)
    }
}

//
// Description: This is a Processor #3 Device
//
Device (CPU3)
{
    Name (_HID, "ACPI0007")
    Name (_UID, 0x3)
    Method (_STA)
    {
        Return(0xf)
    }
}

// Power Engine Plugin
Device (PEP0)
{
  Name (_HID, "NXP0111")
  Name (_UID, 0x0)

  Method (_STA) {
    Return (0xf)
  }
}

