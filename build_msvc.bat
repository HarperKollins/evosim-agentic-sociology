@echo off
setlocal enabledelayedexpansion

echo ================================
echo   Building EvoSim (vswhere method)
echo ================================

REM 1. Find vswhere.exe
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
) else (
    if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" (
        set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
    ) else (
        echo [ERROR] vswhere.exe not found!
        exit /b 1
    )
)

echo Found vswhere at: "!VSWHERE!"

REM 2. Find Visual Studio Installation Path
for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo [ERROR] No Visual Studio with C++ tools found!
    exit /b 1
)

echo Found VS at: "!VS_PATH!"

REM 3. Find vcvarsall.bat
set "VCVARS=!VS_PATH!\VC\Auxiliary\Build\vcvarsall.bat"

if not exist "!VCVARS!" (
    echo [ERROR] vcvarsall.bat not found at: "!VCVARS!"
    exit /b 1
)

echo Found vcvarsall at: "!VCVARS!"

REM 4. Setup Environment
call "!VCVARS!" x64

REM 5. Compile
echo.
echo Compiling...
cl.exe /std:c++17 /EHsc /O2 /Fe:EvoSim.exe main.cpp

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ================================
    echo   Build SUCCESS!
    echo   Run with: EvoSim.exe
    echo ================================
) else (
    echo.
    echo [ERROR] Build Failed!
)
