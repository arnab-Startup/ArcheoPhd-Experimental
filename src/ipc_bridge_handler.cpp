#include "ipc_bridge_handler.hpp"
#include "app_context.hpp"

HRESULT STDMETHODCALLTYPE WebMessageReceivedHandler::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_ICoreWebView2WebMessageReceivedEventHandler) {
        *ppvObject = static_cast<ICoreWebView2WebMessageReceivedEventHandler*>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE WebMessageReceivedHandler::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE WebMessageReceivedHandler::Release() {
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) delete this;
    return count;
}

HRESULT STDMETHODCALLTYPE WebMessageReceivedHandler::Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
    if (!sender || !args) return S_OK;

    LPWSTR msgJson = nullptr;
    HRESULT hr = args->get_WebMessageAsJson(&msgJson);
    if (FAILED(hr) || !msgJson) return S_OK;

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, msgJson, -1, nullptr, 0, nullptr, nullptr);
    std::string rawJson(utf8Len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, msgJson, -1, &rawJson[0], utf8Len, nullptr, nullptr);
    if (!rawJson.empty() && rawJson.back() == '\0') rawJson.pop_back();

    CoTaskMemFree(msgJson);

    // Intercept UI test completion message and JS diagnostic errors
    try {
        json req = json::parse(rawJson);
        if (req.is_string()) {
            try {
                req = json::parse(req.get<std::string>());
            } catch (...) {}
        }
        if (req.is_object()) {
            std::string action = req.value("action", "");
            if (action == "js_error") {
                json p = req.value("payload", json::object());
                LogDebug("[JS_ERROR] " + p.dump());
                return S_OK;
            }
            if (action == "ui_test_step") {
                json p = req.value("payload", json::object());
                LogDebug("[TEST_STEP] " + p.value("step", "") + " -> " + (p.value("ok", false) ? "PASS" : "FAIL") + " (" + p.value("details", "") + ")");
                return S_OK;
            }
            if (action == "ui_test_complete") {
                json payload = req.value("payload", json::object());
                bool ok = payload.value("success", false);
                LogDebug("[TEST_COMPLETE] Final result: " + std::string(ok ? "ALL PASSED" : "FAILED") + " err=" + payload.value("error", "none"));
                std::ofstream out("live_ui_test_results.log");
                out << payload.dump(2) << std::endl;
                out.close();
                std::ofstream outRel("release/live_ui_test_results.log");
                if (outRel.is_open()) {
                    outRel << payload.dump(2) << std::endl;
                    outRel.close();
                }
                std::cout << "\n================================================================================\n";
                std::cout << "  LIVE WEBVIEW2 DOM CLICK-THROUGH TEST RESULTS                                  \n";
                std::cout << "================================================================================\n";
                for (const auto& r : payload.value("results", json::array())) {
                    std::cout << "  " << (r.value("ok", false) ? "[PASS] " : "[FAIL] ") 
                              << r.value("step", "") << "\n        " 
                              << r.value("details", "") << "\n";
                }
                std::cout << "================================================================================\n";
                std::cout << "  OVERALL LIVE DOM VERDICT: " << (ok ? "ALL " + std::to_string(payload.value("results", json::array()).size()) + " DOM CHECKS PASSED (100%)" : "FAILED") << "\n";
                std::cout << "================================================================================\n\n";
                std::cout.flush();
                PostQuitMessage(ok ? 0 : 1);
                return S_OK;
            }
        }
    } catch (const std::exception& ex) {
        LogDebug("[IPC_JSON_PARSE_ERROR] " + std::string(ex.what()) + " raw=" + rawJson);
    }

    std::string unescapedJson = rawJson;
    try {
        json jTest = json::parse(rawJson);
        if (jTest.is_string()) {
            unescapedJson = jTest.get<std::string>();
        }
    } catch (...) {}

    std::string replyJson = DispatchNativeMessage(unescapedJson);

    int wideLen = MultiByteToWideChar(CP_UTF8, 0, replyJson.c_str(), -1, nullptr, 0);
    std::wstring wideReply(wideLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, replyJson.c_str(), -1, &wideReply[0], wideLen);
    if (!wideReply.empty() && wideReply.back() == L'\0') wideReply.pop_back();

    sender->PostWebMessageAsJson(wideReply.c_str());
    return S_OK;
}
