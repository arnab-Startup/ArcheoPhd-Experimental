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

// Navigation Starting Handler (SPA subroute logging & interception)
class NavigationStartingHandler : public ICoreWebView2NavigationStartingEventHandler {
    LONG m_refCount;
public:
    NavigationStartingHandler() : m_refCount(1) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) override;
};

// Navigation Completed Handler (Auto-recovery of virtual subroutes / Fallback)
class NavigationCompletedHandler : public ICoreWebView2NavigationCompletedEventHandler {
    LONG m_refCount;
    HWND m_hWnd;
    std::wstring m_fallbackUrl;
    bool m_attemptedFallback;

public:
    NavigationCompletedHandler(HWND hWnd, const std::wstring& fallbackUrl)
        : m_refCount(1), m_hWnd(hWnd), m_fallbackUrl(fallbackUrl), m_attemptedFallback(false) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) override;
};
