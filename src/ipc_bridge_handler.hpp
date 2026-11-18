#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <EventToken.h>
#include <WebView2.h>

// Native WebView2 In-Memory Message Received Handler (Zero HTTP / Zero Sockets)
class WebMessageReceivedHandler : public ICoreWebView2WebMessageReceivedEventHandler {
    LONG m_refCount;
    HWND m_hWnd;

public:
    WebMessageReceivedHandler(HWND hWnd) : m_refCount(1), m_hWnd(hWnd) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) override;
};
