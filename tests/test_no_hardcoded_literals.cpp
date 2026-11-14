#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include "extraction/entity_extractor.hpp"

// Unit test: Assert that no synthetic hardcoded corruption literals exist in entity_extractor.hpp
int main() {
    std::string header_path = "engine/extraction/entity_extractor.hpp";
    std::ifstream file(header_path);
    if (!file.is_open()) {
        // Try relative to tests dir if executed from build/
        header_path = "../engine/extraction/entity_extractor.hpp";
        file.open(header_path);
    }

    if (!file.is_open()) {
        std::cerr << "FAIL: Could not open " << header_path << " for literal audit.\n";
        return 1;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    const std::vector<std::string> forbidden_literals = {
        "2040",
        "691",
        "1063",
        "1846",
        "1M7"
    };

    bool violation_found = false;
    for (const auto& lit : forbidden_literals) {
        size_t pos = content.find(lit);
        if (pos != std::string::npos) {
            std::cerr << "FAIL: Forbidden hardcoded literal \"" << lit << "\" found in " 
                      << header_path << " at character offset " << pos << "!\n";
            violation_found = true;
        }
    }

    if (violation_found) {
        std::cerr << "Audit FAILED: Synthetic test literals must never be hardcoded into the extractor.\n";
        return 1;
    }

    std::cout << "[PASS] Verified: Zero hardcoded test literals (2040, 691, 1063, 1846, 1M7) found in entity_extractor.hpp\n";
    return 0;
}
