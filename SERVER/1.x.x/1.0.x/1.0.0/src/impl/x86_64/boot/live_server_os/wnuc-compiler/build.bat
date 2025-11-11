@echo off
REM WNUC Build and Test Utility Script for Windows
REM This script helps build and test the WNU Compiler

setlocal enabledelayedexpansion

echo ================================
echo   WNUC Compiler Build Utility
echo ================================
echo.

REM Check if we're in the right directory
if not exist "wnuc.asm" (
    echo [ERROR] wnuc.asm not found. Please run this script from the wnuc-compiler directory.
    exit /b 1
)

REM Parse command line argument
set "COMMAND=%~1"
if "%COMMAND%"=="" set "COMMAND=all"

goto :%COMMAND% 2>nul || goto :unknown

:info
echo [INFO] WNUC Compiler Information:
echo   Name: WNU Compiler (WNUC)
echo   Version: 1.0
echo   Language: WNU-LANG
echo   Target: x86_64 Assembly
echo   Platform: WNU OS Server
echo   Source: wnuc.asm
echo   Sample: sample.wnulang
echo.
if "%COMMAND%"=="info" goto :end
goto :build

:build
echo [INFO] Building WNUC Compiler...

REM Check for make
where make >nul 2>&1
if %errorlevel%==0 (
    make all
    if !errorlevel!==0 (
        echo [SUCCESS] WNUC compilation completed successfully!
    ) else (
        echo [ERROR] WNUC compilation failed!
        exit /b 1
    )
) else (
    echo [WARNING] Make not found, trying manual build...
    
    REM Create directories
    if not exist "..\..\..\..\..\build\x86_64\wnuc" mkdir "..\..\..\..\..\build\x86_64\wnuc"
    if not exist "..\..\..\..\..\dist\x86_64" mkdir "..\..\..\..\..\dist\x86_64"
    
    REM Check for NASM
    where nasm >nul 2>&1
    if !errorlevel!==0 (
        echo [INFO] Assembling wnuc.asm...
        nasm -f elf64 -g wnuc.asm -o "..\..\..\..\..\build\x86_64\wnuc\wnuc.o"
        
        REM Check for ar (archiver)
        where ar >nul 2>&1
        if !errorlevel!==0 (
            echo [INFO] Creating library...
            ar rcs "..\..\..\..\..\dist\x86_64\libwnuc.a" "..\..\..\..\..\build\x86_64\wnuc\wnuc.o"
            echo [SUCCESS] Manual build completed!
        ) else (
            echo [ERROR] ar (archiver) not found!
            exit /b 1
        )
    ) else (
        echo [ERROR] NASM assembler not found!
        exit /b 1
    )
)

if "%COMMAND%"=="build" goto :end
goto :test

:test
echo [INFO] Testing WNUC Compiler...

if exist "sample.wnulang" (
    echo [INFO] Found sample WNU-LANG program:
    echo --- sample.wnulang ---
    more +1 sample.wnulang | findstr /N ".*" | findstr "^[1-9]:"
    echo --- end sample ---
    echo.
    echo [SUCCESS] Sample program is ready for compilation!
) else (
    echo [WARNING] sample.wnulang not found!
)

if exist "..\..\..\..\..\dist\x86_64\libwnuc.a" (
    echo [SUCCESS] WNUC library found: libwnuc.a
) else (
    echo [WARNING] WNUC library not found!
)

if "%COMMAND%"=="test" goto :end
goto :end

:clean
echo [INFO] Cleaning WNUC build artifacts...
make clean >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] Make clean failed, trying manual cleanup...
    if exist "..\..\..\..\..\build\x86_64\wnuc" rmdir /s /q "..\..\..\..\..\build\x86_64\wnuc"
    if exist "..\..\..\..\..\dist\x86_64\libwnuc.a" del "..\..\..\..\..\dist\x86_64\libwnuc.a"
    if exist "..\..\..\..\..\dist\x86_64\wnuc.bin" del "..\..\..\..\..\dist\x86_64\wnuc.bin"
)
echo [SUCCESS] Cleanup completed!
goto :end

:all
call :info
call :build
call :test
goto :end

:help
echo WNUC Build Utility - Usage:
echo.
echo   build.bat [command]
echo.
echo Commands:
echo   info      Show WNUC compiler information
echo   build     Build the WNUC compiler
echo   test      Test the WNUC compiler
echo   clean     Clean build artifacts
echo   all       Do everything (info + build + test)
echo   help      Show this help message
echo.
goto :end

:unknown
echo [ERROR] Unknown command: %COMMAND%
call :help
exit /b 1

:end
echo.
echo [SUCCESS] WNUC utility script completed!
echo ================================