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
#include <algorithm>

namespace archaeophd {

struct NeoNetRecord {
    std::string site;
    std::string country;
    std::string period;
    std::string phase_code;
    std::string labnr;
    double bp = 0.0;
    double std_dev = 40.0;
    std::string material;
    std::string species;
    double tpq = 0.0;
    double taq = 0.0;
    std::string bib;
    CalibratedDate calibrated;
    int layer_depth_rank = -1; // Lower rank = physically higher (younger); higher rank = lower (older)
};

struct TypoChronologicalRange {
    std::string code;
    std::string name;
    double start_bce = 0.0; // Astronomical year (more negative = older)
    double end_bce = 0.0;
};

class NeoNetDatasetLoader {
public:
    static std::map<std::string, TypoChronologicalRange> canonical_periods() {
        return {
            {"EM",   {"EM", "Early Mesolithic", -10000.0, -7500.0}},
            {"MM",   {"MM", "Middle Mesolithic", -7800.0, -6500.0}},
            {"LM",   {"LM", "Late Mesolithic", -6700.0, -5400.0}},
            {"LMEN", {"LMEN", "Late Mesolithic / Early Neolithic", -6200.0, -5200.0}},
            {"UM",   {"UM", "Undefined Mesolithic", -10000.0, -5400.0}},
            {"EN",   {"EN", "Early Neolithic (Cardial/Impressed)", -5800.0, -4700.0}},
            {"EMN",  {"EMN", "Early/Middle Neolithic", -5200.0, -4200.0}},
            {"MN",   {"MN", "Middle Neolithic", -4800.0, -3700.0}},
            {"LN",   {"LN", "Late Neolithic", -3900.0, -2500.0}},
            {"UN",   {"UN", "Undefined Neolithic", -5800.0, -2500.0}}
        };
    }

    static int parse_layer_rank(const std::string& phase) {
        // Extracts stratigraphic depth rank from phase codes like:
        // "c.1", "c.2", "c.5", "c.6", "Layer 2", "Layer 3c", "Layer 4", "7A", "7C", "8A"
        // Lower numeric value = higher in sequence (e.g. Layer 1 is above Layer 2)
        std::string s = phase;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);

        // Pattern: c.X or layer X
        size_t pos = s.find("c.");
        if (pos == std::string::npos) pos = s.find("layer ");
        if (pos == std::string::npos) pos = s.find("c ");
        if (pos == std::string::npos) pos = s.find("nivel ");

        if (pos != std::string::npos) {
            size_t num_start = pos + 2;
            if (s.find("layer ") == pos) num_start = pos + 6;
            if (s.find("nivel ") == pos) num_start = pos + 6;
            while (num_start < s.size() && (s[num_start] == ' ' || s[num_start] == '.')) num_start++;
            if (num_start < s.size() && std::isdigit(s[num_start])) {
                return s[num_start] - '0';
            }
        }

        // Generic single digit extraction if phase starts with number (e.g. "6f-6a2", "7A", "8A")
        if (!s.empty() && std::isdigit(s[0])) {
            return s[0] - '0';
        }

        return -1;
    }

    static std::vector<NeoNetRecord> load_csv(const std::string& csv_path) {
        std::vector<NeoNetRecord> records;
        std::ifstream file(csv_path);
        if (!file.is_open()) return records;

        std::string header;
        if (!std::getline(file, header)) return records;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::vector<std::string> cols;
            std::string cur;
            bool in_quotes = false;
            for (char ch : line) {
                if (ch == '\"') in_quotes = !in_quotes;
                else if (ch == ',' && !in_quotes) { cols.push_back(cur); cur.clear(); }
                else cur += ch;
            }
            cols.push_back(cur);
            if (cols.size() < 12) continue;

            NeoNetRecord r;
            r.site = cols[0];
            r.country = cols[1];
            r.period = cols[2];
            r.phase_code = cols[3];
            r.labnr = cols[4];
            try {
                r.bp = std::stod(cols[5]);
                r.std_dev = cols[6].empty() ? 40.0 : std::stod(cols[6]);
            } catch (...) { continue; }
            r.material = cols[7];
            r.species = cols[8];
            try {
                r.tpq = cols[9].empty() ? 0.0 : std::stod(cols[9]);
                r.taq = cols[10].empty() ? 0.0 : std::stod(cols[10]);
            } catch (...) {}
            r.bib = cols[11];

            r.calibrated = NativeChronologyEngine::calibrate_c14(r.bp, r.std_dev);
            r.layer_depth_rank = parse_layer_rank(r.phase_code);

            records.push_back(r);
        }
        return records;
    }
};

} // namespace archaeophd
