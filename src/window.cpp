#include "window.hpp"
#include "app_context.hpp"
#include <shlobj.h>
#include <string>

const wchar_t* WINDOW_CLASS_NAME = L"ArchaeoPhDNativeWindow";
const wchar_t* WINDOW_TITLE_TEXT = L"ArchaeoPhD — Offline Research Workstation";
const int DEFAULT_WINDOW_WIDTH = 1366;
const int DEFAULT_WINDOW_HEIGHT = 868;
const int MIN_WINDOW_WIDTH = 1024;
const int MIN_WINDOW_HEIGHT = 680;

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
            lpMMI->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
            return 0;
        }
        case WM_SIZE: {
            if (g_controller) {
                RECT bounds;
                GetClientRect(hWnd, &bounds);
                g_controller->put_Bounds(bounds);
            }
            return 0;
        }
        case WM_DESTROY: {
            if (g_webview) {
                g_webview->Release();
                g_webview = nullptr;
            }
            if (g_controller) {
                g_controller->Close();
                g_controller->Release();
                g_controller = nullptr;
            }
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
}

void SetupDpiAwareness() {
    LogDebug("[DPI] Entering SetupDpiAwareness...");
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    typedef void* DPI_AWARENESS_CONTEXT;
    #define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif
    typedef BOOL (WINAPI *SetProcessDpiAwarenessContextFn)(DPI_AWARENESS_CONTEXT);
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        SetProcessDpiAwarenessContextFn pfnSetDpi = 
            (SetProcessDpiAwarenessContextFn)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pfnSetDpi) {
            pfnSetDpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
    }
    LogDebug("[DPI] Exiting SetupDpiAwareness...");
}

void EnsureStartMenuShortcutRegistered() {
    wchar_t programsPath[MAX_PATH];
    if (SHGetFolderPathW(nullptr, CSIDL_PROGRAMS, nullptr, 0, programsPath) != S_OK) return;

    std::wstring shortcutPath = std::wstring(programsPath) + L"\\ArchaeoPhD.lnk";
    if (GetFileAttributesW(shortcutPath.c_str()) != INVALID_FILE_ATTRIBUTES) return;

    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    std::wstring exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) exeDir = exeDir.substr(0, pos);

    IShellLinkW* psl = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&psl))) {
        psl->SetPath(exePath);
        psl->SetWorkingDirectory(exeDir.c_str());
        psl->SetDescription(L"ArchaeoPhD - Offline Research Workstation");
        psl->SetIconLocation(exePath, 0);

        IPersistFile* ppf = nullptr;
        if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
            ppf->Save(shortcutPath.c_str(), TRUE);
            ppf->Release();
        }
        psl->Release();
    }
}

HWND CreateMainWorkstationWindow(HINSTANCE hInstance, int nCmdShow) {
    LogDebug("[WINDOW] Entering CreateMainWorkstationWindow... nCmdShow=" + std::to_string(nCmdShow));
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    if (!wc.hIcon) wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIconSm = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        LogDebug("[WINDOW] RegisterClassExW failed: err=" + std::to_string(GetLastError()));
        return nullptr;
    }
    LogDebug("[WINDOW] Class registered successfully.");

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenWidth - DEFAULT_WINDOW_WIDTH) / 2;
    int posY = (screenHeight - DEFAULT_WINDOW_HEIGHT) / 2;

    HWND hWnd = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE_TEXT,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        posX, posY,
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd) {
        LogDebug("[WINDOW] CreateWindowExW failed: err=" + std::to_string(GetLastError()));
        return nullptr;
    }
    LogDebug("[WINDOW] CreateWindowExW succeeded. hWnd=" + std::to_string((uintptr_t)hWnd));

    if (wc.hIcon) SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
    if (wc.hIconSm) SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)wc.hIconSm);
    SetWindowTextW(hWnd, WINDOW_TITLE_TEXT);

    ShowWindow(hWnd, (nCmdShow <= 0 || nCmdShow == SW_HIDE) ? SW_SHOWNORMAL : nCmdShow);
    UpdateWindow(hWnd);

    return hWnd;
}
