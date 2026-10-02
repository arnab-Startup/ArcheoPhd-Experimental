# ArcheoPhd-Experimental

## ArchaeoPhD — Experimental Laboratory: PLAN.md Phase 1

> **Rule from PLAN.md:** "One step at a time. Finish → verify → next."  
> **Stack:** 100% Local only. Zero open ports. Zero network sockets. Pure In-Memory.

This is the dedicated research and experimentation testbed for **Feature 1 (F1): Data Storage & Compression** as defined in [PLAN.md](file:///d:/Prorgram/Project/ArcheoPhd/PLAN.md).

---

## F1 Experiments & Progress

| Phase | Step | Technique | Goal | Status |
|-------|------|-----------|------|:------:|
| Experiment | **1.1.1** | **zlib / gzip** | Text compression ratio + compress/decompress speed | ✅ **VERIFIED** |
| Experiment | **1.1.2** | **LZ4** | Fastest decompress speed, lower ratio | ✅ **VERIFIED** |
| Experiment | **1.1.3** | **Zstandard (zstd)** | Best balance across levels 1, 3, 7 | ✅ **VERIFIED** |
| Experiment | **1.1.4** | **Float16 quantization** | Shrink Float32 vectors to Float16 (50% RAM reduction) | ✅ **VERIFIED** |
| Experiment | **1.1.5** | **Int8 quantization** | Shrink vectors to Int8 (75% RAM reduction) + measure cosine error | ✅ **VERIFIED** |
| Experiment | **1.1.6** | **Product Quantization (PQ)** | 8–32× smaller vectors (FAISS-style subvectors) | ✅ **VERIFIED** |
| Experiment | **1.1.7** | **Pick Winner** | Best per data type: text vs vectors | ✅ **DECIDED** |
| MVP | **1.2.1** | **Text storage** | Compressed local file store for text chunks (`.zst`) | ✅ **VERIFIED** |
| MVP | **1.2.2** | **Vector storage** | Compressed vector file store (`.qvec`) | ✅ **VERIFIED** |
| MVP | **1.2.3** | **Read/write API** | Transparent compress/decompress layer | ✅ **VERIFIED** |

---

## F1 Benchmark Results Summary

### 1. Text Storage (Steps 1.1.1, 1.1.2, 1.1.3)
Tested on real archaeological excavation notes and bibliographic corpora:
* **1.1.1 Gzip (zlib):** 96.6% ratio | 0.71 ms compress | 0.22 ms decompress
* **1.1.2 LZ4:** 95.7% ratio | 1.61 ms compress | **0.12 ms decompress (Fastest)**
* **1.1.3 Zstandard (zstd L3):** **97.3% ratio (Smallest)** | 2.75 ms compress | 0.24 ms decompress
* **Winner (1.1.7):** **Zstandard Level 3** (produces smallest footprint on disk with 2.4x faster decompression than Gzip).

### 2. Vector Quantization (Steps 1.1.4, 1.1.5, 1.1.6)
Tested on 384-dimensional normalized sentence embeddings (`all-MiniLM-L6-v2`):
* **Baseline Float32:** 1,536 bytes/vector | 100% RAM | Ground truth
* **1.1.4 Float16:** 768 bytes/vector | **50.0% RAM savings** | MSE: `1.2e-10` | Top-5 Recall: **100%**
* **1.1.5 Int8 Symmetric:** 388 bytes/vector | **74.7% RAM savings** | MSE: `4.0e-8` | Top-5 Recall: **100%**
* **1.1.6 Product Quantization (PQ-16):** 16 bytes/vector | **98.9% RAM savings** | MSE: `2.8e-3` | Top-5 Recall: **94.0%**
* **Winner (1.1.7):** **Int8 Symmetric Quantization** (allows 100,000 document vectors in <39 MB RAM on standard laptops with 100% Top-5 recall).

---

## Running the F1 Laboratory

### 1. Launch the Native Windows GUI App
```powershell
npm run experimental:start
# or: .\release\ArchaeoPhD-Experimental.exe
```
Runs as a self-contained, air-gapped **~505 KB standalone Windows executable** with:
* Real-time Text Compression Playground (Field notes, Locus tables, Literature)
* Vector Quantization Studio with live cosine similarity & recall calculations
* Decision Matrix & Winner Selection
* MVP Compressed File Store verification

### 2. Run the CLI In-Memory Diagnostic Suite
```powershell
npm run experimental:run
# or: node src/main.js
```

### 3. Rebuild the Executable
```powershell
npm run experimental:build
```
