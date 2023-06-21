@ECHO OFF
:: Copyright 2022 NXP 
:: All rights reserved. 
::  
:: Redistribution and use in source and binary forms, with or without modification, 
:: are permitted provided that the following conditions are met: 
::
:: Redistributions of source code must retain the above copyright notice, this list 
:: of conditions and the following disclaimer. 
::
:: Redistributions in binary form must reproduce the above copyright notice, this 
:: list of conditions and the following disclaimer in the documentation and/or 
:: other materials provided with the distribution. 
::
:: Neither the name of the copyright holder nor the names of its 
:: contributors may be used to endorse or promote products derived from this 
:: software without specific prior written permission. 
::
:: THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
:: ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
:: WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
:: DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
:: ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
:: (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
:: LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
:: ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
:: (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
:: SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
:: 

::
:: Run this script in order to deploy boot firmware.
::

setlocal enableextensions
setlocal enabledelayedexpansion

:: Clear options
set DEVICE_TYPE=
set TARGET_DRIVE=
set TARGET_DRIVE_OPT=no
set UUU_LOAD2M=
set FIRMWARE_DIR=
set FIRMWARE=
set CFIMAGER=cfimager.exe
set UUU=tools\uuu\uuu.exe
set SECURE_FW=
set TEMP_FIRMWARE=

:GETOPTS
 if /I "%~1" == "/?" ( goto USAGE
 ) else ( if /I "%~1" == "/Help" ( goto USAGE
 ) else ( if /I "%~1" == "/device" ( set DEVICE_TYPE=%2& shift
 ) else ( if /I "%~1" == "/directory" (   set FIRMWARE_DIR=%2& shift
 ) else ( if /I "%~1" == "/target_drive" (  set TARGET_DRIVE=%2& shift
                                            set TARGET_DRIVE_OPT=yes
 ) else ( if /I "%~1" == "/firmware" (  set FIRMWARE=%2& shift
 ) else ( if /I "%~1" == "/secure" (  set /a SECURE_FW=%2& shift
 ) else ( if /I "%~1" == "" (  break
 ) else ( echo Unsupported option: %~1
          exit /b 1
 ))))))))
 shift
if not (%1)==() goto GETOPTS


REM Remove quotes from Strings from options
if not '%DEVICE_TYPE%' == '' (
  set DEVICE_TYPE=%DEVICE_TYPE:"=%
) else (
  echo Device type cannot be empty.
  echo Choose device type with /device option.
  exit /b 1
)

if "%DEVICE_TYPE%" == "8M" (
  set DEVICE_TYPE=MX8M_EVK
) else (
  if "%DEVICE_TYPE%" == "8Mm" (
    set DEVICE_TYPE=MX8M_MINI_EVK
  ) else (
    if "%DEVICE_TYPE%" == "8Mn" (
      set DEVICE_TYPE=MX8M_NANO_EVK
    ) else (
      if "%DEVICE_TYPE%" == "8Mp" (
        set DEVICE_TYPE=MX8M_PLUS_EVK
      ) else (
        if "%DEVICE_TYPE%" == "8X" (
        set DEVICE_TYPE=MX8QXP_MEK
      )
      )
    )
  )
)


if not '%FIRMWARE_DIR%' == '' (
  set FIRMWARE_DIR=%FIRMWARE_DIR:"=%
) else (
  set FIRMWARE_DIR=%DEVICE_TYPE%
)

if not '%TARGET_DRIVE%' == '' (

  set TARGET_DRIVE=%TARGET_DRIVE:"=%
  
  if not exist !target_drive! (
    echo Selected drive^(!target_drive!^) is not present.
    exit /b 1
  )

) else (
  if %TARGET_DRIVE_OPT% == yes (
    echo Option /target_drive needs to have drive selected. E.g. /target_drive f:.
    exit /b 1
  )
)



if not '%FIRMWARE%' == '' (
  set FIRMWARE=%FIRMWARE:"=%
)



if not exist "%FIRMWARE_DIR%" (
  echo Directory %FIRMWARE_DIR% not found.
  echo Either selected device is incorrect, or directory with firmware does not exist.
  exit /b 1
)




if "%FIRMWARE%" == "" (
    set BIN_COUNT=0
    for /f "tokens=*" %%i in ('dir "!FIRMWARE_DIR!\*.bin" /b') do (
        set /a BIN_COUNT+=1
    )
    if !BIN_COUNT! == 0 (
        echo No bin file was found in !FIRMWARE_DIR!\
        exit /b 1
    )

    if !BIN_COUNT! NEQ 1 (
        set BIN_COUNT=0
        echo Multiple bin files were found in  !FIRMWARE_DIR!\.
        echo Specify which firmware to use:
        for /f "tokens=*" %%i in ('dir "!FIRMWARE_DIR!\*.bin" /b') do (
            set /a BIN_COUNT+=1
            echo !BIN_COUNT!. %%i
        )
        set /p BIN_IDX=Enter index:
        if !BIN_IDX! gtr !BIN_COUNT! (
            echo Selected index is out of range
            exit /b 1
        )
        if !BIN_IDX! leq 0 (
            echo Selected index is out of range
            exit /b 1
        )
    ) else (
        set BIN_IDX=1
    )
        
    set BIN_COUNT=0
    for /f "tokens=*" %%i in ('dir "!FIRMWARE_DIR!\*.bin" /b') do (
        set /a BIN_COUNT+=1
        if !BIN_COUNT! == !BIN_IDX! (
            echo Selected: %%i
            echo:
            set FIRMWARE=!FIRMWARE_DIR!\%%i
        )
    )

)


if not exist !FIRMWARE! (
  echo File !FIRMWARE! not found.
  echo Please verify you used correct 
  echo option with the /device, /directory or /firmware flag.
  exit /b 1
)



echo Device:    %DEVICE_TYPE%
echo Firmware:  !FIRMWARE!
echo.

if "%DEVICE_TYPE%" == "MX8M_EVK" (
  :: 66*512=0x8400
  set CF_IMAGER_OFFSET=0x8400
) else (
  if "%DEVICE_TYPE%" == "MX8M_MINI_EVK" (
    set CF_IMAGER_OFFSET=0x8400
  ) else (
    if "%DEVICE_TYPE%" == "MX8M_NANO_EVK" (
      :: 64*512=0x8000
      set CF_IMAGER_OFFSET=0x8000
      set UUU_LOAD2M=1
    ) else (
      if "%DEVICE_TYPE%" == "MX8M_PLUS_EVK" (
        set CF_IMAGER_OFFSET=0x8000
        set UUU_LOAD2M=1
      ) else (
	    if "%DEVICE_TYPE%" == "MX8QXP_MEK" (
          set CF_IMAGER_OFFSET=0x8000
        ) else (
          echo Error. The /device %DEVICE_TYPE% is not valid.
          exit /b 1
		)
      )
    )
  )
)

:: Write firmware to a drive plugged into PC using Cfimager. (https://www.nxp.com/webapp/Download?colCode=CF_IMAGER&appType=license&location=null)
:: Alternatively dd if="!FIRMWARE!" of="\\.\PhysicalDrive%DISK_NUM%" bs=512 seek=%SEEK% skip=0 .
if "%TARGET_DRIVE%"=="" (
  goto:FLASH_eMMC
)

REM Check size of selected drive and determine whether it is an SD card to protect hard-drives from overwriting
REM save size of selected disk in bytes into DISKSIZE
for /f "usebackq skip=1 tokens=*" %%i in (`wmic logicaldisk %target_drive% get size ^| findstr /r /v "^$"`) do set DISKSIZE=%%i

if "%DISKSIZE%" == "" (
  echo Selected drive^(%target_drive%^) is invalid. Select a valid disk using option /target_drive.
  exit /b 1
)
REM remove spaces from DISKSIZE
set DISKSIZE=%DISKSIZE: =%
REM convert DISKSIZE fro B to GB
set /A GB=%DISKSIZE:~0,-3%/1024/1024
REM Check the size of selected drive, if larger than 64GB, it's probably a hard drive, so the script will end
REM conversion is not 100% precise due to limitations of batch scripts, so comparing to 65 instead of 64
if !GB! GEQ 65 (
    echo Selected disk is too big. Make sure to choose a SD card as this operation would clear the selected disk.
    exit /b 1
)


where "%CFIMAGER%" > nul

if errorlevel 1 (
  echo %CFIMAGER% not found.
  echo Refer to User's guide for download.
  exit /b 1
)

%CFIMAGER% -raw -offset %CF_IMAGER_OFFSET% -f "!FIRMWARE!" -d %TARGET_DRIVE%
if not errorlevel 0 (
  echo Failed to write boot firmware to the "%TARGET_DRIVE%". Check whether the TARGET_DRIVE is present and is not write protected.
  exit /b 1
) else (
  goto:eof
)
:: End of SD_CARD_DOWNLOAD

:FLASH_eMMC
:: Write firmware to onboard memory.

@REM Check whether information about secure/nonsecure firmware was passed as argument, show dialog if not...
if not !SECURE_FW! == 0 (
  if not !SECURE_FW! == 1 (
    echo Option /secure was not specified, please select:
    echo 0. nonsecure
    echo 1. secure
    set /p FW_VER=Enter index:
    if not !FW_VER! == 0 (
	  if not !FW_VER! == 1 (
          echo Invalid option!
          exit /b 1
      )
	)
    set SECURE_FW=!FW_VER!
  )
)

@REM Set the default temporary firmware binary for flashing firmware to EMMC
if !SECURE_FW! == 1 (
  set TEMP_FIRMWARE=!FIRMWARE_DIR!\signed_firmware_uuu.bin
) else (
  set TEMP_FIRMWARE=!FIRMWARE_DIR!\firmware_uuu.bin
)

@REM Check whether the temporary firmware exists, show selection dialog if does not exist
if not exist !TEMP_FIRMWARE! (
  echo File !TEMP_FIRMWARE! required for flashing !FIRMWARE! not found.
  echo Please build the required firmware or select custom firmware with UUU tool support
  echo 0. Exit
  set BIN_COUNT=0
  for /f "tokens=*" %%i in ('dir "!FIRMWARE_DIR!\*.bin" /b') do (
      set /a BIN_COUNT+=1
      echo !BIN_COUNT!. %%i
  )
  if !BIN_COUNT! == 0 (
      echo No bin file was found in !FIRMWARE_DIR!\
      exit /b 1
  )

  set /p BIN_IDX=Enter index:
  if !BIN_IDX! gtr !BIN_COUNT! (
      echo Selected index is out of range
      exit /b 1
  )
  if !BIN_IDX! leq 0 (
      echo Exit
      exit /b 1
  )

  set BIN_COUNT=0
  for /f "tokens=*" %%i in ('dir "!FIRMWARE_DIR!\*.bin" /b') do (
      set /a BIN_COUNT+=1
      if !BIN_COUNT! == !BIN_IDX! (
          echo Selected: %%i
          echo:
          set TEMP_FIRMWARE=!FIRMWARE_DIR!\%%i
      )
  )
)

echo Temporary firmware with UUU tool: !TEMP_FIRMWARE!

SET device_mode=0

if not exist %UUU% (
  echo %UUU% not found.
  echo Refer to User's guide for download.
)

call:CHECK_DEVICE_MODE device_mode

IF "%device_mode%" == "none" (
	ECHO Device not in download mode
  ECHO Make sure device is connected to the host PC, turned on and set to download mode.
	pause
	exit /b 1
)



ECHO Device in %device_mode% download mode, start flash!!
ECHO.
%UUU% -b emmc "!TEMP_FIRMWARE!" "!FIRMWARE!"
pause
goto:eof

:CHECK_DEVICE_MODE
%UUU% -lsusb | findstr /r /c:"FB"
IF %ERRORLEVEL% == 0  (
  SET %~1=fastboot
  goto:eof
)

%UUU% -lsusb 2>&1 | findstr /r /c:"SDP"
IF %ERRORLEVEL% == 0  (
  SET %~1=serial
  goto:eof
)
SET %~1=none
goto:eof
:USAGE
echo flash_bootloader.cmd /device <NAME> [/target_drive, /directory, /firmware]
echo.
echo Flashes a firmware image
echo Options:
echo    /device                      {MX8M_EVK, MX8M_MINI_EVK, MX8M_NANO_EVK, MX8M_PLUS_EVK, MX8QXP_MEK}
echo                                 Specifies the device
echo.
echo Optional options:
echo    /target_drive                Specifies whether the firmware binary should be written to this drive
echo                                 instead of internal memory of the development board. Useful for
echo                                 firmware development.
echo                                 Caution, do not overwrite your system drive.
echo.  /directory                    Specifies directory in which firmware.bin is located.
echo   /firmware                     Specifies absolute or relative path to firmware.
echo   /secure                       Specifies which firmware will be loaded to onboard memory:
echo                                 0 = nonsecure, 1 = secure.
echo Examples:
echo.
echo Flash the Windows Enterprise firmware to the MMC of a EVK board.
echo    flash_bootloader.cmd /device MX8M_EVK
echo.
echo Flash the Windows Enterprise firmware to an SD card.
echo    flash_bootloader.cmd /device MX8M_EVK /target_drive f:
echo.
