#pragma once

#include "epistemic_logic.hpp"
#include "chronology.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>

namespace archaeophd {

struct XronosRecord {
    std::string labnr;
    std::string site;
    std::string feature;
    std::string stratum;
    double bp = 0.0;
    double std_dev = 40.0;
    std::string material;
    std::string species;
    std::string country;
    std::string period;
    CalibratedDate calibrated;
    double taphonomic_weight = 1.0;
};

class XronosDatasetLoader {
public:
    static std::vector<XronosRecord> load_csv(const std::string& csv_path, size_t max_rows = 25000) {
        std::vector<XronosRecord> records;
        std::ifstream file(csv_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open XRONOS CSV: " << csv_path << std::endl;
            return records;
        }

        std::string header_line;
        if (!std::getline(file, header_line)) return records;

        std::string line;
        while (std::getline(file, line) && records.size() < max_rows) {
            if (line.empty()) continue;

            // Handle potential quotes in CSV line
            std::vector<std::string> cols;
            std::string cur;
            bool in_quotes = false;
            for (size_t i = 0; i < line.size(); ++i) {
                char ch = line[i];
                if (ch == '\"') {
                    in_quotes = !in_quotes;
                } else if (ch == ',' && !in_quotes) {
                    cols.push_back(cur);
                    cur.clear();
                } else {
                    cur += ch;
                }
            }
            cols.push_back(cur);

            if (cols.size() < 12) continue;

            XronosRecord r;
            r.labnr = cols[0];
            r.site = cols[1];
            r.feature = cols[2];
            r.stratum = cols[3];
            try {
                r.bp = std::stod(cols[4]);
                r.std_dev = cols[5].empty() ? 40.0 : std::stod(cols[5]);
            } catch (...) {
                continue;
            }
            r.material = cols[6];
            r.species = cols[7];
            r.country = cols[8];
            r.period = cols[11];

            // Taphonomic / Old-Wood material weighting:
            // Short-lived samples (seed, fruit, cereal) have minimal in-built age (weight 1.0)
            // Long-lived wood/charcoal has potential old-wood offset (weight 0.70)
            std::string mat_lower = r.material;
            std::transform(mat_lower.begin(), mat_lower.end(), mat_lower.begin(), ::tolower);
            if (mat_lower.find("seed") != std::string::npos || mat_lower.find("fruit") != std::string::npos ||
                mat_lower.find("grain") != std::string::npos || mat_lower.find("bone") != std::string::npos) {
                r.taphonomic_weight = 1.0;
            } else if (mat_lower.find("wood") != std::string::npos || mat_lower.find("charcoal") != std::string::npos) {
                r.taphonomic_weight = 0.70;
            } else {
                r.taphonomic_weight = 0.85;
            }

            // Calibrate using high-precision IntCal20 curve
            r.calibrated = NativeChronologyEngine::calibrate_c14(r.bp, r.std_dev);

            records.push_back(r);
        }

        return records;
    }
};

} // namespace archaeophd
