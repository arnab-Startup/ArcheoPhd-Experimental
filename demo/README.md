# ArchaeoPhD Beta Benchmark Test Pack: Chandraketugarh & Lower Bengal Delta
**Research Period**: Early Historic to Early Medieval Bengal (c. 600 BCE – 1250 CE)  
**Corpus Focus**: Chandraketugarh Excavation, 7-Period Stratigraphy, Fluvial Geoarchaeology, and Gangaridai Maritime Trade Networks  

---

## 1. Overview for Beta Researchers & Evaluators

This folder contains the complete multi-file benchmark dataset designed for testing the ArchaeoPhD Workstation across unstructured field notes, multi-document RAG, tabular excavation registers, and cross-source contradiction detection.

### Included Files

| File | Type | Description |
| :--- | :--- | :--- |
| `Chandraketugarh Archaeological Research & AI Benchmark Dossier.pdf` | Academic Synthesis | Master doctoral research dossier covering 7-period stratigraphy, historiography, and Gangaridai maritime networks. |
| `Chandraketugarh Ceramic Archaeometry and Epigraphy Report.pdf` | Technical Report | Petrographic analysis of Rouletted Ware, Mediterranean amphora affinities, and Kharosthi-Brahmi paleography. |
| `Chandraketugarh Geoarchaeology Temple Architecture and Conservation Report.pdf` | Site & Conservation Report | Fluvial dynamics of the Vidyadhari river, Sarvatobhadra cruciform temple layout, and taphonomic degradation. |
| `chandraketugarh_trench_field_notes.docx` | Unstructured Field Diaries | Raw trench supervisor logs from Sectors KMD, ITK, HAD, BER, and NOO, including the critical Trench B2 ring-well intrusion. |
| `chandraketugarh_excavation_data_and_qa.xlsx` | Master Dataset & QA Matrix | 4-sheet workbook: Summary Dashboard, 18-artifact catalogue, 7-period stratigraphy matrix, and interactive AI test checklist. |
| `chandraketugarh_artifacts_raw.csv` | Flat CSV Table | 18 catalogued artifacts with exact excavation depths (cm), dates, preservation scores (0-100), and conservation status. |
| `Bhabesh-Mondal.pdf` | Archaeological Monograph | Published monograph on the Chandraketugarh private collection and historical provenance. |
| `28.pdf` | Excavation Article | Academic analysis of lower Bengal settlement patterns and material culture. |
| `MUSEUM.pdf` | Museum Catalog | Comprehensive visual and descriptive catalogue of Asutosh Museum & State Archaeological Museum finds (16.8 MB). |
| `Unit-23.pdf` | Course Unit Monograph | Detailed academic curriculum unit on Early Historic Eastern India and maritime contacts (8.3 MB). |

---

## 2. Beta Evaluation Checklist: 5 Core Test Prompts

Beta researchers can test the AI reasoning engine directly using the following 5 benchmark prompts. Each prompt targets a distinct engine subsystem:

### Test 1 (TC-01): Multi-Document Stratigraphic Synthesis
* **Target Module**: Stratigraphic Matrix & Harris Matrix DAG
* **Copy-Paste Prompt**:
  > "Cross-reference the PDF dossier, Word field notes, and Excel stratigraphy matrix: Which layer and depth range correspond to the introduction of kiln-fired brick architecture versus the polygonal temple at Khana-Mihirer Dhipi?"
* **Expected Ground Truth**:
  * Kiln-fired brick architecture (drains, domestic plinths) begins in **Layer 4** (Period IV, Kushana horizon, depth: 270–380 cm).
  * The monumental polygonal Sarvatobhadra cruciform brick temple belongs to **Layer 3** (Period V, Gupta horizon, depth: 180–270 cm).

---

### Test 2 (TC-02): Taphonomic Anomaly & Contamination Detection
* **Target Module**: Ingestion, Anomaly Parser & Stratigraphic Integrity
* **Copy-Paste Prompt**:
  > "Based on the Trench Supervisor Field Notes (DOCX) and Artifact Catalog (CSV/XLSX), identify the stratigraphic anomaly recorded in Trench A4 / B2 and explain the taphonomic cause."
* **Expected Ground Truth**:
  * In Trench B2 (Itkhola sector), 3 sherds of Kushana Rouletted Ware (normally Layer 4, ~345 cm) were found intruding into Layer 6 (Mauryan NBPW horizon, ~520 cm).
  * *Taphonomic Cause*: A Period IV terracotta ring-well shaft (18 superimposed rings, 68 cm diameter) was cut down from Layer 4 through Layer 5 into Layer 6, depositing later pit-fill. This must not be used to back-date Rouletted Ware to the 3rd century BCE.

---

### Test 3 (TC-03): Quantitative & Tabular Filtering
* **Target Module**: Tabular SQL / Pandas Query Engine
* **Copy-Paste Prompt**:
  > "From the Artifact Catalog, list all artifacts excavated between 290 cm and 450 cm depth that have a Preservation Score above 85, grouped by Period_Phase."
* **Expected Ground Truth**: Exactly 5 diagnostic artifacts:
  * **Period III (Shunga)**:
    1. `CKG-BER-004` (Depth: 440 cm, Score: 95, Panchachuda Yakshi Plaque)
    2. `CKG-KMD-005` (Depth: 425 cm, Score: 89, Winged Deity & Mithuna Plaque)
  * **Period IV (Kushana)**:
    3. `CKG-ITK-007` (Depth: 345 cm, Score: 94, Mediterranean-Type Rouletted Ware Bowl)
    4. `CKG-HAD-008` (Depth: 330 cm, Score: 97, Kharosthi-Brahmi Bilingual Vessel Seal)
    5. `CKG-NOO-011` (Depth: 290 cm, Score: 92, Intaglio Carnelian & Jasper Beads)

---

### Test 4 (TC-04): Epigraphic & Maritime Trade RAG
* **Target Module**: Evidence Retrieval, Citation Linking & Epigraphy
* **Copy-Paste Prompt**:
  > "Synthesize B.N. Mukherjee's Kharosthi-Brahmi thesis with the Hadipur sealing (CKG-HAD-008) and Rouletted Ware finds. How do these support the Gangaridai maritime trade hypothesis?"
* **Expected Ground Truth**:
  * Links Northwestern horse-trader communities using mixed Kharosthi-Brahmi script on Triratna-stamped vessels with riverine export via the Vidyadhari-Hooghly delta.
  * Connects estuarine port hubs (Chandraketugarh, Tamluk/Tamralipta) with maritime routes across the Bay of Bengal into Southeast Asia.

---

### Test 5 (TC-05): Conservation Triage & Site Risk Alert
* **Target Module**: Preservation Analytics & Triage Reporting
* **Copy-Paste Prompt**:
  > "Identify all artifacts marked 'Needs Conservation' or with Preservation Score < 80, and generate a conservation priority table with their exact trench and depth."
* **Expected Ground Truth**: Flags exactly 3 high-priority artifacts:
  1. `CKG-NOO-006`: Carved Ivory Comb (Trench E1, Depth: 410 cm, Score: 76, Status: Needs Conservation — Requires PEG consolidation for lamellar delamination)
  2. `CKG-BER-010`: Cast Copper Coin with Ship Motif (Trench D1, Depth: 300 cm, Score: 79, Status: Catalogued — Active copper carbonate corrosion)
  3. `CKG-ITK-015`: Stucco Head of Bodhisattva (Trench B1, Depth: 150 cm, Score: 74, Status: Needs Conservation — Surface flaking from salt efflorescence)

---

## 3. How to Ingest into ArchaeoPhD Desktop

1. Open **ArchaeoPhD Desktop Workstation**.
2. Create a new research project (e.g., `Early Historic Bengal Research Project`) or open your existing project.
3. In the sidebar, navigate to **Library** / **Ingestion**.
4. Drag and drop the files from this `demo/` folder directly into the ingestion dropzone, or click **Upload Source** and select the PDFs, DOCX, and XLSX files.
5. Once ingested, navigate to **AI Research**, **Evidence Graph**, or **Contradiction Detector** to test the prompt suite.
