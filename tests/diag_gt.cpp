#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

int main() {
    std::ifstream f("tests/ocr_benchmark_50/ground_truth.json");
    json gt;
    f >> gt;
    std::cout << "Total facts: " << gt.size() << "\n";

    std::map<std::string, int> by_class;
    std::map<std::string, int> by_type;
    std::map<std::string, std::map<std::string, int>> class_by_type;
    int bare_years = 0;
    int bce_dates = 0;
    int ce_dates = 0;

    for (const auto& item : gt) {
        std::string page = item["page_id"].get<std::string>();
        std::string type = item["type"].get<std::string>();
        std::string val = item["true_value"].get<std::string>();
        
        std::string doc_class = (page.find("sankalia") != std::string::npos) ? "Class_B (Sankalia)" : "Class_A (Rajan/Chakrabarti)";
        by_class[doc_class]++;
        by_type[type]++;
        class_by_type[doc_class][type]++;

        if (type == "date") {
            if (val.find("BC") != std::string::npos || val.find("B.C.") != std::string::npos || val.find("BCE") != std::string::npos) {
                bce_dates++;
            } else if (val.find("AD") != std::string::npos || val.find("A.D.") != std::string::npos || val.find("CE") != std::string::npos) {
                ce_dates++;
            } else {
                bare_years++;
            }
        }
    }

    std::cout << "\nBy Document Class:\n";
    for (const auto& [c, n] : by_class) {
        std::cout << "  " << c << ": " << n << "\n";
    }

    std::cout << "\nBy Fact Type:\n";
    for (const auto& [t, n] : by_type) {
        std::cout << "  " << t << ": " << n << "\n";
    }

    std::cout << "\nBy Document Class and Type:\n";
    for (const auto& [c, tm] : class_by_type) {
        std::cout << "  " << c << ":\n";
        for (const auto& [t, n] : tm) {
            std::cout << "    " << t << ": " << n << "\n";
        }
    }

    std::cout << "\nDate Breakdown:\n";
    std::cout << "  BCE dates (with BC/BCE): " << bce_dates << "\n";
    std::cout << "  CE dates (with AD/CE):  " << ce_dates << "\n";
    std::cout << "  Bare years (e.g. 1764, 1861, 1944): " << bare_years << "\n";

    return 0;
}
