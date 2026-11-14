#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <json.hpp>

using json = nlohmann::json;

namespace archaeophd {

// -------------------------------------------------------------
// Layer A: Physical World Layer (The Ground Truth - Immutable)
// -------------------------------------------------------------

struct Site {
    std::string id;
    std::string project_id = "default";
    std::string site_name;
    std::string country;
    std::string region;
    double latitude = 0.0;
    double longitude = 0.0;
    double elevation = 0.0;
    std::string period;
    std::string site_type;
    std::string excavation_history;
    std::vector<std::string> aliases;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Site, id, project_id, site_name, country, region, latitude, longitude, elevation, period, site_type, excavation_history, aliases, created_date)
};

struct Stratum {
    std::string id;
    std::string project_id = "default";
    std::string site_id;
    std::string stratum_name;
    std::string phase;
    std::vector<std::string> locus_numbers;
    std::string sediment_type;
    std::string chronological_bounds;
    int date_start_bce = 0;
    int date_end_bce = 0;
    int date_start_ce = 0;
    int date_end_ce = 0;
    std::vector<std::string> harris_above;   // Units directly above (younger)
    std::vector<std::string> harris_below;   // Units directly below (older)
    std::vector<std::string> harris_cut_by;  // Units cutting this feature
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Stratum, id, project_id, site_id, stratum_name, phase, locus_numbers, sediment_type, chronological_bounds, date_start_bce, date_end_bce, date_start_ce, date_end_ce, harris_above, harris_below, harris_cut_by, created_date)
};

struct Artifact {
    std::string id;
    std::string project_id = "default";
    std::string artifact_name;
    std::string category = "Ceramic";
    std::string material;
    std::string period;
    std::string date_range;
    std::vector<std::string> site_ids;
    std::string stratum_id;
    std::string locus_findspot;
    std::string typology;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Artifact, id, project_id, artifact_name, category, material, period, date_range, site_ids, stratum_id, locus_findspot, typology, created_date)
};

struct Sample {
    std::string id;
    std::string project_id = "default";
    std::string site_id;
    std::string stratum_id;
    std::string lab_code;
    std::string material;
    std::string method = "C-14";
    std::string cal_range_2sigma;
    int date_cal_start_bce = 0;
    int date_cal_end_bce = 0;
    int date_cal_start_ce = 0;
    int date_cal_end_ce = 0;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Sample, id, project_id, site_id, stratum_id, lab_code, material, method, cal_range_2sigma, date_cal_start_bce, date_cal_end_bce, date_cal_start_ce, date_cal_end_ce, created_date)
};

// -------------------------------------------------------------
// Layer B: The Claims Layer (The Interpretive World)
// -------------------------------------------------------------

struct Claim {
    std::string id;
    std::string project_id = "default";
    std::string claim_text;
    std::string scholar_name;
    std::string source_id;
    std::string publication_year;
    std::string page_ref;
    std::string chapter;
    std::string status = "Contested"; // Verified, Contested, Speculative
    std::string topic;               // Chronology, Warfare, Trade, Function
    std::vector<std::string> site_ids;
    std::vector<std::string> strata_ids;
    std::string origin_type = "digital_stream"; // "digital_stream" or "scanned_ocr"
    std::string verification_status = "VERIFIED"; // "VERIFIED" or "PENDING_VERIFICATION"
    bool is_quantitative = false;
    bool anomaly_flag = false;
    std::string anomaly_reason;
    std::string optical_crop_path;
    double ocr_confidence = 1.0;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Claim, id, project_id, claim_text, scholar_name, source_id, publication_year, page_ref, chapter, status, topic, site_ids, strata_ids, origin_type, verification_status, is_quantitative, anomaly_flag, anomaly_reason, optical_crop_path, ocr_confidence, created_date)
};

// -------------------------------------------------------------
// Layer C: The Evidence Layer (The Bridge)
// -------------------------------------------------------------

struct EvidenceLink {
    std::string id;
    std::string project_id = "default";
    std::string claim_id;
    std::string evidence_text;
    std::string evidence_type = "supporting"; // supporting, contradicting, neutral
    std::string physical_entity_type;
    std::string physical_entity_id;
    std::vector<std::string> source_ids;
    std::string date_info;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EvidenceLink, id, project_id, claim_id, evidence_text, evidence_type, physical_entity_type, physical_entity_id, source_ids, date_info, created_date)
};

struct Source {
    std::string id;
    std::string project_id = "default";
    std::string title;
    std::string author;
    std::string year;
    std::string publication;
    std::string journal;
    std::string pages;
    std::string source_type = "Monograph";
    std::string file_path;
    std::string degradation_class = "CLASS_B"; // Default-safe: CLASS_B (Porous/Letterpress) or CLASS_A (Clean Offset)
    bool confirmed_clean_offset = false;       // Must be explicitly confirmed by researcher to enable Class A
    std::string ingestion_status = "UNVERIFIED_ROUGH_SCAN"; // UNVERIFIED_ROUGH_SCAN, VERIFICATION_PENDING, VERIFIED_MANUAL, COMPLETED
    std::string archive_path;                  // e.g. archives/<id>.pdf.zst
    std::string file_sha256;
    uint64_t file_size_bytes = 0;
    uint64_t compressed_size_bytes = 0;
    int page_count = 0;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Source, id, project_id, title, author, year, publication, journal, pages, source_type, file_path, degradation_class, confirmed_clean_offset, ingestion_status, archive_path, file_sha256, file_size_bytes, compressed_size_bytes, page_count, created_date)
};

struct VerificationItem {
    std::string id;
    std::string project_id = "default";
    std::string source_id;
    int page_number = 1;
    std::string field_type = "measurement"; // date, measurement, count, locus
    std::string context_text;
    std::string crop_image_path;            // Mandatory visual crop for optical verification
    std::string candidate_a;                // Windows Native OCR output
    std::string candidate_b;                // Tesseract LSTM output
    std::string resolved_value;
    std::string status = "PENDING";         // PENDING, RESOLVED_A, RESOLVED_B, RESOLVED_MANUAL_OVERRIDE, REJECTED_DUE_TO_RECLASSIFICATION
    std::string audit_note;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VerificationItem, id, project_id, source_id, page_number, field_type, context_text, crop_image_path, candidate_a, candidate_b, resolved_value, status, audit_note, created_date)
};

struct Note {
    std::string id;
    std::string project_id = "default";
    std::string title;
    std::string content;
    std::vector<std::string> tags;
    std::string updated_date;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Note, id, project_id, title, content, tags, updated_date, created_date)
};

// -------------------------------------------------------------
// Contradiction & Thesis Audit Types
// -------------------------------------------------------------

struct Contradiction {
    std::string id;
    std::string project_id = "default";
    std::string type;       // Type 1: Chronological, Type 2: Interpretive, Type 3: Stratigraphic, Type 4: Cross-Site
    std::string severity;   // CRITICAL, HIGH, MEDIUM, LOW
    std::string title;
    std::string entity_name;
    std::string source_a;
    std::string claim_a;
    std::string source_b;
    std::string claim_b;
    json details;
    std::string resolution_guidance;
    bool is_confirmed = false;
    bool requires_grounding = false;
    std::string grounding_crop_path;
    std::string anomaly_description;
    std::string suggested_correction;
    std::string flagged_claim_id;
    std::string created_date;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Contradiction, id, project_id, type, severity, title, entity_name, source_a, claim_a, source_b, claim_b, details, resolution_guidance, is_confirmed, requires_grounding, grounding_crop_path, anomaly_description, suggested_correction, flagged_claim_id, created_date)
};

} // namespace archaeophd
