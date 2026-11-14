#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include "models.hpp"
#include "storage.hpp"
#include "extraction/ingestion_manager.hpp"

using namespace archaeophd;
namespace fs = std::filesystem;

// ============================================================================
// Phase 2 Step 3 — Class B Automated Commit Gate & Safeguard Test
// 
// EPISTEMIC INVARIANT:
// Any physical measurement, quantitative range, chronological date, or locus
// extracted from a CLASS_B source (unverified historical letterpress or rough scan)
// MUST NEVER be committed to the authoritative Knowledge Graph automatically.
// Auto-commit attempts must fail with an explicit rejection, preserving the entity
// in the quarantined/pending review state until verified by a human researcher.
// ============================================================================

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine — Phase 2 Step 3: Class B Entity Commit Hard-Gate Test      \n";
    std::cout << "================================================================================\n\n";

    std::string test_dir = "test_class_b_gate_data";
    std::error_code ec;
    fs::remove_all(test_dir, ec);
    fs::create_directories(test_dir);

    NativeStorage storage(test_dir);

    // 1. Register a Class B source document (e.g. historical monograph scan)
    Source src_class_b;
    src_class_b.id = "src_sankalia_1974_letterpress";
    src_class_b.title = "Prehistory and Protohistory of India and Pakistan";
    src_class_b.author = "H.D. Sankalia";
    src_class_b.year = "1974";
    src_class_b.degradation_class = "CLASS_B"; // Unverified historical letterpress
    src_class_b.ingestion_status = "UNVERIFIED_ROUGH_SCAN";
    src_class_b.confirmed_clean_offset = false;
    storage.put_source(src_class_b);

    std::cout << "[TEST 1] Verify Source is Registered as Strict CLASS_B...\n";
    auto sources = storage.get_sources();
    assert(sources.size() == 1);
    assert(sources[0].degradation_class == "CLASS_B");
    assert(sources[0].confirmed_clean_offset == false);
    std::cout << "  [PASS] Source " << src_class_b.id << " confirmed CLASS_B.\n\n";

    // 2. Simulate extraction of a physical measurement from this Class B source
    // Passage: "Gravel lenses vary from 20-40 cm in total thickness."
    Claim extracted_quantity;
    extracted_quantity.id = "claim_quant_gravel_001";
    extracted_quantity.source_id = src_class_b.id;
    extracted_quantity.claim_text = "Gravel lenses thickness: 20-40 cm (range [0.20, 0.40] m)";
    extracted_quantity.origin_type = "scanned_ocr";
    extracted_quantity.verification_status = "PENDING_VERIFICATION";
    extracted_quantity.is_quantitative = true;
    extracted_quantity.anomaly_flag = false;

    std::cout << "[TEST 2] Attempt Automated Auto-Commit of Extracted Quantity on Class B...\n";
    // Attempting automated write (is_human_verified = false) MUST be blocked
    bool auto_commit_result = storage.put_claim_safeguarded(extracted_quantity, /*is_human_verified=*/false);
    
    assert(auto_commit_result == false);
    assert(storage.get_claims().empty());
    std::cout << "  [PASS] Automated auto-commit on Class B source was strictly rejected.\n";
    std::cout << "  [PASS] Authoritative Knowledge Graph claims ledger contains 0 unverified claims.\n\n";

    std::cout << "[TEST 3] Attempt Automated Auto-Commit with OCR Anomaly Flag...\n";
    // Passage with corrupted OCR: "2040 cm"
    Claim corrupted_quantity;
    corrupted_quantity.id = "claim_quant_silt_corrupt";
    corrupted_quantity.source_id = src_class_b.id;
    corrupted_quantity.claim_text = "Silt deposit depth: 2040 cm";
    corrupted_quantity.origin_type = "scanned_ocr";
    corrupted_quantity.verification_status = "PENDING_VERIFICATION";
    corrupted_quantity.is_quantitative = true;
    corrupted_quantity.anomaly_flag = true;
    corrupted_quantity.anomaly_reason = "FLAG_SUSPECTED_OCR_ANOMALY: unhyphenated large dimension without space";

    bool corrupt_auto_commit = storage.put_claim_safeguarded(corrupted_quantity, /*is_human_verified=*/false);
    assert(corrupt_auto_commit == false);
    assert(storage.get_claims().empty());
    std::cout << "  [PASS] Anomalous OCR quantity auto-commit rejected without corruption poisoning.\n\n";

    std::cout << "[TEST 4] Human-Verified Transcription Commit Gate (Manual Operator Clearance)...\n";
    // Human researcher inspects the physical crop, verifies the text, and commits
    Claim human_verified = extracted_quantity;
    human_verified.origin_type = "manual_transcription";
    human_verified.verification_status = "VERIFIED";

    bool human_commit_result = storage.put_claim_safeguarded(human_verified, /*is_human_verified=*/true);
    assert(human_commit_result == true);
    assert(storage.get_claims().size() == 1);
    assert(storage.get_claims()[0].id == extracted_quantity.id);
    assert(storage.get_claims()[0].is_quantitative == true);
    std::cout << "  [PASS] Human-verified claim successfully committed with operator signature.\n\n";

    // Teardown
    fs::remove_all(test_dir, ec);

    std::cout << "================================================================================\n";
    std::cout << "  ALL CLASS B ENTITY COMMIT GATE TESTS PASSED (100%)!                           \n";
    std::cout << "================================================================================\n";

    return 0;
}
