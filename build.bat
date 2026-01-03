@echo off
setlocal

REM =========================================================
REM Directories & filenames
REM =========================================================
set BUILD_DIR=build
set OUT=tnfs_test.exe

REM =========================================================
REM Compiler settings
REM =========================================================
set CC=gcc
set CFLAGS=-Wall -Wextra -DDEBUG
set LIBS=-lws2_32

REM =========================================================
REM Create build directory if it doesn't exist
REM =========================================================
if not exist %BUILD_DIR% (
    echo [INFO] Creating build directory "%BUILD_DIR%"
    mkdir %BUILD_DIR%
)

REM =========================================================
REM Build
REM =========================================================
echo [BUILD] Compiling TNFS test client (DEBUG)...

%CC% ^
    %CFLAGS% ^
    main.c ^
    tnfs.c ^
    netw_win32.c ^
    %LIBS% ^
    -o %BUILD_DIR%\%OUT%

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    exit /b 1
)

REM =========================================================
REM Run
REM =========================================================
echo.
echo [RUN] Starting %BUILD_DIR%\%OUT%
echo ----------------------------------------
%BUILD_DIR%\%OUT%

endlocal
