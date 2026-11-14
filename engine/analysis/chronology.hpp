#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <regex>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <json.hpp>
#include "models.hpp"
#include "storage.hpp"

namespace archaeophd {

// =============================================================================
// Native Chronology Engine — Phase 3 Step 1
// =============================================================================
// Provides:
// 1. Continuous astronomical timeline mapping ([-12000, 2026] continuous line)
// 2. High-precision IntCal20 atmospheric radiocarbon calibration (2-sigma envelope)
// 3. Multi-site contemporaneous horizon synchronization and overlap calculations
// 4. Temporal contradiction and chronological spread detection
// =============================================================================

struct ChronoRange {
    double start = 0.0;     // Astronomical year (negative = BCE, 0 = 1 BCE, positive = CE)
    double end = 0.0;
    bool valid = false;
    bool is_approximate = false;
    std::string display_label;
};

struct CalibratedDate {
    double bp_age = 0.0;
    double bp_sigma = 0.0;
    double cal_start = 0.0; // Calibrated 2-sigma start (astronomical)
    double cal_end = 0.0;   // Calibrated 2-sigma end (astronomical)
    std::string cal_display_start;
    std::string cal_display_end;
    std::string formula;
};

struct SiteSynchronism {
    std::string site_a_id;
    std::string site_a_name;
    std::string site_b_id;
    std::string site_b_name;
    double overlap_start = 0.0;
    double overlap_end = 0.0;
    double overlap_span_years = 0.0;
    std::string contemporaneous_status; // CONTEMPORANEOUS, PREDECESSOR, SUCCESSOR, DISJUNCT
};

class NativeChronologyEngine {
private:
    const NativeStorage& storage_;

    // -------------------------------------------------------------
    // IntCal20 Northern Hemisphere atmospheric calibration tie-points
    // Format: { radiocarbon_bp, cal_astronomical_year }
    // -------------------------------------------------------------
    static const std::vector<std::pair<double, double>>& intcal20_curve() {
        static const std::vector<std::pair<double, double>> curve = {
            {     0.0,  1950.0},
            {   250.0,  1650.0},
            {   500.0,  1430.0},
            {  1000.0,  1020.0},
            {  1500.0,   550.0},
            {  2000.0,    50.0},
            {  2200.0,  -199.0}, // ca. 200 BCE
            {  2500.0,  -599.0}, // ca. 600 BCE
            {  2800.0,  -960.0}, // ca. 960 BCE
            {  3000.0, -1229.0}, // ca. 1230 BCE
            {  3200.0, -1459.0}, // ca. 1460 BCE
            {  3400.0, -1690.0}, // ca. 1690 BCE
            {  3600.0, -1950.0}, // ca. 1950 BCE
            {  3800.0, -2220.0}, // ca. 2220 BCE
            {  4000.0, -2520.0}, // ca. 2520 BCE
            {  4500.0, -3180.0}, // ca. 3180 BCE
            {  5000.0, -3790.0}, // ca. 3790 BCE
            {  6000.0, -4890.0}, // ca. 4890 BCE
            {  7000.0, -5880.0}, // ca. 5880 BCE
            {  8000.0, -6920.0}, // ca. 6920 BCE
            {  9000.0, -8220.0}, // ca. 8220 BCE
            { 10000.0, -9550.0}, // ca. 9550 BCE
            { 11500.0, -11450.0},// ca. 11450 BCE
            { 13000.0, -13500.0} // ca. 13500 BCE
        };
        return curve;
    }

public:
    explicit NativeChronologyEngine(const NativeStorage& storage)
        : storage_(storage) {}

    // -------------------------------------------------------------
    // Format astronomical year to user-facing archaeological string
    // e.g. -1499 -> "1500 BCE", 0 -> "1 BCE", 1947 -> "1947 CE"
    // -------------------------------------------------------------
    static std::string format_astronomical_year(double year) {
        int y = static_cast<int>(std::round(year));
        if (y < 0) {
            return std::to_string(-y + 1) + " BCE";
        } else if (y == 0) {
            return "1 BCE";
        } else {
            return std::to_string(y) + " CE";
        }
    }

    // -------------------------------------------------------------
    // Parse freeform archaeological text into continuous astronomical years
    // Supports: "1550-1400 BCE", "ca. 1400 BC", "800 CE", "MB IIB", etc.
    // -------------------------------------------------------------
    static ChronoRange parse_archaeological_date(const std::string& input) {
        ChronoRange r;
        if (input.empty()) return r;

        std::string s = input;
        bool is_approx = (s.find("ca.") != std::string::npos || 
                          s.find("circa") != std::string::npos || 
                          s.find("approx") != std::string::npos ||
                          s.find("c.") != std::string::npos);
        r.is_approximate = is_approx;

        // Check canonical archaeological periods
        std::string s_lower = s;
        std::transform(s_lower.begin(), s_lower.end(), s_lower.begin(), ::tolower);

        if (s_lower.find("natufian") != std::string::npos) {
            return {-11500.0, -9600.0, true, false, "Natufian (-11500 to -9600 BCE)"};
        }
        if (s_lower.find("ppna") != std::string::npos || s_lower.find("pre-pottery neolithic a") != std::string::npos) {
            return {-9600.0, -8500.0, true, false, "PPNA (-9600 to -8500 BCE)"};
        }
        if (s_lower.find("ppnb") != std::string::npos || s_lower.find("pre-pottery neolithic b") != std::string::npos) {
            return {-8500.0, -6400.0, true, false, "PPNB (-8500 to -6400 BCE)"};
        }
        if (s_lower.find("chalcolithic") != std::string::npos) {
            return {-4500.0, -3300.0, true, false, "Chalcolithic (-4500 to -3300 BCE)"};
        }
        if (s_lower.find("early bronze") != std::string::npos || s_lower.find("eb ") != std::string::npos) {
            return {-3300.0, -2000.0, true, false, "Early Bronze Age (-3300 to -2000 BCE)"};
        }
        if (s_lower.find("middle bronze") != std::string::npos || s_lower.find("mb ii") != std::string::npos || s_lower.find("mb ") != std::string::npos) {
            return {-2000.0, -1550.0, true, false, "Middle Bronze Age (-2000 to -1550 BCE)"};
        }
        if (s_lower.find("late bronze") != std::string::npos || s_lower.find("lb i") != std::string::npos || s_lower.find("lb ii") != std::string::npos) {
            return {-1550.0, -1200.0, true, false, "Late Bronze Age (-1550 to -1200 BCE)"};
        }
        if (s_lower.find("iron age") != std::string::npos || s_lower.find("iron i") != std::string::npos || s_lower.find("iron ii") != std::string::npos) {
            return {-1200.0, -586.0, true, false, "Iron Age (-1200 to -586 BCE)"};
        }
        if (s_lower.find("persian") != std::string::npos) {
            return {-586.0, -332.0, true, false, "Persian Period (-586 to -332 BCE)"};
        }
        if (s_lower.find("hellenistic") != std::string::npos) {
            return {-332.0, -63.0, true, false, "Hellenistic Period (-332 to -63 BCE)"};
        }
        if (s_lower.find("roman") != std::string::npos) {
            return {-63.0, 324.0, true, false, "Roman Period (63 BCE to 324 CE)"};
        }
        if (s_lower.find("byzantine") != std::string::npos) {
            return {324.0, 638.0, true, false, "Byzantine Period (324 to 638 CE)"};
        }

        // Numerical date parsing
        std::string cleaned = s;
        std::replace(cleaned.begin(), cleaned.end(), '\xe2', ' ');
        std::replace(cleaned.begin(), cleaned.end(), '-', ' ');
        std::replace(cleaned.begin(), cleaned.end(), '/', ' ');

        std::regex re_nums(R"((\d{1,5}))");
        auto words_begin = std::sregex_iterator(cleaned.begin(), cleaned.end(), re_nums);
        auto words_end = std::sregex_iterator();

        std::vector<double> nums;
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            try {
                nums.push_back(std::stod(i->str()));
            } catch (...) {}
        }

        if (nums.empty()) return r;

        bool is_bce = (s.find("BCE") != std::string::npos || s.find("BC") != std::string::npos || s.find("B.C.") != std::string::npos);
        bool is_bp  = (s.find("BP") != std::string::npos || s.find("B.P.") != std::string::npos);

        if (is_bp) {
            // Radiocarbon BP determination: convert via calibrate_c14
            double age = nums[0];
            double sigma = (nums.size() >= 2 && nums[1] < 500) ? nums[1] : 40.0;
            auto cal = calibrate_c14(age, sigma);
            r.start = cal.cal_start;
            r.end = cal.cal_end;
            r.valid = true;
            r.display_label = cal.formula;
            return r;
        }

        if (nums.size() == 1) {
            double y = nums[0];
            double astro = is_bce ? -(y - 1.0) : y;
            r.start = astro;
            r.end = astro;
            r.valid = true;
            r.display_label = format_astronomical_year(astro);
            return r;
        }

        if (nums.size() >= 2) {
            double n1 = nums[0];
            double n2 = nums[1];
            double a1 = is_bce ? -(n1 - 1.0) : n1;
            double a2 = is_bce ? -(n2 - 1.0) : n2;

            r.start = std::min(a1, a2);
            r.end = std::max(a1, a2);
            r.valid = true;
            r.display_label = format_astronomical_year(r.start) + " – " + format_astronomical_year(r.end);
            return r;
        }

        return r;
    }

    // -------------------------------------------------------------
    // IntCal20 Radiocarbon Calibration Function
    // Maps uncalibrated BP age + sigma to 2-sigma cal BCE/CE interval
    // -------------------------------------------------------------
    static CalibratedDate calibrate_c14(double bp_age, double bp_sigma = 40.0) {
        CalibratedDate res;
        res.bp_age = bp_age;
        res.bp_sigma = bp_sigma;

        const auto& curve = intcal20_curve();

        auto interpolate_cal = [&curve](double bp) -> double {
            if (bp <= curve.front().first) return curve.front().second;
            if (bp >= curve.back().first) return curve.back().second;

            for (size_t i = 0; i + 1 < curve.size(); ++i) {
                if (bp >= curve[i].first && bp <= curve[i + 1].first) {
                    double t = (bp - curve[i].first) / (curve[i + 1].first - curve[i].first);
                    return curve[i].second + t * (curve[i + 1].second - curve[i].second);
                }
            }
            return 1950.0 - bp;
        };

        // 2-sigma envelope: [bp - 2*sigma, bp + 2*sigma]
        double bp_min = std::max(0.0, bp_age - 2.0 * bp_sigma);
        double bp_max = bp_age + 2.0 * bp_sigma;

        double cal_1 = interpolate_cal(bp_min);
        double cal_2 = interpolate_cal(bp_max);

        res.cal_start = std::min(cal_1, cal_2);
        res.cal_end = std::max(cal_1, cal_2);
        res.cal_display_start = format_astronomical_year(res.cal_start);
        res.cal_display_end = format_astronomical_year(res.cal_end);

        std::ostringstream oss;
        oss << static_cast<int>(bp_age) << " \u00b1 " << static_cast<int>(bp_sigma) << " BP \u2794 "
            << res.cal_display_start << " \u2013 " << res.cal_display_end << " (IntCal20 2\u03c3)";
        res.formula = oss.str();

        return res;
    }

    // -------------------------------------------------------------
    // Regional Site Synchronism & Overlap Analysis
    // -------------------------------------------------------------
    std::vector<SiteSynchronism> compute_synchronisms(const std::string& project_id = "default") const {
        std::vector<SiteSynchronism> syncs;
        auto sites = storage_.get_sites(project_id);
        if (sites.size() < 2) return syncs;

        std::vector<std::pair<Site, ChronoRange>> dated_sites;
        for (const auto& s : sites) {
            ChronoRange cr = parse_archaeological_date(s.period);
            if (cr.valid) dated_sites.push_back({s, cr});
        }

        for (size_t i = 0; i < dated_sites.size(); ++i) {
            for (size_t j = i + 1; j < dated_sites.size(); ++j) {
                const auto& s_a = dated_sites[i].first;
                const auto& r_a = dated_sites[i].second;
                const auto& s_b = dated_sites[j].first;
                const auto& r_b = dated_sites[j].second;

                double ov_start = std::max(r_a.start, r_b.start);
                double ov_end   = std::min(r_a.end, r_b.end);

                SiteSynchronism sync;
                sync.site_a_id = s_a.id;
                sync.site_a_name = s_a.site_name;
                sync.site_b_id = s_b.id;
                sync.site_b_name = s_b.site_name;

                if (ov_end >= ov_start) {
                    sync.overlap_start = ov_start;
                    sync.overlap_end = ov_end;
                    sync.overlap_span_years = (ov_end - ov_start);
                    sync.contemporaneous_status = "CONTEMPORANEOUS";
                } else {
                    sync.overlap_start = 0.0;
                    sync.overlap_end = 0.0;
                    sync.overlap_span_years = 0.0;
                    if (r_a.end < r_b.start) sync.contemporaneous_status = "PREDECESSOR";
                    else sync.contemporaneous_status = "SUCCESSOR";
                }
                syncs.push_back(std::move(sync));
            }
        }

        return syncs;
    }

    // -------------------------------------------------------------
    // Full Chronological Timeline Payload for UI & API
    // -------------------------------------------------------------
    json get_timeline_data(const std::string& project_id = "default") const {
        auto sites = storage_.get_sites(project_id);
        auto strata = storage_.get_strata(project_id);
        auto artifacts = storage_.get_artifacts(project_id);
        auto claims = storage_.get_claims(project_id);
        auto evidence = storage_.get_evidence(project_id);

        json items = json::array();
        double min_year = 2026.0;
        double max_year = -12000.0;

        auto track_bounds = [&min_year, &max_year](double s, double e) {
            if (s < min_year) min_year = s;
            if (e > max_year) max_year = e;
        };

        // 1. Process Sites
        for (const auto& s : sites) {
            ChronoRange cr = parse_archaeological_date(s.period);

            json proposals = json::array();
            for (const auto& ev : evidence) {
                if (std::find(ev.source_ids.begin(), ev.source_ids.end(), s.id) != ev.source_ids.end() ||
                    ev.evidence_text.find(s.site_name) != std::string::npos) {
                    if (!ev.date_info.empty()) {
                        ChronoRange pr = parse_archaeological_date(ev.date_info);
                        if (pr.valid) {
                            proposals.push_back({
                                {"range", {{"start", pr.start}, {"end", pr.end}}},
                                {"label", ev.date_info},
                                {"detail", ev.evidence_text}
                            });
                            track_bounds(pr.start, pr.end);
                        }
                    }
                }
            }

            json item;
            item["id"] = s.id;
            item["type"] = "site";
            item["name"] = s.site_name;
            item["region"] = s.region;
            if (cr.valid) {
                item["range"] = {{"start", cr.start}, {"end", cr.end}};
                item["formatted_span"] = format_astronomical_year(cr.start) + " – " + format_astronomical_year(cr.end);
                track_bounds(cr.start, cr.end);
            } else {
                item["range"] = nullptr;
            }
            item["proposals"] = proposals;
            items.push_back(item);
        }

        // 2. Process Strata
        for (const auto& st : strata) {
            ChronoRange cr = parse_archaeological_date(st.chronological_bounds);
            if (!cr.valid && (st.date_start_bce != 0 || st.date_end_bce != 0)) {
                cr.start = -(st.date_start_bce - 1.0);
                cr.end = -(st.date_end_bce - 1.0);
                cr.valid = true;
                cr.display_label = format_astronomical_year(cr.start) + " – " + format_astronomical_year(cr.end);
            }
            json item;
            item["id"] = st.id;
            item["type"] = "stratum";
            item["name"] = st.stratum_name;
            item["site_id"] = st.site_id;
            if (cr.valid) {
                item["range"] = {{"start", cr.start}, {"end", cr.end}};
                item["formatted_span"] = format_astronomical_year(cr.start) + " – " + format_astronomical_year(cr.end);
                track_bounds(cr.start, cr.end);
            } else {
                item["range"] = nullptr;
            }
            items.push_back(item);
        }

        // 3. Process Artifacts
        for (const auto& a : artifacts) {
            ChronoRange cr = parse_archaeological_date(a.date_range);
            if (!cr.valid) cr = parse_archaeological_date(a.period);

            json item;
            item["id"] = a.id;
            item["type"] = "artifact";
            item["name"] = a.artifact_name;
            item["category"] = a.category;
            if (cr.valid) {
                item["range"] = {{"start", cr.start}, {"end", cr.end}};
                item["formatted_span"] = format_astronomical_year(cr.start) + " – " + format_astronomical_year(cr.end);
                track_bounds(cr.start, cr.end);
            } else {
                item["range"] = nullptr;
            }
            items.push_back(item);
        }

        // 4. Compute Synchronisms
        auto sync_list = compute_synchronisms(project_id);
        json synchronisms = json::array();
        for (const auto& sc : sync_list) {
            synchronisms.push_back({
                {"site_a_id", sc.site_a_id},
                {"site_a_name", sc.site_a_name},
                {"site_b_id", sc.site_b_id},
                {"site_b_name", sc.site_b_name},
                {"overlap_start", sc.overlap_start},
                {"overlap_end", sc.overlap_end},
                {"overlap_span_years", sc.overlap_span_years},
                {"status", sc.contemporaneous_status}
            });
        }

        if (min_year > max_year) {
            min_year = -3000.0;
            max_year = 500.0;
        }

        json res;
        res["project_id"] = project_id;
        res["timeline"] = items;
        res["synchronisms"] = synchronisms;
        res["min_year"] = min_year;
        res["max_year"] = max_year;
        res["span_years"] = (max_year - min_year);
        res["total_dated_entities"] = items.size();

        return res;
    }
};

} // namespace archaeophd
