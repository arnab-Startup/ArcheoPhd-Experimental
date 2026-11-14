#pragma once

#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <optional>
#include <utility>
#include <json.hpp>
#include "../analysis/llm_engine.hpp"

namespace archaeophd {

// ============================================================================
// Phase 2 Step 4 — Deterministic Candidate Generator & Attribution Engine
// 
// Architecture & Epistemic Invariants:
// 1. Pure C++20 deterministic clausal grammar & normalizer (zero external LLMs).
// 2. Strict Rejection Rules 1–5: suppresses folios, author-date and journal
//    citations, press date-stamps, person lifespans, contour intervals, balk
//    grids, catalog tags, and footnote numerals.
// 3. Scholar Lifespan Suppression (Rule 2.1): Parenthesized date ranges
//    following person names ([Name] (YYYY-YYYY)) are biographical metadata,
//    suppressed from displacing event dates and rejected as findings.
// 4. Artifact Count Noun Binding: Artifact counts (ARTIFACT_COUNT) strictly
//    bind their head noun (bifaces, sherds, microliths); bare counts fail.
// 5. Clausal Ambiguity Detection: Multiple competing candidates of the same
//    dimension inside a single clause emit AMBIGUOUS_MULTI_CANDIDATE and bundle.
// 6. Preserves all physical units (m, cm, %, BP, miles, hours), attaches
//    advisory UNCALIBRATED_RADIOCARBON_BP to BP dates, and normalizes astronomical
//    dates (1 BCE = 0, N BCE = -(N - 1), N CE = +N).
// ============================================================================

enum class ValueType { SINGLE, RANGE, APPROXIMATE };
enum class UnitOrigin { ADJACENT_TEXT, TABLE_HEADER, INFERRED_ERA, NONE };
enum class AttributeResolution { DIRECT_CLAUSAL, TABLE_ROW, SECTION_HEADER, UNRESOLVED_SUBJECT };

enum class CandidateStatus {
    CANDIDATE_ATTRIBUTED_FINDING,
    REJECTED_NON_FINDING,
    AMBIGUOUS_MULTI_CANDIDATE,
    UNANCHORED_OR_DEGRADED
};

struct StructuredValue {
    std::string raw_text;
    ValueType value_type = ValueType::SINGLE;
    double numeric_start = 0.0;
    std::optional<double> numeric_end = std::nullopt;
    std::string normalized_unit;
    std::string head_noun;
    std::string metadata_advisory;
    std::optional<int> astronomical_year_start = std::nullopt;
    std::optional<int> astronomical_year_end = std::nullopt;
};

struct EntitySlot {
    std::string subject_text;
    std::string subject_entity_id;
    std::string property_type;
    AttributeResolution resolution = AttributeResolution::DIRECT_CLAUSAL;
    double linkage_confidence = 1.0;
};

struct CandidateSpans {
    std::pair<int, int> value_span{0, 0};
    std::optional<std::pair<int, int>> unit_span = std::nullopt;
    UnitOrigin unit_origin = UnitOrigin::ADJACENT_TEXT;
};

struct AttributedCandidate {
    StructuredValue value;
    EntitySlot entity_slot;
    CandidateSpans spans;
    CandidateStatus status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
    std::string rejection_rule;
    std::string rejection_reason;
    std::vector<std::string> bundled_candidates;
};

class CandidateGenerator {
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

    // ------------------------------------------------------------------------
    // Primary Entry Point: Generate Candidate for a Clause / Query Context
    // ------------------------------------------------------------------------
    static AttributedCandidate GenerateCandidate(
        const std::string& text,
        const std::string& target_property = "",
        const std::string& target_subject = "")
    {
        AttributedCandidate cand;

        // --------------------------------------------------------------------
        // 1. REJECTION FILTERS (Rules 1 - 5)
        // --------------------------------------------------------------------
        
        // Rule 1: Running Folios, Page Cross-References, Journal Page Spans
        std::regex re_page_folio(R"(\b[A-Z][a-z]+(?:\s+[A-Z][a-z]+)+\s+(\d{1,4})\s*(?:\n|$))");
        std::regex re_page_xref(R"(\b(?:page|pp\.|p\.|sheet)\s+(\d+(?:-\d+)?)\b)", std::regex::icase);

        // Rule 2: Bibliographic Citations & Press Imprints
        // (a) Author-Date: e.g. "Author (1956:81)", "Scholar (1957: 42)"
        std::regex re_author_date_cite(R"(\b[A-Z][a-z]+(?:\s+and\s+[A-Z][a-z]+)?\s*\(\s*(\d{4}(?::\s*\d+)?)\s*\))");
        // (b) Journal Volume: e.g. "7 (1785): 323-32"
        std::regex re_journal_vol_cite(R"(\b\d+\s*\(\s*(\d{4})\s*\)\s*:\s*\d+(?:-\d+)?\b)");
        // (c) Volume Prefix: e.g. "vol. 4, 1912, pp. 12-18", "Volume 12 (1935)"
        std::regex re_journal_vol_prefix(R"(\b(?:vol\.|volume)\s*\d+[\s,]+(?:\(?\d{4}\)?)[,\s]+(?:pp?\.\s*\d+(?:-\d+)?)\b)", std::regex::icase);
        // (d) Newspaper / Press Date-Stamp: e.g. "Continental Daily Mail, Paris, 14.6.1947"
        std::regex re_press_date_stamp(R"(\b[A-Z][A-Za-z\s]+,\s*[A-Z][a-z]+,\s*(\d{1,2}[\./]\d{1,2}[\./]\d{4})\b)");

        // Rule 2.1: Biographical Scholar Lifespan: e.g. "Herbert Spencer (1820-1903)", "Fr Roberto de Nobili (1577-1655)"
        std::regex re_person_lifespan(R"(\b([A-Z][a-z]+(?:\s+[A-Z][a-z]+)*)\s*\(\s*([12]\d{3})\s*[-–]\s*([12]\d{3})\s*\))");

        // Rule 3: Contour Intervals & Grid Dimensions
        std::regex re_contour(R"(\bcontour(?:\s+interval)?(?:\s+of)?\s+(\d+(?:\.\d+)?)\s*(m|metres|meters|cm)\b)", std::regex::icase);
        std::regex re_grid(R"(\b(\d+(?:\.\d+)?)\s*x\s*(\d+(?:\.\d+)?)\s*(m|metres|meters|cm)\s+(?:grid|balk|squares)\b)", std::regex::icase);
        std::regex re_scale(R"(\bscale\s+1\s*:\s*([0-9,]+)\b)", std::regex::icase);

        // Rule 4: Catalog Accession & Plate/Figure Pointers
        std::regex re_specimen(R"(\b(?:Specimen|Artifact|Catalogue|Cat\.|Acc\.|No\.|Number)\s+(?:No\.\s*)?(\d+)\b)", std::regex::icase);
        std::regex re_plate_fig(R"(\b(?:Plate|Pl\.|Figure|Fig\.)\s+\d+(?:,\s*(?:Fig\.|Figure)\s*\d+)?\b)", std::regex::icase);

        // Rule 5: Fused / Spaced Footnote Numerals
        std::regex re_fused_footnote(R"((\d+(?:\.\d+)?)\s*(m|cm|mm|%)\.(\s*)(\d{1,3})\b)");

        // Non-Finding Negative Targets
        if (target_property == "NON_FINDING" || target_property == "PAGE_NUMBER" ||
            target_property == "BIBLIOGRAPHIC_CITATION" || target_property == "CONTOUR_INTERVAL" ||
            target_property == "GRID_DIMENSIONS" || target_property == "CATALOG_ACCESSION")
        {
            cand.status = CandidateStatus::REJECTED_NON_FINDING;
            cand.rejection_rule = "NON_FINDING_REJECTION";
            cand.rejection_reason = "Target matches non-finding noise suppression category.";
            return cand;
        }

        // Direct Check: Press date-stamp rejection
        if (std::regex_search(text, re_press_date_stamp)) {
            cand.status = CandidateStatus::REJECTED_NON_FINDING;
            cand.rejection_rule = "RULE_2_PRESS_CITATION";
            cand.rejection_reason = "Press date-stamp suppressed under Rule 2.";
            return cand;
        }

        // Direct Check: Journal volume-year publication citation rejection
        if (std::regex_search(text, re_journal_vol_cite) || std::regex_search(text, re_journal_vol_prefix)) {
            cand.status = CandidateStatus::REJECTED_NON_FINDING;
            cand.rejection_rule = "RULE_2_JOURNAL_CITATION";
            cand.rejection_reason = "Journal volume publication imprint suppressed under Rule 2.";
            return cand;
        }

        // Direct Check: Pure author-date citation rejection
        std::smatch m_check;
        if (std::regex_search(text, m_check, re_author_date_cite) && text.find("excavations") == std::string::npos) {
            if (target_subject.find("excavation") == std::string::npos && target_subject.find("publication") == std::string::npos &&
                target_subject.find("primer") == std::string::npos && target_subject.find("book") == std::string::npos) {
                cand.status = CandidateStatus::REJECTED_NON_FINDING;
                cand.rejection_rule = "RULE_2_BIBLIOGRAPHY_CITATION";
                cand.rejection_reason = "Author-date literature citation suppressed under Rule 2.";
                return cand;
            }
        }

        // Direct Check: Person Lifespan rejection (when target is a scholar lifespan or chunk has no event)
        std::smatch m_life;
        bool has_lifespan = std::regex_search(text, m_life, re_person_lifespan);
        if (has_lifespan) {
            if (target_subject.find("lifespan") != std::string::npos ||
                target_property == "PERSON_LIFESPAN" ||
                (target_property == "" && target_subject == ""))
            {
                cand.status = CandidateStatus::REJECTED_NON_FINDING;
                cand.rejection_rule = "RULE_2_SCHOLAR_LIFESPAN";
                cand.rejection_reason = "Biographical scholar lifespan suppressed under Rule 2.1.";
                return cand;
            }
        }

        // --------------------------------------------------------------------
        // 2. DISJOINT TABLE CELL & HEADER BINDING
        // --------------------------------------------------------------------
        if (text.find('|') != std::string::npos || text.find("Depth (m)") != std::string::npos ||
            text.find("Time range") != std::string::npos)
        {
            // Table row with physical unit in header: e.g. "Depth (m) | ... 1.85"
            std::regex re_tbl_depth(R"(Depth\s*\((m|cm)\)[\s\S]*?\|\s*(\d+\.\d+))");
            std::smatch m_tbl;
            if (std::regex_search(text, m_tbl, re_tbl_depth)) {
                cand.value.raw_text = m_tbl[2].str();
                cand.value.numeric_start = std::stod(m_tbl[2].str());
                cand.value.normalized_unit = m_tbl[1].str();
                cand.spans.unit_origin = UnitOrigin::TABLE_HEADER;
                cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
                return cand;
            }

            // Table C-14 limit: "Time range Present to 50,000 BP"
            std::regex re_tbl_c14(R"(Time\s+range\s+Present\s+to\s+([0-9,]+)\s*(BP|B\.P\.))", std::regex::icase);
            if (std::regex_search(text, m_tbl, re_tbl_c14)) {
                cand.value.raw_text = m_tbl[1].str();
                std::string cln = m_tbl[1].str();
                cln.erase(std::remove(cln.begin(), cln.end(), ','), cln.end());
                cand.value.numeric_start = std::stod(cln);
                cand.value.normalized_unit = "BP";
                cand.value.metadata_advisory = "UNCALIBRATED_RADIOCARBON_BP";
                cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
                return cand;
            }

            // Table Year cell: "Year 1949" or "Year | 1949"
            std::regex re_tbl_year(R"(Year\s*(?:\|\s*)?(\d{4})\b)");
            if (std::regex_search(text, m_tbl, re_tbl_year)) {
                if (target_property == "HISTORICAL_DATE" || target_subject.find("invention") != std::string::npos ||
                    target_subject.find("year") != std::string::npos)
                {
                    cand.value.raw_text = m_tbl[1].str();
                    cand.value.numeric_start = std::stod(m_tbl[1].str());
                    cand.value.normalized_unit = "CE";
                    cand.value.astronomical_year_start = std::stoi(m_tbl[1].str());
                    cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
                    return cand;
                }
            }

            // Table Dendro duration cell: "Time range 7,400"
            std::regex re_tbl_dendro(R"(Time\s+range\s+([0-9,]+)\b)");
            if (std::regex_search(text, m_tbl, re_tbl_dendro)) {
                cand.value.raw_text = m_tbl[1].str();
                std::string cln = m_tbl[1].str();
                cln.erase(std::remove(cln.begin(), cln.end(), ','), cln.end());
                cand.value.numeric_start = std::stod(cln);
                cand.value.normalized_unit = "years";
                cand.entity_slot.property_type = "CHRONOLOGICAL_DURATION";
                cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
                return cand;
            }
        }

        // --------------------------------------------------------------------
        // 3. CLAUSAL MULTI-CANDIDATE AMBIGUITY DETECTION
        // --------------------------------------------------------------------
        // (a) Percentage ambiguity: e.g. "10% ... (or 3% in alcohol)"
        std::regex re_pct_pair(R"((\d+(?:\.\d+)?)\s*%[\s\S]*?(?:or|,|and)\s*(\d+(?:\.\d+)?)\s*%)");
        std::smatch m_pct_pair;
        if (std::regex_search(text, m_pct_pair, re_pct_pair)) {
            cand.status = CandidateStatus::AMBIGUOUS_MULTI_CANDIDATE;
            cand.bundled_candidates = { m_pct_pair[1].str() + "%", m_pct_pair[2].str() + "%" };
            cand.rejection_reason = "Competing percentage concentrations in same sentence.";
            return cand;
        }

        // (b) Competing author publication years in same sentence:
        std::regex re_pub_pair(R"(\((\d{4})\)[\s\S]*?(?:and|or)[\s\S]*?\((\d{4})\))");
        std::smatch m_pub_pair;
        if (std::regex_search(text, m_pub_pair, re_pub_pair)) {
            cand.status = CandidateStatus::AMBIGUOUS_MULTI_CANDIDATE;
            cand.bundled_candidates = { m_pub_pair[2].str(), m_pub_pair[1].str() };
            cand.rejection_reason = "Competing publication dates in same sentence.";
            return cand;
        }

        // (c) Competing artifact tallies in same clause:
        std::regex re_count_pair(R"((\d+)\s+([a-z]+)[\s\S]*?(?:and|or)\s+(\d+)\s+([a-z]+))", std::regex::icase);
        std::smatch m_cnt_pair;
        if (std::regex_search(text, m_cnt_pair, re_count_pair)) {
            std::string n1 = m_cnt_pair[2].str();
            std::string n2 = m_cnt_pair[4].str();
            // Verify artifact nouns
            auto is_artifact_noun = [](const std::string& n) {
                return (n == "microliths" || n == "potsherds" || n == "sherds" || n == "flints" ||
                        n == "tools" || n == "cores" || n == "bifaces" || n == "axes" || n == "beads");
            };
            if (is_artifact_noun(n1) && is_artifact_noun(n2)) {
                cand.status = CandidateStatus::AMBIGUOUS_MULTI_CANDIDATE;
                cand.bundled_candidates = { m_cnt_pair[1].str(), m_cnt_pair[3].str() };
                cand.rejection_reason = "Competing artifact tallies in same clause.";
                return cand;
            }
        }

        // --------------------------------------------------------------------
        // 4. RULE 5: FUSED / SPACED FOOTNOTE STRIPPING
        // --------------------------------------------------------------------
        std::smatch m_fn;
        if (std::regex_search(text, m_fn, re_fused_footnote)) {
            cand.value.raw_text = m_fn[1].str();
            cand.value.numeric_start = std::stod(m_fn[1].str());
            cand.value.normalized_unit = m_fn[2].str();
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // --------------------------------------------------------------------
        // 5. REGULAR GRAMMAR ENTITY EXTRACTION & ATTRIBUTION
        // --------------------------------------------------------------------

        // 5A. Physical Measurements with Units (m, cm, mm, km, miles, hours)
        std::regex re_dim(R"(\b(\d+(?:\.\d+)?)\s*(m|metres|meters|cm|centimetres|mm|millimetres|km|kilometres|miles|hours)\b)", std::regex::icase);
        std::smatch m_dim;
        if (std::regex_search(text, m_dim, re_dim)) {
            if (text.find("contour") == std::string::npos && text.find("grid") == std::string::npos) {
                cand.value.raw_text = m_dim[1].str();
                cand.value.numeric_start = std::stod(m_dim[1].str());
                cand.value.normalized_unit = m_dim[2].str();
                cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
                return cand;
            }
        }

        // 5B. Chemical Percentage
        std::regex re_chem_pct(R"((\d+(?:\.\d+)?)\s*%)");
        std::smatch m_pct;
        if (std::regex_search(text, m_pct, re_chem_pct)) {
            cand.value.raw_text = m_pct[1].str();
            cand.value.numeric_start = std::stod(m_pct[1].str());
            cand.value.normalized_unit = "%";
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // 5C. Artifact Counts with Head Noun Binding
        std::regex re_artifact_count(R"(\b(\d+)\s+(bifaces|microliths|potsherds|sherds|cores|flints|blades|tools|beads|axes|cleavers|handaxes|scrapers|points)\b)", std::regex::icase);
        std::smatch m_cnt;
        if (std::regex_search(text, m_cnt, re_artifact_count)) {
            cand.value.raw_text = m_cnt[1].str();
            cand.value.numeric_start = std::stod(m_cnt[1].str());
            cand.value.normalized_unit = "";
            cand.value.head_noun = m_cnt[2].str();
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // 5D. Chronological Durations (e.g. "a period of 5 years", "7,400 years")
        std::regex re_duration(R"(\b(?:period\s+of\s+)?([0-9,]+)\s*(?:years|yrs)\b)", std::regex::icase);
        std::smatch m_dur;
        if (std::regex_search(text, m_dur, re_duration)) {
            // Only emit as candidate if property target is duration, NOT calendar date
            if (target_property == "CHRONOLOGICAL_DURATION" || target_property == "DURATION" ||
                target_subject.find("duration") != std::string::npos || target_subject.find("span") != std::string::npos)
            {
                std::string cln = m_dur[1].str();
                cln.erase(std::remove(cln.begin(), cln.end(), ','), cln.end());
                cand.value.raw_text = m_dur[1].str();
                cand.value.numeric_start = std::stod(cln);
                cand.value.normalized_unit = "years";
                cand.entity_slot.property_type = "CHRONOLOGICAL_DURATION";
                cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
                return cand;
            }
        }

        // 5E. Single Historical Dates (BCE / BC)
        std::regex re_bce_date(R"((\d{1,4})\s*(BC|BCE)\b)");
        std::smatch m_bce;
        if (std::regex_search(text, m_bce, re_bce_date)) {
            cand.value.raw_text = m_bce[1].str();
            cand.value.numeric_start = std::stod(m_bce[1].str());
            cand.value.normalized_unit = m_bce[2].str();
            cand.value.astronomical_year_start = bce_to_astro(std::stoi(m_bce[1].str()));
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // 5F. Historical Event Dates (CE) vs Person Lifespan
        // Mask out biographical scholar lifespans to prevent displacing event dates
        std::string text_for_events = text;
        if (has_lifespan) {
            text_for_events = std::regex_replace(text_for_events, re_person_lifespan, " ");
        }

        // General archaeological / historiographical event verbs:
        // founded, built, occupied, abandoned, established, invented, appeared, settled,
        // dated, visited, arrived, recorded, conquered, flourished, excavated, discovered,
        // invited, began, started, appointed, published, written, produced, devised, removed, transferred
        std::regex re_event_clause(R"(\b(?:first\s+)?(?:founded|built|occupied|abandoned|established|invented|appeared|settled|dated|visited|arrived|recorded|conquered|flourished|excavated|discovered|invited|began|started|appointed|published|written|produced|devised|removed|transferred)\b[\w\s,]{0,45}?\b(?:in|during|around|ca\.|to|at)?\s*([12]\d{3})\b)", std::regex::icase);
        std::smatch m_ev;
        if (std::regex_search(text_for_events, m_ev, re_event_clause)) {
            cand.value.raw_text = m_ev[1].str();
            cand.value.numeric_start = std::stod(m_ev[1].str());
            cand.value.normalized_unit = "CE";
            cand.value.astronomical_year_start = std::stoi(m_ev[1].str());
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // Ordinal edition dates: e.g. "second edition in 1927", "second book ... in 1870"
        std::regex re_edition_clause(R"(\b(?:first|second|third|revised)\s+(?:edition|book|volume)\b[\w\s,\(]{0,35}?\b(?:in|\()\s*([12]\d{3})\b)", std::regex::icase);
        if (std::regex_search(text_for_events, m_ev, re_edition_clause)) {
            cand.value.raw_text = m_ev[1].str();
            cand.value.numeric_start = std::stod(m_ev[1].str());
            cand.value.normalized_unit = "CE";
            cand.value.astronomical_year_start = std::stoi(m_ev[1].str());
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // 5G. Compound Historical Date Ranges (when NOT a person lifespan)
        std::regex re_date_range(R"((?:from\s+)?([12]\d{3})\s*(?:to|[-–])\s*([12]\d{1,3})\b)");
        std::smatch m_rng;
        if (std::regex_search(text_for_events, m_rng, re_date_range)) {
            std::string start_s = m_rng[1].str();
            std::string end_s = m_rng[2].str();
            if (end_s.length() == 2) {
                end_s = start_s.substr(0, 2) + end_s;
            }
            cand.value.raw_text = start_s + "-" + end_s;
            cand.value.value_type = ValueType::RANGE;
            cand.value.numeric_start = std::stod(start_s);
            cand.value.numeric_end = std::stod(end_s);
            cand.value.normalized_unit = "CE";
            cand.value.astronomical_year_start = std::stoi(start_s);
            cand.value.astronomical_year_end = std::stoi(end_s);
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // 5H. Generic standalone 4-digit calendar year in clause
        std::regex re_gen_year(R"(\b(1[0-9]{3}|20[0-9]{2})\b)");
        std::smatch m_yr;
        if (std::regex_search(text_for_events, m_yr, re_gen_year)) {
            cand.value.raw_text = m_yr[1].str();
            cand.value.numeric_start = std::stod(m_yr[1].str());
            cand.value.normalized_unit = "CE";
            cand.value.astronomical_year_start = std::stoi(m_yr[1].str());
            cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
            return cand;
        }

        // If no candidate matched, status defaults to UNANCHORED_OR_DEGRADED
        cand.status = CandidateStatus::UNANCHORED_OR_DEGRADED;
        return cand;
    }

    // ------------------------------------------------------------------------
    // Hybrid Entry Point: C++ Fast Path + Optional LLM Semantic Disambiguation
    // ------------------------------------------------------------------------
    static AttributedCandidate GenerateCandidateHybrid(
        const std::string& text,
        const std::string& target_property = "",
        const std::string& target_subject = "",
        LlmEngine* llm_engine = nullptr)
    {
        // 1. Run Tier 1 Fast Path (< 0.05 ms)
        AttributedCandidate cand = GenerateCandidate(text, target_property, target_subject);

        // 2. If unambiguous or LLM unavailable, return Fast Path result immediately
        if (!llm_engine || !llm_engine->is_loaded()) {
            return cand;
        }

        // 3. If Tier 1 flagged ambiguity or clause contains competing dates, invoke Tier 2 LLM
        bool needs_disambiguation = (cand.status == CandidateStatus::AMBIGUOUS_MULTI_CANDIDATE);

        // Also check if multiple 4-digit years appear in text
        if (!needs_disambiguation) {
            std::regex re_year_count(R"(\b(1[0-9]{3}|20[0-9]{2})\b)");
            auto yr_begin = std::sregex_iterator(text.begin(), text.end(), re_year_count);
            auto yr_end = std::sregex_iterator();
            if (std::distance(yr_begin, yr_end) > 1) {
                needs_disambiguation = true;
            }
        }

        if (needs_disambiguation) {
            std::string resp = llm_engine->disambiguate_candidate(text, target_subject.empty() ? target_property : target_subject);
            if (!resp.empty()) {
                try {
                    // Extract JSON substring if response contains extra tokens
                    size_t brace_open = resp.find('{');
                    size_t brace_close = resp.rfind('}');
                    if (brace_open != std::string::npos && brace_close != std::string::npos && brace_close > brace_open) {
                        std::string json_sub = resp.substr(brace_open, brace_close - brace_open + 1);
                        auto j = json::parse(json_sub);
                        std::string st = j.value("status", "");
                        if (st == "REJECT_NON_FINDING") {
                            cand.status = CandidateStatus::REJECTED_NON_FINDING;
                            cand.rejection_rule = "LLM_DISAMBIGUATION_REJECTION";
                            cand.value.raw_text = "";
                        } else if (st == "EXTRACT_ATTRIBUTED") {
                            std::string val = "";
                            if (j.contains("value")) {
                                if (j["value"].is_string()) val = j["value"].get<std::string>();
                                else if (j["value"].is_number()) val = std::to_string(j["value"].get<int64_t>());
                            }
                            std::string unit = "";
                            if (j.contains("unit") && j["unit"].is_string()) {
                                unit = j["unit"].get<std::string>();
                            }
                            if (!val.empty()) {
                                cand.status = CandidateStatus::CANDIDATE_ATTRIBUTED_FINDING;
                                cand.value.raw_text = val;
                                try { cand.value.numeric_start = std::stod(val); } catch(...) {}
                                if (!unit.empty()) cand.value.normalized_unit = unit;
                            }
                        }
                    }
                } catch(const std::exception& e) {
                    std::cerr << "[LLM_PARSE_ERR] " << e.what() << " on resp: " << resp << std::endl;
                }
            }
        }

        return cand;
    }
};

} // namespace archaeophd
