#include "app_context.hpp"
#include "ipc/native_ipc_dispatcher.hpp"
#include <thread>

// Global State Definitions
HWND g_hWnd = nullptr;
ICoreWebView2Controller* g_controller = nullptr;
ICoreWebView2* g_webview = nullptr;
std::wstring g_appDistFolder = L"";
bool g_runLiveUiTests = false;
std::wstring g_extraBrowserArgs = L"";
std::string g_currentDataRoot = "";

std::unique_ptr<archaeophd::NativeStorage> g_storage = nullptr;
std::unique_ptr<archaeophd::NativeContradictionEngine> g_contradictions = nullptr;
std::unique_ptr<archaeophd::NativeThesisAuditor> g_thesisAuditor = nullptr;

void LogDebug(const std::string& msg) {
    HANDLE hFile = CreateFileA("live_debug.log", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        std::string line = msg + "\r\n";
        DWORD written = 0;
        WriteFile(hFile, line.c_str(), (DWORD)line.size(), &written, nullptr);
        CloseHandle(hFile);
    }
    HANDLE hFile2 = CreateFileA("release/live_debug.log", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile2 != INVALID_HANDLE_VALUE) {
        std::string line = msg + "\r\n";
        DWORD written = 0;
        WriteFile(hFile2, line.c_str(), (DWORD)line.size(), &written, nullptr);
        CloseHandle(hFile2);
    }
}

bool InitNativeEngineWithDataRoot(const std::string& dataRoot) {
    if (dataRoot.empty()) return false;
    try {
        std::string libDir = dataRoot + "\\libraries\\default";
        std::string legacyDir = dataRoot + "\\data";
        std::string actualStorageDir = libDir;
        if (!archaeophd::fs_compat::exists(libDir) && archaeophd::fs_compat::exists(legacyDir)) {
            actualStorageDir = legacyDir;
        } else {
            archaeophd::fs_compat::create_directories(actualStorageDir);
        }

        g_storage = std::unique_ptr<archaeophd::NativeStorage>(new archaeophd::NativeStorage(actualStorageDir));
        g_contradictions = std::unique_ptr<archaeophd::NativeContradictionEngine>(new archaeophd::NativeContradictionEngine(*g_storage));
        g_thesisAuditor = std::unique_ptr<archaeophd::NativeThesisAuditor>(new archaeophd::NativeThesisAuditor(*g_storage, *g_contradictions));

        // Initialize nomic embedding model if present
        std::string modelPath = dataRoot + "\\models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf";
        if (!archaeophd::fs_compat::exists(modelPath)) {
            wchar_t lapBuf[MAX_PATH];
            if (GetEnvironmentVariableW(L"LOCALAPPDATA", lapBuf, MAX_PATH) > 0) {
                std::string lapEmbed = archaeophd::DataRootManager::WideToUtf8(lapBuf) + "\\ArchaeoPhD\\models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf";
                if (archaeophd::fs_compat::exists(lapEmbed)) modelPath = lapEmbed;
            }
        }
        if (!archaeophd::fs_compat::exists(modelPath)) {
            if (archaeophd::fs_compat::exists("desktop\\models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf")) {
                modelPath = "desktop\\models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf";
            } else if (archaeophd::fs_compat::exists("models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf")) {
                modelPath = "models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf";
            } else if (archaeophd::fs_compat::exists("..\\models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf")) {
                modelPath = "..\\models\\embedding\\nomic-embed-text-v1.5.Q4_K_M.gguf";
            }
        }
        if (archaeophd::fs_compat::exists(modelPath)) {
            archaeophd::EmbeddingEngine::instance().load_model(modelPath, 4);
        }

        // Initialize local Qwen 2.5 7B LLM engine if present
        std::string llmPath = dataRoot + "\\models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf";
        if (!archaeophd::fs_compat::exists(llmPath)) {
            wchar_t lapBuf[MAX_PATH];
            if (GetEnvironmentVariableW(L"LOCALAPPDATA", lapBuf, MAX_PATH) > 0) {
                std::string lapLlm = archaeophd::DataRootManager::WideToUtf8(lapBuf) + "\\ArchaeoPhD\\models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf";
                if (archaeophd::fs_compat::exists(lapLlm)) llmPath = lapLlm;
            }
        }
        if (!archaeophd::fs_compat::exists(llmPath)) {
            if (archaeophd::fs_compat::exists("desktop\\models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf")) {
                llmPath = "desktop\\models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf";
            } else if (archaeophd::fs_compat::exists("models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf")) {
                llmPath = "models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf";
            } else if (archaeophd::fs_compat::exists("..\\models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf")) {
                llmPath = "..\\models\\llm\\Qwen2.5-7B-Instruct-Q4_K_M.gguf";
            }
        }
        if (archaeophd::fs_compat::exists(llmPath)) {
            LogDebug("[LLM] Dispatching asynchronous LLM load for: " + llmPath);
            std::thread([llmPath]() {
                LogDebug("[LLM] Background model load started: " + llmPath);
                if (archaeophd::LlmEngine::instance().initialize(llmPath)) {
                    if (g_contradictions) {
                        g_contradictions->set_llm_engine(&archaeophd::LlmEngine::instance());
                    }
                    LogDebug("[LLM] Background model load completed successfully.");
                } else {
                    LogDebug("[LLM] Background model load failed or skipped.");
                }
            }).detach();
        }

        g_currentDataRoot = dataRoot;
        return true;
    } catch (const std::exception& ex) {
        LogDebug(std::string("[ENGINE_INIT_ERROR] ") + ex.what());
        return false;
    } catch (...) {
        LogDebug("[ENGINE_INIT_ERROR] Unknown error initializing native storage");
        return false;
    }
}

std::string DispatchNativeMessage(const std::string& inputJson) {
    archaeophd::NativeIpcDispatcher dispatcher(
        g_storage.get(),
        g_contradictions.get(),
        g_thesisAuditor.get(),
        g_currentDataRoot,
        g_hWnd,
        [](const std::string& path) { return InitNativeEngineWithDataRoot(path); }
    );
    return dispatcher.dispatch(inputJson);
}
