@echo off

set date_dd=%date:~0,2%
set date_mm=%date:~3,2%
set date_yyyy=%date:~6,4%
set time_mm=%time:~3,2%
set time_ss=%time:~6,2%

cd /d %~dp0

@echo //  ***************************************************************************> src/version.h
@echo /// @file    version.h>> src/version.h
@echo /// @author  NeoProg>> src/version.h
@echo /// @brief   Firmware version information>> src/version.h
@echo //  ***************************************************************************>> src/version.h
@echo #ifndef _VERSION_H_>> src/version.h
@echo #define _VERSION_H_>> src/version.h
@echo. >> src/version.h
@echo. >> src/version.h

@echo #define FIRMWARE_VERSION				(^"%date_dd%%date_mm%%date_yyyy% %time_mm%%time_ss%^")>> src/version.h

@echo. >> src/version.h
@echo. >> src/version.h
@echo #endif // _VERSION_H_>> src/version.h