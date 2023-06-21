@echo off
REM Copy ./imx-windows-iot/BSP/firmware/NXPEVK_*/FirmwareCapsuleIMX.cap to ./firmware.bin
REM Created based on https://docs.microsoft.com/en-us/windows-hardware/drivers/bringup/authoring-an-update-driver-package

REM "c:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\makecert.exe" -r -pe -a sha256 -eku 1.3.6.1.5.5.7.3.3 -n CN=NXP -sv fwu.pvk fwu.cer
REM "c:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\pvk2pfx.exe" -pvk fwu.pvk -spc fwu.cer -pi imx8 -spc fwu.cer -pfx fwu.pfx
REM "c:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\inf2cat.exe" /driver:. /os:10_x64 /verbose
REM "c:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\signtool.exe" sign /fd sha256 /f fwu.pfx /p imx8 catalog.cat

@echo on
"c:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\inf2cat.exe" /driver:. /os:10_x64 /verbose
"c:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\signtool.exe" sign /debug /ph /fd sha256 /sha1 749E4CC65EE7B3E1CFF5E51B9EEC5560225A44D7 catalog.cat
