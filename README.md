# ArcheoPhd-Desktop

The bare-metal **Native C++ Windows Desktop Application** for ArchaeoPhD — Offline Research Workstation for archaeology PhDs, postdocs, and cultural heritage researchers.

---

## 🏛️ Architecture & Highlights

* **Single-File Self-Contained Binary**: `release/ArchaeoPhD.exe` (~2.19 MB) — statically linked with zero DLL dependencies.
* **Pure In-Memory IPC**: Zero HTTP servers, zero open ports, zero network sockets. UI communicates directly with the C++ engine via `window.chrome.webview.postMessage` (`window.nativeBridge`).
* **Zero Firewall Warnings**: No network listeners ever created; immune to port conflicts and firewall restrictions.
* **User-Chosen Data Root Architecture**: Full researcher agency over storage location (fast internal NVMe, `D:\ArchaeoPhD-Data`, or external portable SSDs).
* **Cloud-Sync Leakage Detector**: Automatically warns if the selected data folder is inside OneDrive, Dropbox, Google Drive, or iCloud to protect unpublished research findings.
* **100% Air-Gapped Offline**: Embedded vector search, 4-Type Contradiction Engine, and Harris Matrix cycle detection (Tarjan's SCC) run directly on your CPU/RAM with zero cloud telemetry.
* **Per-Monitor DPI V2**: Ultra-crisp typography across 1080p, 1440p, and 4K high-DPI displays.

---

## 📂 Repository Structure

```text
ArcheoPhd-Desktop/
├── release/
│   ├── ArchaeoPhD.exe          # 100% self-contained standalone desktop executable (~2.19 MB)
│   └── ArchaeoPhD-Windows.zip  # Portable release package
├── src/
│   ├── main.cpp                # Win32 + WebView2 integration & in-memory IPC dispatcher
│   ├── app.ico                 # Multi-resolution application icon (16x16 to 256x256)
│   └── resource.rc             # Windows PE resource script
├── engine/src/
│   ├── data_root.hpp           # User-chosen Data Root, cloud detector, & folder picker
│   ├── storage.hpp             # Compressed storage & relational state engine
│   ├── contradictions.hpp      # 4-Type contradiction engine
│   ├── thesis_audit.hpp        # Pre-viva defense thesis auditor
│   ├── vector_index.hpp        # Quantized Matryoshka vector index
│   └── system_inspector.hpp    # Hardware & storage breakdown inspector
├── dist/                       # Embedded production workstation UI bundle (all 41 screens)
├── include/                    # MinGW COM and WebView2 header files
├── lib/                        # Microsoft Edge WebView2Loader library
├── build.bat                   # 1-Click compiler script (MinGW G++ static build)
├── create_shortcut.ps1         # Windows Start Menu & Desktop shortcut generator
├── PLAN.md                     # Roadmap, feature status, and engineering specs
└── pricing_security.md         # Subscription architecture, offline lease & encryption specs
```

---

## 🚀 Running the Workstation

Double-click the release executable:
```powershell
.\release\ArchaeoPhD.exe
```

Or extract and run the portable zip:
```powershell
Expand-Archive .\release\ArchaeoPhD-Windows.zip -DestinationPath .\release\
```

---

## 🛠️ Building from Source

### Prerequisites
* Windows 10 or 11 (64-bit)
* MinGW-w64 G++ (with `windres` and `tar.exe`)
* WebView2 Runtime (installed by default on Windows 10/11)

### 1-Click Build Script
Run the automated build script:
```powershell
.\build.bat
```

The script automatically:
1. Packages the runtime payload (`dist/` + `WebView2Loader.dll`).
2. Compiles the embedded PE resources with `windres`.
3. Statically links the single-file executable (`-static -static-libgcc -static-libstdc++`).
4. Outputs the finished binary to `release/ArchaeoPhD.exe`.
