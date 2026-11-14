#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <cassert>
#include <iomanip>
#include <cmath>
#include <json.hpp>
#include "extraction/dual_engine_ensemble.hpp"

using json = nlohmann::json;
using namespace archaeophd;

// Helper to read whole file into string
std::string ReadFileToString(const std::string& path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f.is_open()) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// C++ ground truth matcher matching evaluate_benchmark_v2.js
bool MatchCandidateInText(const std::string& text, const std::string& trueVal, std::string& outFound) {
    if (text.empty() || trueVal.empty()) return false;

    // Direct substring check
    if (text.find(trueVal) != std::string::npos) {
        outFound = trueVal;
        return true;
    }

    // Dash / range check (e.g., 20-40 cm)
    if (trueVal.find('-') != std::string::npos) {
        auto pos = trueVal.find('-');
        std::string p1 = trueVal.substr(0, pos);
        std::string p2 = trueVal.substr(pos + 1);
        // regex for p1 ... p2
        std::string pat = "\\b" + p1 + "\\s*[-–—]\\s*" + p2;
        try {
            std::regex re(pat, std::regex_constants::icase);
            std::smatch m;
            if (std::regex_search(text, m, re)) {
                outFound = m.str();
                return true;
            }
        } catch (...) {}
    }

    // Comma normalization (10,000 vs 10000 or 10.000)
    if (trueVal.find(',') != std::string::npos) {
        std::string noComma = trueVal;
        noComma.erase(std::remove(noComma.begin(), noComma.end(), ','), noComma.end());
        if (text.find(noComma) != std::string::npos) {
            outFound = noComma;
            return true;
        }
        std::string pat = "\\b" + std::regex_replace(trueVal, std::regex(","), "[,.]?") + "\\b";
        try {
            std::regex re(pat);
            std::smatch m;
            if (std::regex_search(text, m, re)) {
                outFound = m.str();
                return true;
            }
        } catch (...) {}
    }

    // Units normalization (e.g. "8 m." vs "8 m" or "8 metres")
    // Strictly for single numeric value followed by unit (not ranges like "6 to 8 m.")
    static const std::regex singleUnitRegex("^\\d+\\s*(m\\.|mtrs|metres|cm|pieces)$", std::regex_constants::icase);
    if (std::regex_match(trueVal, singleUnitRegex)) {
        std::string numPart = "";
        for (char c : trueVal) {
            if (isdigit(c)) numPart += c;
            else break;
        }
        if (!numPart.empty()) {
            std::string pat = "\\b" + numPart + "\\s*(m\\.|mtrs|metres|m|cm|pieces)\\b";
            try {
                std::regex re(pat, std::regex_constants::icase);
                std::smatch m;
                if (std::regex_search(text, m, re)) {
                    outFound = m.str();
                    return true;
                }
            } catch (...) {}
        }
    }

    // Strict word boundary for pure integers
    bool allDigits = true;
    for (char c : trueVal) { if (!isdigit(c)) { allDigits = false; break; } }
    if (allDigits) {
        std::string pat = "\\b" + trueVal + "\\b";
        try {
            std::regex re(pat);
            std::smatch m;
            if (std::regex_search(text, m, re)) {
                outFound = m.str();
                return true;
            }
        } catch (...) {}
    }

    // Multi-word phrases / flexible spaces and periods (e.g. "1966 to 1969", "2000 B.C.", "431 A.D.", "1881 and 1896")
    if (trueVal.find(' ') != std::string::npos) {
        std::string pat = "";
        for (char c : trueVal) {
            if (c == ' ') pat += "\\s+";
            else if (c == '.') pat += "\\.?";
            else pat += c;
        }
        try {
            std::regex re(pat, std::regex_constants::icase);
            std::smatch m;
            if (std::regex_search(text, m, re)) {
                outFound = m.str();
                return true;
            }
        } catch (...) {}
    }

    return false;
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine — Unanchored Routing-Count Check (Historical Phase 0, N=166)\n";
    std::cout << "  (Routing-count check only: unanchored page search; false consensus was unmeasured)\n";
    std::cout << "================================================================================\n\n";

    std::string prefix = std::filesystem::exists("tests/ocr_benchmark_50/ground_truth.json") ? "" : "desktop/";
    const std::string gtPath = prefix + "tests/ocr_benchmark_50/ground_truth.json";
    const std::string winDir = prefix + "tests/ocr_benchmark_50/results_windows_ocr";
    const std::string tessDir = prefix + "tests/ocr_benchmark_50/results_tesseract";

    std::string gtRaw = ReadFileToString(gtPath);
    if (gtRaw.empty()) {
        std::cerr << "[ERROR] Cannot open ground truth file at " << gtPath << std::endl;
        return 1;
    }

    json gt = json::parse(gtRaw);
    int totalFacts = static_cast<int>(gt.size());
    std::cout << "[INFO] Loaded " << totalFacts << " hand-labeled ground-truth facts from " << gtPath << "\n";
    assert(totalFacts == 166);

    std::vector<ExtractedFact> allFacts;
    std::vector<ExtractedFact> classAFacts; // Chakrabarti + Rajan
    std::vector<ExtractedFact> classBFacts; // Sankalia

    int winHits = 0;
    int tessHits = 0;
    int bothCorrectCount = 0;
    int winOnlyCount = 0;
    int tessOnlyCount = 0;
    int bothFailCount = 0;
    int totalErrorsCount = 0;

    for (int i = 0; i < totalFacts; ++i) {
        const auto& item = gt[i];
        std::string pageId = item["page_id"].get<std::string>();
        std::string trueVal = item["true_value"].get<std::string>();
        std::string type = item["type"].get<std::string>();
        std::string desc = item["description"].get<std::string>();

        std::string winFile = winDir + "/" + pageId + ".txt";
        std::string tessFile = tessDir + "/" + pageId + ".txt";

        std::string winText = ReadFileToString(winFile);
        std::string tessText = ReadFileToString(tessFile);

        std::string winCandidate = "";
        std::string tessCandidate = "";

        bool winHit = MatchCandidateInText(winText, trueVal, winCandidate);
        bool tessHit = MatchCandidateInText(tessText, trueVal, tessCandidate);

        if (winHit) winHits++;
        if (tessHit) tessHits++;

        if (winHit && tessHit) bothCorrectCount++;
        else if (winHit && !tessHit) { winOnlyCount++; totalErrorsCount++; }
        else if (!winHit && tessHit) { tessOnlyCount++; totalErrorsCount++; }
        else { bothFailCount++; totalErrorsCount++; }

        ExtractedFact fact;
        fact.fact_id = "fact-" + std::to_string(i + 1);
        fact.page_id = pageId;
        fact.type = type;
        fact.entity_name = desc;
        fact.context_snippet = "";
        fact.value_engine_a = winHit ? winCandidate : "";
        fact.value_engine_b = tessHit ? tessCandidate : "";
        fact.status = FactVerificationStatus::FLAGGED_DISAGREEMENT;
        fact.confidence_score = 0.0;

        allFacts.push_back(fact);

        if (pageId.find("sankalia") != std::string::npos) {
            classBFacts.push_back(fact);
        } else {
            classAFacts.push_back(fact);
        }
    }

    std::cout << "\n--------------------------------------------------------------------------------\n";
    std::cout << "1. HISTORICAL UNANCHORED STRING-MATCHING HIT COUNT CHECK (N=166)\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << "  Total Facts Evaluated:             " << totalFacts << "\n";
    std::cout << "  Windows OCR Hits:                  " << winHits << " (" << std::fixed << std::setprecision(2) << (winHits * 100.0 / totalFacts) << "%)\n";
    std::cout << "  Tesseract 5.4 Hits:                " << tessHits << " (" << (tessHits * 100.0 / totalFacts) << "%)\n";
    std::cout << "  Both Engines Correct:              " << bothCorrectCount << "\n";
    std::cout << "  Windows Correct / Tess Failed:     " << winOnlyCount << "\n";
    std::cout << "  Tesseract Correct / Win Failed:    " << tessOnlyCount << "\n";
    std::cout << "  Both Engines Failed:               " << bothFailCount << "\n";
    std::cout << "  Total Errors Observed:             " << totalErrorsCount << "\n" << std::flush;

    std::cout << "[DEBUG] winHits: " << winHits << ", tessHits: " << tessHits 
              << ", bothCorrect: " << bothCorrectCount 
              << ", winOnly: " << winOnlyCount 
              << ", tessOnly: " << tessOnlyCount 
              << ", bothFail: " << bothFailCount << "\n" << std::flush;

    // Verify historical routing count reproduction against Phase 0 dataset
    assert(totalFacts == 166);
    assert(winHits == 128);
    assert(tessHits == 139);
    assert(bothCorrectCount == 119);
    assert(winOnlyCount == 9);
    assert(tessOnlyCount == 20);
    assert(bothFailCount == 18);
    assert(totalErrorsCount == 47);
    std::cout << "  ✓ Historical hit counts reproduced (unanchored page search; false consensus was unmeasured).\n";

    std::cout << "\n--------------------------------------------------------------------------------\n";
    std::cout << "2. C++ ROUTER PARTITIONING ON CLASS A (HISTORICAL UNANCHORED INPUTS)\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    auto classAResult = DualEngineEnsembleRouter::ProcessDocument(
        "batch-class-a",
        DocumentDegradationClass::MODERN_CLEAN,
        classAFacts
    );

    std::cout << "  Total Class A Facts:               " << classAFacts.size() << "\n";
    std::cout << "  Automated Ingestion Permitted:     " << (classAResult.automated_ingestion_permitted ? "YES" : "NO") << "\n";
    std::cout << "  Auto-Accepted Facts (Consensus):   " << classAResult.facts_auto_accepted << " (" 
              << std::fixed << std::setprecision(1) << (classAResult.facts_auto_accepted * 100.0 / classAFacts.size()) << "%)\n";
    std::cout << "  Routed to Verification Queue:      " << classAResult.facts_routed_to_verification_queue << "\n";
    std::cout << "  Policy Verdict:                    " << classAResult.policy_verdict << "\n";

    assert(classAFacts.size() == 104);
    assert(classAResult.automated_ingestion_permitted == true);
    assert(classAResult.facts_auto_accepted == 92);
    assert(classAResult.facts_routed_to_verification_queue == 12);
    std::cout << "  ✓ Class A Router partitioned 92 consensus pairs and 12 queue items (Note: Class A auto-accept is paused in production).\n";

    std::cout << "\n--------------------------------------------------------------------------------\n";
    std::cout << "3. TESTING PRODUCTION C++ ROUTER ON CLASS B (DEGRADED LETTERPRESS: SANKALIA)\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    auto classBResult = DualEngineEnsembleRouter::ProcessDocument(
        "batch-class-b",
        DocumentDegradationClass::POROUS_BLEEDTHROUGH,
        classBFacts
    );

    std::cout << "  Total Class B Facts:               " << classBFacts.size() << "\n";
    std::cout << "  Automated Ingestion Permitted:     " << (classBResult.automated_ingestion_permitted ? "YES" : "NO") << "\n";
    std::cout << "  Auto-Accepted Facts:               " << classBResult.facts_auto_accepted << " (Mandatory 0)\n";
    std::cout << "  Gated for Manual Review:           " << classBResult.facts_gated_for_manual_review << " (100% Gated)\n";
    std::cout << "  Policy Verdict:                    " << classBResult.policy_verdict << "\n";

    assert(classBFacts.size() == 62);
    assert(classBResult.automated_ingestion_permitted == false);
    assert(classBResult.facts_auto_accepted == 0);
    assert(classBResult.facts_gated_for_manual_review == 62);
    std::cout << "  ✓ Class B Hard Gating verified: ZERO facts auto-accepted; 100% routed to manual review.\n";

    std::cout << "\n================================================================================\n";
    std::cout << "  C++ ROUTER ROUTING-COUNT REPRODUCTION CHECK COMPLETED                         \n";
    std::cout << "  (Routing counts verified; false consensus unmeasured here; auto-accept paused)\n";
    std::cout << "================================================================================\n";

    return 0;
}
