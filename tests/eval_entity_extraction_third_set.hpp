#pragma once

#include <string>
#include <vector>
#include "eval_entity_extraction_dataset.hpp"

namespace archaeophd {

// ============================================================================
// Phase 2 Step 3 — Third Evaluation Set
//
// PROVENANCE: Authored 2026-10-05.  Same tool author (limitation acknowledged).
// Intent:  Cover edge-case boundaries, rarer surface forms, and multi-entity
//          complexity NOT deliberately targeted by the first two sets.
//
// GATE: P >= 92%  |  R >= 80%  |  Spec >= 95%
//       (pre-registered 2026-10-05 alongside second dev set gate)
// ============================================================================

inline std::vector<ExtractionTestCase> get_third_evaluation_dataset() {
    return {

        // ====================================================================
        // A. PUNCTUATION / SURFACE-FORM EDGE CASES (10 Cases)
        // ====================================================================

        // TE-01: B.C. form
        {"TE-01", EntityCategory::EXACT_DATE_BCE,
         "The destruction horizon was radiocarbon-dated to 3100 B.C. within the lower tell.",
         {{"3100 B.C.", "-3099", "exact_bce", false, false}},
         "[Authored] Exact BCE with B.C. surface form"},

        // TE-02: A.D. form
        {"TE-02", EntityCategory::EXACT_DATE_CE,
         "The Byzantine mosaic floor was laid down no earlier than 530 A.D. in the eastern nave.",
         {{"530 A.D.", "530", "exact_ce", false, false}},
         "[Authored] Exact CE with A.D. surface form"},

        // TE-03: c. approximate BCE
        {"TE-03", EntityCategory::APPROX_DATE_BCE,
         "The city gate was blocked and sealed c. 925 BCE after the Shishak campaign.",
         {{"c. 925 BCE", "~-924", "approx_bce", false, false}},
         "[Authored] Approximate BCE with c. abbreviation prefix"},

        // TE-04: en-dash mm range
        {"TE-04", EntityCategory::LINEAR_RANGE,
         "The blade width varies between 18 and 24 mm across the assemblage.",
         {{"18 and 24 mm", "[18.00, 24.00] mm", "mm_range", false, false}},
         "[Authored] Linear range: between N and M mm form"},

        // TE-05: compound 'by' connector
        {"TE-05", EntityCategory::COMPOUND_DIMENSION,
         "The fired clay tablet measures 8.5 by 12 cm and preserves 40 lines of cuneiform.",
         {{"8.5 by 12 cm", "8.5 x 12.0 cm", "cm_compound", false, false}},
         "[Authored] Compound dimension using by connector"},

        // TE-06: depth decimal
        {"TE-06", EntityCategory::DEPTH_ELEVATION,
         "The Early Bronze Age floor surface was encountered at depth 1.75 m below the modern surface.",
         {{"depth 1.75 m", "1.750 m", "depth", false, false}},
         "[Authored] Depth with decimal sub-metre value"},

        // TE-07: mass decimal grams
        {"TE-07", EntityCategory::MASS_WEIGHT,
         "The bronze arrowhead weighs 14.3 g and shows evidence of heat exposure.",
         {{"14.3 g", "14.3 g", "g", false, false}},
         "[Authored] Mass measurement: decimal grams"},

        // TE-08: cal BC with to separator
        {"TE-08", EntityCategory::AUTHOR_CALIBRATED_DATE,
         "Dendrochronological calibration gives a range of 2180 to 2100 cal BC at 1-sigma.",
         {{"2180 to 2100 cal BC", "[2180, 2100] cal BC", "cal_bc_range", false, false}},
         "[Authored] Calibrated BC range using to separator"},

        // TE-09: CE date range from-to form
        {"TE-09", EntityCategory::DATE_RANGE_CE,
         "Sasanian administrative occupation of the site is attested from 224 to 651 CE.",
         {{"from 224 to 651 CE", "[224, 651] CE", "ce_range", false, false}},
         "[Authored] CE date range using from N to M CE form"},

        // TE-10: C14 BP
        {"TE-10", EntityCategory::UNCALIBRATED_C14_BP,
         "Charcoal from the hearth gave an AMS date of 4820 +/- 60 BP.",
         {{"4820 +/- 60 BP", "4820±60 BP", "c14_bp", false, true}},
         "[Authored] Uncalibrated BP with +/- symbol"},

        // ====================================================================
        // B. HARDER NEGATIVE CONTROLS (10 Cases)
        // ====================================================================

        {"TE-11", EntityCategory::NEGATIVE_CONTROL,
         "The site report was published in 2003 and revised in 2011 by the same team.",
         {},
         "[Authored Negative] Modern calendar publication years: must not extract as CE dates"},

        {"TE-12", EntityCategory::NEGATIVE_CONTROL,
         "A total of 12 chapters are devoted to the ceramic typology in Volume 2 of the report.",
         {},
         "[Authored Negative] Publication chapter count and volume number: must not extract"},

        {"TE-13", EntityCategory::NEGATIVE_CONTROL,
         "The isotope ratios were measured as delta-13C values relative to VPDB standard.",
         {},
         "[Authored Negative] Isotopic ratio description: no numeric entity should fire"},

        {"TE-14", EntityCategory::NEGATIVE_CONTROL,
         "Excavation resumed after a hiatus of 15 years due to funding constraints.",
         {},
         "[Authored Negative] Modern elapsed duration (15 years): must not extract as chronological date"},

        {"TE-15", EntityCategory::NEGATIVE_CONTROL,
         "The pottery vessel has a rim diameter of 23 and a base diameter of 11 (no units given).",
         {},
         "[Authored Negative] Bare integers without measurement unit: must not extract"},

        {"TE-16", EntityCategory::NEGATIVE_CONTROL,
         "Grid references for the site are 32 degrees 14 minutes North, 35 degrees 11 minutes East.",
         {},
         "[Authored Negative] Geographic coordinates in DMS notation: must not extract"},

        {"TE-17", EntityCategory::NEGATIVE_CONTROL,
         "The stone tool fragment was recovered at site grid square Q17, sub-square d.",
         {},
         "[Authored Negative] Site grid square alphanumeric (Q17): must not extract as locus or trench"},

        {"TE-18", EntityCategory::NEGATIVE_CONTROL,
         "Samples were collected at regular 10-cm intervals throughout the entire profile.",
         {},
         "[Authored Negative] Sampling interval as methodology parameter: must not extract as discrete measurement"},

        {"TE-19", EntityCategory::NEGATIVE_CONTROL,
         "The site map was produced at 1:2000 scale with 0.5 m contour intervals.",
         {},
         "[Authored Negative] Cartographic scale ratio and contour interval: must not extract"},

        {"TE-20", EntityCategory::NEGATIVE_CONTROL,
         "The archaeological permit numbered 1991/34 was renewed under the new heritage law.",
         {},
         "[Authored Negative] Government permit code with slash: must not extract as date or ratio"},

        // ====================================================================
        // C. MULTI-ENTITY DENSITY STRESS TESTS (5 Cases)
        // ====================================================================

        {"TE-21", EntityCategory::STRATUM_NAME,
         "Stratum IVB yielded 47 bronze arrowheads in 732 BC; the largest specimen was 9.2 cm long.",
         {{"Stratum IVB", "IVB", "stratum", false, false},
          {"47 bronze arrowheads", "47", "count", false, false},
          {"732 BC", "-731", "exact_bce", false, false},
          {"9.2 cm", "0.092 m", "m", false, false}},
         "[Authored] Dense 4-entity: stratum + count + exact BCE + linear dim"},

        {"TE-22", EntityCategory::LOCUS_PROVENANCE,
         "Locus 14 produced a granite quern stone at depth 1.10 m weighing 3.4 kg.",
         {{"Locus 14", "14", "locus", false, false},
          {"depth 1.10 m", "1.100 m", "depth", false, false},
          {"3.4 kg", "3.4 kg", "kg", false, false}},
         "[Authored] Dense 3-entity: locus + depth + mass"},

        {"TE-23", EntityCategory::TRENCH_ID,
         "In Trench III, Stratum VIII material produced 12 obsidian blades from the floor surface.",
         {{"Trench III", "III", "trench", false, false},
          {"Stratum VIII", "VIII", "stratum", false, false},
          {"12 obsidian blades", "12", "count", false, false}},
         "[Authored] Dense 3-entity: trench + stratum + count"},

        {"TE-24", EntityCategory::SPATIAL_AREA,
         "Area G mudbrick wall fragment measured 30 x 60 cm and is dated c. 1500 BCE.",
         {{"Area G", "G", "area", false, false},
          {"30 x 60 cm", "30.0 x 60.0 cm", "cm_compound", false, false},
          {"c. 1500 BCE", "~-1499", "approx_bce", false, false}},
         "[Authored] Dense 3-entity: area + compound dim + approx BCE"},

        {"TE-25", EntityCategory::UNCALIBRATED_C14_BP,
         "The wooden beam measuring 12 x 15 cm and weighing 4.2 kg gave 2850 +/- 40 BP.",
         {{"2850 +/- 40 BP", "2850±40 BP", "c14_bp", false, true},
          {"12 x 15 cm", "12.0 x 15.0 cm", "cm_compound", false, false},
          {"4.2 kg", "4.2 kg", "kg", false, false}},
         "[Authored] Dense 3-entity: C14 BP + compound dim + mass"},

        // ====================================================================
        // D/E. OUT-OF-SCOPE UNITS (5 Cases)
        // ====================================================================

        {"TE-26", EntityCategory::OUT_OF_SCOPE_UNIT,
         "The water reservoir held an estimated capacity of 500 cubic metres.",
         {},
         "[Authored Out-of-Scope] Volumetric capacity (cubic metres): excluded from core grammar"},

        {"TE-27", EntityCategory::OUT_OF_SCOPE_UNIT,
         "The buried channel had a hydraulic gradient of approximately 0.002 metres per metre.",
         {},
         "[Authored Out-of-Scope] Hydraulic gradient: excluded from measurement grammar"},

        {"TE-28", EntityCategory::OUT_OF_SCOPE_UNIT,
         "OSL ages suggest deposition around twelve thousand years ago.",
         {},
         "[Authored Out-of-Scope] Spelled-out ka age: excluded by no-word-numerals rule"},

        {"TE-29", EntityCategory::OUT_OF_SCOPE_UNIT,
         "The magnetometer survey covered a grid of 20 by 40 metres at 0.25-metre traverse spacing.",
         {},
         "[Authored Out-of-Scope] Survey grid described as methodology parameter; 0.25-metre interval must not fire"},

        {"TE-30", EntityCategory::OUT_OF_SCOPE_UNIT,
         "Occupation debris accumulated at a sedimentation rate of approximately one millimetre per year.",
         {},
         "[Authored Out-of-Scope] Spelled-out sedimentation rate: excluded by no-word-numerals rule"}
    };
}

} // namespace archaeophd
