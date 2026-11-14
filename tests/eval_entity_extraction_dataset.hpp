#pragma once

#include <string>
#include <vector>
#include <optional>

namespace archaeophd {

// ============================================================================
// Phase 2 Step 3 — Entity Extraction Development Dataset (DEV SET ONLY)
// 
// NOT the held-out evaluation set. This 40-case synthetic suite serves as an
// internal unit development and regression test bench during component construction.
// Formal evaluation is conducted strictly against:
// 1. The independent held-out evaluation set (tests/eval_entity_extraction_held_out.hpp)
// 2. The 166-fact real OCR benchmark (tests/ocr_benchmark_50/ground_truth.json)
// 
// Principles:
// 1. Deterministic regular grammar & canonical normalizer only (zero LLM).
// 2. "Never Repair" Invariant: extracted raw substring must equal exact input slice.
// 3. Provenance: Class B / unverified origin tracked on all mentions.
// 4. Distinction between negative controls (non-entities) and out-of-scope units.
// 5. Spelled-out words (e.g. "forty") are excluded from numeric digit extraction.
// ============================================================================

enum class EntityCategory {
    LINEAR_DIMENSION,
    LINEAR_RANGE,
    COMPOUND_DIMENSION,
    DEPTH_ELEVATION,
    MASS_WEIGHT,
    EXACT_DATE_BCE,
    EXACT_DATE_CE,
    APPROX_DATE_BCE,
    DATE_RANGE,
    DATE_RANGE_CE,
    UNCALIBRATED_C14_BP,
    AUTHOR_CALIBRATED_DATE,
    TEMPERATURE,
    ARTIFACT_SPECIMEN_COUNT,
    LOCUS_PROVENANCE,
    SPATIAL_AREA,
    STRATUM_NAME,
    TRENCH_ID,
    BASKET_UNIT,
    OCR_CORRUPTION_ANOMALY,
    NEGATIVE_CONTROL,
    OUT_OF_SCOPE_UNIT
};

struct ExpectedEntity {
    std::string raw_match;
    std::string normalized_value;
    std::string unit_or_type;
    bool requires_ocr_flag = false;
    bool is_uncalibrated_bp = false;
};

struct ExtractionTestCase {
    std::string id;
    EntityCategory category;
    std::string passage_text;
    std::vector<ExpectedEntity> expected_entities;
    std::string note;
};

inline std::vector<ExtractionTestCase> get_entity_extraction_dev_dataset() {
    return {
        // --- Positive Extraction Cases: Dimensions & Metrics ---
        {"TC-01", EntityCategory::LINEAR_DIMENSION,
         "The trench revealed a 2.5 cm layer of bitumen backing.",
         {{"2.5 cm", "0.025 m", "cm", false, false}},
         "Single linear measurement in cm"},

        {"TC-02", EntityCategory::LINEAR_DIMENSION,
         "Mudbrick wall width measured 1.2 m across the crest.",
         {{"1.2 m", "1.200 m", "m", false, false}},
         "Single linear measurement in m"},

        {"TC-03", EntityCategory::LINEAR_RANGE,
         "Gravel lenses vary from 20-40 cm in total thickness.",
         {{"20-40 cm", "[0.20, 0.40] m", "cm_range", false, false}},
         "Linear range with hyphen"},

        {"TC-04", EntityCategory::LINEAR_RANGE,
         "The revetment stood between 4.5 and 5.0 metres in height.",
         {{"between 4.5 and 5.0 metres", "[4.50, 5.00] m", "m_range", false, false}},
         "Natural language bounded linear range"},

        {"TC-05", EntityCategory::COMPOUND_DIMENSION,
         "Burnt bricks conformed strictly to 1:2:4, measuring 7 x 14 x 28 cm.",
         {{"7 x 14 x 28 cm", "7x14x28 cm", "compound_dim", false, false}},
         "Three-dimensional architectural compound measurement"},

        {"TC-06", EntityCategory::COMPOUND_DIMENSION,
         "The soundings uncovered a pillar base 45 by 60 cm.",
         {{"45 by 60 cm", "45x60 cm", "compound_dim", false, false}},
         "Two-dimensional cross-section"},

        {"TC-07", EntityCategory::DEPTH_ELEVATION,
         "Water shaft descends forty metres through bedrock to depth 40 m.",
         {{"depth 40 m", "40.0 m", "depth", false, false}},
         "Vertical depth coordinate (numeric form). Note: Spelled-out words ('forty metres') are intentionally excluded from regex digit extraction"},

        {"TC-08", EntityCategory::MASS_WEIGHT,
         "Chert weights included cubic specimens of 13.65 g and 27.3 g.",
         {{"13.65 g", "13.65 g", "g", false, false},
          {"27.3 g", "27.30 g", "g", false, false}},
         "Multiple metrological mass measurements"},

        // --- Positive Extraction Cases: Chronological Dates ---
        {"TC-09", EntityCategory::APPROX_DATE_BCE,
         "Destruction of City IV occurred c. 1550 BC following intense burning.",
         {{"c. 1550 BC", "-1549", "approx_bce", false, false}},
         "Circa approximate BCE date with astronomical normalization"},

        {"TC-10", EntityCategory::APPROX_DATE_BCE,
         "Pottery horizon dates to circa 1400 BCE based on bichrome ware.",
         {{"circa 1400 BCE", "-1399", "approx_bce", false, false}},
         "Circa approximate BCE date"},

        {"TC-11", EntityCategory::EXACT_DATE_BCE,
         "Terminal incineration by Tiglath-Pileser III in 732 BC.",
         {{"732 BC", "-731", "exact_bce", false, false}},
         "Historical exact BCE date"},

        {"TC-12", EntityCategory::EXACT_DATE_CE,
         "Reoccupation continued until 135 CE under Hadrianic administration.",
         {{"135 CE", "+135", "exact_ce", false, false}},
         "Historical exact CE date"},

        {"TC-13", EntityCategory::DATE_RANGE,
         "Iron Age IIA occupation span dates to 1000-925 BCE.",
         {{"1000-925 BCE", "[-999, -924]", "date_range_bce", false, false}},
         "Chronological range BCE"},

        {"TC-14", EntityCategory::DATE_RANGE_CE,
         "The basilica floor was in continuous use from 320 to 390 CE.",
         {{"from 320 to 390 CE", "[+320, +390]", "date_range_ce", false, false}},
         "Chronological range CE"},

        {"TC-15", EntityCategory::UNCALIBRATED_C14_BP,
         "Short-lived seed samples yielded 3310 ± 45 BP (uncalibrated).",
         {{"3310 ± 45 BP", "3310±45 BP", "c14_bp", false, true}},
         "Raw uncalibrated radiocarbon determination requiring BP flag"},

        {"TC-16", EntityCategory::UNCALIBRATED_C14_BP,
         "Charcoal speck determination gave 1550 +/- 50 rcybp.",
         {{"1550 +/- 50 rcybp", "1550±50 rcybp", "c14_bp", false, true}},
         "Raw radiocarbon years BP requiring BP flag"},

        {"TC-17", EntityCategory::AUTHOR_CALIBRATED_DATE,
         "Calibrated 2-sigma calendar range of 1620-1530 cal BC.",
         {{"1620-1530 cal BC", "[-1619, -1529]", "cal_bc", false, false}},
         "Author-reported calibrated 2-sigma BCE range"},

        {"TC-18", EntityCategory::AUTHOR_CALIBRATED_DATE,
         "Radiocarbon calibration yields 1430 to 1390 cal BCE at 95.4% confidence.",
         {{"1430 to 1390 cal BCE", "[-1429, -1389]", "cal_bce", false, false}},
         "Author-reported calibrated BCE range"},

        {"TC-19", EntityCategory::TEMPERATURE,
         "Thermoluminescence plateau test reached 500 °C during annealing.",
         {{"500 °C", "500 °C", "temperature", false, false}},
         "Archaeometric heating temperature"},

        // --- Positive Extraction Cases: Spatial Loci & Provenance ---
        {"TC-20", EntityCategory::LOCUS_PROVENANCE,
         "Excavated from Locus 102 immediately below the ash layer.",
         {{"Locus 102", "102", "locus", false, false}},
         "Standard locus identifier"},

        {"TC-21", EntityCategory::LOCUS_PROVENANCE,
         "Skeletal remains from Loc. 45-B adjacent to the north wall.",
         {{"Loc. 45-B", "45-B", "locus", false, false}},
         "Abbreviated alphanumeric locus"},

        {"TC-22", EntityCategory::SPATIAL_AREA,
         "Temple courtyard revealed in Area H south sounding.",
         {{"Area H", "H", "area", false, false}},
         "Excavation area designator"},

        {"TC-23", EntityCategory::STRATUM_NAME,
         "Terminal destruction horizon identified as Stratum VI.",
         {{"Stratum VI", "VI", "stratum", false, false}},
         "Stratum Roman numeral designation"},

        {"TC-24", EntityCategory::TRENCH_ID,
         "Mudbrick collapse documented in Trench I east face.",
         {{"Trench I", "I", "trench", false, false}},
         "Trench identifier"},

        {"TC-25", EntityCategory::BASKET_UNIT,
         "Diagnostic cooking pot sherds recorded under Basket 104.",
         {{"Basket 104", "104", "basket", false, false}},
         "Field basket/bucket unit"},

        // --- Corrupted OCR Benchmark Strings: Extract As-Is & Flag, Never Repair ---
        {"TC-26", EntityCategory::OCR_CORRUPTION_ANOMALY,
         "Silt deposit with cobbles measuring 2040 cm depth.",
         {{"2040 cm", "2040 cm", "raw_corrupt_cm", true, false}},
         "OCR fused '20-40 cm' into '2040 cm': MUST extract as-is and flag anomaly, NEVER repair to '20-40'"},

        {"TC-27", EntityCategory::OCR_CORRUPTION_ANOMALY,
         "Artifact recorded from Locus 691 during season two.",
         {{"Locus 691", "691", "raw_corrupt_locus", true, false}},
         "OCR lost hyphen from '69-1': MUST extract as-is and flag anomaly, NEVER repair to '69-1'"},

        {"TC-28", EntityCategory::OCR_CORRUPTION_ANOMALY,
         "Sample 1063 yielded contradictory stratigraphy.",
         {{"Sample 1063", "1063", "raw_corrupt_sample", true, false}},
         "OCR digit cluster: MUST extract as-is and flag anomaly, NEVER repair"},

        {"TC-29", EntityCategory::OCR_CORRUPTION_ANOMALY,
         "Terminal burn level recorded at elevation 1846 m.",
         {{"1846 m", "1846 m", "raw_corrupt_elev", true, false}},
         "OCR anomalous elevation: MUST extract as-is and flag anomaly, NEVER repair"},

        {"TC-30", EntityCategory::OCR_CORRUPTION_ANOMALY,
         "Ceramic sherd registered as locus 1M7 on field bag.",
         {{"locus 1M7", "1M7", "raw_corrupt_locus", true, false}},
         "OCR letter confusion '1M7': MUST extract as-is and flag anomaly, NEVER repair"},

        // --- Negative Control Cases: Zero Archaeological Entities Extracted ---
        {"TC-31", EntityCategory::NEGATIVE_CONTROL,
         "As noted in Kenyon (1981: 142), the wall was solid.",
         {},
         "Bibliographic citation with year and page number must not be extracted as date or measurement"},

        {"TC-32", EntityCategory::NEGATIVE_CONTROL,
         "See bibliography reference 128 for further discussion.",
         {},
         "Bibliographic reference pointer must not extract"},

        {"TC-33", EntityCategory::NEGATIVE_CONTROL,
         "Figure 4 shows the schematic section of the mound.",
         {},
         "Figure caption number must not extract as measurement"},

        {"TC-34", EntityCategory::NEGATIVE_CONTROL,
         "Plate 16 illustrates the burnished carinated bowls.",
         {},
         "Plate number must not extract"},

        {"TC-35", EntityCategory::OUT_OF_SCOPE_UNIT,
         "The site spans approximately 15 hectares across the tell.",
         {},
         "Out-of-scope non-targeted spatial unit (hectares): not a negative control; grammar excludes non-targeted units"},

        {"TC-36", EntityCategory::NEGATIVE_CONTROL,
         "On page 320, Schiffer outlines n-transform models.",
         {},
         "Page reference must not extract"},

        {"TC-37", EntityCategory::NEGATIVE_CONTROL,
         "The ratio of cattle to caprine bones was 3 to 1.",
         {},
         "Dimensionless ratio must not extract as physical measurement"},

        {"TC-38", EntityCategory::NEGATIVE_CONTROL,
         "Trench supervisors counted 45 workers on the sounding.",
         {},
         "Human headcount must not extract as measurement or locus"},

        {"TC-39", EntityCategory::NEGATIVE_CONTROL,
         "ISBN 0-19-813190-8 catalogued in institutional library.",
         {},
         "Catalog / ISBN number must not extract"},

        {"TC-40", EntityCategory::NEGATIVE_CONTROL,
         "Volume 2 of the final excavation report.",
         {},
         "Volume number must not extract"}
    };
}

// Backward-compatible alias for existing test runners
inline std::vector<ExtractionTestCase> get_preregistered_evaluation_dataset() {
    return get_entity_extraction_dev_dataset();
}

} // namespace archaeophd
