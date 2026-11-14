// test_real_ocr_eval.cpp
// Real-OCR Plausibility Evaluation — Phase 2 Step 3 Baseline Run
// Evaluates the unmodified extractor (a6cf766) against all 50 Tesseract and
// 50 Windows OCR pages from desktop/tests/ocr_benchmark_50/.
// Measures:
//   - Invariant 1: Never-Repair (raw_match == text slice)
//   - Extraction Recall: ground-truth facts found (by type + value substring)
//   - Plausibility Specificity: clean facts extracted without anomaly_flag
//   - Plausibility Sensitivity: corrupted facts with any anomaly_flag on page
// This is the FIRST RUN on unmodified extractor. No gates applied.

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "include/json.hpp"
#include "engine/extraction/entity_extractor.hpp"

using namespace archaeophd;
using json = nlohmann::json;
namespace fs = std::filesystem;

std::string read_file(const fs::path& p) {
    std::ifstream f(p);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Is this entity type a date type?
bool is_date_entity(const std::string& t) {
    return t.find("DATE") != std::string::npos ||
           t.find("C14_BP") != std::string::npos ||
           t.find("CALIBRATED") != std::string::npos ||
           t.find("APPROX") != std::string::npos;
}

// Is this entity type a measurement type?
bool is_meas_entity(const std::string& t) {
    return t.find("DIMENSION") != std::string::npos ||
           t.find("RANGE") != std::string::npos ||
           t.find("DEPTH") != std::string::npos ||
           t.find("MASS") != std::string::npos ||
           t.find("AREA") != std::string::npos;
}

// Case-insensitive substring search
bool icontains(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    std::string h = haystack, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != std::string::npos;
}

// Does any extracted entity match this ground-truth type/value?
bool entity_matches_fact(const std::vector<ExtractedEntity>& entities,
                         const std::string& gt_type, const std::string& gt_value) {
    for (const auto& e : entities) {
        // Type congruence
        bool type_ok = false;
        if (gt_type == "date")        type_ok = is_date_entity(e.entity_type);
        if (gt_type == "measurement") type_ok = is_meas_entity(e.entity_type);
        if (gt_type == "count")       type_ok = (e.entity_type == "ARTIFACT_SPECIMEN_COUNT");
        if (!type_ok) continue;
        // Value proximity: true_value substring in raw_match or vice versa
        if (icontains(e.raw_match, gt_value) || icontains(gt_value, e.raw_match))
            return true;
        if (icontains(e.normalized_value, gt_value) || icontains(gt_value, e.normalized_value))
            return true;
    }
    return false;
}

// Find matched entity and return its anomaly_flag
bool entity_anomaly_flag(const std::vector<ExtractedEntity>& entities,
                         const std::string& gt_type, const std::string& gt_value) {
    for (const auto& e : entities) {
        bool type_ok = false;
        if (gt_type == "date")        type_ok = is_date_entity(e.entity_type);
        if (gt_type == "measurement") type_ok = is_meas_entity(e.entity_type);
        if (gt_type == "count")       type_ok = (e.entity_type == "ARTIFACT_SPECIMEN_COUNT");
        if (!type_ok) continue;
        if (icontains(e.raw_match, gt_value) || icontains(gt_value, e.raw_match))
            return e.anomaly_flag;
    }
    return false;
}

// Any anomaly-flagged entity on the whole page?
bool any_anomaly_on_page(const std::vector<ExtractedEntity>& entities) {
    for (const auto& e : entities) {
        if (e.anomaly_flag) return true;
    }
    return false;
}

struct PageResult {
    std::string page_id;
    int never_repair_violations = 0;
    int total_entities = 0;
};

struct EvalResult {
    std::string engine_name;
    // Never-Repair
    int nr_total = 0;
    int nr_violations = 0;
    // Extraction recall denominator: how many facts had text found in page?
    int clean_facts = 0;      // true_value substring found in OCR text
    int corrupted_facts = 0;  // true_value not found (OCR mangled it)
    // For clean facts
    int clean_extracted = 0;        // extracted at all
    int clean_no_false_alarm = 0;   // extracted AND anomaly_flag==false (TN_anom)
    int clean_false_alarm = 0;      // extracted AND anomaly_flag==true (FP_anom)
    // For corrupted facts
    int corrupted_extracted = 0;    // the mangled value was somehow extracted
    int corrupted_flagged = 0;      // TP_anom: page has anomaly_flag entity
    // By type
    int clean_date=0, clean_date_extracted=0;
    int clean_meas=0, clean_meas_extracted=0;
    int clean_count=0, clean_count_extracted=0;
    int corrupt_date=0, corrupt_date_flagged=0;
    int corrupt_meas=0, corrupt_meas_flagged=0;
    int corrupt_count=0, corrupt_count_flagged=0;
};

EvalResult run_engine_eval(
    const std::string& engine_name,
    const fs::path& ocr_dir,
    const json& ground_truth)
{
    EvalResult res;
    res.engine_name = engine_name;

    // Index ground truth by page_id
    std::map<std::string, std::vector<json>> gt_by_page;
    for (const auto& item : ground_truth) {
        gt_by_page[item["page_id"].get<std::string>()].push_back(item);
    }

    for (const auto& entry : fs::directory_iterator(ocr_dir)) {
        if (entry.path().extension() != ".txt") continue;
        std::string page_id = entry.path().stem().string();
        std::string text = read_file(entry.path());
        if (text.empty()) continue;

        auto entities = EntityExtractor::extract_entities(text);

        // Never-Repair check
        for (const auto& e : entities) {
            res.nr_total++;
            if (e.span_end > e.span_start && e.span_end <= text.size()) {
                std::string slice = text.substr(e.span_start, e.span_end - e.span_start);
                if (slice != e.raw_match) res.nr_violations++;
            }
        }

        // Ground-truth matching for this page
        if (gt_by_page.count(page_id) == 0) continue;
        for (const auto& gt : gt_by_page[page_id]) {
            std::string gt_type = gt["type"].get<std::string>();
            std::string gt_val  = gt["true_value"].get<std::string>();

            bool val_in_text = icontains(text, gt_val);
            bool extracted   = entity_matches_fact(entities, gt_type, gt_val);

            if (val_in_text) {
                // Clean fact: OCR produced the correct value
                res.clean_facts++;
                if (gt_type == "date")        res.clean_date++;
                if (gt_type == "measurement") res.clean_meas++;
                if (gt_type == "count")       res.clean_count++;

                if (extracted) {
                    res.clean_extracted++;
                    if (gt_type == "date")        res.clean_date_extracted++;
                    if (gt_type == "measurement") res.clean_meas_extracted++;
                    if (gt_type == "count")       res.clean_count_extracted++;
                    bool flagged = entity_anomaly_flag(entities, gt_type, gt_val);
                    if (!flagged) res.clean_no_false_alarm++;
                    else          res.clean_false_alarm++;
                }
            } else {
                // Corrupted fact: OCR mangled the true value
                res.corrupted_facts++;
                if (gt_type == "date")        res.corrupt_date++;
                if (gt_type == "measurement") res.corrupt_meas++;
                if (gt_type == "count")       res.corrupt_count++;

                // Proxy: did the page yield any anomaly-flagged entity?
                if (any_anomaly_on_page(entities)) {
                    res.corrupted_flagged++;
                    if (gt_type == "date")        res.corrupt_date_flagged++;
                    if (gt_type == "measurement") res.corrupt_meas_flagged++;
                    if (gt_type == "count")       res.corrupt_count_flagged++;
                }
            }
        }
    }
    return res;
}

std::pair<double,double> wilson95(int k, int n) {
    if (n == 0) return {0.0, 0.0};
    double z = 1.95996;
    double p = (double)k / n;
    double d = 1.0 + z*z/n;
    double c = (p + z*z/(2.0*n)) / d;
    double m = z * std::sqrt(p*(1.0-p)/n + z*z/(4.0*n*n)) / d;
    return {std::max(0.0, c-m)*100.0, std::min(1.0, c+m)*100.0};
}

void print_result(const EvalResult& r) {
    std::cout << "\n================================================================================\n";
    std::cout << "  Engine: " << r.engine_name << "\n";
    std::cout << "================================================================================\n";

    // Never-Repair
    std::cout << "\n[INVARIANT 1] Never-Repair (raw_match == text slice)\n";
    std::cout << "  Total entity mentions:  " << r.nr_total << "\n";
    std::cout << "  Violations:             " << r.nr_violations
              << (r.nr_violations == 0 ? "  [PASS]" : "  [FAIL]") << "\n";

    // Extraction recall on clean facts
    double recall = r.clean_facts > 0
        ? (double)r.clean_extracted / r.clean_facts * 100.0 : 0.0;
    auto r_ci = wilson95(r.clean_extracted, r.clean_facts);
    std::cout << "\n[EXTRACTION RECALL on clean facts  n=" << r.clean_facts << "]\n";
    std::cout << "  Extracted:     " << r.clean_extracted << "/" << r.clean_facts
              << std::fixed << std::setprecision(1)
              << "  (" << recall << "%)  Wilson 95% CI: ["
              << r_ci.first << "%, " << r_ci.second << "%]\n";
    std::cout << "    by type:  date "  << r.clean_date_extracted << "/" << r.clean_date
              << "  meas " << r.clean_meas_extracted << "/" << r.clean_meas
              << "  count " << r.clean_count_extracted << "/" << r.clean_count << "\n";

    // Plausibility Specificity (TN_anom / clean_extracted)
    int spec_denom = r.clean_extracted;
    double spec = spec_denom > 0
        ? (double)r.clean_no_false_alarm / spec_denom * 100.0 : 0.0;
    auto sp_ci = wilson95(r.clean_no_false_alarm, spec_denom);
    std::cout << "\n[PLAUSIBILITY SPECIFICITY  n=" << spec_denom << " extracted clean facts]\n";
    std::cout << "  No false alarm: " << r.clean_no_false_alarm << "/" << spec_denom
              << "  (" << spec << "%)  Wilson 95% CI: ["
              << sp_ci.first << "%, " << sp_ci.second << "%]\n";
    std::cout << "  False alarms:   " << r.clean_false_alarm << "\n";

    // Plausibility Sensitivity (proxy: page-level anomaly flag)
    double sens = r.corrupted_facts > 0
        ? (double)r.corrupted_flagged / r.corrupted_facts * 100.0 : 0.0;
    auto se_ci = wilson95(r.corrupted_flagged, r.corrupted_facts);
    std::cout << "\n[PLAUSIBILITY SENSITIVITY  n=" << r.corrupted_facts << " corrupted facts  (page-level proxy)]\n";
    std::cout << "  Page had anomaly flag: " << r.corrupted_flagged << "/" << r.corrupted_facts
              << "  (" << sens << "%)  Wilson 95% CI: ["
              << se_ci.first << "%, " << se_ci.second << "%]\n";
    std::cout << "    by type:  date "  << r.corrupt_date_flagged << "/" << r.corrupt_date
              << "  meas " << r.corrupt_meas_flagged << "/" << r.corrupt_meas
              << "  count " << r.corrupt_count_flagged << "/" << r.corrupt_count << "\n";
    std::cout << "  NOTE: sensitivity proxy is PAGE-LEVEL (not mention-level).\n";
    std::cout << "        A page gets credit if ANY entity on it has anomaly_flag=true.\n";
    std::cout << "        This overstates true mention-level sensitivity.\n";
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD — Phase 2 Step 3: Real-OCR Plausibility Evaluation\n";
    std::cout << "  Extractor commit: a6cf766  |  Protocol: 02_real_ocr_plausibility_protocol\n";
    std::cout << "  FIRST RUN on unmodified extractor. No hard gates applied.\n";
    std::cout << "================================================================================\n";

    // Load ground truth
    fs::path base = "tests/ocr_benchmark_50";
    std::ifstream gt_file(base / "ground_truth.json");
    if (!gt_file) { std::cerr << "Cannot open ground_truth.json\n"; return 1; }
    json gt;
    gt_file >> gt;
    std::cout << "\nGround truth: " << gt.size() << " facts loaded.\n";

    // Run both engines
    auto tes = run_engine_eval("Tesseract",
                               base / "results_tesseract", gt);
    auto win = run_engine_eval("Windows OCR",
                               base / "results_windows_ocr", gt);

    print_result(tes);
    print_result(win);

    std::cout << "\n================================================================================\n";
    std::cout << "  COMBINED INTERPRETATION\n";
    std::cout << "================================================================================\n";
    int total_clean = tes.clean_facts + win.clean_facts;
    int total_extracted = tes.clean_extracted + win.clean_extracted;
    int total_corrupt = tes.corrupted_facts + win.corrupted_facts;
    int total_flagged = tes.corrupted_flagged + win.corrupted_flagged;
    std::cout << "  Clean facts across both engines:     " << total_clean
              << "  extracted: " << total_extracted << "\n";
    std::cout << "  Corrupted facts across both engines: " << total_corrupt
              << "  page-level flagged: " << total_flagged << "\n";
    std::cout << "================================================================================\n";
    return 0;
}
