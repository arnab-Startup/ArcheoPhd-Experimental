@echo off
setlocal
echo ============================================================
echo   ArchaeoPhD Desktop Workstation — Windows C++ Build
echo ============================================================

cd /d "%~dp0"

REM Terminate any running instance of ArchaeoPhD.exe to avoid file lock
powershell -NoProfile -Command "Get-Process -Name ArchaeoPhD -ErrorAction SilentlyContinue | Stop-Process -Force" >nul 2>&1
if exist "release\ArchaeoPhD.old.exe" del /f /q "release\ArchaeoPhD.old.exe" >nul 2>&1
if exist "release\ArchaeoPhD.exe" move /y "release\ArchaeoPhD.exe" "release\ArchaeoPhD_%RANDOM%.old.exe" >nul 2>&1
del /f /q "release\*.old.exe" >nul 2>&1

REM Prefer modern 64-bit C++20 toolchain (WinLibs / GCC 16)
if exist "%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin" (
    set "PATH=%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin;%PATH%"
)

REM 1. Package runtime payload archive (WebView2Loader.dll + dist)
echo Packaging runtime payload archive...
copy /y "lib\WebView2Loader.dll" "WebView2Loader.dll" >nul
tar -a -cf payload.zip WebView2Loader.dll dist
del "WebView2Loader.dll" >nul

REM 2. Compile Windows PE resource with windres
echo Compiling embedded resources with windres...
windres -I . src\resource.rc -O coff -o src\resource.res
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Resource compilation failed.
    exit /b %ERRORLEVEL%
)

REM 3. Ensure release directory exists and is populated with up-to-date dist assets
if not exist "release" mkdir release
xcopy /E /I /Y "dist" "release\dist" >nul
copy /y "lib\WebView2Loader.dll" "release\WebView2Loader.dll" >nul

REM 4. Compile standalone ArchaeoPhD.exe with MinGW G++ (Statically linked: zero MinGW DLL dependencies)
echo Compiling self-contained ArchaeoPhD.exe with MinGW G++ C++20 (statically linked)...
g++ -std=c++20 -O2 -s -mwindows -static -static-libgcc -static-libstdc++ ^
    -DHAS_LLAMA_CPP ^
    -I include ^
    -I engine ^
    -I engine\core ^
    -I engine\storage ^
    -I engine\analysis ^
    -I engine\extraction ^
    -I engine\validation ^
    -I third_party\llama.cpp\include ^
    -I third_party\llama.cpp\ggml\include ^
    src\main.cpp ^
    src\resource.res ^
    third_party\llama.cpp\build\src\libllama.a ^
    third_party\llama.cpp\build\ggml\src\ggml.a ^
    third_party\llama.cpp\build\ggml\src\ggml-cpu.a ^
    third_party\llama.cpp\build\ggml\src\ggml-base.a ^
    -fopenmp ^
    -o release\ArchaeoPhD.exe ^
    -lole32 -loleaut32 -luuid -luser32 -lshell32 -lshlwapi -lcomdlg32

if %ERRORLEVEL% EQU 0 (
    REM Clean up temporary build artifacts
    if exist "payload.zip" del "payload.zip" >nul
    if exist "src\resource.res" del "src\resource.res" >nul

    echo.
    echo [SUCCESS] ArchaeoPhD.exe built successfully!
    echo Output location: release\ArchaeoPhD.exe
    echo Binary size:
    dir release\ArchaeoPhD.exe | findstr /i "ArchaeoPhD.exe"
    echo.
    echo To run the desktop application:
    echo   .\release\ArchaeoPhD.exe
) else (
    echo.
    echo [ERROR] Build failed with exit code %ERRORLEVEL%.
)
endlocal
