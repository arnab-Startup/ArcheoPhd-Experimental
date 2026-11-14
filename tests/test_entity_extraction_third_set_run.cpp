// test_entity_extraction_third_set_run.cpp
// Evaluation harness for the 30-case third evaluation set.
// Gate criteria (Spec v2.0 Section 6):
//   Precision   >= 90.0%
//   Recall      >= 90.0%
//   Specificity >= 95.0%

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include "eval_entity_extraction_third_set.hpp"
#include "extraction/entity_extractor.hpp"

using namespace archaeophd;

std::pair<double, double> wilson95(int k, int n) {
    if (n == 0) return {0.0, 0.0};
    double z = 1.95996;
    double p = static_cast<double>(k) / n;
    double d = 1.0 + z * z / n;
    double c = (p + z * z / (2.0 * n)) / d;
    double m = z * std::sqrt(p * (1.0 - p) / n + z * z / (4.0 * n * n)) / d;
    return {std::max(0.0, c - m) * 100.0, std::min(1.0, c + m) * 100.0};
}

std::string category_str(EntityCategory cat) {
    switch (cat) {
        case EntityCategory::LINEAR_DIMENSION:       return "Linear Dim";
        case EntityCategory::LINEAR_RANGE:           return "Linear Range";
        case EntityCategory::COMPOUND_DIMENSION:     return "Compound Dim";
        case EntityCategory::DEPTH_ELEVATION:        return "Depth/Elev";
        case EntityCategory::MASS_WEIGHT:            return "Mass/Weight";
        case EntityCategory::EXACT_DATE_BCE:         return "Exact BCE";
        case EntityCategory::EXACT_DATE_CE:          return "Exact CE";
        case EntityCategory::APPROX_DATE_BCE:        return "Approx BCE";
        case EntityCategory::DATE_RANGE:             return "Date Rng BCE";
        case EntityCategory::DATE_RANGE_CE:          return "Date Rng CE";
        case EntityCategory::UNCALIBRATED_C14_BP:    return "Uncal C14 BP";
        case EntityCategory::AUTHOR_CALIBRATED_DATE: return "Author Cal";
        case EntityCategory::TEMPERATURE:            return "Temperature";
        case EntityCategory::ARTIFACT_SPECIMEN_COUNT:return "Artifact Cnt";
        case EntityCategory::LOCUS_PROVENANCE:       return "Locus ID";
        case EntityCategory::SPATIAL_AREA:           return "Spatial Area";
        case EntityCategory::STRATUM_NAME:           return "Stratum";
        case EntityCategory::TRENCH_ID:              return "Trench ID";
        case EntityCategory::BASKET_UNIT:            return "Basket Unit";
        case EntityCategory::OCR_CORRUPTION_ANOMALY: return "OCR Corrupt";
        case EntityCategory::NEGATIVE_CONTROL:       return "Neg Control";
        case EntityCategory::OUT_OF_SCOPE_UNIT:      return "Out-of-Scope";
        default:                                     return "Unknown";
    }
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine -- Phase 2 Step 3: Third Evaluation Set Run\n";
    std::cout << "  Extractor: entity_extractor.hpp (commit 116995d+)\n";
    std::cout << "  PROVENANCE: 30-case evaluation set authored 2026-10-05\n";
    std::cout << "================================================================================\n\n";

    auto cases = get_third_evaluation_dataset();
    std::cout << "Loaded " << cases.size() << " third-set test cases.\n\n";

    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << std::left
              << std::setw(8)  << "ID"
              << std::setw(16) << "Category"
              << std::setw(6)  << "Exp"
              << std::setw(6)  << "Ext"
              << std::setw(6)  << "TP"
              << std::setw(6)  << "FP"
              << std::setw(6)  << "FN"
              << "Status\n";
    std::cout << "--------------------------------------------------------------------------------\n";

    int total_tp = 0, total_fp = 0, total_fn = 0;
    int total_tn = 0;
    int total_tn_cases = 0;
    int total_expected = 0;

    std::vector<std::string> failed_ids;

    for (const auto& tc : cases) {
        auto extracted = EntityExtractor::extract_entities(tc.passage_text);

        int tp = 0, fp = 0, fn = 0;
        bool is_negative = (tc.category == EntityCategory::NEGATIVE_CONTROL ||
                            tc.category == EntityCategory::OUT_OF_SCOPE_UNIT);

        if (is_negative) {
            total_tn_cases++;
            if (extracted.empty()) {
                total_tn++;
            } else {
                fp = static_cast<int>(extracted.size());
                total_fp += fp;
            }
        } else {
            total_expected += static_cast<int>(tc.expected_entities.size());
            std::vector<bool> exp_matched(tc.expected_entities.size(), false);

            for (const auto& actual : extracted) {
                bool matched = false;
                for (size_t i = 0; i < tc.expected_entities.size(); ++i) {
                    if (!exp_matched[i]) {
                        const auto& exp = tc.expected_entities[i];
                        if (actual.raw_match.find(exp.raw_match) != std::string::npos ||
                            exp.raw_match.find(actual.raw_match) != std::string::npos ||
                            actual.normalized_value == exp.normalized_value) {
                            matched = true;
                            exp_matched[i] = true;
                            tp++;
                            break;
                        }
                    }
                }
                if (!matched) fp++;
            }
            for (size_t i = 0; i < tc.expected_entities.size(); ++i) {
                if (!exp_matched[i]) fn++;
            }

            total_tp += tp;
            total_fp += fp;
            total_fn += fn;
        }

        std::string status = (fp == 0 && fn == 0) ? "[PASS]" : "[FAIL]";
        if (status == "[FAIL]") failed_ids.push_back(tc.id);

        std::cout << std::left
                  << std::setw(8)  << tc.id
                  << std::setw(16) << category_str(tc.category)
                  << std::setw(6)  << tc.expected_entities.size()
                  << std::setw(6)  << extracted.size()
                  << std::setw(6)  << tp
                  << std::setw(6)  << fp
                  << std::setw(6)  << fn
                  << status << "\n";
    }

    std::cout << "--------------------------------------------------------------------------------\n\n";

    double precision = (total_tp + total_fp > 0)
        ? (static_cast<double>(total_tp) / (total_tp + total_fp) * 100.0) : 0.0;
    double recall = (total_tp + total_fn > 0)
        ? (static_cast<double>(total_tp) / (total_tp + total_fn) * 100.0) : 0.0;
    double specificity = (total_tn_cases > 0)
        ? (static_cast<double>(total_tn) / total_tn_cases * 100.0) : 0.0;

    auto p_ci  = wilson95(total_tp, total_tp + total_fp);
    auto r_ci  = wilson95(total_tp, total_tp + total_fn);
    auto sp_ci = wilson95(total_tn, total_tn_cases);

    std::cout << "================================================================================\n";
    std::cout << "  THIRD EVALUATION SET RESULTS\n";
    std::cout << "================================================================================\n";
    std::cout << "  Positive cases:       15 test cases (" << total_expected << " expected entities)\n";
    std::cout << "  Negative controls:    10 (must suppress all)\n";
    std::cout << "  Out-of-scope units:    5 (must suppress all)\n";
    std::cout << "  TP: " << total_tp << "  FP: " << total_fp
              << "  FN: " << total_fn
              << "  TN: " << total_tn << "/" << total_tn_cases << "\n\n";

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  [METRIC 1] Precision:     " << precision << "% ("
              << total_tp << "/" << (total_tp + total_fp) << ")\n";
    std::cout << "             Wilson 95% CI: [" << p_ci.first << "%, " << p_ci.second << "%]\n\n";

    std::cout << "  [METRIC 2] Recall:        " << recall << "% ("
              << total_tp << "/" << (total_tp + total_fn) << ")\n";
    std::cout << "             Wilson 95% CI: [" << r_ci.first << "%, " << r_ci.second << "%]\n\n";

    std::cout << "  [METRIC 3] Specificity:   " << specificity << "% ("
              << total_tn << "/" << total_tn_cases << ")\n";
    std::cout << "             Wilson 95% CI: [" << sp_ci.first << "%, " << sp_ci.second << "%]\n\n";

    std::cout << "  Failed cases (" << failed_ids.size() << "):\n";
    for (const auto& id : failed_ids) {
        std::cout << "    " << id << "\n";
    }

    // -------------------------------------------------------------------------
    // Hard gate evaluation
    // Gate criteria (Spec v2.0 Section 6):
    //   Precision   >= 90.0%
    //   Recall      >= 90.0%
    //   Specificity >= 95.0%
    // -------------------------------------------------------------------------
    constexpr double GATE_PRECISION    = 90.0;
    constexpr double GATE_RECALL       = 90.0;
    constexpr double GATE_SPECIFICITY  = 95.0;

    bool gate_p   = (precision   >= GATE_PRECISION);
    bool gate_r   = (recall      >= GATE_RECALL);
    bool gate_sp  = (specificity >= GATE_SPECIFICITY);
    bool all_pass = gate_p && gate_r && gate_sp;

    std::cout << "================================================================================\n";
    std::cout << "  GATE EVALUATION  (pre-registered 2026-10-05)\n";
    std::cout << "================================================================================\n";
    std::cout << "  [GATE P ]  Precision  >= " << std::fixed << std::setprecision(1)
              << GATE_PRECISION    << "%  -> " << precision   << "%  "
              << (gate_p  ? "[PASS]" : "[FAIL]") << "\n";
    std::cout << "  [GATE R ]  Recall     >= " << GATE_RECALL
              << "%  -> " << recall      << "%  "
              << (gate_r  ? "[PASS]" : "[FAIL]") << "\n";
    std::cout << "  [GATE SP]  Specificity>= " << GATE_SPECIFICITY
              << "%  -> " << specificity << "%  "
              << (gate_sp ? "[PASS]" : "[FAIL]") << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  OVERALL GATE: " << (all_pass ? "[PASS]" : "[FAIL]") << "\n";
    std::cout << "================================================================================\n";

    return all_pass ? 0 : 1;
}
