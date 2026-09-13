@echo off

set "HEADERFILE=%~dp0buildDateTime.h"
set "VBSFILE=%TEMP%\get_build_datetime_%RANDOM%.vbs"

echo #pragma once> "%HEADERFILE%"
echo.>> "%HEADERFILE%"

> "%VBSFILE%" echo d = Now
>> "%VBSFILE%" echo WScript.Echo Year(d) ^& Right("0" ^& Month(d), 2) ^& Right("0" ^& Day(d), 2) ^& Right("0" ^& Hour(d), 2) ^& Right("0" ^& Minute(d), 2) ^& Right("0" ^& Second(d), 2)

for /f %%a in ('cscript.exe //nologo "%VBSFILE%"') do set "CUR_DATE=%%a"

del /q "%VBSFILE%"

echo #define BUILD_DATE "%CUR_DATE:~0,8%">> "%HEADERFILE%"
echo #define BUILD_TIME "%CUR_DATE:~8,6%">> "%HEADERFILE%"