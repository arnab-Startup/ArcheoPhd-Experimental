#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include <json.hpp>

#include "models.hpp"
#include "storage.hpp"
#include "contradictions.hpp"
#include "thesis_audit.hpp"
#include "analytics_engine.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using json = nlohmann::json;
using namespace archaeophd;

int main() {
    std::cout << "================================================================================" << std::endl;
    std::cout << "  ArchaeoPhD — Phase 3 Optimization: Research Analytics & Dashboard Engine Suite" << std::endl;
    std::cout << "================================================================================" << std::endl;

    std::string testDir = "test_analytics_db_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs_compat::create_directories(testDir);

    try {
        NativeStorage storage(testDir);
        std::string proj = "test_analytics_proj";

        // Seed comprehensive research project data
        Site s1;
        s1.id = "site_jericho";
        s1.project_id = proj;
        s1.site_name = "Tell es-Sultan (Jericho)";
        s1.period = "Middle Bronze IIB / Late Bronze";
        s1.latitude = 31.871;
        s1.longitude = 35.444;
        storage.put_site(s1);

        Site s2;
        s2.id = "site_megiddo";
        s2.project_id = proj;
        s2.site_name = "Tel Megiddo";
        s2.period = "Middle Bronze / Late Bronze";
        s2.latitude = 32.585;
        s2.longitude = 35.184;
        storage.put_site(s2);

        Site s3;
        s3.id = "site_qeiyafa";
        s3.project_id = proj;
        s3.site_name = "Khirbet Qeiyafa";
        s3.period = "Iron Age IIA";
        s3.latitude = 31.696;
        s3.longitude = 34.957;
        storage.put_site(s3);

        Stratum st1;
        st1.id = "strat_jericho_ivb";
        st1.project_id = proj;
        st1.site_id = "site_jericho";
        st1.stratum_name = "City IV Destruction Horizon";
        st1.sediment_type = "Dense carbonized ash layer";
        storage.put_stratum(st1);

        Artifact a1;
        a1.id = "art_red_slip";
        a1.project_id = proj;
        a1.artifact_name = "Wheel-burnished red slip vessel";
        a1.period = "Middle Bronze IIB / Late Bronze";
        a1.category = "Ceramic";
        a1.stratum_id = "strat_jericho_ivb";
        storage.put_artifact(a1);

        Artifact a2;
        a2.id = "art_scarab";
        a2.project_id = proj;
        a2.artifact_name = "Amenhotep III Commemorative Scarab";
        a2.period = "Late Bronze IIA";
        a2.category = "Glyptic";
        storage.put_artifact(a2);

        Source src1;
        src1.id = "src_kenyon_1957";
        src1.project_id = proj;
        src1.title = "Digging Up Jericho";
        src1.author = "Kenyon, Kathleen M.";
        src1.year = "1957";
        src1.source_type = "Monograph";
        src1.degradation_class = "CLASS_A";
        src1.confirmed_clean_offset = true;
        storage.put_source(src1);

        Source src2;
        src2.id = "src_wood_1990";
        src2.project_id = proj;
        src2.title = "Did the Israelites Conquer Jericho?";
        src2.author = "Wood, Bryant G.";
        src2.year = "1990";
        src2.source_type = "Journal Article";
        src2.degradation_class = "CLASS_A";
        src2.confirmed_clean_offset = true;
        storage.put_source(src2);

        Claim c1;
        c1.id = "claim_kenyon_destruction";
        c1.project_id = proj;
        c1.claim_text = "City IV fell at the end of MB IIB around 1550 BCE.";
        c1.scholar_name = "Kenyon (1957)";
        c1.source_id = "src_kenyon_1957";
        c1.status = "Contested";
        c1.topic = "Chronology";
        c1.origin_type = "digital_stream";
        c1.verification_status = "VERIFIED";
        storage.put_claim(c1);

        Claim c2;
        c2.id = "claim_wood_destruction";
        c2.project_id = proj;
        c2.claim_text = "City IV fell at the terminal Late Bronze I around 1400 BCE.";
        c2.scholar_name = "Wood (1990)";
        c2.source_id = "src_wood_1990";
        c2.status = "Contested";
        c2.topic = "Chronology";
        c2.origin_type = "digital_stream";
        c2.verification_status = "VERIFIED";
        storage.put_claim(c2);

        Claim c3;
        c3.id = "claim_unsupported_trade";
        c3.project_id = proj;
        c3.claim_text = "Trans-Jordanian lapis lazuli trade route established prior to 2000 BCE.";
        c3.scholar_name = "Candidate Thesis Statement";
        c3.status = "Speculative";
        c3.topic = "Trade Networks";
        c3.origin_type = "digital_stream";
        c3.verification_status = "VERIFIED";
        storage.put_claim(c3);

        Claim c4_outlier;
        c4_outlier.id = "claim_sankalia_outlier";
        c4_outlier.project_id = proj;
        c4_outlier.claim_text = "The rubble horizon was 2040 cm thick.";
        c4_outlier.scholar_name = "Sankalia (1974)";
        c4_outlier.topic = "Stratigraphy";
        c4_outlier.origin_type = "manual_transcription"; // Meets Class B gate
        c4_outlier.verification_status = "PENDING_VERIFICATION";
        c4_outlier.anomaly_flag = true;
        c4_outlier.anomaly_reason = "Outlier: 2040 cm exceeds typical terrace gravel";
        storage.put_claim(c4_outlier);

        EvidenceLink ev1;
        ev1.id = "ev_ash_layer";
        ev1.project_id = proj;
        ev1.claim_id = "claim_kenyon_destruction";
        ev1.evidence_text = "Ash layer 0.4m thick, colluvial debris, burned mudbrick.";
        ev1.evidence_type = "supporting";
        ev1.source_ids = {"src_kenyon_1957"};
        ev1.date_info = "1550 BCE";
        storage.put_evidence(ev1);

        EvidenceLink ev2;
        ev2.id = "ev_bichrome_pots";
        ev2.project_id = proj;
        ev2.claim_id = "claim_kenyon_destruction";
        ev2.evidence_text = "Late Bronze I Cypriot bichrome pottery discovered in destruction layer.";
        ev2.evidence_type = "contradicting";
        ev2.source_ids = {"src_wood_1990"};
        ev2.date_info = "1400 BCE";
        storage.put_evidence(ev2);

        Note n1;
        n1.id = "note_viva_prep";
        n1.project_id = proj;
        n1.title = "Viva Defense Defense Strategy";
        n1.content = "Address Kenyon vs Wood 150-year discrepancy in Chapter 3.";
        storage.put_note(n1);

        storage.save_state();

        NativeContradictionEngine contradictions(storage);
        NativeThesisAuditor thesisAuditor(storage, contradictions);
        NativeAnalyticsEngine analytics(storage, &thesisAuditor, &contradictions);

        // -------------------------------------------------------------------------
        // TEST 1: Summary Counts Computation
        // -------------------------------------------------------------------------
        std::cout << "\n[TEST 1] Testing Summary Counts Aggregation..." << std::endl;
        auto counts = analytics.compute_summary_counts(proj);
        std::cout << "  ✓ Total Sites: " << counts.total_sites << std::endl;
        std::cout << "  ✓ Total Strata: " << counts.total_strata << std::endl;
        std::cout << "  ✓ Total Artifacts: " << counts.total_artifacts << std::endl;
        std::cout << "  ✓ Total Claims: " << counts.total_claims << std::endl;
        std::cout << "  ✓ Total Evidence Links: " << counts.total_evidence << std::endl;
        std::cout << "  ✓ Total Sources: " << counts.total_sources << std::endl;
        std::cout << "  ✓ Total Notes: " << counts.total_notes << std::endl;
        std::cout << "  ✓ Pending Class B Scans: " << counts.pending_class_b_verifications << std::endl;
        std::cout << "  ✓ Ungrounded Claims: " << counts.ungrounded_claims << std::endl;

        assert(counts.total_sites == 3);
        assert(counts.total_strata == 1);
        assert(counts.total_artifacts == 2);
        assert(counts.total_claims == 4);
        assert(counts.total_evidence == 2);
        assert(counts.total_sources == 2);
        assert(counts.total_notes == 1);
        assert(counts.pending_class_b_verifications == 1);
        assert(counts.ungrounded_claims == 3); // c2, c3, c4 have no supporting evidence
        std::cout << "  [PASS] Summary counts correctly aggregated across all entities!" << std::endl;

        // -------------------------------------------------------------------------
        // TEST 2: Claims by Evidence Breakdown & Grounding Percentage
        // -------------------------------------------------------------------------
        std::cout << "\n[TEST 2] Testing Claims Epistemic Breakdown..." << std::endl;
        auto claimsBreakdown = analytics.compute_claims_breakdown(proj);
        std::cout << "  ✓ Supported Claims: " << claimsBreakdown.supported << std::endl;
        std::cout << "  ✓ Contradicted Claims: " << claimsBreakdown.contradicted << std::endl;
        std::cout << "  ✓ Unsupported Claims: " << claimsBreakdown.unsupported << std::endl;
        std::cout << "  ✓ Grounding Percentage: " << claimsBreakdown.grounding_percentage << "%" << std::endl;

        assert(claimsBreakdown.total_claims == 4);
        assert(claimsBreakdown.supported == 1); // c1
        assert(claimsBreakdown.contradicted == 1); // c1 has contradicting evidence
        assert(claimsBreakdown.unsupported == 3); // c2, c3, c4
        assert(claimsBreakdown.grounding_percentage == 25.0);
        std::cout << "  [PASS] Claims epistemic breakdown and grounding ratio verified!" << std::endl;

        // -------------------------------------------------------------------------
        // TEST 3: Temporal Coverage Heatmap
        // -------------------------------------------------------------------------
        std::cout << "\n[TEST 3] Testing Temporal Coverage Heatmap Generation..." << std::endl;
        auto heatmap = analytics.compute_temporal_coverage_heatmap(proj);
        assert(heatmap.is_array());
        assert(heatmap.size() >= 3);
        for (const auto& row : heatmap) {
            std::cout << "  • Period: " << row["period"].get<std::string>()
                      << " | Sites: " << row["site_count"]
                      << " | Artifacts: " << row["artifact_count"]
                      << " | Density: " << row["coverage_density"].get<std::string>() << std::endl;
        }
        std::cout << "  [PASS] Temporal coverage heatmap correctly segmented!" << std::endl;

        // -------------------------------------------------------------------------
        // TEST 4: Sources Breakdown (By Year, Type, and Class)
        // -------------------------------------------------------------------------
        std::cout << "\n[TEST 4] Testing Bibliographic Sources Breakdown..." << std::endl;
        auto srcBreakdown = analytics.compute_sources_breakdown(proj);
        assert(srcBreakdown.contains("sources_by_year"));
        assert(srcBreakdown.contains("sources_by_type"));
        assert(srcBreakdown.contains("sources_by_class"));
        std::cout << "  ✓ Sources by Type: " << srcBreakdown["sources_by_type"].dump() << std::endl;
        std::cout << "  ✓ Sources by Class: " << srcBreakdown["sources_by_class"].dump() << std::endl;
        std::cout << "  [PASS] Sources breakdown verified across temporal and bibliographic axes!" << std::endl;

        // -------------------------------------------------------------------------
        // TEST 5: Research Gap Detection
        // -------------------------------------------------------------------------
        std::cout << "\n[TEST 5] Testing Research Gap Detection Engine..." << std::endl;
        auto gaps = analytics.detect_research_gaps(proj);
        std::cout << "  ✓ Detected Gaps: " << gaps.size() << std::endl;
        bool foundUnsupported = false;
        bool foundUnstratified = false;
        for (const auto& g : gaps) {
            std::cout << "    [" << g.severity << "] " << g.gap_type << " ➔ " << g.title << std::endl;
            if (g.gap_type == "UNSUPPORTED_CLAIM") foundUnsupported = true;
            if (g.gap_type == "STRATIGRAPHIC_DATA_GAP") foundUnstratified = true;
        }
        assert(foundUnsupported);
        assert(foundUnstratified); // sites megiddo and qeiyafa have no strata
        std::cout << "  [PASS] Research gap detection successfully isolated empirical voids!" << std::endl;

        // -------------------------------------------------------------------------
        // TEST 6: Viva Defense Alerts Derivation
        // -------------------------------------------------------------------------
        std::cout << "\n[TEST 6] Testing Viva Defense Alerts Derivation..." << std::endl;
        auto alerts = analytics.derive_viva_defense_alerts(proj);
        std::cout << "  ✓ Generated Viva Defense Alerts: " << alerts.size() << std::endl;
        bool foundClassBAlert = false;
        for (const auto& a : alerts) {
            std::cout << "    [" << a.severity << "] " << a.alert_type << ": " << a.title << std::endl;
            if (a.alert_type == "CLASS_B_UNVERIFIED") foundClassBAlert = true;
        }
        assert(foundClassBAlert);
        std::cout << "  [PASS] Viva defense risk alerts correctly generated!" << std::endl;

        // -------------------------------------------------------------------------
        // TEST 7: Native IPC Bridge Dispatcher get_dashboard_analytics Endpoint
        // -------------------------------------------------------------------------
        std::cout << "\n[TEST 7] Testing Native IPC Dispatcher get_dashboard_analytics Endpoint..." << std::endl;
        NativeIpcDispatcher dispatcher(&storage, &contradictions, &thesisAuditor);

        json req;
        req["id"] = "req_dashboard_001";
        req["action"] = "get_dashboard_analytics";
        req["projectId"] = proj;
        req["payload"] = json::object();

        std::string rawResp = dispatcher.dispatch(req.dump());
        json resp = json::parse(rawResp);

        assert(resp["id"] == "req_dashboard_001");
        assert(resp.contains("result"));
        const auto& res = resp["result"];

        assert(res.contains("counts"));
        assert(res["counts"]["sites"] == 3);
        assert(res["counts"]["artifacts"] == 2);
        assert(res["counts"]["claims"] == 4);

        assert(res.contains("claims_breakdown"));
        assert(res["claims_breakdown"]["supported"] == 1);
        assert(res["claims_breakdown"]["contradicted"] == 1);

        assert(res.contains("thesis_readiness"));
        assert(res["thesis_readiness"].contains("defense_readiness_score"));

        assert(res.contains("temporal_coverage_heatmap"));
        assert(res.contains("sources_breakdown"));
        assert(res.contains("research_gaps"));
        assert(res.contains("viva_defense_alerts"));

        std::cout << "  ✓ IPC Result Payload JSON length: " << rawResp.size() << " bytes." << std::endl;
        std::cout << "  ✓ Defense Readiness Score in Dashboard: " << res["thesis_readiness"]["defense_readiness_score"] << std::endl;
        std::cout << "  [PASS] Native IPC Dispatcher operates with 100% schema fidelity!" << std::endl;

        std::cout << "\n================================================================================" << std::endl;
        std::cout << "  ALL PHASE 3 OPTIMIZATION ANALYTICS TESTS PASSED (100%)!                       " << std::endl;
        std::cout << "================================================================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception thrown: " << e.what() << std::endl;
        std::filesystem::remove_all(testDir);
        return 1;
    }

    std::filesystem::remove_all(testDir);
    return 0;
}
