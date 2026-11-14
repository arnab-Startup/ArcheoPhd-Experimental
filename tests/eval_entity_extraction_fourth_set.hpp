#pragma once

#include <string>
#include <vector>
#include "eval_entity_extraction_dataset.hpp"

namespace archaeophd {

// ============================================================================
// Phase 2 Step 3 — Fourth Evaluation Set (Fresh Evaluation Suite)
//
// PROVENANCE: Authored 2026-10-05 under Spec v2.0 Section 2.3 rules.
// Governed by:
//   - Section 2.3: Mention-Level Extraction vs. Attribution Relevance Boundary.
//   - Cartographic & methodology dimensions (FE-19, FE-20) are treated as
//     in-scope physical mentions.
//   - Pure negative controls (FE-21..26) test non-archaeological strings
//     (bibliographic dates, pages, accession codes, isotopes, coordinates).
//   - Out-of-scope units (FE-27..30) test macro/non-stratigraphic physical units.
//
// GATES (Pre-Registered in Spec v2.0):
//   Precision   >= 90.0%
//   Recall      >= 90.0%
//   Specificity >= 95.0%
// ============================================================================

inline std::vector<ExtractionTestCase> get_fourth_evaluation_dataset() {
    return {

        // ====================================================================
        // A. POSITIVE PHYSICAL & SPATIAL EXTRACTION TARGETS (8 Cases)
        // ====================================================================

        // FE-01: Linear dimension with metres
        {"FE-01", EntityCategory::LINEAR_DIMENSION,
         "The megalithic capstone measures 3.85 m in length and rests upon two upright orthostats.",
         {{"3.85 m", "3.850 m", "m", false, false}},
         "[Fresh Set] Megalithic linear dimension in metres"},

        // FE-02: Hyphenated linear range cm
        {"FE-02", EntityCategory::LINEAR_RANGE,
         "Excavators recorded a pebble layer 15-25 cm thick across the northern quadrant.",
         {{"15-25 cm", "[0.15, 0.25] m", "cm_range", false, false}},
         "[Fresh Set] Stratigraphic layer thickness range in cm"},

        // FE-03: Compound dimension with by
        {"FE-03", EntityCategory::COMPOUND_DIMENSION,
         "The mudbrick silo foundation measures 1.80 by 2.40 m in plan.",
         {{"1.80 by 2.40 m", "1.80 x 2.40 m", "m_compound", false, false}},
         "[Fresh Set] Compound 2D architectural dimension in metres"},

        // FE-04: Vertical depth
        {"FE-04", EntityCategory::DEPTH_ELEVATION,
         "Virgin soil was reached at depth 3.60 m below the tell summit.",
         {{"depth 3.60 m", "3.600 m", "depth", false, false}},
         "[Fresh Set] Stratigraphic depth below datum"},

        // FE-05: Mass in kilograms
        {"FE-05", EntityCategory::MASS_WEIGHT,
         "The hoard included a copper ingot weighing 1.85 kg beside several scrap fragments.",
         {{"1.85 kg", "1.85 kg", "kg", false, false}},
         "[Fresh Set] Metal ingot mass in kg"},

        // FE-06: Archaeometric firing temperature
        {"FE-06", EntityCategory::TEMPERATURE,
         "Ceramic firing temperatures were estimated at 850 °C from XRD mineral phases.",
         {{"850 °C", "850 °C", "temperature", false, false}},
         "[Fresh Set] Pyrotechnology firing temperature in °C"},

        // FE-07: Artifact count: obsidian blades
        {"FE-07", EntityCategory::ARTIFACT_SPECIMEN_COUNT,
         "A cluster of 18 obsidian blades was discovered near the hearth.",
         {{"18 obsidian blades", "18", "count", false, false}},
         "[Fresh Set] Lithic tool count"},

        // FE-08: Artifact count: shell beads
        {"FE-08", EntityCategory::ARTIFACT_SPECIMEN_COUNT,
         "The burial pit yielded 34 shell beads scattered around the cranium.",
         {{"34 shell beads", "34", "count", false, false}},
         "[Fresh Set] Ornament specimen count"},

        // ====================================================================
        // B. POSITIVE CHRONOLOGICAL DATES (7 Cases)
        // ====================================================================

        // FE-09: Approximate BCE date with around prefix
        {"FE-09", EntityCategory::APPROX_DATE_BCE,
         "The lower city was destroyed around 1185 BCE during regional disturbances.",
         {{"around 1185 BCE", "~-1184", "approx_bce", false, false}},
         "[Fresh Set] Approximate historical BCE date"},

        // FE-10: Exact BCE date with BC suffix
        {"FE-10", EntityCategory::EXACT_DATE_BCE,
         "Radiocarbon dating placed the palisade construction at 840 BC based on post timbers.",
         {{"840 BC", "-839", "exact_bce", false, false}},
         "[Fresh Set] Exact BCE date with BC marker"},

        // FE-11: Exact CE date with CE suffix
        {"FE-11", EntityCategory::EXACT_DATE_CE,
         "The monumental basilica was dedicated in 348 CE by the metropolitan bishop.",
         {{"348 CE", "+348", "exact_ce", false, false}},
         "[Fresh Set] Exact CE date with CE marker"},

        // FE-12: Exact CE date with preceding A.D. marker
        {"FE-12", EntityCategory::EXACT_DATE_CE,
         "The coin hoard was buried no earlier than A.D. 260 under Gallienus.",
         {{"A.D. 260", "+260", "exact_ce", false, false}},
         "[Fresh Set] Exact CE date with prefix A.D. form"},

        // FE-13: Chronological range: between X and Y BCE
        {"FE-13", EntityCategory::DATE_RANGE,
         "Regional ceramic production flourished between 650 and 500 BCE.",
         {{"between 650 and 500 BCE", "[-649, -499] BCE", "bce_range", false, false}},
         "[Fresh Set] BCE chronological range"},

        // FE-14: Uncalibrated radiocarbon BP
        {"FE-14", EntityCategory::UNCALIBRATED_C14_BP,
         "Charred grain from the storage pit returned a date of 4120 +/- 50 BP.",
         {{"4120 +/- 50 BP", "4120±50 BP", "c14_bp", false, true}},
         "[Fresh Set] Uncalibrated radiocarbon BP determination"},

        // FE-15: Author-calibrated BC range with en-dash
        {"FE-15", EntityCategory::AUTHOR_CALIBRATED_DATE,
         "Dendro-calibrated age determination yielded 1420–1310 cal BC at 2-sigma.",
         {{"1420–1310 cal BC", "[-1419, -1309] cal BC", "cal_bc", false, false}},
         "[Fresh Set] Calibrated calendar BCE range with en-dash"},

        // ====================================================================
        // C. MULTI-ENTITY PROVENANCE & DENSE FINDS (3 Cases)
        // ====================================================================

        // FE-16: Locus + Trench
        {"FE-16", EntityCategory::LOCUS_PROVENANCE,
         "Locus 42 contained a stone altar standing adjacent to Trench IV.",
         {{"Locus 42", "42", "locus", false, false},
          {"Trench IV", "IV", "trench", false, false}},
         "[Fresh Set] Dual entity: Locus + Trench"},

        // FE-17: Stratum with sub-phase letter + linear dimension
        {"FE-17", EntityCategory::STRATUM_NAME,
         "Stratum IIa domestic architecture featured walls preserved up to 1.4 m high.",
         {{"Stratum IIa", "IIa", "stratum", false, false},
          {"1.4 m", "1.400 m", "m", false, false}},
         "[Fresh Set] Dual entity: Stratum with lowercase sub-phase + linear dimension"},

        // FE-18: Spatial Area + Count + Depth
        {"FE-18", EntityCategory::SPATIAL_AREA,
         "In Area D, excavators recovered 120 potsherds from an ash deposit at depth 0.90 m.",
         {{"Area D", "D", "area", false, false},
          {"120 potsherds", "120", "count", false, false},
          {"depth 0.90 m", "0.900 m", "depth", false, false}},
         "[Fresh Set] Dense 3-entity: Area + ceramic count + depth"},

        // ====================================================================
        // D. METHODOLOGICAL & SURVEY MENTIONS (Spec Section 2.3: In-Scope) (2 Cases)
        // ====================================================================

        // FE-19: Cartographic contour interval
        {"FE-19", EntityCategory::LINEAR_DIMENSION,
         "Topographic mapping of the lower terrace used a 1.0 m contour interval.",
         {{"1.0 m", "1.000 m", "m", false, false}},
         "[Fresh Set Section 2.3] Valid physical dimension mention in cartographic context"},

        // FE-20: Methodological sampling interval
        {"FE-20", EntityCategory::LINEAR_DIMENSION,
         "Sediment columns were sub-sampled at 5 cm intervals for micro-botanical analysis.",
         {{"5 cm", "0.050 m", "cm", false, false}},
         "[Fresh Set Section 2.3] Valid physical dimension mention in sampling context"},

        // ====================================================================
        // E. PURE NEGATIVE CONTROLS (Zero targeted units or dates) (6 Cases)
        // ====================================================================

        // FE-21: Modern bibliographic citation
        {"FE-21", EntityCategory::NEGATIVE_CONTROL,
         "The preliminary excavation report was published in 1987 in Syria, Volume 64.",
         {},
         "[Fresh Negative] Modern publication year and volume number: must suppress"},

        // FE-22: Page citation span
        {"FE-22", EntityCategory::NEGATIVE_CONTROL,
         "Comparative typologies are discussed on pages 142–158 of the companion monograph.",
         {},
         "[Fresh Negative] Monograph page range: must suppress"},

        // FE-23: Figure and table cross-references
        {"FE-23", EntityCategory::NEGATIVE_CONTROL,
         "See Figure 12 and Table 4 for the complete stratigraphic matrix diagram.",
         {},
         "[Fresh Negative] Publication cross-references: must suppress"},

        // FE-24: Accession number
        {"FE-24", EntityCategory::NEGATIVE_CONTROL,
         "The archaeological collection is cataloged under accession code BM-1984/22.",
         {},
         "[Fresh Negative] Museum accession number with slash: must suppress"},

        // FE-25: Geochemical delta isotope notation
        {"FE-25", EntityCategory::NEGATIVE_CONTROL,
         "Carbon isotope ratios averaged -24.2 per mil relative to the VPDB standard.",
         {},
         "[Fresh Negative] Stable isotope per-mil notation: must suppress"},

        // FE-26: Geographic coordinates in DMS format
        {"FE-26", EntityCategory::NEGATIVE_CONTROL,
         "The trench corner benchmark is located at 31 degrees 46 minutes North, 35 degrees 14 minutes East.",
         {},
         "[Fresh Negative] Coordinate string: must suppress"},

        // ====================================================================
        // F. OUT-OF-SCOPE MACRO/PHYSICAL UNITS (4 Cases)
        // ====================================================================

        // FE-27: Hectares
        {"FE-27", EntityCategory::OUT_OF_SCOPE_UNIT,
         "The fortified enclosure encompasses an area of approximately 14 hectares.",
         {},
         "[Fresh Out-of-Scope] Land area in hectares: excluded from micro-stratigraphic grammar"},

        // FE-28: Square kilometres
        {"FE-28", EntityCategory::OUT_OF_SCOPE_UNIT,
         "The catchment survey covered 250 square kilometres around the primary center.",
         {},
         "[Fresh Out-of-Scope] Regional survey area in square kilometres: excluded"},

        // FE-29: Degrees Fahrenheit
        {"FE-29", EntityCategory::OUT_OF_SCOPE_UNIT,
         "Field laboratory refrigeration was maintained at 38 degrees Fahrenheit.",
         {},
         "[Fresh Out-of-Scope] Non-metric temperature scale: excluded"},

        // FE-30: Knots
        {"FE-30", EntityCategory::OUT_OF_SCOPE_UNIT,
         "The coastal supply ship recorded offshore winds of 25 knots during transport.",
         {},
         "[Fresh Out-of-Scope] Nautical velocity unit: excluded"}
    };
}

} // namespace archaeophd
