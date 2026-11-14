#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <algorithm>
#include "json.hpp"
#include "extraction/entity_extractor.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace archaeophd;

std::string read_file(const fs::path& p) {
    std::ifstream f(p);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

bool icontains(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    std::string h = haystack, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != std::string::npos;
}

bool is_in_scope(const std::string& type, const std::string& val) {
    if (type == "measurement" || type == "count") return true;
    if (type == "date") {
        if (val.find("BC") != std::string::npos || val.find("B.C.") != std::string::npos || 
            val.find("BCE") != std::string::npos || val.find("AD") != std::string::npos || 
            val.find("A.D.") != std::string::npos || val.find("CE") != std::string::npos ||
            val.find("BP") != std::string::npos) return true;
        return false; // Bare year
    }
    return false;
}

bool is_date_entity(const std::string& t) {
    return t.find("DATE") != std::string::npos || t.find("C14_BP") != std::string::npos || t.find("CALIBRATED") != std::string::npos || t.find("APPROX") != std::string::npos;
}
bool is_meas_entity(const std::string& t) {
    return t.find("DIMENSION") != std::string::npos || t.find("RANGE") != std::string::npos || t.find("DEPTH") != std::string::npos || t.find("MASS") != std::string::npos || t.find("AREA") != std::string::npos;
}

bool entity_matches(const std::vector<ExtractedEntity>& entities, const std::string& type, const std::string& val, bool& out_anom) {
    for (const auto& e : entities) {
        bool type_ok = false;
        if (type == "date") type_ok = is_date_entity(e.entity_type);
        if (type == "measurement") type_ok = is_meas_entity(e.entity_type);
        if (type == "count") type_ok = (e.entity_type == "ARTIFACT_SPECIMEN_COUNT");
        if (!type_ok) continue;
        if (icontains(e.raw_match, val) || icontains(val, e.raw_match) || icontains(e.normalized_value, val) || icontains(val, e.normalized_value)) {
            out_anom = e.anomaly_flag;
            return true;
        }
    }
    return false;
}

void eval_engine(const std::string& name, const fs::path& dir, const json& gt) {
    std::cout << "\n==================================================\n";
    std::cout << "Engine: " << name << "\n";
    std::cout << "==================================================\n";

    std::map<std::string, std::vector<json>> gt_by_page;
    for (const auto& item : gt) gt_by_page[item["page_id"].get<std::string>()].push_back(item);

    int in_scope_clean = 0, in_scope_clean_ext = 0;
    int in_scope_class_a_clean = 0, in_scope_class_a_clean_ext = 0;
    int in_scope_class_b_clean = 0, in_scope_class_b_clean_ext = 0;

    int corrupt_in_scope = 0, corrupt_in_scope_produced_mention = 0, corrupt_in_scope_flagged = 0;
    int corrupt_all = 0, corrupt_all_produced_mention = 0, corrupt_all_flagged = 0;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() != ".txt") continue;
        std::string page_id = entry.path().stem().string();
        std::string text = read_file(entry.path());
        if (text.empty() || gt_by_page.count(page_id) == 0) continue;

        bool is_class_a = (page_id.find("sankalia") == std::string::npos);
        auto entities = EntityExtractor::extract_entities(text);

        for (const auto& item : gt_by_page[page_id]) {
            std::string type = item["type"].get<std::string>();
            std::string val = item["true_value"].get<std::string>();
            bool in_scope = is_in_scope(type, val);

            bool clean = icontains(text, val);
            bool anom = false;
            bool ext = entity_matches(entities, type, val, anom);

            if (clean) {
                if (in_scope) {
                    in_scope_clean++;
                    if (ext) in_scope_clean_ext++;
                    if (is_class_a) {
                        in_scope_class_a_clean++;
                        if (ext) in_scope_class_a_clean_ext++;
                    } else {
                        in_scope_class_b_clean++;
                        if (ext) in_scope_class_b_clean_ext++;
                    }
                }
            } else {
                // Corrupted
                corrupt_all++;
                // Did this corrupted fact produce any entity mention overlapping this context?
                // Or did any entity get extracted on this page?
                bool produced_mention = false;
                for (const auto& e : entities) {
                    if ((type == "date" && is_date_entity(e.entity_type)) ||
                        (type == "measurement" && is_meas_entity(e.entity_type)) ||
                        (type == "count" && e.entity_type == "ARTIFACT_SPECIMEN_COUNT")) {
                        // check if entity is in this page
                        produced_mention = true;
                        break;
                    }
                }
                if (ext) { // mention matched the fact value/slice
                    corrupt_all_produced_mention++;
                    if (anom) corrupt_all_flagged++;
                }
                if (in_scope) {
                    corrupt_in_scope++;
                }
            }
        }
    }

    std::cout << "IN-SCOPE Clean Facts Recall: " << in_scope_clean_ext << "/" << in_scope_clean 
              << " (" << (in_scope_clean > 0 ? in_scope_clean_ext * 100.0 / in_scope_clean : 0) << "%)\n";
    std::cout << "  Class A (Clean Offset):   " << in_scope_class_a_clean_ext << "/" << in_scope_class_a_clean
              << " (" << (in_scope_class_a_clean > 0 ? in_scope_class_a_clean_ext * 100.0 / in_scope_class_a_clean : 0) << "%)\n";
    std::cout << "  Class B (Letterpress):    " << in_scope_class_b_clean_ext << "/" << in_scope_class_b_clean
              << " (" << (in_scope_class_b_clean > 0 ? in_scope_class_b_clean_ext * 100.0 / in_scope_class_b_clean : 0) << "%)\n";
    std::cout << "Corrupted Facts Total: " << corrupt_all << "\n";
    std::cout << "Corrupted Facts that produced matching mention: " << corrupt_all_produced_mention << "\n";
}

int main() {
    std::ifstream f("tests/ocr_benchmark_50/ground_truth.json");
    json gt; f >> gt;
    eval_engine("Tesseract", "tests/ocr_benchmark_50/results_tesseract", gt);
    eval_engine("Windows OCR", "tests/ocr_benchmark_50/results_windows_ocr", gt);
    return 0;
}
