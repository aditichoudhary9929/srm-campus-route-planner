@echo off
REM ============================================================================
REM build.bat - Windows Batch Build Script for SRM IST Campus Navigation
REM Allows building with pure GCC when 'make' is not installed.
REM Usage:
REM   build.bat           - Compiles campus_nav.exe
REM   build.bat test      - Compiles and runs test_runner.exe
REM   build.bat sanitize  - Compiles with Address & Undefined Sanitizer
REM   build.bat clean     - Removes compiled executables and object files
REM ============================================================================

set CC=gcc
set CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -g
set MODULE_SRCS=graph.c heap.c trie.c hash.c history.c algo.c file.c

if "%1"=="clean" goto clean
if "%1"=="test" goto test
if "%1"=="sanitize" goto sanitize

:build
echo [BUILD] Compiling campus_nav.exe with GCC (C11, strict warnings)...
%CC% %CFLAGS% -o campus_nav.exe %MODULE_SRCS% main.c
if %ERRORLEVEL% equ 0 (
    echo [BUILD SUCCESS] campus_nav.exe generated successfully!
) else (
    echo [BUILD ERROR] Compilation failed!
)
goto end

:test
echo [TEST] Compiling and running tests...
if not exist tests mkdir tests
%CC% %CFLAGS% -I. -o test_runner.exe %MODULE_SRCS% tests\test_main.c
if %ERRORLEVEL% equ 0 (
    echo [TEST BUILD SUCCESS] Running test_runner.exe:
    echo ----------------------------------------------------
    test_runner.exe
    echo ----------------------------------------------------
) else (
    echo [TEST BUILD ERROR] Test compilation failed!
)
goto end

:sanitize
echo [SANITIZE] Compiling with -fsanitize=address,undefined...
%CC% %CFLAGS% -fsanitize=address,undefined -o campus_nav.exe %MODULE_SRCS% main.c
if %ERRORLEVEL% equ 0 (
    echo [SANITIZE SUCCESS] Sanitized campus_nav.exe generated!
) else (
    echo [SANITIZE ERROR] Compilation with sanitizer failed!
)
goto end

:clean
echo [CLEAN] Removing object files and binaries...
if exist *.o del /Q *.o
if exist campus_nav.exe del /Q campus_nav.exe
if exist test_runner.exe del /Q test_runner.exe
echo [CLEAN] Finished.
goto end

:end
