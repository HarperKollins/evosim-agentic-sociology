@echo off
echo Building tests...
g++ -std=c++17 -O2 -o run_tests.exe tests/unit_tests.cpp
if %errorlevel% neq 0 exit /b %errorlevel%

echo Running tests...
.\run_tests.exe
echo Done.
