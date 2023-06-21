/** @file
*  Description: i.MX8M VPU Encoder/Decoder
*
*  Copyright (c) 2019 Microsoft Corporation. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

Device (VPU0)
{
  Name (_HID, "NXP010A")
  Name (_UID, 0x0)

  Method (_STA) {
    Return (0xf)
  }

  Name (_CRS, ResourceTemplate () {
    MEMORY32FIXED (ReadWrite, 0x2C000000, 0x01000000, ) // VPU encoder/decoder registers
    MEMORY32FIXED (ReadWrite, 0x2D000000, 0x00020000, ) // VPU decoder MU0 base 
    // MEMORY32FIXED (ReadWrite, 0x2D020000, 0x00020000, ) // VPU ecoder MU1 base 
    MEMORY32FIXED (ReadWrite, 0xF0100000, 0x02000000, ) // VPU decoder boot addr
    // MEMORY32FIXED (ReadWrite, 0xF2100000, 0x00200000, ) // VPU encodel boot addr
    MEMORY32FIXED (ReadWrite, 0xFD000000, 0x00200000, ) // VPU decoder RPC - uncached
    // MEMORY32FIXED (ReadWrite, 0xFD200000, 0x00200000, ) // VPU encoder RPC - uncached
    // MEMORY32FIXED (ReadWrite, 0xFD400000, 0x00400000, ) // VPU encoder RESERVED

    // VPU MU decoder interrupt
    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 501 }
    // VPU MU encoder interrupt
    // Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 502 }
    I2CSerialBus(0x40, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.SCFW")
  })
}
