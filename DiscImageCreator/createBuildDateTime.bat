@echo off

set HEADERFILE=%~dp0buildDateTime.h

echo #pragma once> "%HEADERFILE%"
echo.>> "%HEADERFILE%"
echo #define BUILD_DATE "%DATE:~-10,4%%DATE:~-5,2%%DATE:~-2%">> "%HEADERFILE%"
echo #define BUILD_TIME "%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%">> "%HEADERFILE%"
