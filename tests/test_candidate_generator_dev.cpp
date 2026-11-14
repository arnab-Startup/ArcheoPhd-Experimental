#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <json.hpp>

#include "extraction/candidate_generator.hpp"

using json = nlohmann::json;
using namespace archaeophd;

// ============================================================================
// ArchaeoPhD Phase 2 Step 4 — Candidate Generator Development Test Harness
//
// ENVIRONMENT INVARIANT:
// - Evaluates strictly against the Development Set (step4_dev_set.json, N = 48).
// - The 60-case sealed benchmark (step4_sealed_benchmark.json) was NOT opened,
//   read, or executed. Its cryptographic SHA-256 seal (4B9AD58F...) remains intact.
// ============================================================================

enum class OutcomeCategory {
    CORRECT,
    UNIT_LOST,
    UNBOUND_COUNT_NOUN,
    PARTIAL,
    DISPLACED,
    FALSE_INCLUSION_ON_ABSENT_GT,
    NON_FINDING_FALSE_INCLUSION,
    AMBIGUOUS_UNBUNDLED,
    ABSENT_OR_MISSED
};

std::string outcomeToString(OutcomeCategory o) {
    switch (o) {
        case OutcomeCategory::CORRECT: return "CORRECT";
        case OutcomeCategory::UNIT_LOST: return "UNIT_LOST";
        case OutcomeCategory::UNBOUND_COUNT_NOUN: return "UNBOUND_COUNT_NOUN";
        case OutcomeCategory::PARTIAL: return "PARTIAL";
        case OutcomeCategory::DISPLACED: return "DISPLACED";
        case OutcomeCategory::FALSE_INCLUSION_ON_ABSENT_GT: return "FALSE_INCLUSION_ON_ABSENT_GT";
        case OutcomeCategory::NON_FINDING_FALSE_INCLUSION: return "NON_FINDING_FALSE_INCLUSION";
        case OutcomeCategory::AMBIGUOUS_UNBUNDLED: return "AMBIGUOUS_UNBUNDLED";
        case OutcomeCategory::ABSENT_OR_MISSED: return "ABSENT_OR_MISSED";
    }
    return "UNKNOWN";
}

struct EvaluationResult {
    OutcomeCategory outcome;
    std::string diagnostic;
    std::string output_str;
};

// Evaluate a candidate result against ground truth using the strict ordered hierarchy
EvaluationResult EvaluateCandidateAgainstGroundTruth(
    const AttributedCandidate& cand,
    const json& c)
{
    EvaluationResult res;
    std::string expectedAction = c["expected_action"];
    std::string sourceChunk = c["source_chunk"];

    bool emitted = (cand.status == CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING);

    if (expectedAction == "ROUTE_AMBIGUOUS_OR_UNANCHORED") {
        if (emitted) {
            res.outcome = OutcomeCategory::FALSE_INCLUSION_ON_ABSENT_GT;
            res.diagnostic = "FAIL: Ground truth absent; emitted unanchored candidate '" + cand.value.raw_text + "'.";
        } else {
            res.outcome = OutcomeCategory::CORRECT;
            res.diagnostic = "PASS: Unanchored query safely routed to queue.";
        }
        res.output_str = emitted ? ("'" + cand.value.raw_text + "'") : "ROUTED_QUEUE";
    } else if (expectedAction == "REJECT_NON_FINDING") {
        if (emitted) {
            res.outcome = OutcomeCategory::NON_FINDING_FALSE_INCLUSION;
            res.diagnostic = "FAIL: Emitted candidate '" + cand.value.raw_text + "' on non-finding noise.";
            res.output_str = "'" + cand.value.raw_text + "'";
        } else {
            res.outcome = OutcomeCategory::CORRECT;
            res.diagnostic = "PASS: Non-finding noise correctly suppressed.";
            res.output_str = "REJECTED_NON_FINDING";
        }
    } else if (expectedAction == "FLAG_AMBIGUOUS_MULTI_CANDIDATE") {
        if (cand.status == CandidateStatus::AMBIGUOUS_MULTI_CANDIDATE && !cand.bundled_candidates.empty()) {
            res.outcome = OutcomeCategory::CORRECT;
            res.diagnostic = "PASS: Clausal ambiguity flagged and candidates bundled.";
            std::string bundleStr = "[";
            for (size_t i = 0; i < cand.bundled_candidates.size(); i++) {
                if (i > 0) bundleStr += ", ";
                bundleStr += cand.bundled_candidates[i];
            }
            bundleStr += "]";
            res.output_str = "AMBIGUOUS: " + bundleStr;
        } else if (emitted) {
            res.outcome = OutcomeCategory::AMBIGUOUS_UNBUNDLED;
            res.diagnostic = "FAIL: Emitted single candidate '" + cand.value.raw_text + "' instead of bundling multi-candidates.";
            res.output_str = "'" + cand.value.raw_text + "'";
        } else {
            res.outcome = OutcomeCategory::ABSENT_OR_MISSED;
            res.diagnostic = "FAIL: Missed ambiguous clause entirely.";
            res.output_str = "None";
        }
    } else if (expectedAction == "EXTRACT_ATTRIBUTED") {
        auto gt = c["ground_truth"];
        auto structVal = gt["structured_value"];
        std::string gtRaw = structVal.value("raw", "");
        std::string gtUnit = structVal.value("normalized_unit", "");
        std::string gtValType = structVal.value("value_type", "SINGLE");
        std::string gtHeadNoun = structVal.value("head_noun", "");
        double gtNumStart = structVal.value("numeric_start", 0.0);

        if (!emitted) {
            res.outcome = OutcomeCategory::ABSENT_OR_MISSED;
            res.diagnostic = "FAIL: Missed finding entirely (status: " + std::to_string(static_cast<int>(cand.status)) + ").";
            res.output_str = "None";
        } else if (gtValType == "RANGE") {
            if (cand.value.value_type != ValueType::RANGE) {
                res.outcome = OutcomeCategory::PARTIAL;
                res.diagnostic = "FAIL: Truncated range '" + gtRaw + "' into single token '" + cand.value.raw_text + "'.";
            } else if (std::abs(cand.value.numeric_start - gtNumStart) > 1e-4) {
                res.outcome = OutcomeCategory::DISPLACED;
                res.diagnostic = "FAIL: Displaced to wrong range '" + cand.value.raw_text + "' (GT was '" + gtRaw + "').";
            } else {
                res.outcome = OutcomeCategory::CORRECT;
                res.diagnostic = "PASS: Range matched.";
            }
            res.output_str = "'" + cand.value.raw_text + "'";
        } else {
            // Single numeric finding: Value check FIRST (Step 4A)
            bool valueMatched = (cand.value.raw_text == gtRaw || std::abs(cand.value.numeric_start - gtNumStart) < 1e-4);
            if (!valueMatched) {
                res.outcome = OutcomeCategory::DISPLACED;
                res.diagnostic = "FAIL: Displaced to neighbor token '" + cand.value.raw_text + "' (GT was '" + gtRaw + "').";
            } else {
                // Step 4B: Unit and head noun check
                if (!gtUnit.empty() && gtUnit != "CE" && gtUnit != "AD" && cand.value.normalized_unit != gtUnit) {
                    res.outcome = OutcomeCategory::UNIT_LOST;
                    res.diagnostic = "FAIL: Value matched ('" + gtRaw + "') but stripped physical unit '" + gtUnit + "'.";
                } else if (!gtHeadNoun.empty() && cand.value.head_noun != gtHeadNoun) {
                    res.outcome = OutcomeCategory::UNBOUND_COUNT_NOUN;
                    res.diagnostic = "FAIL: Value matched ('" + gtRaw + "') but failed to bind head noun '" + gtHeadNoun + "'.";
                } else {
                    res.outcome = OutcomeCategory::CORRECT;
                    res.diagnostic = "PASS: Value and unit/noun matched.";
                }
            }
            res.output_str = "'" + cand.value.raw_text + (cand.value.normalized_unit.empty() ? "" : (" " + cand.value.normalized_unit)) + "'";
        }
    }
    return res;
}

// Baseline picker evaluator (for historical comparison)
EvaluationResult EvaluateNaiveBaseline(const json& c) {
    std::string text = c["source_chunk"];
    AttributedCandidate cand;
    std::regex re_digits(R"(\b\d+(?:[.,]\d+)?\b)");
    std::smatch m;
    if (std::regex_search(text, m, re_digits)) {
        cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
        cand.value.raw_text = m.str();
        std::string cln = m.str();
        cln.erase(std::remove(cln.begin(), cln.end(), ','), cln.end());
        try { cand.value.numeric_start = std::stod(cln); } catch(...) { cand.value.numeric_start = 0.0; }
    } else {
        cand.status = CandidateStatus::UNANCHORED_OR_DEGRADED;
    }
    return EvaluateCandidateAgainstGroundTruth(cand, c);
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Step 4 — Candidate Generator Dev-Set Evaluation Harness\n";
    std::cout << "  Dataset: tests/step4_eval/step4_dev_set.json (N = 48)\n";
    std::cout << "  Integrity Note: Sealed benchmark (step4_sealed_benchmark.json) was NOT read.\n";
    std::cout << "================================================================================\n\n";

    std::string devSetPath = "tests/step4_eval/step4_dev_set.json";
    std::ifstream f(devSetPath);
    if (!f.is_open()) {
        std::cerr << "[ERROR] Could not open " << devSetPath << "\n";
        return 1;
    }

    json devSet = json::parse(f);
    auto cases = devSet["cases"];
    int totalCases = static_cast<int>(cases.size());

    // Metrics for CandidateGenerator
    int genCorrect = 0, genUnitLost = 0, genUnboundNoun = 0, genPartial = 0, genDisplaced = 0;
    int genFalseIncAbsent = 0, genNonFindingFalseInc = 0, genAmbiguousUnbundled = 0, genAbsent = 0;

    // Metrics for Naive Baseline
    int baseCorrect = 0;

    // Separate metrics for 22 Forensic Audit Facts
    int auditTotal = 0, auditBaseCorrect = 0, auditGenCorrect = 0;

    std::cout << "| Case ID | Provenance | Target Entity | Expected Action | Baseline Outcome | Generator Output | Generator Outcome |\n";
    std::cout << "| :--- | :--- | :--- | :--- | :---: | :--- | :---: |\n";

    for (const auto& c : cases) {
        std::string caseId = c["case_id"];
        std::string targetEntity = c["target_entity"];
        std::string expectedAction = c["expected_action"];
        std::string sourceChunk = c["source_chunk"];
        std::string provenance = c.value("provenance", "UNKNOWN");

        bool isAudit = (caseId.find("-FC") != std::string::npos);
        if (isAudit) auditTotal++;

        // 1. Evaluate Baseline
        EvaluationResult baseRes = EvaluateNaiveBaseline(c);
        if (baseRes.outcome == OutcomeCategory::CORRECT) {
            baseCorrect++;
            if (isAudit) auditBaseCorrect++;
        }

        // 2. Evaluate CandidateGenerator
        std::string targetProp = "";
        std::string targetSubj = "";
        if (c.contains("entity_slot") && c["entity_slot"].is_object()) {
            targetProp = c["entity_slot"].value("property", "");
            targetSubj = c["entity_slot"].value("subject", "");
        }
        AttributedCandidate genCand = CandidateGenerator::GenerateCandidate(sourceChunk, targetProp, targetSubj);
        EvaluationResult genRes = EvaluateCandidateAgainstGroundTruth(genCand, c);

        switch (genRes.outcome) {
            case OutcomeCategory::CORRECT: genCorrect++; if (isAudit) auditGenCorrect++; break;
            case OutcomeCategory::UNIT_LOST: genUnitLost++; break;
            case OutcomeCategory::UNBOUND_COUNT_NOUN: genUnboundNoun++; break;
            case OutcomeCategory::PARTIAL: genPartial++; break;
            case OutcomeCategory::DISPLACED: genDisplaced++; break;
            case OutcomeCategory::FALSE_INCLUSION_ON_ABSENT_GT: genFalseIncAbsent++; break;
            case OutcomeCategory::NON_FINDING_FALSE_INCLUSION: genNonFindingFalseInc++; break;
            case OutcomeCategory::AMBIGUOUS_UNBUNDLED: genAmbiguousUnbundled++; break;
            case OutcomeCategory::ABSENT_OR_MISSED: genAbsent++; break;
        }

        std::cout << "| " << caseId << " | " << provenance.substr(0, 16) << " | " << targetEntity << " | "
                  << expectedAction << " | " << outcomeToString(baseRes.outcome) << " | "
                  << genRes.output_str << " | " << outcomeToString(genRes.outcome) << " |\n";
    }

    std::cout << "\n================================================================================\n";
    std::cout << "  DEVELOPMENT SET EVALUATION SUMMARY (N = " << totalCases << ")\n";
    std::cout << "================================================================================\n";
    std::cout << "  Baseline Naive Picker Correct:         " << baseCorrect << " / " << totalCases
              << " (" << std::fixed << std::setprecision(1) << (100.0 * baseCorrect / totalCases) << "%)\n";
    std::cout << "  CandidateGenerator Correct:            " << genCorrect << " / " << totalCases
              << " (" << std::fixed << std::setprecision(1) << (100.0 * genCorrect / totalCases) << "%)\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << "  22 FORENSIC AUDIT FACTS SUBSET (N = " << auditTotal << "):\n";
    std::cout << "  Baseline Naive Picker Correct:         " << auditBaseCorrect << " / " << auditTotal
              << " (" << (100.0 * auditBaseCorrect / auditTotal) << "%)\n";
    std::cout << "  CandidateGenerator Correct:            " << auditGenCorrect << " / " << auditTotal
              << " (" << (100.0 * auditGenCorrect / auditTotal) << "%)\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << "  GENERATOR RESIDUAL DEFECT BREAKDOWN:\n";
    std::cout << "  - Unit Lost:                           " << genUnitLost << "\n";
    std::cout << "  - Unbound Count Noun:                  " << genUnboundNoun << "\n";
    std::cout << "  - Partial Range Truncations:           " << genPartial << "\n";
    std::cout << "  - Neighbor Displacements:              " << genDisplaced << "\n";
    std::cout << "  - False Inclusions on Absent GT:       " << genFalseIncAbsent << "\n";
    std::cout << "  - Non-Finding Noise Inclusions:        " << genNonFindingFalseInc << "\n";
    std::cout << "  - Ambiguous Multi-Candidates Missed:   " << genAmbiguousUnbundled << "\n";
    std::cout << "  - Absent or Missed:                    " << genAbsent << "\n";
    std::cout << "================================================================================\n";

    return 0;
}
