@echo off
REM ============================================================================
REM EvoSim Build Script
REM Uses the bundled w64devkit g++ compiler
REM ============================================================================

echo.
echo ================================
echo   Building EvoSim...
echo ================================
echo.

set "DEVKIT=%~dp0w64devkit\bin"
set "GPP=%DEVKIT%\g++.exe"

if not exist "%GPP%" (
    echo [ERROR] g++ not found at: %GPP%
    echo Make sure w64devkit is extracted in the EvoSim folder.
    pause
    exit /b 1
)

REM Add w64devkit/bin to PATH so g++ can find assembler, linker, etc.
set "PATH=%DEVKIT%;%PATH%"

echo Using compiler: %GPP%
echo.

"%GPP%" -std=c++17 -O2 -o EvoSim.exe main.cpp -I.

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ================================
    echo   BUILD SUCCESS!
    echo   Run with: EvoSim.exe
    echo   Or:       EvoSim.exe --help
    echo ================================
    echo.
) else (
    echo.
    echo ================================
    echo   BUILD FAILED!
    echo   Check the errors above.
    echo ================================
    echo.
)
pause
