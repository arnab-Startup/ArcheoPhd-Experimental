#pragma once

#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace archaeophd {

// ============================================================================
// Phase 2 Step 3 — Deterministic Quantitative & Physical Entity Extractor
// 
// Epistemic Invariants:
// 1. Deterministic regular grammar and normalizer (zero LLM dependency).
// 2. OCR anomalies extracted as-is and flagged, NEVER repaired, split, or rewritten.
// 3. No in-house calibration: uncalibrated BP flagged UNCALIBRATED_RADIOCARBON_BP.
// 4. Astronomical normalization: 1 BCE = 0, N BCE = -(N - 1), N CE = +N.
// 5. Negative controls (citations, references, figures) strictly rejected.
// ============================================================================

struct ExtractedEntity {
    std::string raw_match;
    std::string entity_type; // LINEAR_DIMENSION, LINEAR_RANGE, COMPOUND_DIMENSION, DEPTH, MASS,
                             // EXACT_DATE, APPROX_DATE, DATE_RANGE, C14_BP, CALIBRATED_DATE,
                             // TEMPERATURE, LOCUS, AREA, STRATUM, TRENCH, BASKET
    std::string normalized_value;
    std::string unit;
    double numeric_val = 0.0;
    double range_min = 0.0;
    double range_max = 0.0;
    int astro_year_start = 0;
    int astro_year_end = 0;
    bool is_range = false;
    bool is_approximate = false;
    bool is_uncalibrated_bp = false;
    bool is_author_calibrated = false;
    bool anomaly_flag = false;
    std::string anomaly_reason;
    size_t span_start = 0;
    size_t span_end = 0;
};

class EntityExtractor {
public:
    static inline int bce_to_astro(int bce_year) {
        if (bce_year <= 0) return bce_year;
        return -(bce_year - 1);
    }

    static inline int ce_to_astro(int ce_year) {
        return ce_year;
    }

    static inline std::string trim(const std::string& s) {
        auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c){ return std::isspace(c); });
        auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c){ return std::isspace(c); }).base();
        return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
    }

    static inline double parse_double(const std::string& s) {
        try {
            return std::stod(s);
        } catch (...) {
            return 0.0;
        }
    }

    static inline int parse_int(const std::string& s) {
        try {
            return std::stoi(s);
        } catch (...) {
            return 0;
        }
    }

    // Main extraction pipeline
    static std::vector<ExtractedEntity> extract_entities(const std::string& text) {
        std::vector<ExtractedEntity> results;

        // 1. Negative Filter Pre-Scan & Exclusion Regions
        // Build exclusion spans for bibliographic citations, catalog numbers, etc.
        std::vector<std::pair<size_t, size_t>> exclusion_spans;

        // Citation pattern: e.g. "(1981: 142)" or "Author (1981: 142)"
        std::regex re_citation(R"(\([A-Za-z\s]*,?\s*(\d{4})\s*:\s*(\d+)\))");
        for (std::sregex_iterator it(text.begin(), text.end(), re_citation), end; it != end; ++it) {
            exclusion_spans.push_back({it->position(), it->position() + it->length()});
        }

        // Reference / Bibliography pointer: e.g. "reference 128", "bibliography reference 128"
        std::regex re_bib_ref(R"((?:reference|ref|bibliography)\s+\d+)", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_bib_ref), end; it != end; ++it) {
            exclusion_spans.push_back({it->position(), it->position() + it->length()});
        }

        // Figure / Plate / Table / Page / Volume references
        std::regex re_pub_ptr(R"((?:figure|fig\.|plate|table|page|p\.|pp\.|volume|vol\.)\s+\d+)", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_pub_ptr), end; it != end; ++it) {
            exclusion_spans.push_back({it->position(), it->position() + it->length()});
        }

        // ISBN / Catalog numbers: e.g. "ISBN 0-19-813190-8"
        std::regex re_isbn(R"(ISBN\s+[\d\-]+)", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_isbn), end; it != end; ++it) {
            exclusion_spans.push_back({it->position(), it->position() + it->length()});
        }

        // Headcount exclusion: e.g. "45 workers"
        std::regex re_headcount(R"(\d+\s+(?:workers|laborers|people|men|women))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_headcount), end; it != end; ++it) {
            exclusion_spans.push_back({it->position(), it->position() + it->length()});
        }

        // Dimensionless ratio exclusion: e.g. "3 to 1" without units
        std::regex re_ratio(R"(\b\d+\s+to\s+\d+\b(?!\s*(?:cal\s+)?(?:m|cm|mm|metres|meters|km|g|kg|BCE|CE|BC|AD)))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_ratio), end; it != end; ++it) {
            exclusion_spans.push_back({it->position(), it->position() + it->length()});
        }

        // Lambda to check if a span overlaps any exclusion span
        auto is_excluded = [&](size_t start, size_t end) -> bool {
            for (const auto& span : exclusion_spans) {
                if (start < span.second && end > span.first) {
                    return true;
                }
            }
            return false;
        };

        // Lambda to check if an entity span overlaps already extracted entities
        auto is_already_extracted = [&](size_t start, size_t end) -> bool {
            for (const auto& r : results) {
                if (start < r.span_end && end > r.span_start) {
                    return true;
                }
            }
            return false;
        };

        // --------------------------------------------------------------------
        // 3. Compound Dimensions: 7 x 14 x 28 cm or 45 by 60 cm
        // --------------------------------------------------------------------
        std::regex re_compound_3d(R"((\d+(?:\.\d+)?)\s*(?:x|×)\s*(\d+(?:\.\d+)?)\s*(?:x|×)\s*(\d+(?:\.\d+)?)\s*(mm|cm|m))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_compound_3d), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "COMPOUND_DIMENSION";
            std::string u = it->str(4);
            e.unit = u;
            e.normalized_value = it->str(1) + "x" + it->str(2) + "x" + it->str(3) + " " + u;
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        std::regex re_compound_2d(R"((\d+(?:\.\d+)?)\s*(?:by|x|×)\s*(\d+(?:\.\d+)?)\s*(mm|cm|m))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_compound_2d), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "COMPOUND_DIMENSION";
            std::string u = it->str(3);
            e.unit = u;
            e.normalized_value = it->str(1) + "x" + it->str(2) + " " + u;
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // Brick/aggregate proportion ratio: 1:2:4 (colon separator, no physical unit)
        std::regex re_compound_ratio(R"(\b(\d+)\s*:\s*(\d+)\s*:\s*(\d+)\b)");
        for (std::sregex_iterator it(text.begin(), text.end(), re_compound_ratio), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "COMPOUND_DIMENSION";
            e.unit = "compound_ratio";
            e.normalized_value = it->str(1) + ":" + it->str(2) + ":" + it->str(3);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 4. Linear Ranges: 20-40 cm, between 4.5 and 5.0 metres
        // --------------------------------------------------------------------
        std::regex re_range_hyphen(R"((\d+(?:\.\d+)?)\s*(?:-|–)\s*(\d+(?:\.\d+)?)\s*(mm|cm|m|metres|meters))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_range_hyphen), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "LINEAR_RANGE";
            e.is_range = true;
            double v1 = parse_double(it->str(1));
            double v2 = parse_double(it->str(2));
            std::string u = it->str(3);
            e.unit = u;

            // Convert to meters in normalization
            double factor = 1.0;
            if (u == "cm") factor = 0.01;
            else if (u == "mm") factor = 0.001;

            e.range_min = v1 * factor;
            e.range_max = v2 * factor;

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << "[" << e.range_min << ", " << e.range_max << "] m";
            e.normalized_value = oss.str();
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        std::regex re_range_between(R"(between\s+(\d+(?:\.\d+)?)\s+and\s+(\d+(?:\.\d+)?)\s+(mm|cm|m|metres|meters))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_range_between), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "LINEAR_RANGE";
            e.is_range = true;
            double v1 = parse_double(it->str(1));
            double v2 = parse_double(it->str(2));
            std::string u = it->str(3);
            e.unit = u;

            double factor = 1.0;
            if (u == "cm") factor = 0.01;
            else if (u == "mm") factor = 0.001;

            e.range_min = v1 * factor;
            e.range_max = v2 * factor;

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << "[" << e.range_min << ", " << e.range_max << "] m";
            e.normalized_value = oss.str();
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // from X to Y metres: "from 10 to 15 metres" (lexical range form)
        std::regex re_range_from_to(R"(from\s+(\d+(?:\.\d+)?)\s+to\s+(\d+(?:\.\d+)?)\s+(mm|cm|m|metres|meters))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_range_from_to), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "LINEAR_RANGE";
            e.is_range = true;
            double v1 = parse_double(it->str(1));
            double v2 = parse_double(it->str(2));
            std::string u = it->str(3);
            e.unit = u;
            double factor = 1.0;
            if (u == "cm") factor = 0.01;
            else if (u == "mm") factor = 0.001;
            e.range_min = v1 * factor;
            e.range_max = v2 * factor;
            std::ostringstream oss2;
            oss2 << std::fixed << std::setprecision(2) << "[" << e.range_min << ", " << e.range_max << "] m";
            e.normalized_value = oss2.str();
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 5. Vertical Depth Coordinates: depth 40 m
        // --------------------------------------------------------------------
        std::regex re_depth(R"(depth\s+(\d+(?:\.\d+)?)\s*(mm|cm|m|metres|meters))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_depth), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "DEPTH_ELEVATION";
            double v = parse_double(it->str(1));
            std::string u = it->str(2);
            e.unit = u;
            e.numeric_val = v;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << v << " m";
            e.normalized_value = oss.str();
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 6. Single Linear Dimensions: 2.5 cm, 1.2 m
        // --------------------------------------------------------------------
        std::regex re_linear(R"(\b(\d+(?:\.\d+)?)\s*(mm|cm|m)\b(?!\s*[:x×\d]))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_linear), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "LINEAR_DIMENSION";
            double v = parse_double(it->str(1));
            std::string u = it->str(2);
            e.unit = u;
            e.numeric_val = v;

            double in_meters = v;
            if (u == "cm") in_meters = v * 0.01;
            else if (u == "mm") in_meters = v * 0.001;

            std::ostringstream oss;
            if (u == "cm") {
                oss << std::fixed << std::setprecision(3) << in_meters << " m";
            } else {
                oss << std::fixed << std::setprecision(3) << in_meters << " m";
            }
            e.normalized_value = oss.str();
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 7. Masses / Weights: 13.65 g, 27.3 g
        // --------------------------------------------------------------------
        std::regex re_mass(R"(\b(\d+(?:\.\d+)?)\s*(g|kg|mg|grams)\b)", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_mass), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "MASS_WEIGHT";
            double v = parse_double(it->str(1));
            std::string u = it->str(2);
            e.unit = u;
            e.numeric_val = v;
            std::ostringstream oss;
            if (v == std::floor(v * 10) / 10) {
                oss << std::fixed << std::setprecision(2) << v << " g";
            } else {
                oss << std::fixed << std::setprecision(2) << v << " g";
            }
            e.normalized_value = oss.str();
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 8. Temperatures: 500 °C
        // --------------------------------------------------------------------
        std::regex re_temp(R"(\b(\d+(?:\.\d+)?)\s*(?:°C|deg\s*C|degrees\s*Celsius)\b)", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_temp), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "TEMPERATURE";
            e.unit = "°C";
            e.numeric_val = parse_double(it->str(1));
            e.normalized_value = it->str(1) + " °C";
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 9. Radiocarbon C-14 BP (Uncalibrated): 3310 ± 45 BP, 1550 +/- 50 rcybp
        // --------------------------------------------------------------------
        std::regex re_c14_bp(R"((\d+)\s*(?:±|\+/-)\s*(\d+)\s*(BP|rcybp))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_c14_bp), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "UNCALIBRATED_C14_BP";
            e.unit = it->str(3);
            e.is_uncalibrated_bp = true;
            e.anomaly_flag = true;
            e.anomaly_reason = "UNCALIBRATED_RADIOCARBON_BP: Raw isotopic age determination requiring author/stratigraphic calibration prior";
            e.normalized_value = it->str(1) + "±" + it->str(2) + " " + e.unit;
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 10. Author Calibrated Dates: 1620-1530 cal BC, 1430 to 1390 cal BCE
        // --------------------------------------------------------------------
        // Handles ASCII hyphen (-), Unicode en-dash (–), or 'to' as separator.
        // Handles BC, BCE, B.C., B.C.E. era markers.
        std::regex re_cal_bce(R"((\d+)\s*(?:-|–|to)\s*(\d+)\s*cal\s*(BCE|BC|B\.C\.E\.|B\.C\.))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_cal_bce), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "AUTHOR_CALIBRATED_DATE";
            e.is_author_calibrated = true;
            e.is_range = true;
            int y1 = parse_int(it->str(1));
            int y2 = parse_int(it->str(2));
            e.unit = "cal " + it->str(3);
            e.astro_year_start = bce_to_astro(y1);
            e.astro_year_end = bce_to_astro(y2);
            e.normalized_value = "[" + std::to_string(e.astro_year_start) + ", " + std::to_string(e.astro_year_end) + "]";
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // Author calibrated cal BP: 3200 cal BP
        std::regex re_cal_bp(R"((\d+)\s*cal\s+BP)", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_cal_bp), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "AUTHOR_CALIBRATED_DATE";
            e.is_author_calibrated = true;
            e.unit = "cal BP";
            e.normalized_value = it->str(1) + " cal BP";
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 11. Approximate Historical Dates: c. 1550 BC, circa 1400 BCE
        // --------------------------------------------------------------------
        // Approximation prefixes: c., ca., circa, approx., about, approximately
        // Era markers: BCE, BC, B.C.E., B.C.
        std::regex re_approx_bce(R"((c\.|ca\.|circa|approx\.?|about|approximately)\s*(\d+)\s*(BCE|BC|B\.C\.E\.|B\.C\.))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_approx_bce), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "APPROX_DATE_BCE";
            e.is_approximate = true;
            int y = parse_int(it->str(2));
            e.unit = it->str(3);
            e.astro_year_start = bce_to_astro(y);
            e.normalized_value = std::to_string(e.astro_year_start);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // Approximate CE dates: c. 79 CE, about 400 AD, circa 300 A.D.
        std::regex re_approx_ce(R"((c\.|ca\.|circa|approx\.?|about|approximately)\s*(\d+)\s*(CE|AD|A\.D\.|C\.E\.))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_approx_ce), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "APPROX_DATE_CE";
            e.is_approximate = true;
            int y = parse_int(it->str(2));
            e.unit = it->str(3);
            e.astro_year_start = ce_to_astro(y);
            e.normalized_value = "+" + std::to_string(e.astro_year_start);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 12. Chronological Date Ranges: 1000-925 BCE, from 320 to 390 CE
        // --------------------------------------------------------------------
        // ASCII hyphen or Unicode en-dash; BC, BCE, B.C., B.C.E.
        std::regex re_date_range_bce(R"((\d+)\s*(?:-|–)\s*(\d+)\s*(BCE|BC|B\.C\.E\.|B\.C\.))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_date_range_bce), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "DATE_RANGE";
            e.is_range = true;
            int y1 = parse_int(it->str(1));
            int y2 = parse_int(it->str(2));
            e.unit = it->str(3);
            e.astro_year_start = bce_to_astro(y1);
            e.astro_year_end = bce_to_astro(y2);
            e.normalized_value = "[" + std::to_string(e.astro_year_start) + ", " + std::to_string(e.astro_year_end) + "]";
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        std::regex re_date_range_ce(R"(from\s+(\d+)\s+to\s+(\d+)\s*(CE|AD|A\.D\.|C\.E\.))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_date_range_ce), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "DATE_RANGE_CE";
            e.is_range = true;
            int y1 = parse_int(it->str(1));
            int y2 = parse_int(it->str(2));
            e.unit = it->str(3);
            e.astro_year_start = ce_to_astro(y1);
            e.astro_year_end = ce_to_astro(y2);
            e.normalized_value = "[+" + std::to_string(e.astro_year_start) + ", +" + std::to_string(e.astro_year_end) + "]";
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 13a. Exact Historical BCE: 3100 B.C., 732 BC, 1200 B.C.E.
        // Handles BC, BCE, B.C., B.C.E. era markers with any leading preposition or none.
        // --------------------------------------------------------------------
        std::regex re_exact_bce(R"(\b(\d+)\s*(BCE|BC|B\.C\.E\.|B\.C\.)(?!\w))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_exact_bce), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "EXACT_DATE_BCE";
            int y = parse_int(it->str(1));
            e.unit = it->str(2);
            e.astro_year_start = bce_to_astro(y);
            e.normalized_value = std::to_string(e.astro_year_start);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 13b. Exact Historical CE: 530 A.D., 70 AD, 135 CE, A.D. 70
        // Handles CE, AD, A.D., C.E. era markers preceding or following the year.
        // --------------------------------------------------------------------
        std::regex re_exact_ce(R"(\b(?:(\d+)\s*(CE|AD|A\.D\.|C\.E\.)|(AD|A\.D\.)\s*(\d+))(?!\w))", std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_exact_ce), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "EXACT_DATE_CE";
            int y = 0;
            if (!it->str(1).empty()) {
                y = parse_int(it->str(1));
                e.unit = it->str(2);
            } else {
                e.unit = it->str(3);
                y = parse_int(it->str(4));
            }
            e.astro_year_start = ce_to_astro(y);
            e.normalized_value = "+" + std::to_string(e.astro_year_start);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 14. Artifact & Specimen Counts: 694 tools, 330 handaxes, 6 pieces, 47 arrowheads
        // Matches N [optional-qualifier] artifact-noun. Headcounts (workers, people)
        // are excluded by the exclusion_spans pre-scan above.
        // --------------------------------------------------------------------
        std::regex re_artifact_count(
            R"(\b(\d+)\s+(?:\S+\s+)?(artifacts?|tools?|implements?|pieces?|cores?|flakes?|handaxes?|hand\s+axes?|choppers?|scrapers?|blades?|burins?|bifaces?|knives?|specimens?|assemblages?|pebble\s+tools?|stone\s+tools?|arrowheads?|points?|microliths?|sherds?|potsherds?|vessels?|beads?|figurines?|querns?|weights?|pithoi|pithos))",
            std::regex::icase);
        for (std::sregex_iterator it(text.begin(), text.end(), re_artifact_count), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "ARTIFACT_SPECIMEN_COUNT";
            e.unit = "count";
            e.numeric_val = parse_double(it->str(1));
            e.normalized_value = it->str(1);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // --------------------------------------------------------------------
        // 15. Spatial & Locus Provenance: Locus, Loc., Locality, Area, Stratum, Trench, Basket
        // --------------------------------------------------------------------
        std::regex re_locus(R"(\b(Locus|Loc\.|Locality)\s+([A-Za-z0-9\-]+))");
        for (std::sregex_iterator it(text.begin(), text.end(), re_locus), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "LOCUS_PROVENANCE";
            e.unit = "locus";
            e.normalized_value = it->str(2);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        std::regex re_area(R"(\bArea\s+([A-Za-z0-9]+))");
        for (std::sregex_iterator it(text.begin(), text.end(), re_area), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "SPATIAL_AREA";
            e.unit = "area";
            e.normalized_value = it->str(1);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        std::regex re_stratum(R"(\bStratum\s+([IVXLCDM]+[A-Za-z]*|\d+[A-Za-z]*))");
        for (std::sregex_iterator it(text.begin(), text.end(), re_stratum), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "STRATUM_NAME";
            e.unit = "stratum";
            e.normalized_value = it->str(1);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        std::regex re_trench(R"(\bTrench\s+([IVXLCDM]+[A-Za-z]*|\d+[A-Za-z]*))");
        for (std::sregex_iterator it(text.begin(), text.end(), re_trench), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "TRENCH_ID";
            e.unit = "trench";
            e.normalized_value = it->str(1);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        std::regex re_basket(R"(\bBasket\s+(\d+[A-Za-z]*))");
        for (std::sregex_iterator it(text.begin(), text.end(), re_basket), end; it != end; ++it) {
            if (is_excluded(it->position(), it->position() + it->length()) ||
                is_already_extracted(it->position(), it->position() + it->length())) continue;
            ExtractedEntity e;
            e.raw_match = it->str();
            e.entity_type = "BASKET_UNIT";
            e.unit = "basket";
            e.normalized_value = it->str(1);
            e.span_start = it->position();
            e.span_end = it->position() + it->length();
            results.push_back(e);
        }

        // Sort results by span_start
        std::sort(results.begin(), results.end(), [](const ExtractedEntity& a, const ExtractedEntity& b) {
            return a.span_start < b.span_start;
        });

        return results;
    }
};

} // namespace archaeophd
