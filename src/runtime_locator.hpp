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

#define IDR_APP_PAYLOAD 101

// Converts Windows file path to file:/// URI
std::wstring PathToFileUri(const std::wstring& path);

// Embedded payload extraction for single-file self-contained deployment
bool EnsureRuntimeExtracted(std::wstring& outDistFolder, std::wstring& outLoaderPath);

// WebView2 lockfile and stale directory cleaner
void CleanStaleWebView2Locks(const std::wstring& userDataDir);

// File permissions probe
bool IsDirectoryFullyWritable(const std::wstring& dir);

// Resolves resilient WebView2 User Data Directory across Priority 1-5 fallback tiers
std::wstring ResolveResilientUserDataDir(const std::wstring& exeDir, bool isTest);
