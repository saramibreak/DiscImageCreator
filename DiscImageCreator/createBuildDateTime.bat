@echo off

set HEADERFILE=%~dp0buildDateTime.h

echo #pragma once> "%HEADERFILE%"
echo.>> "%HEADERFILE%"
echo #define BUILD_DATE "%DATE:~-10,4%%DATE:~-5,2%%DATE:~-2%">> "%HEADERFILE%"
set TIME2=%TIME: =0%
echo #define BUILD_TIME "%TIME2:~0,2%%TIME2:~3,2%%TIME2:~6,2%">> "%HEADERFILE%"
