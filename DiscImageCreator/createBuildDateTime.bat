@echo off

set HEADERFILE=%~dp0buildDateTime.h

echo #pragma once> "%HEADERFILE%"
echo.>> "%HEADERFILE%"

for /f %%a in ('wmic os get LocalDateTime ^| findstr \.') DO set CUR_DATE=%%a

echo #define BUILD_DATE "%CUR_DATE:~0,8%">> "%HEADERFILE%"
echo #define BUILD_TIME "%CUR_DATE:~8,6%">> "%HEADERFILE%"
