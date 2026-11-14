#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <json.hpp>

namespace archaeophd {

class DataRootManager {
public:
    struct DataRootStatus {
        bool configured = false;
        std::string data_root;
        bool is_missing = false;
        std::string cloud_service;
        std::string default_path;
        std::string models_dir;
        std::string libraries_dir;
        std::string logs_dir;
    };

    static inline std::wstring Utf8ToWide(const std::string& str) {
        if (str.empty()) return L"";
        int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        std::wstring wstr(count, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], count);
        if (!wstr.empty() && wstr.back() == L'\0') wstr.pop_back();
        return wstr;
    }

    static inline std::string WideToUtf8(const std::wstring& wstr) {
        if (wstr.empty()) return "";
        int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string str(count, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, nullptr, nullptr);
        if (!str.empty() && str.back() == '\0') str.pop_back();
        return str;
    }

    static inline bool CreateDirectoriesRecursive(const std::wstring& path) {
        if (path.empty()) return false;
        if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) return true;

        // 1. First attempt: Official Win32 Shell API for recursive folder creation
        int res = SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
        if (res == ERROR_SUCCESS || res == ERROR_ALREADY_EXISTS || res == ERROR_FILE_EXISTS) {
            return true;
        }

        // 2. Clean up trailing slashes
        std::wstring clean = path;
        while (!clean.empty() && (clean.back() == L'\\' || clean.back() == L'/')) {
            clean.pop_back();
        }
        if (clean.empty()) return false;

        // 3. Fallback: robust step-by-step directory creation without trailing slashes
        std::wstring sub;
        for (size_t i = 0; i < clean.length(); ++i) {
            if ((clean[i] == L'\\' || clean[i] == L'/') && sub.length() > 2) {
                CreateDirectoryW(sub.c_str(), nullptr);
            }
            sub += clean[i];
        }
        CreateDirectoryW(clean.c_str(), nullptr);
        return (GetFileAttributesW(clean.c_str()) != INVALID_FILE_ATTRIBUTES);
    }

    static inline bool IsDirWritable(const std::wstring& dir) {
        if (dir.empty()) return false;
        try {
            if (!CreateDirectoriesRecursive(dir)) return false;
        } catch (...) {
            return false;
        }
        std::wstring probe = dir + L"\\.apd_probe_" + std::to_wstring(GetCurrentProcessId());
        HANDLE h = CreateFileW(probe.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        CloseHandle(h);
        DeleteFileW(probe.c_str());
        return true;
    }

    static inline std::wstring GetSettingsDirectory() {
        wchar_t buf[MAX_PATH];
        // 1. Try %LOCALAPPDATA%\ArchaeoPhD
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH) > 0) {
            std::wstring cand = std::wstring(buf) + L"\\ArchaeoPhD";
            if (IsDirWritable(cand)) return cand;
        }
        // 2. Try %APPDATA%\ArchaeoPhD
        if (GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH) > 0) {
            std::wstring cand = std::wstring(buf) + L"\\ArchaeoPhD";
            if (IsDirWritable(cand)) return cand;
        }
        // 3. Try Documents directory %USERPROFILE%\Documents\ArchaeoPhD\.settings
        wchar_t docBuf[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, docBuf))) {
            std::wstring cand = std::wstring(docBuf) + L"\\ArchaeoPhD\\.settings";
            if (IsDirWritable(cand)) return cand;
        }
        // 3. Try %USERPROFILE%\.archaeophd
        if (GetEnvironmentVariableW(L"USERPROFILE", buf, MAX_PATH) > 0) {
            std::wstring cand = std::wstring(buf) + L"\\.archaeophd";
            if (IsDirWritable(cand)) return cand;
        }
        // 4. Try current exe directory
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::wstring exeDir = exePath;
        size_t lastSlash = exeDir.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            std::wstring cand = exeDir.substr(0, lastSlash);
            if (IsDirWritable(cand)) return cand;
        }
        return L".";
    }

    static inline std::wstring GetLocalAppDataDir() {
        return GetSettingsDirectory();
    }

    static inline std::wstring GetSettingsFilePath() {
        std::wstring dir = GetSettingsDirectory();
        return dir + L"\\settings.json";
    }

    static inline std::wstring GetDefaultDataRoot() {
        // Priority 1: D:\ drive (fixed drive, dedicated data volume)
        UINT dType = GetDriveTypeW(L"D:\\");
        if (dType == DRIVE_FIXED) {
            std::wstring cand = L"D:\\ArchaeoPhD-Data";
            if (IsDirWritable(cand)) return cand;
        }

        // Priority 2: User Documents %USERPROFILE%\Documents\ArchaeoPhD-Data
        wchar_t docBuf[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, docBuf))) {
            std::wstring cand = std::wstring(docBuf) + L"\\ArchaeoPhD-Data";
            if (IsDirWritable(cand)) return cand;
        }

        // Priority 2: User profile %USERPROFILE%\ArchaeoPhD-Data
        wchar_t profBuf[MAX_PATH];
        if (GetEnvironmentVariableW(L"USERPROFILE", profBuf, MAX_PATH) > 0) {
            std::wstring cand = std::wstring(profBuf) + L"\\ArchaeoPhD-Data";
            if (IsDirWritable(cand)) return cand;
        }

        // Priority 3: LocalAppData if writable
        wchar_t buf[MAX_PATH];
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH) > 0) {
            std::wstring cand = std::wstring(buf) + L"\\ArchaeoPhD\\data";
            if (IsDirWritable(cand)) return cand;
        }

        // Priority 4: Roaming AppData if writable
        if (GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH) > 0) {
            std::wstring cand = std::wstring(buf) + L"\\ArchaeoPhD\\data";
            if (IsDirWritable(cand)) return cand;
        }

        return L"C:\\ArchaeoPhD-Data";
    }

    static inline std::string DetectCloudSyncService(const std::wstring& pathW) {
        if (pathW.empty()) return "";
        std::wstring lower = pathW;
        for (auto& ch : lower) ch = towlower(ch);

        // 1. OneDrive
        wchar_t envBuf[MAX_PATH];
        if (GetEnvironmentVariableW(L"OneDrive", envBuf, MAX_PATH) > 0) {
            std::wstring od(envBuf);
            for (auto& ch : od) ch = towlower(ch);
            if (!od.empty() && lower.find(od) == 0) return "OneDrive";
        }
        if (GetEnvironmentVariableW(L"OneDriveConsumer", envBuf, MAX_PATH) > 0) {
            std::wstring od(envBuf);
            for (auto& ch : od) ch = towlower(ch);
            if (!od.empty() && lower.find(od) == 0) return "OneDrive";
        }
        if (GetEnvironmentVariableW(L"OneDriveCommercial", envBuf, MAX_PATH) > 0) {
            std::wstring od(envBuf);
            for (auto& ch : od) ch = towlower(ch);
            if (!od.empty() && lower.find(od) == 0) return "OneDrive";
        }
        if (lower.find(L"onedrive") != std::wstring::npos) {
            return "OneDrive";
        }

        // 2. Dropbox
        if (lower.find(L"\\dropbox") != std::wstring::npos || lower.find(L"/dropbox") != std::wstring::npos) {
            return "Dropbox";
        }

        // 3. iCloud Drive
        if (lower.find(L"iclouddrive") != std::wstring::npos || lower.find(L"icloud") != std::wstring::npos) {
            return "iCloud Drive";
        }

        // 4. Google Drive
        if (lower.find(L"google drive") != std::wstring::npos ||
            lower.find(L"my drive") != std::wstring::npos ||
            lower.find(L"googledrive") != std::wstring::npos) {
            return "Google Drive";
        }

        // Volume check for Google Drive virtual drive
        if (pathW.length() >= 3 && pathW[1] == L':' && (pathW[2] == L'\\' || pathW[2] == L'/')) {
            std::wstring root = pathW.substr(0, 3);
            wchar_t volName[MAX_PATH] = { 0 };
            if (GetVolumeInformationW(root.c_str(), volName, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0)) {
                std::wstring volLower = volName;
                for (auto& ch : volLower) ch = towlower(ch);
                if (volLower.find(L"google") != std::wstring::npos) {
                    return "Google Drive";
                }
            }
        }

        return "";
    }

    static inline bool IsPathMissing(const std::wstring& pathW) {
        if (pathW.empty()) return true;
        DWORD attr = GetFileAttributesW(pathW.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES) return false;

        // Drive check: e.g. E: drive
        if (pathW.length() >= 2 && pathW[1] == L':') {
            std::wstring drive = pathW.substr(0, 2) + L"\\";
            UINT driveType = GetDriveTypeW(drive.c_str());
            if (driveType == DRIVE_NO_ROOT_DIR) {
                return true;
            }
        }
        return true;
    }

    static inline DataRootStatus GetStatus() {
        DataRootStatus status;
        status.default_path = WideToUtf8(GetDefaultDataRoot());

        std::wstring settingsPath = GetSettingsFilePath();
        if (GetFileAttributesW(settingsPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            status.configured = false;
            status.data_root = "";
            status.is_missing = false;
            status.cloud_service = "";
            return status;
        }

        std::ifstream in(WideToUtf8(settingsPath).c_str());
        if (!in.is_open()) {
            status.configured = false;
            return status;
        }

        try {
            nlohmann::json j;
            in >> j;
            if (j.contains("data_root") && j["data_root"].is_string()) {
                status.data_root = j["data_root"].get<std::string>();
                status.configured = !status.data_root.empty();
            }
        } catch (...) {
            status.configured = false;
            return status;
        }

        if (status.configured) {
            std::wstring wideRoot = Utf8ToWide(status.data_root);
            status.is_missing = IsPathMissing(wideRoot);
            if (!IsDirWritable(wideRoot)) {
                status.is_missing = true;
            }
            status.cloud_service = DetectCloudSyncService(wideRoot);
            status.models_dir = status.data_root + "\\models";
            status.libraries_dir = status.data_root + "\\libraries";
            status.logs_dir = status.data_root + "\\logs";
        }

        return status;
    }

    static inline bool SaveDataRoot(const std::string& pathUtf8, std::string& outError) {
        if (pathUtf8.empty()) {
            outError = "Data Root path cannot be empty.";
            return false;
        }

        std::wstring widePath = Utf8ToWide(pathUtf8);

        // Normalize trailing slashes
        while (widePath.length() > 3 && (widePath.back() == L'\\' || widePath.back() == L'/')) {
            widePath.pop_back();
        }

        // Verify root drive exists if drive-specified
        if (widePath.length() >= 2 && widePath[1] == L':') {
            std::wstring drive = widePath.substr(0, 2) + L"\\";
            UINT driveType = GetDriveTypeW(drive.c_str());
            if (driveType == DRIVE_NO_ROOT_DIR) {
                outError = "Drive " + WideToUtf8(drive) + " does not exist or is disconnected.";
                return false;
            }
        }

        // Create main Data Root directory with resilient fallback
        if (!CreateDirectoriesRecursive(widePath)) {
            std::wstring safeFallback = GetDefaultDataRoot();
            if (widePath != safeFallback && CreateDirectoriesRecursive(safeFallback)) {
                widePath = safeFallback;
            } else {
                outError = "Failed to create directory: " + pathUtf8 + ". Please click 'Browse...' and select an accessible directory such as D:\\ArchaeoPhD-Data or your Documents folder.";
                return false;
            }
        }

        // Create standard hierarchy:
        // models/embedding, models/llm, logs, libraries/default
        CreateDirectoriesRecursive(widePath + L"\\models\\embedding");
        CreateDirectoriesRecursive(widePath + L"\\models\\llm");
        CreateDirectoriesRecursive(widePath + L"\\logs");
        CreateDirectoriesRecursive(widePath + L"\\libraries\\default");

        // Write config.json inside Data Root
        std::wstring configPath = widePath + L"\\config.json";
        if (GetFileAttributesW(configPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            nlohmann::json cfg;
            cfg["schema_version"] = "1.0.0";
            cfg["app"] = "ArchaeoPhD Desktop Workstation";
            cfg["created_at"] = "2026-10-03";
            cfg["zero_cloud_leakage"] = true;
            cfg["models_dir"] = "models";
            cfg["libraries_dir"] = "libraries";
            cfg["logs_dir"] = "logs";

            std::ofstream cfgFile(WideToUtf8(configPath).c_str());
            if (cfgFile.is_open()) {
                cfgFile << cfg.dump(2);
                cfgFile.close();
            }
        }

        // Write pointer file: %LOCALAPPDATA%\ArchaeoPhD\settings.json
        std::wstring localAppDir = GetLocalAppDataDir();
        CreateDirectoriesRecursive(localAppDir);
        std::wstring settingsPath = GetSettingsFilePath();

        nlohmann::json settings;
        // Keep any existing keys if file existed
        std::ifstream existingSettings(WideToUtf8(settingsPath).c_str());
        if (existingSettings.is_open()) {
            try { existingSettings >> settings; } catch (...) {}
            existingSettings.close();
        }

        settings["data_root"] = WideToUtf8(widePath);

        std::ofstream settingsFile(WideToUtf8(settingsPath).c_str());
        if (!settingsFile.is_open()) {
            outError = "Failed to save settings pointer file to AppData.";
            return false;
        }
        settingsFile << settings.dump(2);
        settingsFile.close();

        return true;
    }

    static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
        if (uMsg == BFFM_INITIALIZED && lpData != 0) {
            SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, lpData);
        }
        return 0;
    }

    static inline bool BrowseForFolder(HWND hWndOwner, const std::string& initialPathUtf8, std::string& outSelectedPath) {
        HRESULT hrInit = OleInitialize(NULL);
        bool found = false;

        IFileOpenDialog* pfd = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if (SUCCEEDED(hr) && pfd) {
            DWORD dwOptions = 0;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
                pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
            }
            pfd->SetTitle(L"Select ArchaeoPhD Research Data Root Folder");

            if (!initialPathUtf8.empty()) {
                std::wstring initWide = Utf8ToWide(initialPathUtf8);
                IShellItem* psiFolder = nullptr;
                if (SUCCEEDED(SHCreateItemFromParsingName(initWide.c_str(), NULL, IID_PPV_ARGS(&psiFolder))) && psiFolder) {
                    pfd->SetFolder(psiFolder);
                    psiFolder->Release();
                }
            }

            if (SUCCEEDED(pfd->Show(hWndOwner))) {
                IShellItem* psiResult = nullptr;
                if (SUCCEEDED(pfd->GetResult(&psiResult)) && psiResult) {
                    PWSTR pszPath = nullptr;
                    if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
                        outSelectedPath = WideToUtf8(pszPath);
                        CoTaskMemFree(pszPath);
                        found = true;
                    }
                    psiResult->Release();
                }
            }
            pfd->Release();
        }

        if (!found) {
            std::wstring initialWide = Utf8ToWide(initialPathUtf8);
            BROWSEINFOW bi = { 0 };
            bi.hwndOwner = hWndOwner;
            bi.lpszTitle = L"Select ArchaeoPhD Research Data Root Folder (e.g. D:\\ArchaeoPhD-Data or External SSD):";
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
            bi.lpfn = BrowseCallbackProc;
            bi.lParam = (LPARAM)initialWide.c_str();

            LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
            if (pidl) {
                wchar_t path[MAX_PATH];
                if (SHGetPathFromIDListW(pidl, path)) {
                    outSelectedPath = WideToUtf8(path);
                    found = true;
                }
                CoTaskMemFree(pidl);
            }
        }

        if (SUCCEEDED(hrInit)) {
            OleUninitialize();
        }

        return found;
    }

    static inline bool BrowseForFile(HWND hWndOwner, const std::string& initialDirUtf8, std::string& outSelectedPath) {
        wchar_t szFile[MAX_PATH] = { 0 };
        OPENFILENAMEW ofn = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWndOwner;
        static const wchar_t filter[] = L"PDF Documents (*.pdf)\0*.pdf\0Excavation Data (*.pdf;*.txt;*.csv)\0*.pdf;*.txt;*.csv\0All Files (*.*)\0*.*\0\0";
        ofn.lpstrFilter = filter;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        std::wstring initialDirWide = Utf8ToWide(initialDirUtf8);
        if (!initialDirWide.empty()) {
            ofn.lpstrInitialDir = initialDirWide.c_str();
        }
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameW(&ofn)) {
            outSelectedPath = WideToUtf8(szFile);
            return true;
        }
        return false;
    }
};

} // namespace archaeophd
