#include "app_context.hpp"
#include "window.hpp"
#include "runtime_locator.hpp"
#include "webview_host.hpp"
#include "live_ui_tester.hpp"
#include <shellapi.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow) {
    // 1. Parse command-line arguments
    int nArgs = 0;
    LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    std::wstring wsCmd = GetCommandLineW();
    LogDebug("[WINMAIN] Command Line: " + std::string(wsCmd.begin(), wsCmd.end()) + " | nArgs=" + std::to_string(nArgs));
    if (szArglist) {
        for (int i = 1; i < nArgs; i++) {
            std::wstring argW = szArglist[i];
            LogDebug("[WINMAIN] Arg[" + std::to_string(i) + "]: " + std::string(argW.begin(), argW.end()));
            if (wcscmp(szArglist[i], L"--test-ui-live") == 0 || wcscmp(szArglist[i], L"--run-ui-tests") == 0) {
                g_runLiveUiTests = true;
            } else if (wcsncmp(szArglist[i], L"--remote-debugging-port", 23) == 0) {
                g_extraBrowserArgs += szArglist[i];
                g_extraBrowserArgs += L" ";
            }
        }
        LocalFree(szArglist);
    }
    LogDebug("[WINMAIN] g_runLiveUiTests=" + std::string(g_runLiveUiTests ? "TRUE" : "FALSE"));

    if (g_runLiveUiTests) {
        CreateTestPdfFixtureIfTesting();
    }

    // 2. Initialize COM
    LogDebug("[WINMAIN] Initializing COM...");
    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    LogDebug("[WINMAIN] COM initialized: hr=" + std::to_string(hrCo));
    if (FAILED(hrCo) && hrCo != (HRESULT)0x80010106L) {
        MessageBoxW(nullptr, L"Failed to initialize COM apartment.", L"ArchaeoPhD Error", MB_ICONERROR);
        return 1;
    }

    // 3. Single-Instance Window Guard
    if (!g_runLiveUiTests) {
        LogDebug("[WINMAIN] Checking for existing window...");
        HWND existingHwnd = FindWindowW(WINDOW_CLASS_NAME, WINDOW_TITLE_TEXT);
        LogDebug("[WINMAIN] FindWindow returned: " + std::to_string((uintptr_t)existingHwnd));
        if (existingHwnd) {
            LogDebug("[WINMAIN] Found existing ArchaeoPhD window. Bringing to foreground.");
            if (IsIconic(existingHwnd)) ShowWindow(existingHwnd, SW_RESTORE);
            SetForegroundWindow(existingHwnd);
            CoUninitialize();
            return 0;
        }
    }

    // 4. Configure Shell and DPI
    LogDebug("[WINMAIN] Registering Start Menu shortcut...");
    EnsureStartMenuShortcutRegistered();
    LogDebug("[WINMAIN] Setting DPI awareness...");
    SetupDpiAwareness();

    // 5. Create Main Application Window
    g_hWnd = CreateMainWorkstationWindow(hInstance, nCmdShow);
    if (!g_hWnd) {
        MessageBoxW(nullptr, L"Failed to create ArchaeoPhD desktop window.", L"ArchaeoPhD Error", MB_ICONERROR);
        CoUninitialize();
        return 1;
    }
    LogDebug("[WINMAIN] Window created successfully. g_hWnd=" + std::to_string((uintptr_t)g_hWnd));

    // 6. Locate executable folder and ensure runtime assets are extracted
    wchar_t exePathBuffer[MAX_PATH];
    GetModuleFileNameW(nullptr, exePathBuffer, MAX_PATH);
    std::wstring exeDir = exePathBuffer;
    size_t lastSlash = exeDir.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) exeDir = exeDir.substr(0, lastSlash);

    std::wstring runtimeDistFolder;
    std::wstring runtimeLoaderPath;
    LogDebug("[WINMAIN] Calling EnsureRuntimeExtracted...");
    EnsureRuntimeExtracted(runtimeDistFolder, runtimeLoaderPath);
    LogDebug("[WINMAIN] EnsureRuntimeExtracted completed. runtimeDistFolder=" + std::string(runtimeDistFolder.begin(), runtimeDistFolder.end()));
    g_appDistFolder = runtimeDistFolder;

    std::wstring finalHtmlPath;
    if (!runtimeDistFolder.empty() && GetFileAttributesW((runtimeDistFolder + L"\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
        finalHtmlPath = runtimeDistFolder + L"\\index.html";
    } else if (GetFileAttributesW((exeDir + L"\\dist\\index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
        finalHtmlPath = exeDir + L"\\dist\\index.html";
    } else {
        finalHtmlPath = exeDir + L"\\index.html";
    }

    std::wstring targetUrl = PathToFileUri(finalHtmlPath);

    // 7. Setup User Data Directory with multi-tier resilience
    std::wstring userDataDir = ResolveResilientUserDataDir(exeDir, g_runLiveUiTests);
    CleanStaleWebView2Locks(userDataDir);
    LogDebug("[WINMAIN] Step 7 complete, calling GetStatus...");

    // 8. Initialize Native C++ Offline Engine
    try {
        auto rootStatus = archaeophd::DataRootManager::GetStatus();
        LogDebug("[WINMAIN] GetStatus returned: configured=" + std::to_string(rootStatus.configured));
        if (rootStatus.configured && !rootStatus.is_missing) {
            if (!InitNativeEngineWithDataRoot(rootStatus.data_root)) {
                LogDebug("[WINMAIN] Configured data root was inaccessible. Prompting user to select accessible data root.");
                rootStatus.configured = false;
            }
        } else if (g_runLiveUiTests) {
            InitNativeEngineWithDataRoot("test_ipc_bridge_data");
        }
    } catch (const std::exception& e) {
        LogDebug(std::string("[WINMAIN] Exception in GetStatus: ") + e.what());
    } catch (...) {
        LogDebug("[WINMAIN] Unknown exception in GetStatus");
    }

    if (g_runLiveUiTests && g_storage) {
        archaeophd::VerificationItem v_chirki;
        v_chirki.id = "vitem-chirki-rubble";
        v_chirki.project_id = "default";
        v_chirki.source_id = "src_test_sample_1";
        v_chirki.candidate_a = "2040 cm";
        v_chirki.candidate_b = "20-40 cm";
        v_chirki.crop_image_path = "/crops/crop_2040_raw.png";
        v_chirki.status = "PENDING";
        g_storage->put_verification_item(v_chirki);

        archaeophd::VerificationItem v_hazor;
        v_hazor.id = "vitem-hazor-unlinked";
        v_hazor.project_id = "default";
        v_hazor.source_id = "src_test_sample_1";
        v_hazor.candidate_a = "Stratum IV";
        v_hazor.candidate_b = "Stratum V";
        v_hazor.crop_image_path = "";
        v_hazor.status = "PENDING";
        g_storage->put_verification_item(v_hazor);
    }
    LogDebug("[WINMAIN] Storage & benchmark setup complete.");

    // 9. Start WebView2 Host initialization
    if (!StartWebView2Initialization(g_hWnd, targetUrl, userDataDir, runtimeLoaderPath, g_extraBrowserArgs)) {
        CoUninitialize();
        return 1;
    }

    // 10. Win32 Message Loop
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CoUninitialize();
    return (int)msg.wParam;
}
