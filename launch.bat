@echo off
setlocal
echo ============================================================
echo   Launching ArchaeoPhD Experimental Workstation (.EXE Mode)
echo   Zero HTTP / Pure In-Memory / 100%% Offline Native Windows
echo ============================================================

cd /d "%~dp0"

REM If the compiled native .exe doesn't exist yet, build it automatically
if not exist "release\ArchaeoPhD-Experimental.exe" (
    echo Native executable not found. Compiling with MinGW G++...
    call build.bat
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] Build failed. Please ensure MinGW g++ is in PATH.
        exit /b %ERRORLEVEL%
    )
)

echo Starting ArchaeoPhD-Experimental.exe...
start "" "release\ArchaeoPhD-Experimental.exe"
endlocal

