#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "models.hpp"
#include "storage.hpp"
#include "extraction/ingestion_manager.hpp"

using namespace archaeophd;

int main() {
#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
    EmbeddingEngine::instance().enable_test_mock_mode(true);
#endif
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine — Adversarial Ingestion Gating & Safeguard Test Suite       \n";
    std::cout << "================================================================================\n\n";

    std::string testDir = "test_adversarial_data";
    // Clean previous run
    std::error_code ec;
    std::filesystem::remove_all(testDir, ec);

    // Create dummy PDF file for ingestion tests
    std::string dummyPdf = "test_sample_paper.pdf";
    {
        std::ofstream f(dummyPdf, std::ios::binary);
        f << "%PDF-1.4 sample archaeological excavation report with stratigraphy data";
    }

    std::string sourceIdForUpgradeTest;

    {
        NativeStorage storage(testDir);
        std::cout << "[TEST 1] Ingestion Defaults strictly to Class B (Default-Safe)...\n";
        auto res = IngestionManager::IngestDocument(storage, dummyPdf, "Excavation Report", "Sankalia", "1974");
        assert(res.success == true);
        assert(res.degradation_class == "CLASS_B");
        assert(res.status == "UNVERIFIED_ROUGH_SCAN");
        sourceIdForUpgradeTest = res.source_id;

        auto sources = storage.get_sources();
        assert(sources.size() == 1);
        assert(sources[0].degradation_class == "CLASS_B");
        assert(sources[0].confirmed_clean_offset == false);
        std::cout << "  ✓ Ingested document automatically defaults to CLASS_B with zero automated writes.\n\n";

        std::cout << "[TEST 2] Malformed & Date-Heuristic Class Strings Fail-Safe to Class B...\n";
        std::vector<std::string> malformedInputs = {"", "CLASS_C", "1980", ">1980", "MODERN_OFFSET", "random_junk"};
        for (const auto& badInput : malformedInputs) {
            std::string err;
            bool ok = IngestionManager::ClassifySource(storage, res.source_id, badInput, false, err);
            auto sList = storage.get_sources();
            assert(sList[0].degradation_class == "CLASS_B");
        }
        std::cout << "  ✓ 100% of malformed inputs ('', '1980', 'CLASS_C', etc.) fail-safe strictly to CLASS_B.\n\n";

        std::cout << "[TEST 3] Class A Rejection Without Explicit Physical Print Confirmation...\n";
        std::string err;
        bool ok = IngestionManager::ClassifySource(storage, res.source_id, "CLASS_A", false, err);
        assert(ok == false);
        assert(!err.empty());
        assert(storage.get_sources()[0].degradation_class == "CLASS_B");
        std::cout << "  ✓ Unconfirmed Class A upgrade blocked: '" << err << "'\n\n";

        std::cout << "[TEST 4] Direct Automated Write Injection on Class B is Programmatically Blocked...\n";
        Claim c1;
        c1.id = "claim_injected_1";
        c1.source_id = res.source_id;
        c1.claim_text = "Trench VII depth: 20-40 cm";
        c1.origin_type = "scanned_ocr";
        c1.verification_status = "PENDING_VERIFICATION"; // Machine unconfirmed
        bool writeSuccess = storage.put_claim_safeguarded(c1, false);
        assert(writeSuccess == false);
        assert(storage.get_claims().empty());
        std::cout << "  ✓ Attempted automated claim write to Class B rejected by storage hard-gate.\n\n";

        std::cout << "[TEST 5] Valid Class A Upgrade and Dual-Engine Verification Routing...\n";
        ok = IngestionManager::ClassifySource(storage, res.source_id, "CLASS_A", true, err);
        assert(ok == true);
        assert(storage.get_sources()[0].degradation_class == "CLASS_A");
        assert(storage.get_sources()[0].confirmed_clean_offset == true);

        // Feed candidate facts (1 consensus, 1 disagreement)
        ExtractedFact f1;
        f1.fact_id = "fact_001";
        f1.entity_name = "Chirki Discovery Year";
        f1.value_engine_a = "1963";
        f1.value_engine_b = "1963"; // Exact consensus!

        ExtractedFact f2;
        f2.fact_id = "fact_002";
        f2.entity_name = "Rubble Thickness";
        f2.value_engine_a = "2040 cm";
        f2.value_engine_b = "20-40 cm"; // Disagreement!

        IngestionManager::ProcessClassAFacts(storage, res.source_id, {f1, f2});

        // Also add 1 prior manual transcription to verify selective survival
        Claim manualPreReflag;
        manualPreReflag.id = "manual_prior_001";
        manualPreReflag.source_id = res.source_id;
        manualPreReflag.claim_text = "Excavator: Gudrun Corvinus";
        manualPreReflag.origin_type = "manual_transcription";
        manualPreReflag.verification_status = "VERIFIED";
        bool manualSaved = storage.put_claim_safeguarded(manualPreReflag, true);
        assert(manualSaved == true);

        // Verify: 0 machine claims auto-committed (auto-commit paused); 1 manual claim committed;
        // both consensus and discrepancy routed to verification queue
        assert(storage.get_claims().size() == 1);
        assert(storage.get_claims()[0].id == "manual_prior_001");
        assert(storage.get_verification_items().size() == 2);
        assert(storage.get_verification_items()[0].status == "PENDING");
        assert(storage.get_verification_items()[1].status == "PENDING");

        // ADVERSARIAL TEST: Attempt direct auto-commit of unverified dual-consensus claim (MUST BE BLOCKED)
        Claim unverifiedConsensusClaim;
        unverifiedConsensusClaim.id = "claim_consensus_adversarial_001";
        unverifiedConsensusClaim.source_id = res.source_id;
        unverifiedConsensusClaim.claim_text = "Chirki Discovery Year: 1963";
        unverifiedConsensusClaim.origin_type = "scanned_ocr_dual_consensus";
        unverifiedConsensusClaim.verification_status = "AUTO_ACCEPTED_CONSENSUS";
        bool autoCommitBlocked = storage.put_claim_safeguarded(unverifiedConsensusClaim, false);
        assert(autoCommitBlocked == false); // MUST FAIL: Auto-commit is paused!
        std::cout << "  ✓ Class A auto-commit paused: 0 machine claims auto-committed; both consensus & discrepancy queued.\n";
        std::cout << "  ✓ Direct unverified consensus auto-commit attempt was programmatically blocked.\n\n";

        std::cout << "[TEST 6] RETROACTIVE PURGE on Mid-Session Reflag (A -> B)...\n";
        // Researcher notices bleed-through on page 41 and clicks 1-click reflag to Class B!
        bool reflagged = IngestionManager::ReflagSourceToClassB(storage, res.source_id);
        assert(reflagged == true);
        assert(storage.get_sources()[0].degradation_class == "CLASS_B");
        assert(storage.get_sources()[0].confirmed_clean_offset == false);

        // Verify retroactive purge:
        // 1. The human manual transcription ('manual_prior_001') MUST SURVIVE!
        auto claimsAfterReflag = storage.get_claims();
        assert(claimsAfterReflag.size() == 1);
        assert(claimsAfterReflag[0].id == "manual_prior_001");
        assert(claimsAfterReflag[0].origin_type == "manual_transcription");

        // Verify pending verification items are purged / rejected
        auto vItems = storage.get_verification_items();
        for (const auto& vi : vItems) {
            assert(vi.status == "REJECTED_DUE_TO_RECLASSIFICATION");
        }

        // Verify that subsequent automated writes are instantly rejected
        bool writeAfterReflag = storage.put_claim_safeguarded(c1, false);
        assert(writeAfterReflag == false);
        std::cout << "  ✓ Retroactive purge wiped unverified items while preserving human double-entry.\n\n";

        std::cout << "[TEST 7] Search Indexing vs Knowledge Graph Truth-Plane Isolation...\n";
        // Rough text indexed for discovery
        int chunks = IngestionManager::IndexRoughTextForSearch(
            storage,
            res.source_id,
            {{53, "In trench VII the rubble horizon overlying trap basalt was 20-40 cm thick and produced 694 early stone age tools."}}
        );
        assert(chunks > 0);
        // Verify vector index has chunks
        auto queryEmb = VectorIndex::embed_text("rubble horizon tools");
        auto searchMatches = storage.vectors().search(queryEmb, 1);
        assert(!searchMatches.empty());

        // BUT assert Knowledge Graph remains strictly protected
        // Only the 1 manual claim that survived the reflag exists
        assert(storage.get_claims().size() == 1);
        std::cout << "  ✓ Rough scan indexed for semantic search while Knowledge Graph remains strictly protected.\n\n";

        std::cout << "[TEST 8] Manual Transcription Commits Clean Grounded Facts (No Pre-Fill)...\n";
        json manualFacts = json::array({
            {{"entity_name", "Trench VII Rubble Thickness"}, {"value", "20-40 cm"}, {"type", "measurement"}},
            {{"entity_name", "Early Stone Age Tools Count"}, {"value", "694"}, {"type", "count"}}
        });
        bool manualSaved2 = IngestionManager::SaveManualTranscription(storage, res.source_id, 53, manualFacts);
        assert(manualSaved2 == true);
        assert(storage.get_sources()[0].ingestion_status == "VERIFIED_MANUAL");

        auto finalClaims = storage.get_claims();
        assert(finalClaims.size() == 3); // 1 previous manual + 2 new manual
        bool foundManualCount = false;
        for (const auto& c : finalClaims) {
            if (c.claim_text == "Early Stone Age Tools Count: 694") {
                foundManualCount = true;
                assert(c.origin_type == "manual_transcription");
                assert(c.verification_status == "VERIFIED");
            }
        }
        assert(foundManualCount == true);
        std::cout << "  ✓ Manual transcription recorded verbatim human-entered facts into Knowledge Graph.\n\n";
    }

    std::cout << "[TEST 9] Crash/Restart State Recovery Preserves Class B Gating from Disk...\n";
    {
        // Re-open brand new NativeStorage instance from the saved directory
        NativeStorage recoveredStorage(testDir);
        auto sources = recoveredStorage.get_sources();
        assert(sources.size() == 1);
        assert(sources[0].degradation_class == "CLASS_B");
        assert(sources[0].confirmed_clean_offset == false);
        assert(sources[0].ingestion_status == "VERIFIED_MANUAL");

        // Attempt automated injection on recovered instance
        Claim badClaim;
        badClaim.id = "bad_after_crash";
        badClaim.source_id = sources[0].id;
        badClaim.claim_text = "Phantom Date 1656";
        badClaim.origin_type = "scanned_ocr";
        badClaim.verification_status = "PENDING";
        bool badWrite = recoveredStorage.put_claim_safeguarded(badClaim, false);
        assert(badWrite == false);
        std::cout << "  ✓ Recovered state across simulated app crash retained Class B hard-gate.\n\n";
    }

    std::cout << "[TEST 10] Interrupted Ingestion Atomicity (IO Failure Cleanup)...\n";
    {
        NativeStorage storage(testDir);
        size_t sourceCountBefore = storage.get_sources().size();

        // Attempt ingesting a non-existent file
        auto failedRes = IngestionManager::IngestDocument(
            storage,
            "non_existent_file_path_12345.pdf",
            "Ghost Document",
            "Ghost Author",
            "2026"
        );
        assert(failedRes.success == false);
        assert(!failedRes.message.empty());

        // Assert zero orphaned source records were committed
        auto sources = storage.get_sources();
        assert(sources.size() == sourceCountBefore);

        // Verify archives directory does not contain lingering .tmp files
        std::string archivesDir = testDir + "/archives";
        if (std::filesystem::exists(archivesDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(archivesDir)) {
                assert(entry.path().extension() != ".tmp");
            }
        }
        std::cout << "  ✓ Failed ingestion rolled back atomically with zero orphaned records or leaked temp files.\n\n";
    }

    std::cout << "[TEST 11] Upgrading B -> A Mid-Session Preserves All Grounded Manual Facts...\n";
    {
        NativeStorage storage(testDir);
        assert(!sourceIdForUpgradeTest.empty());

        // Source currently has 3 manual claims
        assert(storage.get_claims().size() == 3);

        // Upgrade source to Class A with physical print confirmation
        std::string err;
        bool upgraded = IngestionManager::ClassifySource(storage, sourceIdForUpgradeTest, "CLASS_A", true, err);
        assert(upgraded == true);
        assert(storage.get_sources()[0].degradation_class == "CLASS_A");

        // Verify that all 3 manual claims remain completely intact and VERIFIED
        auto claims = storage.get_claims();
        assert(claims.size() == 3);
        for (const auto& c : claims) {
            assert(c.origin_type == "manual_transcription");
            assert(c.verification_status == "VERIFIED");
        }
        std::cout << "  ✓ Upgrading B -> A preserved 100% of human-transcribed claims without alteration.\n\n";
    }

    // Clean up test file and directory
    std::filesystem::remove(dummyPdf, ec);
    std::filesystem::remove_all(testDir, ec);

    std::cout << "================================================================================\n";
    std::cout << "  ALL 11 ADVERSARIAL INGESTION & GATING TESTS PASSED WITH ZERO FAILURES!        \n";
    std::cout << "================================================================================\n";
    return 0;
}
