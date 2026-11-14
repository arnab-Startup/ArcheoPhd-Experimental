#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "storage.hpp"
#include "vector_index.hpp"
#include "analysis/contradictions.hpp"
#include "analysis/thesis_audit.hpp"
#include "extraction/ingestion_manager.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using namespace archaeophd;
namespace fs = std::filesystem;

void run_test(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "  [PASS] " << name << "\n";
    } else {
        std::cerr << "  [FAIL] " << name << "\n";
        exit(1);
    }
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine — Step 4A: VectorIndex Restart Durability & Lifecycle Suite \n";
    std::cout << "================================================================================\n\n";

#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
    // Explicit opt-in for unit test mock hashing mode (durability testing without 80MB weights)
    EmbeddingEngine::instance().enable_test_mock_mode(true);
#endif

    std::string test_dir = "test_durability_data";
    std::error_code ec;
    fs::remove_all(test_dir, ec);

    // -------------------------------------------------------------------------
    // TEST 1: Clean Startup on Non-Existent Store
    // -------------------------------------------------------------------------
    std::cout << "[TEST 1] Clean Startup on Non-Existent Store...\n";
    {
        NativeStorage storage(test_dir);
        run_test("VectorIndex starts with size 0", storage.vectors().size() == 0);
    }

    // -------------------------------------------------------------------------
    // TEST 2: Ingestion & Atomic Persistence of vectors.bin
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 2] Ingestion & Atomic Persistence of vectors.bin...\n";
    std::vector<VectorIndex::SearchResult> baseline_results;
    std::vector<float> query_emb;
    std::vector<float> emb1, emb2, emb3;
    {
        NativeStorage storage(test_dir);

        // Create 3 realistic archaeological vectors
        emb1 = VectorIndex::embed_text("Acheulian rubble trench boulder gravels Chirki basalt");
        emb2 = VectorIndex::embed_text("Tell es-Sultan Middle Bronze destruction burnt brick wall Kenyon");
        emb3 = VectorIndex::embed_text("Late Bronze bichrome ware Jericho ceramic sequence Wood");

        storage.vectors().insert("chunk_chirki_001", "src_sankalia_1974", 53, emb1);
        storage.vectors().insert("chunk_jericho_kenyon", "src_kenyon_1981", 104, emb2);
        storage.vectors().insert("chunk_jericho_wood", "src_wood_1990", 212, emb3);

        run_test("VectorIndex has 3 records in memory", storage.vectors().size() == 3);

        // Run query before saving
        query_emb = VectorIndex::embed_text("Chirki Acheulian basalt gravel");
        baseline_results = storage.vectors().search(query_emb, 3);
        run_test("Baseline search returned 3 results", baseline_results.size() == 3);
        run_test("Top match is Chirki before shutdown", baseline_results[0].chunk_id == "chunk_chirki_001");

        // Trigger persistence
        storage.save_state();

        std::string vpath = test_dir + "/vectors.bin";
        run_test("vectors.bin exists on disk", fs::exists(vpath));
        run_test("vectors.bin is non-zero bytes", fs::file_size(vpath) > 0);

        // Verify magic header on disk
        std::ifstream bin_in(vpath, std::ios::binary);
        char magic[4] = {0};
        bin_in.read(magic, 4);
        run_test("vectors.bin has APV1 magic header", magic[0] == 'A' && magic[1] == 'P' && magic[2] == 'V' && magic[3] == '1');
        bin_in.close();
    } // Storage instance completely destroyed here (simulating process termination)

    // -------------------------------------------------------------------------
    // TEST 3: Process Restart & 100% Bit-for-Bit Vector Recovery
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 3] Process Restart & 100% Bit-for-Bit Vector Recovery...\n";
    {
        // Re-instantiate storage from scratch pointing to the same data directory
        NativeStorage storage_restarted(test_dir);

        run_test("Restarted storage restored all 3 vectors", storage_restarted.vectors().size() == 3);

        // Execute identical query against restored vector store
        auto post_restart_results = storage_restarted.vectors().search(query_emb, 3);
        run_test("Post-restart search returned 3 results", post_restart_results.size() == 3);

        for (size_t i = 0; i < 3; ++i) {
            run_test("Rank " + std::to_string(i) + " chunk_id matches identically",
                     post_restart_results[i].chunk_id == baseline_results[i].chunk_id);
            run_test("Rank " + std::to_string(i) + " doc_id matches identically",
                     post_restart_results[i].doc_id == baseline_results[i].doc_id);
            run_test("Rank " + std::to_string(i) + " page_ref matches identically",
                     post_restart_results[i].page_ref == baseline_results[i].page_ref);
            run_test("Rank " + std::to_string(i) + " cosine score is bit-for-bit exact (diff == 0.0f)",
                     post_restart_results[i].score == baseline_results[i].score);
        }

        const auto& recs = storage_restarted.vectors().records();
        run_test("Vector 0 raw bytes match pre-shutdown memory bit-for-bit (memcmp == 0)",
                 std::memcmp(recs[0].embedding.data(), emb1.data(), VECTOR_DIM * sizeof(float)) == 0);
        run_test("Vector 1 raw bytes match pre-shutdown memory bit-for-bit (memcmp == 0)",
                 std::memcmp(recs[1].embedding.data(), emb2.data(), VECTOR_DIM * sizeof(float)) == 0);
        run_test("Vector 2 raw bytes match pre-shutdown memory bit-for-bit (memcmp == 0)",
                 std::memcmp(recs[2].embedding.data(), emb3.data(), VECTOR_DIM * sizeof(float)) == 0);
    }

    // -------------------------------------------------------------------------
    // TEST 4: Atomic Mutation & Append Across Restarts
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 4] Atomic Mutation & Append Across Restarts...\n";
    {
        NativeStorage storage_session2(test_dir);
        std::vector<float> emb4 = VectorIndex::embed_text("Hazor Solomonic gate fortification phase Stratum X");
        storage_session2.vectors().insert("chunk_hazor_001", "src_yadin_1972", 73, emb4);
        storage_session2.save_state();
    }
    {
        NativeStorage storage_session3(test_dir);
        run_test("Session 3 restored 4 vectors after mutation", storage_session3.vectors().size() == 4);
    }

    // -------------------------------------------------------------------------
    // TEST 5: Adversarial Malformed / Corrupt vectors.bin Header Resilience
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 5] Adversarial Corrupt vectors.bin Header Resilience...\n";
    {
        std::string corrupt_dir = "test_corrupt_data";
        fs::remove_all(corrupt_dir, ec);
        fs::create_directories(corrupt_dir);

        // Write a garbage/corrupted file as vectors.bin
        std::ofstream corrupt_file(corrupt_dir + "/vectors.bin", std::ios::binary);
        corrupt_file << "CORRUPT_NOT_APV1_HEADER_1234567890";
        corrupt_file.close();

        // NativeStorage must not crash, must handle corruption gracefully, and start with empty index
        NativeStorage storage_corrupt(corrupt_dir);
        run_test("Corrupted file handled safely with zero crashes", true);
        run_test("Corrupted vector index resets safely to size 0", storage_corrupt.vectors().size() == 0);

        fs::remove_all(corrupt_dir, ec);
    }

    // -------------------------------------------------------------------------
    // TEST 6: Dual-Write Out-of-Sync & Orphaned Vector Resilience
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 6] Dual-Write Out-of-Sync & Orphaned Vector Resilience...\n";
    {
        std::string sync_dir = "test_sync_data";
        fs::remove_all(sync_dir, ec);
        fs::create_directories(sync_dir);

        // Step 1: Normal storage with 1 source and 1 vector
        {
            NativeStorage s(sync_dir);
            Source src;
            src.id = "src_normal_01";
            src.title = "Normal Document";
            s.put_source(src);
            s.vectors().insert("chunk_normal_01", "src_normal_01", 1, VectorIndex::embed_text("normal text"));
            s.save_state();
        }

        // Step 2: Simulate crash between vectors.bin write and relational_state.json write.
        // We write an extra vector directly into vectors.bin that has no corresponding source in relational_state.
        {
            VectorIndex orphan_index;
            orphan_index.load(sync_dir + "/vectors.bin");
            orphan_index.insert("chunk_orphan_99", "src_ghost_missing", 99, VectorIndex::embed_text("orphan uncommitted"));
            orphan_index.save(sync_dir + "/vectors.bin");
        }

        // Step 3: Reload storage. Verify storage loads safely, preserves relational authority,
        // and does NOT hallucinate sources or crash when resolving chunks.
        {
            NativeStorage s_recovered(sync_dir);
            run_test("Vector store loaded both normal and orphaned vectors", s_recovered.vectors().size() == 2);
            auto sources = s_recovered.get_sources();
            run_test("Relational state strictly preserves authoritative sources (count == 1)", sources.size() == 1);
            run_test("Authoritative source is src_normal_01", sources[0].id == "src_normal_01");
            bool found_ghost = false;
            for (const auto& s : sources) {
                if (s.id == "src_ghost_missing") found_ghost = true;
            }
            run_test("Ghost source is completely absent from relational ledger", !found_ghost);
        }

        fs::remove_all(sync_dir, ec);
    }

    // -------------------------------------------------------------------------
    // TEST 7: Startup Orphaned .tmp File Auto-Cleanup
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 7] Startup Orphaned .tmp File Auto-Cleanup...\n";
    {
        std::string tmp_dir = "test_tmp_cleanup_data";
        fs::remove_all(tmp_dir, ec);
        fs::create_directories(tmp_dir);
        fs::create_directories(tmp_dir + "/archives");
        fs::create_directories(tmp_dir + "/chunks");

        // Write normal baseline files
        {
            NativeStorage s(tmp_dir);
            Source src;
            src.id = "src_persisted_01";
            src.title = "Persisted Doc";
            s.put_source(src);
            s.vectors().insert("chunk_01", "src_persisted_01", 1, VectorIndex::embed_text("persisted text"));
            s.store_compressed_chunk("chunk_01", "persisted chunk text");
            s.save_state();
        }

        // Intentionally plant orphaned .tmp files simulating crash mid-write
        std::string orphan1 = tmp_dir + "/relational_state.json.tmp";
        std::string orphan2 = tmp_dir + "/vectors.bin.tmp";
        std::string orphan3 = tmp_dir + "/archives/test_scan.pdf.bin.tmp";
        std::string orphan4 = tmp_dir + "/chunks/stale_chunk.txt.tmp";

        {
            std::ofstream o1(orphan1); o1 << "broken partial write 1";
            std::ofstream o2(orphan2); o2 << "broken partial write 2";
            std::ofstream o3(orphan3); o3 << "broken partial write 3";
            std::ofstream o4(orphan4); o4 << "broken partial write 4";
        }

        run_test("Orphaned .tmp 1 exists before restart", fs::exists(orphan1));
        run_test("Orphaned .tmp 2 exists before restart", fs::exists(orphan2));
        run_test("Orphaned .tmp 3 exists before restart", fs::exists(orphan3));
        run_test("Orphaned .tmp 4 exists before restart", fs::exists(orphan4));

        // Cold restart of storage
        {
            NativeStorage s_clean(tmp_dir);

            // Verify legitimate data survives intact
            run_test("Legitimate source survived intact", s_clean.get_sources().size() == 1);
            run_test("Legitimate vector survived intact", s_clean.vectors().size() == 1);
            run_test("Legitimate chunk survived intact", s_clean.read_compressed_chunk("chunk_01") == "persisted chunk text");

            // Verify all orphaned .tmp files were purged
            run_test("Orphaned relational_state.json.tmp cleaned up", !fs::exists(orphan1));
            run_test("Orphaned vectors.bin.tmp cleaned up", !fs::exists(orphan2));
            run_test("Orphaned archives/*.pdf.bin.tmp cleaned up", !fs::exists(orphan3));
            run_test("Orphaned chunks/*.tmp cleaned up", !fs::exists(orphan4));
        }

        fs::remove_all(tmp_dir, ec);
    }

    // -------------------------------------------------------------------------
    // TEST 8: Read-Path Resilience on Reverse Failure Mode (Chunk in Ledger, Vector Missing)
    // -------------------------------------------------------------------------
    std::cout << "\n[TEST 8] Read-Path Resilience on Reverse Dual-Write Gap (Missing Vector)...\n";
    {
        std::string rev_dir = "test_reverse_gap_data";
        fs::remove_all(rev_dir, ec);
        fs::create_directories(rev_dir);

        // Step 1: Create relational source, claim, and stored chunk text, but NO vector
        {
            NativeStorage s(rev_dir);
            Source src;
            src.id = "src_unvectorized_doc";
            src.title = "Unvectorized Excavation Log";
            src.degradation_class = "CLASS_B";
            src.ingestion_status = "UNVERIFIED_ROUGH_SCAN";
            s.put_source(src);

            Claim c;
            c.id = "claim_001";
            c.source_id = "src_unvectorized_doc";
            c.page_ref = "p.42";
            c.claim_text = "Layer 4 contains basalt handaxes";
            c.verification_status = "VERIFIED";
            s.put_claim(c);

            s.store_compressed_chunk("chunk_unvectorized_01", "Excavation unit 42-B Acheulian handaxes in basalt bedrock");

            // Save state so relational ledger and chunk text exist on disk
            s.save_state();

            // Explicitly ensure vectors.bin is completely empty (zero vectors)
            s.vectors().clear();
            s.vectors().save(rev_dir + "/vectors.bin");
        }

        // Step 2: Initialize NativeStorage + NativeIpcDispatcher on this data root
        {
            NativeStorage s_rev(rev_dir);
            NativeContradictionEngine contradictions(s_rev);
            NativeThesisAuditor thesisAuditor(s_rev, contradictions);
            NativeIpcDispatcher dispatcher(&s_rev, &contradictions, &thesisAuditor, rev_dir);

            // Assert baseline state
            run_test("Relational source loaded successfully", s_rev.get_sources().size() == 1);
            run_test("Relational claim loaded successfully", s_rev.get_claims().size() == 1);
            run_test("Chunk text accessible from disk", s_rev.read_compressed_chunk("chunk_unvectorized_01").size() > 0);
            run_test("Vector store is strictly empty (0 vectors)", s_rev.vectors().size() == 0);

            // Direct VectorIndex read path: search returns empty list, does not throw or crash
            auto direct_results = s_rev.vectors().search(VectorIndex::embed_text("Acheulian handaxes"), 5);
            run_test("Direct VectorIndex search on missing vectors returns empty cleanly", direct_results.empty());

            // IPC read path: search_semantic_passages must return empty results array without crashing or throwing
            json req = {
                {"id", "test-req-01"},
                {"action", "search_semantic_passages"},
                {"payload", {
                    {"query", "Acheulian handaxes"},
                    {"top_k", 5}
                }}
            };
            std::string resp_str = dispatcher.dispatch(req.dump());
            json resp = json::parse(resp_str);

            run_test("IPC search_semantic_passages returned valid JSON response", !resp.contains("error"));
            run_test("IPC search result contains 'result' field", resp.contains("result"));
            run_test("IPC search result is a valid array", resp["result"].is_array());
            run_test("IPC search result is cleanly empty (no garbage returned)", resp["result"].empty());
        }

        fs::remove_all(rev_dir, ec);
    }

    // Teardown test artifacts
    fs::remove_all(test_dir, ec);

    std::cout << "\n================================================================================\n";
    std::cout << "  ALL 8 VECTOR INDEX RESTART DURABILITY TESTS PASSED WITH ZERO FAILURES!        \n";
    std::cout << "================================================================================\n";

    return 0;
}
