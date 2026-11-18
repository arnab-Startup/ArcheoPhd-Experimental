#include "webview_host.hpp"
#include "app_context.hpp"
#include "runtime_locator.hpp"
#include "navigation_handlers.hpp"
#include "ipc_bridge_handler.hpp"
#include "window.hpp"
#include "bridge_script.hpp"

// Global pointers for fallback handling
static CreateCoreWebView2EnvironmentWithOptionsFn g_pfnCreateEnv = nullptr;
static ICoreWebView2EnvironmentOptions* g_envOptions = nullptr;
static std::wstring g_targetUrl = L"";

// -----------------------------------------------------------------------------
// CustomEnvironmentOptions Implementation
// -----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_ICoreWebView2EnvironmentOptions) {
        *ppv = static_cast<ICoreWebView2EnvironmentOptions*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE CustomEnvironmentOptions::AddRef() { return InterlockedIncrement(&m_refCount); }
ULONG STDMETHODCALLTYPE CustomEnvironmentOptions::Release() {
    ULONG c = InterlockedDecrement(&m_refCount);
    if (c == 0) delete this;
    return c;
}
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::get_AdditionalBrowserArguments(LPWSTR* value) {
    if (!value) return E_POINTER;
    *value = (LPWSTR)CoTaskMemAlloc((m_args.size() + 1) * sizeof(wchar_t));
    wcscpy(*value, m_args.c_str());
    return S_OK;
}
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::put_AdditionalBrowserArguments(LPCWSTR value) {
    m_args = value ? value : L"";
    return S_OK;
}
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::get_Language(LPWSTR* value) { if (value) *value = nullptr; return S_OK; }
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::put_Language(LPCWSTR value) { return S_OK; }
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::get_TargetCompatibleBrowserVersion(LPWSTR* value) { if (value) *value = nullptr; return S_OK; }
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::put_TargetCompatibleBrowserVersion(LPCWSTR value) { return S_OK; }
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::get_AllowSingleSignOnUsingOSPrimaryAccount(BOOL* allow) { if (allow) *allow = FALSE; return S_OK; }
HRESULT STDMETHODCALLTYPE CustomEnvironmentOptions::put_AllowSingleSignOnUsingOSPrimaryAccount(BOOL allow) { return S_OK; }

// Forward declaration of handlers
class EnvironmentCompletedHandler;
class ControllerCompletedHandler;

// -----------------------------------------------------------------------------
// Controller Creation Handler
// -----------------------------------------------------------------------------
class ControllerCompletedHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
    LONG m_refCount;
    HWND m_hWnd;
    std::wstring m_targetUrl;
    bool m_isFallback;

public:
    ControllerCompletedHandler(HWND hWnd, const std::wstring& targetUrl, bool isFallback = false)
        : m_refCount(1), m_hWnd(hWnd), m_targetUrl(targetUrl), m_isFallback(isFallback) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler) {
            *ppvObject = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Controller* controller) override;
};

// -----------------------------------------------------------------------------
// Environment Creation Handler
// -----------------------------------------------------------------------------
class EnvironmentCompletedHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
    LONG m_refCount;
    HWND m_hWnd;
    std::wstring m_targetUrl;
    bool m_isFallback;

public:
    EnvironmentCompletedHandler(HWND hWnd, const std::wstring& targetUrl, bool isFallback = false)
        : m_refCount(1), m_hWnd(hWnd), m_targetUrl(targetUrl), m_isFallback(isFallback) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler) {
            *ppvObject = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Environment* env) override {
        char hrBuf[64];
        snprintf(hrBuf, sizeof(hrBuf), "0x%08lX", (unsigned long)result);
        LogDebug(std::string("[ENV] Environment callback: hr=") + hrBuf + " env=" + (env ? "NON-NULL" : "NULL"));
        if (FAILED(result) || !env) {
            LogDebug(std::string("[ENV] Environment creation failed: hr=") + hrBuf);
            if (g_runLiveUiTests) {
                std::cout << "[ERROR] CoreWebView2 environment creation failed with hr=" << hrBuf << std::endl;
                PostQuitMessage(1);
                return result;
            }
            if (!m_isFallback && g_pfnCreateEnv) {
                wchar_t tempPath[MAX_PATH];
                if (GetTempPathW(MAX_PATH, tempPath) > 0) {
                    std::wstring fallbackDir = std::wstring(tempPath) + L"ArchaeoPhD_WV2_EnvFB_" + std::to_wstring(GetCurrentProcessId());
                    try { archaeophd::fs_compat::create_directories(archaeophd::DataRootManager::WideToUtf8(fallbackDir)); } catch (...) {}
                    LogDebug("[ENV] Attempting environment fallback with dir=" + archaeophd::DataRootManager::WideToUtf8(fallbackDir));
                    HRESULT hrFallback = g_pfnCreateEnv(
                        nullptr,
                        fallbackDir.c_str(),
                        g_envOptions,
                        new EnvironmentCompletedHandler(m_hWnd, m_targetUrl, /*isFallback=*/true)
                    );
                    if (SUCCEEDED(hrFallback)) return S_OK;
                }
            }
            std::wstring errMsg = L"Failed to initialize Microsoft Edge WebView2 Environment (" + std::wstring(hrBuf, hrBuf + strlen(hrBuf)) + L").\n"
                                  L"Please ensure the WebView2 Runtime is installed (standard on Windows 10/11).";
            MessageBoxW(m_hWnd, errMsg.c_str(), L"ArchaeoPhD Error", MB_ICONERROR);
            return result;
        }

        return env->CreateCoreWebView2Controller(m_hWnd, new ControllerCompletedHandler(m_hWnd, m_targetUrl, m_isFallback));
    }
};

// -----------------------------------------------------------------------------
// ControllerCompletedHandler::Invoke Implementation
// -----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE ControllerCompletedHandler::Invoke(HRESULT result, ICoreWebView2Controller* controller) {
    LogDebug(std::string("[CONTROLLER] Controller callback: hr=") + std::to_string(result) + " controller=" + (controller ? "NON-NULL" : "NULL"));
    if (FAILED(result) || !controller) {
        char hrBuf[64];
        snprintf(hrBuf, sizeof(hrBuf), "0x%08lX", (unsigned long)result);
        LogDebug(std::string("[CONTROLLER] Controller creation failed: hr=") + hrBuf);
        if (result == (HRESULT)0x800700AA && !g_runLiveUiTests) {
            HWND existingHwnd = FindWindowW(WINDOW_CLASS_NAME, WINDOW_TITLE_TEXT);
            if (existingHwnd && existingHwnd != m_hWnd) {
                if (IsIconic(existingHwnd)) ShowWindow(existingHwnd, SW_RESTORE);
                SetForegroundWindow(existingHwnd);
                PostQuitMessage(0);
                return S_OK;
            }
        }
        if (!m_isFallback && g_pfnCreateEnv) {
            LogDebug("[CONTROLLER] Initial controller failed (" + std::string(hrBuf) + "). Triggering automatic recovery with isolated temp directory...");
            wchar_t tempPath[MAX_PATH];
            if (GetTempPathW(MAX_PATH, tempPath) > 0) {
                std::wstring fallbackDir = std::wstring(tempPath) + L"ArchaeoPhD_WV2_CtrlFB_" + std::to_wstring(GetCurrentProcessId());
                try { archaeophd::fs_compat::create_directories(archaeophd::DataRootManager::WideToUtf8(fallbackDir)); } catch (...) {}
                CleanStaleWebView2Locks(fallbackDir);
                HRESULT hrFallback = g_pfnCreateEnv(
                    nullptr,
                    fallbackDir.c_str(),
                    g_envOptions,
                    new EnvironmentCompletedHandler(m_hWnd, m_targetUrl, /*isFallback=*/true)
                );
                if (SUCCEEDED(hrFallback)) {
                    LogDebug("[CONTROLLER] Fallback environment dispatched successfully.");
                    return S_OK;
                }
            }
        }
        std::wstring errMsg = L"Failed to create CoreWebView2 controller (" + std::wstring(hrBuf, hrBuf + strlen(hrBuf)) + L").\n";
        if (result == (HRESULT)0x800700AA) {
            errMsg += L"Please ensure another instance of ArchaeoPhD is not currently active.";
        } else {
            errMsg += L"Please verify Microsoft Edge WebView2 Runtime is intact and permissions allow write access.";
        }
        MessageBoxW(m_hWnd, errMsg.c_str(), L"ArchaeoPhD Error", MB_ICONERROR);
        return result;
    }

    g_controller = controller;
    g_controller->AddRef();

    g_controller->get_CoreWebView2(&g_webview);
    if (!g_webview) {
        MessageBoxW(m_hWnd, L"Failed to obtain CoreWebView2 instance.", L"ArchaeoPhD Error", MB_ICONERROR);
        return E_FAIL;
    }

    // Resize WebView2 to fit window bounds
    RECT bounds;
    GetClientRect(m_hWnd, &bounds);
    g_controller->put_Bounds(bounds);

    // Configure Settings for Native Desktop Workstation Experience
    ICoreWebView2Settings* settings = nullptr;
    if (SUCCEEDED(g_webview->get_Settings(&settings)) && settings) {
        settings->put_IsScriptEnabled(TRUE);
        settings->put_AreDefaultScriptDialogsEnabled(FALSE);
        settings->put_IsWebMessageEnabled(TRUE);
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_AreDefaultContextMenusEnabled(TRUE);
        settings->put_IsZoomControlEnabled(FALSE);
        settings->put_AreDevToolsEnabled(TRUE);
        settings->Release();
    }

    // Locate executable folder
    wchar_t exePathBuf[MAX_PATH];
    GetModuleFileNameW(nullptr, exePathBuf, MAX_PATH);
    std::wstring exeFolder = exePathBuf;
    size_t slashPos = exeFolder.find_last_of(L"\\/");
    if (slashPos != std::wstring::npos) {
        exeFolder = exeFolder.substr(0, slashPos);
    }

    // Determine target folder (from embedded runtime extraction or local directory)
    std::wstring targetFolder = g_appDistFolder;
    if (targetFolder.empty() || GetFileAttributesW((targetFolder + L"\\index.html").c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::wstring distCandidate = exeFolder + L"\\dist";
        if (GetFileAttributesW((distCandidate + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
            targetFolder = distCandidate;
        } else if (GetFileAttributesW((exeFolder + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
            targetFolder = exeFolder;
        }
    }

    std::wstring fallbackHtmlPath;
    if (!targetFolder.empty() && GetFileAttributesW((targetFolder + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
        fallbackHtmlPath = targetFolder + L"\\index.html";
    }

    std::wstring fallbackUri = fallbackHtmlPath.empty() ? L"" : PathToFileUri(fallbackHtmlPath);
    EventRegistrationToken tokenNavStarting;
    g_webview->add_NavigationStarting(new NavigationStartingHandler(), &tokenNavStarting);

    EventRegistrationToken tokenNavCompleted;
    g_webview->add_NavigationCompleted(new NavigationCompletedHandler(m_hWnd, fallbackUri), &tokenNavCompleted);

    EventRegistrationToken tokenWebMessage;
    g_webview->add_WebMessageReceived(new WebMessageReceivedHandler(m_hWnd), &tokenWebMessage);

    // Inject Native IPC bridge client into window.nativeBridge
    const wchar_t* bridgeScript = GetNativeBridgeScript();

    g_webview->AddScriptToExecuteOnDocumentCreated(bridgeScript, nullptr);

    bool mappedVirtualHost = false;
    if (!targetFolder.empty()) {
        ICoreWebView2_3* wv3 = nullptr;
        if (SUCCEEDED(g_webview->QueryInterface(IID_ICoreWebView2_3, (void**)&wv3)) && wv3) {
            HRESULT hrMap1 = wv3->SetVirtualHostNameToFolderMapping(
                L"appassets.example",
                targetFolder.c_str(),
                COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW
            );
            wv3->SetVirtualHostNameToFolderMapping(
                L"app.archaeophd.local",
                targetFolder.c_str(),
                COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW
            );
            wv3->Release();
            if (SUCCEEDED(hrMap1)) {
                mappedVirtualHost = true;
                g_webview->Navigate(L"https://appassets.example/index.html");
            }
        }
    }

    if (!mappedVirtualHost) {
        g_webview->Navigate(m_targetUrl.c_str());
    }

    return S_OK;
}

bool StartWebView2Initialization(
    HWND hWnd,
    const std::wstring& targetUrl,
    const std::wstring& userDataDir,
    const std::wstring& runtimeLoaderPath,
    const std::wstring& extraBrowserArgs
) {
    wchar_t exePathBuffer[MAX_PATH];
    GetModuleFileNameW(nullptr, exePathBuffer, MAX_PATH);
    std::wstring exeDir = exePathBuffer;
    size_t lastSlash = exeDir.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) exeDir = exeDir.substr(0, lastSlash);

    // Load WebView2Loader.dll dynamically
    HMODULE hLoader = nullptr;
    if (!runtimeLoaderPath.empty() && GetFileAttributesW(runtimeLoaderPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        hLoader = LoadLibraryW(runtimeLoaderPath.c_str());
    }
    if (!hLoader) {
        std::wstring loaderPath = exeDir + L"\\WebView2Loader.dll";
        hLoader = LoadLibraryW(loaderPath.c_str());
    }
    if (!hLoader) {
        hLoader = LoadLibraryW(L"WebView2Loader.dll");
    }
    if (!hLoader) {
        std::wstring pkgLoader = exeDir + L"\\packages\\webview2\\build\\native\\x86\\WebView2Loader.dll";
        hLoader = LoadLibraryW(pkgLoader.c_str());
    }

    if (!hLoader) {
        MessageBoxW(hWnd,
            L"WebView2Loader.dll could not be loaded.\n"
            L"Please verify Microsoft Edge WebView2 Runtime is installed.",
            L"ArchaeoPhD Startup Error", MB_ICONERROR);
        return false;
    }
    LogDebug("[WINMAIN] WebView2Loader.dll loaded successfully.");

    CreateCoreWebView2EnvironmentWithOptionsFn pfnCreateEnv =
        (CreateCoreWebView2EnvironmentWithOptionsFn)GetProcAddress(hLoader, "CreateCoreWebView2EnvironmentWithOptions");

    if (!pfnCreateEnv) {
        MessageBoxW(hWnd,
            L"Failed to locate CreateCoreWebView2EnvironmentWithOptions in WebView2Loader.dll.",
            L"ArchaeoPhD Startup Error", MB_ICONERROR);
        FreeLibrary(hLoader);
        return false;
    }

    ICoreWebView2EnvironmentOptions* envOptions = nullptr;
    if (!extraBrowserArgs.empty()) {
        envOptions = new CustomEnvironmentOptions(extraBrowserArgs);
    }

    g_pfnCreateEnv = pfnCreateEnv;
    g_envOptions = envOptions;
    g_targetUrl = targetUrl;

    LogDebug("[WINMAIN] Calling pfnCreateEnv with userDataDir=" + std::string(userDataDir.begin(), userDataDir.end()));
    HRESULT hr = pfnCreateEnv(
        nullptr,
        userDataDir.c_str(),
        envOptions,
        new EnvironmentCompletedHandler(hWnd, targetUrl)
    );
    LogDebug("[WINMAIN] pfnCreateEnv returned: hr=" + std::to_string(hr));

    if (FAILED(hr)) {
        MessageBoxW(hWnd,
            L"Failed to start WebView2 initialization.\n"
            L"Verify Microsoft Edge WebView2 Runtime is installed.",
            L"ArchaeoPhD Startup Error", MB_ICONERROR);
        return false;
    }

    return true;
}
