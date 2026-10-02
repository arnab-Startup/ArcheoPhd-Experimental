@echo off
setlocal
echo ============================================================
echo   ArchaeoPhD Experimental Workstation — Standalone .EXE Build
echo ============================================================

cd /d "%~dp0"

REM Terminate any running instance of ArchaeoPhD-Experimental.exe to avoid file lock
taskkill /F /IM ArchaeoPhD-Experimental.exe >nul 2>&1

REM Clean stale experimental AppData cache if present
if exist "%LOCALAPPDATA%\ArchaeoPhD_Experimental\app" rmdir /s /q "%LOCALAPPDATA%\ArchaeoPhD_Experimental\app" >nul 2>&1

REM 1. Ensure dist and release\dist folders have latest experimental index.html
if not exist "dist" mkdir dist
copy /y "index.html" "dist\index.html" >nul
if not exist "release" mkdir release
if not exist "release\dist" mkdir release\dist
copy /y "index.html" "release\dist\index.html" >nul

REM Copy WebView2Loader.dll beside binary in both root and release
copy /y "..\desktop\lib\WebView2Loader.dll" "release\WebView2Loader.dll" >nul
copy /y "..\desktop\lib\WebView2Loader.dll" "WebView2Loader.dll" >nul

REM 2. Package runtime payload archive (WebView2Loader.dll + dist)
echo Packaging experimental runtime payload archive...
tar -a -cf payload.zip WebView2Loader.dll dist

REM 3. Compile Windows PE resource with windres
echo Compiling embedded resources with windres...
windres -I . src\resource.rc -O coff -o src\resource.res
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Resource compilation failed.
    exit /b %ERRORLEVEL%
)

REM 4. Compile standalone ArchaeoPhD-Experimental.exe with MinGW G++ (Statically linked: zero MinGW DLL dependencies)
echo Compiling self-contained ArchaeoPhD-Experimental.exe with MinGW G++ (statically linked)...
g++ -std=c++14 -O2 -s -mwindows -static -static-libgcc -static-libstdc++ ^
    -I ..\desktop\include ^
    -I ..\desktop\engine\src ^
    src\app_main.cpp ^
    src\resource.res ^
    -o release\ArchaeoPhD-Experimental.exe ^
    -lole32 -loleaut32 -luuid -luser32 -lshell32 -lshlwapi

if %ERRORLEVEL% EQU 0 (
    REM Clean up temporary build artifacts
    if exist "payload.zip" del "payload.zip" >nul
    if exist "src\resource.res" del "src\resource.res" >nul

    echo.
    echo [SUCCESS] ArchaeoPhD-Experimental.exe built successfully!
    echo Output location: release\ArchaeoPhD-Experimental.exe
    echo Binary size:
    dir release\ArchaeoPhD-Experimental.exe | findstr /i "ArchaeoPhD-Experimental.exe"
    echo.
    echo To run the experimental desktop application:
    echo   .\experimental\ArchaeoPhD-Experimental.exe
) else (
    echo.
    echo [ERROR] Build failed with exit code %ERRORLEVEL%.
)
endlocal
