# Pre-Registered 50-Page OCR Ground-Truth Benchmark Specification

> **Status:** FROZEN BEFORE EXECUTION  
> **Date:** October 3, 2026  
> **Objective:** Evaluate Windows.Media.Ocr vs. Tesseract 5.4 (Docker) on 50 real archaeological scan pages to determine the production extraction architecture.  
> **ERRATUM (October 6, 2026):** In the Phase 0 evaluation of Tier 2, the evaluation script (`evaluate_benchmark_v2.js`) reported 47/47 errors caught (100.0% disagreement recall, Wilson 95% CI: [92.44%, 100.0%]) and "0% false consensus." However, forensic re-evaluation under span anchoring (`../../../docs/desktop/specs/03_anchored_scorer_rules.md`) revealed that false consensus on unanchored search was unmeasured. While true optical character false consensus at located positions is 0.00% (0/80 [0.00%, 4.58%]), candidate agreement admits 25.0% errors from dropped units (5.0%) and neighbor token displacements (16.3%). Consequently, Class A auto-accept is **paused in code**, routing all candidate facts to human verification with source optical crops pending Step 4 attribution and unit validation.

---

## 1. Pre-Registered Pass / Fail Thresholds

The acceptance criteria are established prior to running the benchmark and will not be altered post-hoc:

| Outcome Tier | Condition | Architectural Decision |
| :--- | :--- | :--- |
| **Tier 1: Production Ready ("Ship It")** | • Digit-level accuracy on quantitative facts (dates, measurements, counts) $\ge \mathbf{98.0\%}$<br>• Zero unflagged 2-order-of-magnitude errors ($0\%$) | Adopt Tesseract 5 as the primary offline OCR engine. |
| **Tier 2: Dual-Engine Cross-Validation Required** | • Digit-level accuracy between $\mathbf{90.0\%}$ and $\mathbf{97.9\%}$<br>• Cross-Engine Disagreement Recall $\ge \mathbf{95.0\%}$ (disagreement flags $\ge 95\%$ of silent errors) | Implement Dual-Engine Ensemble (Windows OCR + Tesseract). Auto-flag discrepancies for 1-click human verification. |
| **Tier 3: Unacceptable ("Try Different Engine")** | • Digit-level accuracy $< \mathbf{90.0\%}$<br>• OR Cross-Engine Disagreement Recall $< \mathbf{90.0\%}$ (too many silent errors escape) | Abandon classical OCR for degraded scans; evaluate local multimodal Vision-Language Models (e.g., SmolVLM / Qwen2-VL via llama.cpp) or enforce mandatory manual data entry. |

---

## 2. Page Range Selection & Justification (50 Pages Total)

To prevent accidental cherry-picking while ensuring representation of critical excavation contexts, the 50 pages are split between **core thematic sequences** and **stratified random samples**:

### A. H.D. Sankalia (1974) — *Studies in Indian Archaeology* (25 Pages)
- **Selection Basis**: Deccan College regional letterpress on porous acidic paper with severe ink bleed-through. The highest optical stress test in the corpus.
- **Thematic Core (20 Pages: pp. 52–71)**:
  - *Justification*: Complete excavation chapter for *"The Acheulian Workshop at Chirki on Pravara"*. Contains primary stratigraphic profiles, bedrock rubble layer measurements, artifact tallies (bifaces, cleavers), and trench designations (Tr. VII, IX, XII).
- **Stratified Random Samples (5 Pages)**:
  - `Page 25`: Dense chronological bibliography (two-column list of dates and journal issues).
  - `Page 104`: Typological discussion of early chalcolithic ceramics.
  - `Page 150`: Microlithic industry dimensions and flake measurements.
  - `Page 210`: Megalithic burial cist stratigraphy.
  - `Page 280`: Concluding synthesis and radiometric chronological summary.

### B. K. Rajan (2002) — *Archaeology: Principles and Methods* (15 Pages)
- **Selection Basis**: Modern regional offset printing with tight font kerning, scanned as two-page spreads.
- **Thematic Core (10 Pages: pp. 16–25)**:
  - *Justification*: Chapter 2: *"History of Archaeology"*. Dense historical narrative packed with BC/AD calendar dates, centuries (`17th`, `18th`), and excavator dates (Winckelmann 1764, Champollion 1822).
- **Stratified Random Samples (5 Pages/Spreads)**:
  - `Sheet 50`: Radiometric and scientific dating formulas (C-14 half-life, tree-ring sequences).
  - `Sheet 75`: Excavation recording methods, layer stratigraphy, and datum elevations.
  - `Sheet 100`: Underwater archaeology depths, coastal surveys, and marine coordinates.
  - `Sheet 110`: Ceramic typology, rim diameter measurements, and vessel counts.
  - `Sheet 140`: Chronological concordance, site coordinates, and discovery years.

### C. D.K. Chakrabarti (1988) — *History of Indian Archaeology* (10 Pages)
- **Selection Basis**: Archival opaque flatbed scan (University of Michigan / Google Books). Control benchmark for clean scans.
- **Thematic Core (7 Pages: pp. 15–21)**:
  - *Justification*: Chapter 1: *"The Early Phase"*. Archival colonial records, footnote citations, and 17th–18th century dates (1676, 1758).
- **Stratified Random Samples (3 Pages)**:
  - `Page 65`: Asiatic Society records and early 19th-century survey dates.
  - `Page 130`: Alexander Cunningham Archaeological Survey reports.
  - `Page 215`: John Marshall Harappan excavation documentation.

---

## 3. Blind Ground-Truth Transcription Protocol

To ensure true independence:
1. **Scan-First Inspection**: The 50 pages are rendered to 200 DPI PNGs.
2. **Blind Extraction**: Human transcription of every printed numerical fact (dates, strata measurements, artifact tallies, page numbers, locus IDs) is performed by inspecting the raw image pixels directly, with all OCR outputs hidden.
3. **Ground-Truth Lock**: The extracted ground truth is saved to `ground_truth.json` and committed before any OCR engine is executed against the batch.

---

## 4. Dual-Engine Cross-Validation & Metric Tracking

Every page will be evaluated using:
1. **Engine A**: `Windows.Media.Ocr` (Windows Native WinRT API).
2. **Engine B**: `Tesseract 5.4` (Isolated Docker Container, `--psm 3` and `--psm 6`).

### Quantitative Outputs Measured:
1. **Digit Confusion Matrix**: Systematic character-level substitution frequencies:
   - $9 \rightarrow \{0, 8, M, \text{other}\}$
   - $4 \rightarrow \{1, A, H, \text{other}\}$
   - $1 \rightarrow \{I, l, \|, \text{other}\}$
   - Hyphen preservation rate (`"-"` retained vs. merged into digits).
2. **Crop-vs-Full-Page Consistency**:
   - Compare full-page recognition against isolated line bounding-box crops to detect LSTM baseline instability.
3. **Dual-Engine Disagreement Efficacy**:
   - Measure whether disagreement between Engine A and Engine B reliably identifies errors, providing an automated anomaly detection signal without requiring manual confidence heuristics.
