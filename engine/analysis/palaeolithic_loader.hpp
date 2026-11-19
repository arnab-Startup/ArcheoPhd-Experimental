#pragma once

#include "epistemic_logic.hpp"
#include "chronology.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cctype>

namespace archaeophd {

enum class DatingMethodType {
    Radiocarbon_AMS,
    Radiocarbon_Conv,
    Luminescence_TL,
    Luminescence_OSL,
    TrappedCharge_ESR,
    UraniumSeries_UTh,
    Other
};

struct PalaeolithicRecord {
    std::string site;
    std::string layer_id;
    std::string country;
    std::string period;
    std::string method_raw;
    DatingMethodType method_type = DatingMethodType::Radiocarbon_AMS;
    double age = 0.0;
    double std_dev = 0.0;
    std::string labnr;
    std::string material;
    std::string reliability;
    std::string bib;
    bool is_open_lower_bound = false; // Open lower bound: t >= age (e.g. > 50,000 BP or sigma=0)
    CalibratedDate calibrated;
    double taphonomic_weight = 1.0;
};

class PalaeolithicDatasetLoader {
public:
    static DatingMethodType parse_method(const std::string& m) {
        std::string s = m;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s.find("ams") != std::string::npos) return DatingMethodType::Radiocarbon_AMS;
        if (s.find("14c") != std::string::npos || s.find("conv") != std::string::npos) return DatingMethodType::Radiocarbon_Conv;
        if (s.find("tl") != std::string::npos) return DatingMethodType::Luminescence_TL;
        if (s.find("osl") != std::string::npos) return DatingMethodType::Luminescence_OSL;
        if (s.find("esr") != std::string::npos) return DatingMethodType::TrappedCharge_ESR;
        if (s.find("u/th") != std::string::npos || s.find("th/u") != std::string::npos) return DatingMethodType::UraniumSeries_UTh;
        return DatingMethodType::Other;
    }

    static double parse_taphonomic_weight(const std::string& mat, DatingMethodType method) {
        std::string m = mat;
        std::transform(m.begin(), m.end(), m.begin(), ::tolower);
        if (m.find("tooth") != std::string::npos || m.find("enamel") != std::string::npos) return 0.90;
        if (m.find("bone") != std::string::npos || m.find("seed") != std::string::npos) return 1.00;
        if (m.find("flint") != std::string::npos || m.find("silex") != std::string::npos) return 0.85;
        if (m.find("charcoal") != std::string::npos || m.find("wood") != std::string::npos) return 0.70;
        if (m.find("sediment") != std::string::npos || m.find("sand") != std::string::npos) return 0.75;
        if (method == DatingMethodType::Luminescence_TL) return 0.85;
        if (method == DatingMethodType::Luminescence_OSL) return 0.75;
        if (method == DatingMethodType::TrappedCharge_ESR) return 0.90;
        return 0.80;
    }

    static std::vector<PalaeolithicRecord> load_csv(const std::string& csv_path) {
        std::vector<PalaeolithicRecord> records;
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
            if (cols.size() < 10) continue;

            PalaeolithicRecord r;
            r.site = cols[0];
            r.layer_id = cols[1];
            r.country = cols[2];
            r.period = cols[3];
            r.method_raw = cols[4];
            r.method_type = parse_method(r.method_raw);

            std::string age_str = cols[5];
            std::string sd_str = cols[6];
            r.labnr = cols[7];
            r.material = cols[8];
            r.reliability = cols[9];
            if (cols.size() > 10) r.bib = cols[10];

            // Parse open bounds indicator (e.g. "> 45000", "+- 0", or age >= 50,000 BP)
            if (age_str.find('>') != std::string::npos) r.is_open_lower_bound = true;

            // Extract numeric values
            std::string clean_age;
            for (char c : age_str) if (std::isdigit(c) || c == '.') clean_age += c;
            std::string clean_sd;
            for (char c : sd_str) if (std::isdigit(c) || c == '.') clean_sd += c;

            if (clean_age.empty()) continue;
            try {
                r.age = std::stod(clean_age);
                r.std_dev = clean_sd.empty() ? 0.0 : std::stod(clean_sd);
            } catch (...) { continue; }

            if (r.std_dev <= 0.0 || r.age >= 50000.0) {
                r.is_open_lower_bound = true;
            }

            r.taphonomic_weight = parse_taphonomic_weight(r.material, r.method_type);

            // Chronological calibration handling
            bool is_c14 = (r.method_type == DatingMethodType::Radiocarbon_AMS ||
                           r.method_type == DatingMethodType::Radiocarbon_Conv);

            if (is_c14 && !r.is_open_lower_bound && r.age < 50000.0 && r.std_dev > 0.0) {
                // High-precision IntCal20 calibration for C-14 within valid range
                r.calibrated = NativeChronologyEngine::calibrate_c14(r.age, r.std_dev);
            } else if (is_c14 && r.is_open_lower_bound) {
                // Open lower bound beyond IntCal20 ceiling: [age, infinity)
                r.calibrated.cal_start = -(r.age + 1950.0);
                r.calibrated.cal_end = -150000.0; // Open lower bound towards deep time
                r.calibrated.cal_display_start = "> " + std::to_string(static_cast<int>(r.age)) + " BP";
                r.calibrated.cal_display_end = "Open Lower Bound";
            } else {
                // Direct calendar methods (TL, OSL, ESR, U/Th): Directly dates calendar years ago
                double cal_yr = -(r.age - 1950.0);
                double span = (r.std_dev > 0.0) ? (r.std_dev * 2.0) : 2000.0;
                r.calibrated.cal_start = cal_yr + span;
                r.calibrated.cal_end = cal_yr - span;
                r.calibrated.cal_display_start = std::to_string(static_cast<int>(std::abs(cal_yr + span))) + " BCE";
                r.calibrated.cal_display_end = std::to_string(static_cast<int>(std::abs(cal_yr - span))) + " BCE";
            }

            records.push_back(r);
        }
        return records;
    }
};

} // namespace archaeophd
