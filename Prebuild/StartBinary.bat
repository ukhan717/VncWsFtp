@echo off
if exist "JLink.exe" (
  JLink.exe -device STM32F746NG -CommanderScript CommandFile.jlink
  goto END
) else (
  echo JLink.exe not present in folder. Press any key in order to use windows registry to locate JLink.exe
)
pause

reg query HKCU\Software\SEGGER\J-Link /v InstallPath
IF ERRORLEVEL 1 goto ERR
for /f "tokens=3,*" %%b in ('reg query "HKCU\Software\SEGGER\J-Link" /v InstallPath  ^|findstr /ri "REG_SZ"') do set _JLINKEXE="%%b %%cjlink.exe"
%_JLINKEXE% -device STM32F746NG -CommanderScript CommandFile.jlink

:END
pause
goto :EOF

:ERR
echo Error: J-Link software and documentation package not installed or not found.
pause



