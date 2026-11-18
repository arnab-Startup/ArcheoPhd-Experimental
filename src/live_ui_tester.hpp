#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WebView2.h>

// Creates test PDF fixture and runs the live DOM test runner script inside WebView2
void CreateTestPdfFixtureIfTesting();
void RunLiveUiClickthroughTest(ICoreWebView2* sender);
