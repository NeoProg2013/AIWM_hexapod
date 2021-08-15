@echo off

set date_dd=%date:~0,2%
set date_mm=%date:~3,2%
set date_yyyy=%date:~8,2%

cd /d %~dp0

set /p build=<build_number
set /a build=%build%+1
@echo %build%> build_number

@echo //  ***************************************************************************> ../src/version.h
@echo /// @file    version.h>> ../src/version.h
@echo /// @author  NeoProg>> ../src/version.h
@echo /// @brief   Firmware version information>> ../src/version.h
@echo //  ***************************************************************************>> ../src/version.h
@echo #ifndef _VERSION_H_>> ../src/version.h
@echo #define _VERSION_H_>> ../src/version.h
@echo. >> ../src/version.h
@echo. >> ../src/version.h

@echo #define FIRMWARE_VERSION				(^"1.01.%date_yyyy%%date_mm%%date_dd% %build% %1^")>> ../src/version.h

@echo. >> ../src/version.h
@echo. >> ../src/version.h
@echo #endif // _VERSION_H_>> ../src/version.h