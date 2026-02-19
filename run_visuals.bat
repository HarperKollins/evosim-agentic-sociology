@echo off
echo ==================================================
echo   EvoSim: Visual Experience Launcher
echo ==================================================

:: 1. Start Python HTTP Server in background
echo [1/3] Starting Local Web Server...
start /B python -m http.server 8000
timeout 2 > nul

:: 2. Run the Simulation (Parallel)
echo [2/3] Running Simulation (Background)...
start EvoSim.exe --maxpop 1000

:: 3. Open Viewer (Immediately)
echo [3/3] Opening Visualizer...
echo       The viewer will wait for the logs to appear...
timeout 2 > nul
start http://localhost:8000/viewer/index.html?autoload=true

echo.
echo ==================================================
echo   Simulation Complete!
echo   Watch the history unfold in your browser.
echo ==================================================
pause
taskkill /IM python.exe /F
