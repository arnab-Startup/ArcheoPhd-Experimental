#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <EventToken.h>
#include <WebView2.h>

// Typedef for dynamically loaded CreateCoreWebView2EnvironmentWithOptions
typedef HRESULT (STDAPICALLTYPE *CreateCoreWebView2EnvironmentWithOptionsFn)(
    PCWSTR browserExecutableFolder,
    PCWSTR userDataFolder,
    ICoreWebView2EnvironmentOptions* environmentOptions,
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler
);

// Custom Environment Options (for CDP remote debugging & extra CLI flags)
class CustomEnvironmentOptions : public ICoreWebView2EnvironmentOptions {
    LONG m_refCount = 1;
    std::wstring m_args;
public:
    CustomEnvironmentOptions(const std::wstring& args = L"") : m_args(args) {}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE get_AdditionalBrowserArguments(LPWSTR* value) override;
    HRESULT STDMETHODCALLTYPE put_AdditionalBrowserArguments(LPCWSTR value) override;
    HRESULT STDMETHODCALLTYPE get_Language(LPWSTR* value) override;
    HRESULT STDMETHODCALLTYPE put_Language(LPCWSTR value) override;
    HRESULT STDMETHODCALLTYPE get_TargetCompatibleBrowserVersion(LPWSTR* value) override;
    HRESULT STDMETHODCALLTYPE put_TargetCompatibleBrowserVersion(LPCWSTR value) override;
    HRESULT STDMETHODCALLTYPE get_AllowSingleSignOnUsingOSPrimaryAccount(BOOL* allow) override;
    HRESULT STDMETHODCALLTYPE put_AllowSingleSignOnUsingOSPrimaryAccount(BOOL allow) override;
};

// Asynchronously initializes WebView2 environment, controller, virtual hosts, and client IPC bridge
bool StartWebView2Initialization(
    HWND hWnd,
    const std::wstring& targetUrl,
    const std::wstring& userDataDir,
    const std::wstring& runtimeLoaderPath,
    const std::wstring& extraBrowserArgs
);
