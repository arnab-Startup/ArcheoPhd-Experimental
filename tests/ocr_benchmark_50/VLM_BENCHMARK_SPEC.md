# Pre-Registered Local VLM Benchmark Specification on Degraded Archaeological Scans

> **Status:** FROZEN BEFORE EXECUTION  
> **Date:** October 3, 2026  
> **Evaluation Target:** Local Vision-Language Models (VLM) on Class B (Porous Acidic Paper with Ink Bleed-Through)  
> **Corpus:** H.D. Sankalia (1974) — *Studies in Indian Archaeology* (25 Scan Pages, 62 Blind Hand-Labeled Facts)

---

## 1. Context & Research Problem

In the 50-page OCR benchmark, classical OCR engines failed on Sankalia-class scans:
- **Windows Native OCR**: 53.23% accuracy (33 / 62 facts)
- **Tesseract 5.4**: 62.90% accuracy (39 / 62 facts)
- **Classical Verdict**: Tier 3 Triggered. Automated extraction was 100% hard-gated.

The architectural question is:
**Does a lightweight, local, attention-based Vision-Language Model (VLM) possess sufficient spatial and contextual reasoning to read faint and bled-through print where line-segmentation LSTMs fail, or does Class B stay permanently manual?**

---

## 2. Pre-Registered Pass / Fail Acceptance Tiers

These criteria are locked prior to running any model evaluations and cannot be adjusted post-hoc:

| Outcome Tier | Condition | Architectural Decision for Class B Scans |
| :--- | :--- | :--- |
| **Tier 1: Automated Path Unlocked ("Ship It")** | • Digit-level accuracy on quantitative facts $\ge \mathbf{90.0\%}$<br>• Hallucination rate $\le \mathbf{2.0\%}$ (phantom dates/numbers)<br>• Average inference latency $\le \mathbf{15 \text{ sec/page}}$ on consumer hardware | Unlock automated VLM extraction pipeline for Class B documents with confidence thresholding. |
| **Tier 2: Assistive Pre-Fill Only ("Gating Maintained")** | • Digit-level accuracy between $\mathbf{75.0\%}$ and $\mathbf{89.9\%}$<br>• Hallucination rate $\le \mathbf{5.0\%}$ | Hard gate remains active. VLM outputs pre-fill the verification queue form to accelerate human transcription, but zero facts are auto-committed without human sign-off. |
| **Tier 3: VLM Rejected ("Permanently Manual")** | • Digit-level accuracy $< \mathbf{75.0\%}$<br>• OR Hallucination rate $> \mathbf{5.0\%}$<br>• OR Latency $> \mathbf{45 \text{ sec/page}}$ | Abandon all automated extraction attempts for degraded letterpress. Class B becomes permanently manual; P1 UI is scoped strictly for split-screen manual entry. |

---

## 3. Evaluation Dataset & Ground Truth

- **Pages Evaluated (25 Pages)**:
  - `sankalia_p052-052` to `p071-071` (20 core excavation pages: Chirki-on-Pravara Acheulian workshop, stratigraphy, artifact tallies, measurements).
  - `sankalia_p025-025` (Dense bibliography column, dates, issue citations).
  - `sankalia_p104-104` (Quaternary geological literature citations).
  - `sankalia_p150-150` (Microlithic gravel strata depths).
  - `sankalia_p210-210` (Vakataka dynasty chronology & regnal dates).
  - `sankalia_p280-280` (Megalithic passage tombs & Bombay Gazetteer citations).
- **Ground Truth**:
  - Exactly the 62 hand-labeled quantitative facts established blind in `ground_truth.json`.
- **Zero Cloud Leakage Guarantee**:
  - The model must run completely offline on local CPU / integrated graphics, preserving strict privacy with zero external telemetry.
