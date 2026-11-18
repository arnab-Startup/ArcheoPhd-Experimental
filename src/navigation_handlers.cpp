#include "navigation_handlers.hpp"
#include "app_context.hpp"
#include "live_ui_tester.hpp"

// -----------------------------------------------------------------------------
// NavigationStartingHandler
// -----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE NavigationStartingHandler::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_ICoreWebView2NavigationStartingEventHandler) {
        *ppvObject = static_cast<ICoreWebView2NavigationStartingEventHandler*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE NavigationStartingHandler::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE NavigationStartingHandler::Release() {
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) delete this;
    return count;
}

HRESULT STDMETHODCALLTYPE NavigationStartingHandler::Invoke(ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) {
    if (!args || !sender) return S_OK;
    LPWSTR uri = nullptr;
    args->get_Uri(&uri);
    if (uri) {
        std::wstring uriStr(uri);
        CoTaskMemFree(uri);
        LogDebug("[NAV] NavigationStarting: " + archaeophd::DataRootManager::WideToUtf8(uriStr));

        // Intercept SPA subroutes on virtual hosts (e.g. https://appassets.example/projects)
        // SetVirtualHostNameToFolderMapping serves static files but does not rewrite non-existent subpaths,
        // which causes a white-screen 404 void if navigated at the browser document level.
        size_t hostPos = uriStr.find(L"appassets.example");
        if (hostPos == std::wstring::npos) {
            hostPos = uriStr.find(L"app.archaeophd.local");
        }

        if (hostPos != std::wstring::npos) {
            size_t slashPos = uriStr.find(L"/", hostPos);
            std::wstring subpath = (slashPos != std::wstring::npos) ? uriStr.substr(slashPos) : L"/";

            size_t dotPos = subpath.rfind(L'.');
            bool hasExtension = (dotPos != std::wstring::npos && dotPos > subpath.rfind(L'/'));

            if (!hasExtension && subpath != L"/" && subpath != L"/index.html") {
                LogDebug("[NAV] Intercepting SPA subroute navigation to: " + archaeophd::DataRootManager::WideToUtf8(subpath));
                args->put_Cancel(TRUE);

                std::wstring js = L"(function() { "
                                  L"  var path = '" + subpath + L"'; "
                                  L"  if (typeof window.__appNavigate === 'function') { "
                                  L"    window.__appNavigate(path); "
                                  L"  } else if (window.__historyNavigator && typeof window.__historyNavigator.push === 'function') { "
                                  L"    window.__historyNavigator.push(path); "
                                  L"  } else { "
                                  L"    var link = document.querySelector('a[href=\"' + path + '\"]'); "
                                  L"    if (link) link.click(); "
                                  L"  } "
                                  L"})()";
                sender->ExecuteScript(js.c_str(), nullptr);
                return S_OK;
            }
        }
    }
    return S_OK;
}

// -----------------------------------------------------------------------------
// NavigationCompletedHandler
// -----------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE NavigationCompletedHandler::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_ICoreWebView2NavigationCompletedEventHandler) {
        *ppvObject = static_cast<ICoreWebView2NavigationCompletedEventHandler*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE NavigationCompletedHandler::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE NavigationCompletedHandler::Release() {
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) delete this;
    return count;
}

HRESULT STDMETHODCALLTYPE NavigationCompletedHandler::Invoke(ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) {
    if (!args || !sender) return S_OK;
    BOOL isSuccess = FALSE;
    args->get_IsSuccess(&isSuccess);
    COREWEBVIEW2_WEB_ERROR_STATUS errorStatus = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
    args->get_WebErrorStatus(&errorStatus);

    LogDebug(std::string("[NAV] NavigationCompleted: isSuccess=") + (isSuccess ? "TRUE" : "FALSE") + 
             " status=" + std::to_string(errorStatus) +
             " attemptedFallback=" + (m_attemptedFallback ? "TRUE" : "FALSE") + 
             " g_runLiveUiTests=" + (g_runLiveUiTests ? "TRUE" : "FALSE"));

    if (isSuccess) {
        m_attemptedFallback = false;
    }

    if (!isSuccess) {
        LPWSTR currentUri = nullptr;
        sender->get_Source(&currentUri);
        std::wstring uriStr = currentUri ? currentUri : L"";
        if (currentUri) CoTaskMemFree(currentUri);

        LogDebug("[NAV] Navigation failed on URI: " + archaeophd::DataRootManager::WideToUtf8(uriStr));

        // Auto-recover virtual subroute failures (like /projects or /dashboard) to /index.html
        if (uriStr.find(L"appassets.example") != std::wstring::npos || uriStr.find(L"app.archaeophd.local") != std::wstring::npos) {
            if (uriStr.find(L"/index.html") == std::wstring::npos) {
                LogDebug("[NAV] Auto-recovering failed subroute navigation to https://appassets.example/index.html");
                sender->Navigate(L"https://appassets.example/index.html");
                return S_OK;
            }
        }

        if (!m_attemptedFallback) {
            m_attemptedFallback = true;
            if (!m_fallbackUrl.empty()) {
                LogDebug("[NAV] Falling back to file URI: " + archaeophd::DataRootManager::WideToUtf8(m_fallbackUrl));
                sender->Navigate(m_fallbackUrl.c_str());
            } else {
                const wchar_t* fallbackHtml = 
                    L"<!DOCTYPE html><html><head><meta charset='utf-8'>"
                    L"<style>body{background:#080b12;color:#f1f5f9;font-family:sans-serif;display:flex;align-items:center;justify-content:center;height:100vh;margin:0;}"
                    L".box{text-align:center;padding:2rem;background:rgba(255,255,255,0.05);border:1px solid rgba(255,255,255,0.1);border-radius:12px;max-width:500px;}"
                    L"h1{color:#e2b35a;margin-bottom:0.5rem;}p{color:#94a3b8;font-size:0.9rem;}"
                    L"button{background:#e2b35a;color:#080b12;border:none;padding:10px 20px;border-radius:6px;font-weight:bold;cursor:pointer;margin-top:1rem;}"
                    L"</style></head><body><div class='box'><h1>ArchaeoPhD Workstation</h1>"
                    L"<p>Offline research engine initialized. Click below to load interface.</p>"
                    L"<button onclick=\"location.href='/index.html'\">Load Interface</button></div></body></html>";
                sender->NavigateToString(fallbackHtml);
            }
        }
    } else if (isSuccess) {
        if (g_runLiveUiTests) {
            RunLiveUiClickthroughTest(sender);
        }
    }
    return S_OK;
}
