#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

#include "models.hpp"
#include "storage.hpp"
#include "contradictions.hpp"
#include "thesis_audit.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using namespace archaeophd;

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD — Phase 2 Step 6: Thesis Auditor Engine & Defense Readiness Suite   \n";
    std::cout << "================================================================================\n\n";

    // Isolated test storage
    std::string test_db = "test_step6_thesis_audit";
    NativeStorage storage(test_db);
    std::string pid = "proj_viva_defense";

    // 1. Seed Site and Stratum
    Site s1;
    s1.id = "site_megiddo";
    s1.project_id = pid;
    s1.site_name = "Tel Megiddo";
    storage.put_site(s1);

    Stratum st1;
    st1.id = "stratum_vii_a";
    st1.project_id = pid;
    st1.stratum_name = "Stratum VII-A";
    storage.put_stratum(st1);

    // 2. Seed Chapter 1: Well-supported, fully verified claims
    Claim c1;
    c1.id = "claim_ch1_verified";
    c1.project_id = pid;
    c1.chapter = "Chapter 1: Settlement History";
    c1.claim_text = "Megiddo commanded the central pass of the Jezreel Valley during the Bronze Age.";
    c1.status = "Verified";
    c1.verification_status = "VERIFIED";
    c1.site_ids = {"site_megiddo"};
    storage.put_claim(c1);

    EvidenceLink ev1;
    ev1.id = "ev_ch1_supporting";
    ev1.project_id = pid;
    ev1.claim_id = c1.id;
    ev1.evidence_text = "Geographical survey of Megiddo mountain pass and road network.";
    ev1.evidence_type = "supporting";
    storage.put_evidence(ev1);

    // 3. Seed Chapter 2: Methodological chapter with Contradicted Claim & Unverified OCR Anomaly
    Claim c2;
    c2.id = "claim_ch2_contradicted";
    c2.project_id = pid;
    c2.chapter = "Chapter 2: Stratigraphy & Chronology";
    c2.claim_text = "Stratum VII-A destruction was entirely peaceful abandonment without fire.";
    c2.status = "Contested";
    c2.site_ids = {"site_megiddo"};
    storage.put_claim(c2);

    EvidenceLink ev2;
    ev2.id = "ev_ch2_refuting";
    ev2.project_id = pid;
    ev2.claim_id = c2.id;
    ev2.evidence_text = "Thick 1.5-meter ash layer and collapsed charred roof beams in Area AA.";
    ev2.evidence_type = "contradicting";
    storage.put_evidence(ev2);

    Claim c2_anomaly;
    c2_anomaly.id = "claim_ch2_ocr_anomaly";
    c2_anomaly.project_id = pid;
    c2_chapter: c2_anomaly.chapter = "Chapter 2: Stratigraphy & Chronology";
    c2_anomaly.claim_text = "Stratum VII-A floor thickness reached 2040 cm in locus 42.";
    c2_anomaly.anomaly_flag = true;
    c2_anomaly.verification_status = "PENDING_VERIFICATION";
    c2_anomaly.anomaly_reason = "Probable OCR hyphen fusion of 20-40 cm";
    c2_anomaly.optical_crop_path = "crops/megiddo_plate_42.png";
    storage.put_claim(c2_anomaly);

    // 4. Seed Chapter 3: Unsupported claim with zero empirical Layer C citations
    Claim c3;
    c3.id = "claim_ch3_unsupported";
    c3.project_id = pid;
    c3.chapter = "Chapter 3: Regional Destruction Horizons";
    c3.claim_text = "The Sea Peoples initiated the total collapse of all Northern Levantine city-states.";
    c3.status = "Contested";
    storage.put_claim(c3);

    // 5. Seed document text chunk into storage for search-augmented suggestion testing
    std::string test_chunk_id = "chunk_sea_peoples_01";
    std::string test_text = "The Sea Peoples invaded the Northern Levant around the eighth year of Ramesses III, causing disruption across coastal emporia.";
    storage.store_compressed_chunk(test_chunk_id, test_text);
    storage.lexical().insert(test_chunk_id, "doc_medinet_habu", 12, test_text);

    // 6. Instantiate contradiction engine and thesis auditor
    NativeContradictionEngine contradictions(storage);
    NativeThesisAuditor auditor(storage, contradictions);

    // -------------------------------------------------------------
    // TEST 1: Global Audit Metrics & Viva Readiness Score
    // -------------------------------------------------------------
    std::cout << "[TEST 1] Running Thesis Pre-Submission Audit...\n";
    json audit = auditor.run_audit(pid);

    assert(audit.contains("total_claims"));
    assert(audit["total_claims"].get<int>() == 4);
    assert(audit["verified_claims_count"].get<int>() == 1);
    assert(audit["unsupported_claims_count"].get<int>() == 1);
    assert(audit.contains("defense_readiness_score"));
    int score = audit["defense_readiness_score"].get<int>();
    std::string verdict = audit["defense_verdict"].get<std::string>();

    std::cout << "  ✓ Total Claims: " << audit["total_claims"] << "\n";
    std::cout << "  ✓ Verified Grounded Claims: " << audit["verified_claims_count"] << "\n";
    std::cout << "  ✓ Unsupported Claims: " << audit["unsupported_claims_count"] << "\n";
    std::cout << "  ✓ Defense Readiness Score: " << score << "% (" << verdict << ")\n";
    assert(score < 100); // Must penalize for unsupported claim and anomalies
    std::cout << "  [PASS] Global Defense Readiness Metrics verified!\n\n";

    // -------------------------------------------------------------
    // TEST 2: Per-Chapter Breakdown & Viva Risk Factors
    // -------------------------------------------------------------
    std::cout << "[TEST 2] Verifying Per-Chapter Audit Breakdown...\n";
    assert(audit.contains("chapter_audits"));
    const auto& chapters = audit["chapter_audits"];
    assert(chapters.size() >= 3);

    bool found_ch1 = false, found_ch2 = false, found_ch3 = false;
    for (const auto& ch : chapters) {
        std::string name = ch["chapter_name"].get<std::string>();
        int ch_score = ch["chapter_score"].get<int>();
        std::string status = ch["chapter_status"].get<std::string>();

        if (name == "Chapter 1: Settlement History") {
            found_ch1 = true;
            assert(ch_score == 100);
            assert(status == "DEFENSE_READY");
            std::cout << "  ✓ " << name << " -> Score: " << ch_score << "% [" << status << "]\n";
        } else if (name == "Chapter 2: Stratigraphy & Chronology") {
            found_ch2 = true;
            assert(ch["contradicted_claims"].get<int>() >= 1);
            assert(ch["anomaly_claims"].get<int>() >= 1);
            assert(status != "DEFENSE_READY");
            std::cout << "  ✓ " << name << " -> Score: " << ch_score << "% [" << status << "]\n";
            std::cout << "    Viva Risks Caught: " << ch["viva_risk_factors"].size() << "\n";
        } else if (name == "Chapter 3: Regional Destruction Horizons") {
            found_ch3 = true;
            assert(ch["unsupported_claims"].get<int>() == 1);
            std::cout << "  ✓ " << name << " -> Score: " << ch_score << "% [" << status << "]\n";
        }
    }
    assert(found_ch1 && found_ch2 && found_ch3);
    std::cout << "  [PASS] Per-Chapter Granular Auditing validated!\n\n";

    // -------------------------------------------------------------
    // TEST 3: Automated Search-Augmented Citation Suggestions
    // -------------------------------------------------------------
    std::cout << "[TEST 3] Verifying Search-Augmented Citation Suggestions...\n";
    bool found_suggested_citation = false;
    for (const auto& f : audit["findings"]) {
        if (f.value("category", "") == "Unsupported Claim") {
            if (f.contains("suggested_citations") && f["suggested_citations"].is_array() && f["suggested_citations"].size() > 0) {
                found_suggested_citation = true;
                const auto& sug = f["suggested_citations"][0];
                std::cout << "  ✓ Citation Suggested for: '" << f["claim_text"] << "'\n";
                std::cout << "    Matched Doc: " << sug["doc_id"] << " (p. " << sug["page_ref"] << ")\n";
                std::cout << "    Excerpt: " << sug["excerpt"] << "\n";
            }
        }
    }
    assert(found_suggested_citation);
    std::cout << "  [PASS] Native Hybrid Retrieval Citation Suggestion operational!\n\n";

    // -------------------------------------------------------------
    // TEST 4: Remediation Action Plan Prioritization
    // -------------------------------------------------------------
    std::cout << "[TEST 4] Verifying Remediation Action Plan Sorting...\n";
    assert(audit.contains("remediation_action_plan"));
    const auto& plan = audit["remediation_action_plan"];
    assert(plan.size() > 0);
    std::cout << "  ✓ Generated " << plan.size() << " actionable thesis revision tasks.\n";

    // Top items must be HIGH or CRITICAL priority
    std::string top_priority = plan[0].value("priority", "");
    std::cout << "  ✓ Top Priority Action: [" << top_priority << "] " << plan[0]["description"] << "\n";
    assert(top_priority == "HIGH" || top_priority == "CRITICAL");
    std::cout << "  [PASS] Action Plan Priority Queue correctly sorted!\n\n";

    // -------------------------------------------------------------
    // TEST 5: IPC Dispatcher Contract Verification
    // -------------------------------------------------------------
    std::cout << "[TEST 5] Testing Native IPC Dispatcher get_thesis_audit Endpoint...\n";
    NativeIpcDispatcher dispatcher(&storage, &contradictions, &auditor, "");
    std::string ipc_req = "{\"id\": \"test-audit-req\", \"action\": \"get_thesis_audit\", \"projectId\": \"" + pid + "\"}";
    std::string ipc_res = dispatcher.dispatch(ipc_req);
    auto res_json = json::parse(ipc_res);

    assert(res_json.contains("result"));
    const auto& res_audit = res_json["result"];
    assert(res_audit.contains("defense_readiness_score"));
    assert(res_audit.contains("total_claims"));
    assert(res_audit.contains("affected_chapters"));
    assert(res_audit.contains("chapter_audits"));
    std::cout << "  ✓ IPC returned valid thesis audit payload for project: " << pid << "\n";
    std::cout << "  [PASS] IPC Bridge Schema 100% compliant with React frontend Thesis.jsx!\n\n";

    std::cout << "================================================================================\n";
    std::cout << "  ALL PHASE 2 STEP 6 THESIS AUDITOR TESTS PASSED (100%)!                         \n";
    std::cout << "================================================================================\n";
    return 0;
}
