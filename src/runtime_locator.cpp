#include "runtime_locator.hpp"
#include "app_context.hpp"
#include <shellapi.h>
#include <vector>
#include <fstream>

std::wstring PathToFileUri(const std::wstring& path) {
    std::wstring uri = L"file:///";
    for (wchar_t c : path) {
        if (c == L'\\') {
            uri += L'/';
        } else {
            uri += c;
        }
    }
    return uri;
}

bool EnsureRuntimeExtracted(std::wstring& outDistFolder, std::wstring& outLoaderPath) {
    wchar_t exePathBuffer[MAX_PATH];
    GetModuleFileNameW(nullptr, exePathBuffer, MAX_PATH);
    std::wstring exeDir = exePathBuffer;
    size_t lastSlash = exeDir.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        exeDir = exeDir.substr(0, lastSlash);
    }

    // 1. Fast path: check if dist/index.html and WebView2Loader.dll already exist beside the EXE
    std::wstring localDist = exeDir + L"\\dist";
    std::wstring localLoader = exeDir + L"\\WebView2Loader.dll";
    if (GetFileAttributesW((localDist + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES &&
        GetFileAttributesW(localLoader.c_str()) != INVALID_FILE_ATTRIBUTES) {
        outDistFolder = localDist;
        outLoaderPath = localLoader;
        return true;
    }

    // 1b. Developer path: check if running from release/ inside the desktop workspace
    std::wstring parentDist = exeDir + L"\\..\\dist";
    std::wstring parentLoader = exeDir + L"\\..\\lib\\WebView2Loader.dll";
    if (GetFileAttributesW((parentDist + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES &&
        GetFileAttributesW(parentLoader.c_str()) != INVALID_FILE_ATTRIBUTES) {
        outDistFolder = parentDist;
        outLoaderPath = parentLoader;
        return true;
    }

    // 2. Persistent AppData location for extracted standalone payload
    wchar_t localAppData[MAX_PATH];
    std::wstring baseDir;
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH) > 0) {
        baseDir = std::wstring(localAppData) + L"\\ArchaeoPhD";
    } else {
        baseDir = exeDir;
    }

    std::wstring appPayloadDir = baseDir + L"\\app";
    std::wstring targetDist = appPayloadDir + L"\\dist";
    std::wstring targetLoader = appPayloadDir + L"\\WebView2Loader.dll";
    std::wstring stampFile = appPayloadDir + L"\\build_signature.txt";
    static const std::string currentSignature = std::string("ARCHAEOPHD_BUILD_") + __DATE__ + "_" + __TIME__;

    // Check if previously extracted files match current binary signature
    bool alreadyExtracted = (GetFileAttributesW((targetDist + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES &&
                             GetFileAttributesW((targetDist + L"\\ingestion-workflow.js").c_str()) != INVALID_FILE_ATTRIBUTES &&
                             GetFileAttributesW(targetLoader.c_str()) != INVALID_FILE_ATTRIBUTES);
    if (alreadyExtracted) {
        std::ifstream sigIn(archaeophd::DataRootManager::WideToUtf8(stampFile));
        std::string existingSig;
        if (sigIn.is_open()) {
            std::getline(sigIn, existingSig);
            sigIn.close();
        }
        if (existingSig == currentSignature) {
            outDistFolder = targetDist;
            outLoaderPath = targetLoader;
            return true;
        }
    }

    // 3. Extract embedded payload from resource
    HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(IDR_APP_PAYLOAD), (LPCWSTR)RT_RCDATA);
    if (!hRes) {
        outDistFolder = localDist;
        outLoaderPath = localLoader;
        return false;
    }

    HGLOBAL hData = LoadResource(nullptr, hRes);
    if (!hData) return false;
    DWORD resSize = SizeofResource(nullptr, hRes);
    void* resData = LockResource(hData);
    if (!resData || resSize == 0) return false;

    CreateDirectoryW(baseDir.c_str(), nullptr);
    CreateDirectoryW(appPayloadDir.c_str(), nullptr);
    std::wstring doubleNullTarget = targetDist;
    doubleNullTarget.push_back(L'\0');
    SHFILEOPSTRUCTW shfo = { 0 };
    shfo.wFunc = FO_DELETE;
    shfo.pFrom = doubleNullTarget.c_str();
    shfo.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_SILENT;
    SHFileOperationW(&shfo);

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring tempZip = std::wstring(tempPath) + L"archaeophd_payload.zip";

    HANDLE hFile = CreateFileW(tempZip.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0;
        WriteFile(hFile, resData, resSize, &bytesWritten, nullptr);
        CloseHandle(hFile);

        // Try extraction with tar.exe first
        std::wstring tarCmd = L"tar.exe -xf \"" + tempZip + L"\" -C \"" + appPayloadDir + L"\"";
        STARTUPINFOW si = { sizeof(si) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = { 0 };

        std::vector<wchar_t> cmdBuf(tarCmd.begin(), tarCmd.end());
        cmdBuf.push_back(L'\0');

        bool extracted = false;
        if (CreateProcessW(nullptr, cmdBuf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 10000);
            DWORD ec = 0;
            GetExitCodeProcess(pi.hProcess, &ec);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            if (ec == 0 && GetFileAttributesW((targetDist + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
                extracted = true;
            }
        }

        // Fallback to PowerShell Expand-Archive if tar failed
        if (!extracted) {
            std::wstring psCmd = L"powershell.exe -NoProfile -NonInteractive -WindowStyle Hidden -Command \"Expand-Archive -Path '" + tempZip + L"' -DestinationPath '" + appPayloadDir + L"' -Force\"";
            std::vector<wchar_t> psBuf(psCmd.begin(), psCmd.end());
            psBuf.push_back(L'\0');
            STARTUPINFOW psSi = { sizeof(psSi) };
            psSi.dwFlags = STARTF_USESHOWWINDOW;
            psSi.wShowWindow = SW_HIDE;
            PROCESS_INFORMATION psPi = { 0 };
            if (CreateProcessW(nullptr, psBuf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &psSi, &psPi)) {
                WaitForSingleObject(psPi.hProcess, 15000);
                CloseHandle(psPi.hProcess);
                CloseHandle(psPi.hThread);
            }
        }

        DeleteFileW(tempZip.c_str());
    }

    if (GetFileAttributesW((targetDist + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
        std::ofstream sigOut(archaeophd::DataRootManager::WideToUtf8(stampFile));
        if (sigOut.is_open()) {
            sigOut << currentSignature << std::endl;
            sigOut.close();
        }
        outDistFolder = targetDist;
        outLoaderPath = targetLoader;
        return true;
    }

    outDistFolder = localDist;
    outLoaderPath = localLoader;
    return false;
}

void CleanStaleWebView2Locks(const std::wstring& userDataDir) {
    std::wstring eb = userDataDir + L"\\EBWebView";
    DeleteFileW((eb + L"\\SingletonLock").c_str());
    DeleteFileW((eb + L"\\SingletonCookie").c_str());
    DeleteFileW((eb + L"\\SingletonSocket").c_str());
    DeleteFileW((eb + L"\\lockfile").c_str());
}

bool IsDirectoryFullyWritable(const std::wstring& dir) {
    if (dir.empty()) return false;
    try {
        archaeophd::fs_compat::create_directories(archaeophd::DataRootManager::WideToUtf8(dir));
    } catch (...) {
        return false;
    }
    DWORD attribs = GetFileAttributesW(dir.c_str());
    if (attribs == INVALID_FILE_ATTRIBUTES || !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
        return false;
    }
    // Check if EBWebView lock file exists and is locked exclusively by another process
    std::wstring lockPath = dir + L"\\EBWebView\\lockfile";
    if (GetFileAttributesW(lockPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        HANDLE hLock = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hLock == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION || err == ERROR_ACCESS_DENIED) {
                return false;
            }
        } else {
            CloseHandle(hLock);
        }
    }
    // Test write and delete
    std::wstring probeFile = dir + L"\\.apd_wv2_probe_" + std::to_wstring(GetCurrentProcessId());
    HANDLE hProbe = CreateFileW(probeFile.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hProbe == INVALID_HANDLE_VALUE) {
        return false;
    }
    CloseHandle(hProbe);
    DeleteFileW(probeFile.c_str());
    return true;
}

std::wstring ResolveResilientUserDataDir(const std::wstring& exeDir, bool isTest) {
    if (isTest) {
        return exeDir + L"\\WebView2Data_Test_" + std::to_wstring(GetCurrentProcessId());
    }

    wchar_t pathBuf[MAX_PATH];

    // Priority 1: %LOCALAPPDATA%\ArchaeoPhD\WebView2Data
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", pathBuf, MAX_PATH) > 0) {
        std::wstring candidate = std::wstring(pathBuf) + L"\\ArchaeoPhD\\WebView2Data";
        if (IsDirectoryFullyWritable(candidate)) {
            LogDebug("[WINMAIN] Selected resilient user data dir (Priority 1: LOCALAPPDATA): " + archaeophd::DataRootManager::WideToUtf8(candidate));
            return candidate;
        }
    }

    // Priority 2: %APPDATA%\ArchaeoPhD\WebView2Data
    if (GetEnvironmentVariableW(L"APPDATA", pathBuf, MAX_PATH) > 0) {
        std::wstring candidate = std::wstring(pathBuf) + L"\\ArchaeoPhD\\WebView2Data";
        if (IsDirectoryFullyWritable(candidate)) {
            LogDebug("[WINMAIN] Selected resilient user data dir (Priority 2: APPDATA): " + archaeophd::DataRootManager::WideToUtf8(candidate));
            return candidate;
        }
    }

    // Priority 3: %TEMP%\ArchaeoPhD\WebView2Data
    if (GetTempPathW(MAX_PATH, pathBuf) > 0) {
        std::wstring candidate = std::wstring(pathBuf) + L"ArchaeoPhD\\WebView2Data";
        if (IsDirectoryFullyWritable(candidate)) {
            LogDebug("[WINMAIN] Selected resilient user data dir (Priority 3: TEMP): " + archaeophd::DataRootManager::WideToUtf8(candidate));
            return candidate;
        }
    }

    // Priority 4: exeDir\WebView2Data (portable mode)
    std::wstring portableCandidate = exeDir + L"\\WebView2Data";
    if (IsDirectoryFullyWritable(portableCandidate)) {
        LogDebug("[WINMAIN] Selected resilient user data dir (Priority 4: PORTABLE): " + archaeophd::DataRootManager::WideToUtf8(portableCandidate));
        return portableCandidate;
    }

    // Ultimate fallback: isolated PID directory in Temp
    if (GetTempPathW(MAX_PATH, pathBuf) > 0) {
        std::wstring candidate = std::wstring(pathBuf) + L"ArchaeoPhD_WV2_" + std::to_wstring(GetCurrentProcessId());
        try { archaeophd::fs_compat::create_directories(archaeophd::DataRootManager::WideToUtf8(candidate)); } catch (...) {}
        LogDebug("[WINMAIN] Selected resilient user data dir (Priority 5: PID TEMP): " + archaeophd::DataRootManager::WideToUtf8(candidate));
        return candidate;
    }

    return exeDir + L"\\WebView2Data";
}
