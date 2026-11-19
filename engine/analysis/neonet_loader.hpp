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
    std::string sector;
    double taphonomic_weight = 1.0;
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
            {"EM",   {"EM", "Early Mesolithic", -10000.0, -6500.0}},
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

    static std::string parse_sector(const std::string& phase) {
        std::string lower = phase;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        const std::vector<std::string> prefixes = {"bdd", "ldg", "zone ", "secteur ", "carré ", "corte "};
        for (const auto& pfx : prefixes) {
            size_t idx = lower.find(pfx);
            if (idx != std::string::npos) {
                size_t end = idx + pfx.size();
                while (end < lower.size() && (std::isalnum(lower[end]) || lower[end] == ' ' || lower[end] == '-' || lower[end] == '/')) {
                    end++;
                }
                return lower.substr(idx, end - idx);
            }
        }
        return "";
    }

    static int parse_layer_rank(const std::string& phase) {
        // Multi-digit integer stratigraphic depth rank parser
        // e.g. "c.11" -> 11, "c.4" -> 4, "Layer 3c" -> 3, "Nivel 8" -> 8
        // Smaller rank = physically higher (younger); larger rank = physically lower (older)
        std::string lower = phase;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        size_t pos = std::string::npos;
        size_t offset = 0;
        if ((pos = lower.find("couche ")) != std::string::npos) offset = 7;
        else if ((pos = lower.find("couche")) != std::string::npos) offset = 6;
        else if ((pos = lower.find("layer ")) != std::string::npos) offset = 6;
        else if ((pos = lower.find("layer")) != std::string::npos) offset = 5;
        else if ((pos = lower.find("nivel ")) != std::string::npos) offset = 6;
        else if ((pos = lower.find("nivel")) != std::string::npos) offset = 5;
        else if ((pos = lower.find("niv. ")) != std::string::npos) offset = 5;
        else if ((pos = lower.find("niv.")) != std::string::npos) offset = 4;
        else if ((pos = lower.find("c.")) != std::string::npos) offset = 2;
        else if ((pos = lower.find("c ")) != std::string::npos) offset = 2;

        if (pos != std::string::npos) {
            size_t num_start = pos + offset;
            while (num_start < lower.size() && (lower[num_start] == ' ' || lower[num_start] == '.')) num_start++;
            if (num_start < lower.size() && std::isdigit(lower[num_start])) {
                size_t num_end = num_start;
                while (num_end < lower.size() && std::isdigit(lower[num_end])) num_end++;
                try {
                    int val = std::stoi(lower.substr(num_start, num_end - num_start));
                    if (val < 100) return val; // Filter out arbitrary context/unit numbers > 100
                } catch (...) {}
            }
        }

        // Direct small number at start of phase code (e.g. "5", "8", "17", "7A")
        if (!lower.empty() && std::isdigit(lower[0])) {
            size_t num_end = 0;
            while (num_end < lower.size() && std::isdigit(lower[num_end])) num_end++;
            try {
                int val = std::stoi(lower.substr(0, num_end));
                if (val < 100) return val;
            } catch (...) {}
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
            // Clean quotes or whitespace
            while (!r.period.empty() && (r.period.front() == ' ' || r.period.front() == '\"')) r.period.erase(r.period.begin());
            while (!r.period.empty() && (r.period.back() == ' ' || r.period.back() == '\"')) r.period.pop_back();

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

            // Taphonomic material weighting
            std::string mat_lower = r.material;
            std::transform(mat_lower.begin(), mat_lower.end(), mat_lower.begin(), ::tolower);
            if (mat_lower.find("seed") != std::string::npos || mat_lower.find("grain") != std::string::npos ||
                mat_lower.find("bone") != std::string::npos) {
                r.taphonomic_weight = 1.0;
            } else if (mat_lower.find("wood") != std::string::npos || mat_lower.find("charcoal") != std::string::npos) {
                r.taphonomic_weight = 0.70;
            } else {
                r.taphonomic_weight = 0.85;
            }

            // Normalization: In French/Spanish datasets, Early/Middle Neolithic (EMN) dates
            // (BP < 6500 or tpq > -6000) labeled as "EM" belong to EMN, not Early Mesolithic.
            if (r.period == "EM" && (r.bp < 6500.0 || (r.tpq != 0.0 && r.tpq > -6000.0))) {
                r.period = "EMN";
            }

            r.calibrated = NativeChronologyEngine::calibrate_c14(r.bp, r.std_dev);
            r.layer_depth_rank = parse_layer_rank(r.phase_code);
            r.sector = parse_sector(r.phase_code);

            records.push_back(r);
        }
        return records;
    }
};

} // namespace archaeophd
