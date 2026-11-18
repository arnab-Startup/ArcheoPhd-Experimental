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
#include <memory>
#include <fstream>
#include <iostream>

#include <EventToken.h>
#include <WebView2.h>
#include <json.hpp>

#include "core/models.hpp"
#include "storage/storage.hpp"
#include "analysis/vector_index.hpp"
#include "analysis/contradictions.hpp"
#include "analysis/llm_engine.hpp"
#include "analysis/thesis_audit.hpp"
#include "storage/data_root.hpp"
#include "extraction/document_extractor.hpp"
#include "extraction/ingestion_manager.hpp"

// Global Application State & Engine Handles
extern HWND g_hWnd;
extern ICoreWebView2Controller* g_controller;
extern ICoreWebView2* g_webview;
extern std::wstring g_appDistFolder;
extern bool g_runLiveUiTests;
extern std::wstring g_extraBrowserArgs;
extern std::string g_currentDataRoot;

extern std::unique_ptr<archaeophd::NativeStorage> g_storage;
extern std::unique_ptr<archaeophd::NativeContradictionEngine> g_contradictions;
extern std::unique_ptr<archaeophd::NativeThesisAuditor> g_thesisAuditor;

// Logging and Diagnostics
void LogDebug(const std::string& msg);

// Native Engine Initialization & In-Memory Dispatch
bool InitNativeEngineWithDataRoot(const std::string& dataRoot);
std::string DispatchNativeMessage(const std::string& inputJson);
