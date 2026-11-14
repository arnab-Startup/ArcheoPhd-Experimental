#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <filesystem>
#include <fstream>

#include "storage.hpp"
#include "vector_index.hpp"
#include "embedding_engine.hpp"

using namespace archaeophd;
namespace fs = std::filesystem;

// =============================================================================
// Ground-Truth Benchmark Types
// =============================================================================
struct BenchmarkPassage {
    std::string chunk_id;
    std::string doc_id;
    int page_ref;
    std::string text;
};

struct BenchmarkQuery {
    std::string query_text;
    std::string target_chunk_id;
    std::string description;
};

// =============================================================================
// Benchmark Ground-Truth Dataset (50 Passages across 4 Classical Excavations)
// =============================================================================
std::vector<BenchmarkPassage> get_50_benchmark_passages() {
    return {
        // --- Sankalia (1974) Chirki-on-Pravara (Palaeolithic Lithics) ---
        {"chirki_c01", "src_sankalia_1974", 12, "Excavation at Locality VII exposed early Acheulian basalt handaxes and heavy cleavers embedded within the cemented boulder conglomerate horizon."},
        {"chirki_c02", "src_sankalia_1974", 15, "Morphometric analysis of Acheulian bifaces revealed distinct thickness-to-breadth ratios indicative of primary hard-hammer percussion."},
        {"chirki_c03", "src_sankalia_1974", 22, "Abundant basalt flakes display wide striking platforms, prominent bulbs of percussion, and Clactonian flaking angles exceeding 110 degrees."},
        {"chirki_c04", "src_sankalia_1974", 28, "Fluvial transport abrasion on artefact edges is minimal, proving the assemblage represents an in-situ hominin knapping floor on the ancient Pravara terrace."},
        {"chirki_c05", "src_sankalia_1974", 34, "Polyhedral cores, heavy-duty hammerstones, and spheroids were discovered directly overlying the weathered amygdaloidal basalt bedrock surface."},
        {"chirki_c06", "src_sankalia_1974", 41, "Middle Palaeolithic scrapers and points manufactured on fine-grained chalcedony and chert occur exclusively in the upper sandy silt stratum."},
        {"chirki_c07", "src_sankalia_1974", 48, "Faunal remains associated with the lithic horizon include fossilized teeth of Elephas namadicus and Bos namadicus in secondary calcified matrix."},
        {"chirki_c08", "src_sankalia_1974", 53, "Reworked colluvial debris blankets the Acheulian horizon, indicating a sharp arid climatic pulsation during the terminal Pleistocene."},
        {"chirki_c09", "src_sankalia_1974", 60, "Trihedral picks shaped from compact basalt blocks suggest specialized woodworking or digging activities along the riparian river channel."},
        {"chirki_c10", "src_sankalia_1974", 67, "Petrographic thin sections of the basalt tools demonstrate low chemical weathering index, preserving pristine unpatinated flake scars."},

        // --- Kenyon (1981) vs. Wood (1990) Jericho / Tell es-Sultan (Stratigraphy Dispute) ---
        {"jericho_c11", "src_kenyon_1981", 102, "Trench I revealed the catastrophic collapse of the red mudbrick city wall onto the lower stone revetment, sealing the terminal Middle Bronze Age horizon."},
        {"jericho_c12", "src_kenyon_1981", 108, "Intense conflagration debris up to one metre thick contains charred wooden beams and fully carbonized grain jars dated by Kenyon to 1550 BC."},
        {"jericho_c13", "src_wood_1990", 215, "Wood argues that imported Cypriot bichrome ware ceramics found in City IV destruction debris securely date the fall of Jericho to Late Bronze I around 1400 BC."},
        {"jericho_c14", "src_kenyon_1981", 119, "Middle Bronze Age rampart defenses incorporated a plastered glacis slope designed to impede battering rams and sapping attempts."},
        {"jericho_c15", "src_wood_1990", 228, "Stratigraphic re-examination of Kenyon's East Field soundings demonstrates continuity of domestic cooking pots bridging the MBA-LBA cultural transition."},
        {"jericho_c16", "src_kenyon_1981", 134, "Pre-Pottery Neolithic A monumental stone tower and defensive ditch represent the earliest known communal masonry architecture in the Levant."},
        {"jericho_c17", "src_kenyon_1981", 142, "Plastered human skulls with inlaid cowrie shell eyes recovered from PPNB domestic floors reflect elaborate ancestral funerary rituals."},
        {"jericho_c18", "src_wood_1990", 240, "Radiocarbon determinations from short-lived grain samples in City IV yield uncalibrated dates centering on late 15th century BC."},
        {"jericho_c19", "src_kenyon_1981", 155, "Early Bronze Age multi-phase brick defenses were repeatedly undermined by earthquake faulting along the Jordan rift valley fault line."},
        {"jericho_c20", "src_wood_1990", 255, "The absence of Late Bronze II Mycenaean imported pottery confirms abandonment of the tell prior to the 13th century BC."},

        // --- Yadin (1972) Hazor (Iron Age Stratigraphy & Fortifications) ---
        {"hazor_c21", "src_yadin_1972", 73, "The monumental six-chambered gateway in Stratum X upper city features identical architectural dimensions to contemporary gates at Megiddo and Gezer."},
        {"hazor_c22", "src_yadin_1972", 81, "A solid casemate fortification wall connects directly into the flanks of the Solomonic gatehouse, forming a unified Iron Age IIA defensive perimeter."},
        {"hazor_c23", "src_yadin_1972", 94, "Stratum IX represents a significant rebuilding phase with solid masonry walls following the military campaign of Ben-Hadad of Damascus."},
        {"hazor_c24", "src_yadin_1972", 112, "The Area L subterranean water shaft descends forty metres through bedrock to tap the subterranean aquifer during prolonged sieges."},
        {"hazor_c25", "src_yadin_1972", 128, "Stratum VI public pillared storehouses contained dozens of standardized administrative storage pithoi bearing stamped handle impressions."},
        {"hazor_c26", "src_yadin_1972", 145, "Canaanite basalt orthostats depicting crouched lions protected the monumental entrance to the Late Bronze Age Area H temple."},
        {"hazor_c27", "src_yadin_1972", 160, "Severe earthquake displacement visible in tilted ashlar pillar alignments marks the terminal destruction of Iron IIB Stratum VI."},
        {"hazor_c28", "src_yadin_1972", 178, "Cuneiform clay tablets discovered in the palace archives record legal disputes between royal administrators written in Old Babylonian script."},
        {"hazor_c29", "src_yadin_1972", 195, "Stratum V witnessed complete razing and incineration by the Assyrian armies of Tiglath-Pileser III in 732 BC."},
        {"hazor_c30", "src_yadin_1972", 210, "Post-destruction squatters reoccupied the citadel ruins in Stratum IV, utilizing fragmentary partition walls and crude hearths."},

        // --- Marshall & Mackay (1931-1938) Mohenjo-daro & Indus Urbanism ---
        {"indus_c31", "src_marshall_1931", 35, "The Great Bath on the western citadel mound features a watertight floor constructed of gypsum mortar and backed by a 2.5 cm layer of bitumen."},
        {"indus_c32", "src_marshall_1931", 49, "Brick-lined corbelled drains ran underneath main streets, fitted with inspection manholes and settling sumps to prevent clogging."},
        {"indus_c33", "src_marshall_1931", 62, "Standardized burnt brick proportions of 1:2:4 were strictly maintained across domestic houses, public wells, and peripheral walls."},
        {"indus_c34", "src_mackay_1938", 88, "The DK-G area craft quarters yielded micro-drill bits of green chert specialized for perforating hard carnelian beads."},
        {"indus_c35", "src_marshall_1931", 104, "Steatite stamp seals engraved with unicorn motifs, brahmani bulls, and Indus pictographic script occurred on residential room floors."},
        {"indus_c36", "src_mackay_1938", 120, "Copper and bronze metallurgy utilized arsenic-alloyed smelting to increase blade hardness of chisels, saws, and spearheads."},
        {"indus_c37", "src_marshall_1931", 140, "The raised monumental granary incorporates brick sleeper podiums with air ducts underneath to prevent dampness and mildew in stored cereal grains."},
        {"indus_c38", "src_mackay_1938", 165, "Chert cubic balance weights conform to a binary weight system ascending from 1, 2, 4, 8 to decimal multiples of 160 and 320."},
        {"indus_c39", "src_marshall_1931", 188, "Terracotta figurine assemblages depict seated females with elaborate fan-shaped headdresses, pannier side projections, and pellet eyes."},
        {"indus_c40", "src_mackay_1938", 214, "Late Harappan Jhukar phase occupations reflect architectural decline, reused broken bricks, and loss of municipal sanitary drainage control."},

        // --- Archaeological Method & Theory (Stratigraphy, C-14, Harris Matrix) ---
        {"method_c41", "src_harris_1989", 18, "The Harris Matrix establishes chronological sequence by representing stratigraphic units of stratification as non-redundant topological directed acyclic graphs."},
        {"method_c42", "src_harris_1989", 32, "The law of superposition states that upper strata are younger than lower strata unless post-depositional tectonic inversions have occurred."},
        {"method_c43", "src_harris_1989", 45, "Interfacial boundaries representing cuts, negative features, and robber trenches carry equal chronological weight to positive volumetric deposits."},
        {"method_c44", "src_aitken_1990", 25, "Thermoluminescence dating measures accumulated radioactive dose in quartz crystals since the last heating event above 500 degrees Celsius."},
        {"method_c45", "src_aitken_1990", 60, "Radiocarbon age determinations require calibration against dendrochronological tree-ring curves to correct for atmospheric 14C fluctuations."},
        {"method_c46", "src_aitken_1990", 92, "Accelerated Mass Spectrometry (AMS) enables precise isotopic dating on milligram-sized samples of single cereal seeds and charcoal specks."},
        {"method_c47", "src_schiffer_1987", 14, "Formation processes are divided into cultural formation processes (c-transforms) and natural environmental transforms (n-transforms)."},
        {"method_c48", "src_schiffer_1987", 38, "Bioturbation caused by burrowing animals and deep root penetration can translocate diagnostic micro-artefacts across distinct stratum boundaries."},
        {"method_c49", "src_courty_1989", 55, "Soil micromorphology of intact resin-impregnated thin sections reveals micro-laminations, trampled living surfaces, and stabling dung accumulations."},
        {"method_c50", "src_courty_1989", 88, "Phytolith and pollen micro-botanical extractions document climatic transitions from humid riparian wetlands to arid savannah environments."}
    };
}

// =============================================================================
// 20 Pre-Registered Test Queries (Engineered Lexical Mismatches / Synonyms)
// =============================================================================
std::vector<BenchmarkQuery> get_20_benchmark_queries() {
    return {
        // Query 1 -> chirki_c01: "early pleistocene stone biface tools from river rubble" (synonym mismatch)
        {"early pleistocene stone biface tools from river rubble", "chirki_c01", "Chirki Locality VII boulder bed bifaces"},

        // Query 2 -> chirki_c04: "unabraded hominin manufacturing site near paleochannel"
        {"unabraded hominin manufacturing site near paleochannel", "chirki_c04", "Chirki in-situ knapping floor preservation"},

        // Query 3 -> chirki_c07: "fossil elephant molars and bovine fauna with lithics"
        {"fossil elephant molars and bovine fauna with lithics", "chirki_c07", "Chirki Elephas and Bos fossil associations"},

        // Query 4 -> jericho_c11: "burnt collapsed mud brick defensive fortification"
        {"burnt collapsed mud brick defensive fortification", "jericho_c11", "Jericho City IV collapsed mudbrick wall"},

        // Query 5 -> jericho_c12: "charred food grain vessels preserved in fiery destruction"
        {"charred food grain vessels preserved in fiery destruction", "jericho_c12", "Jericho carbonized grain storage jars"},

        // Query 6 -> jericho_c13: "cypriot painted bichrome pottery dating controversy"
        {"cypriot painted bichrome pottery dating controversy", "jericho_c13", "Wood vs Kenyon bichrome ware chronology"},

        // Query 7 -> jericho_c16: "neolithic circular stone watchtower and moat"
        {"neolithic circular stone watchtower and moat", "jericho_c16", "PPNA monumental tower architecture"},

        // Query 8 -> jericho_c17: "modeled facial features on ancestral human skulls"
        {"modeled facial features on ancestral human skulls", "jericho_c17", "PPNB plastered skulls with shell eyes"},

        // Query 9 -> hazor_c21: "iron age six-chambered monumental gateway fortifications"
        {"iron age six-chambered monumental gateway fortifications", "hazor_c21", "Hazor Solomonic 6-chamber gate"},

        // Query 10 -> hazor_c22: "hollow casemate curtain wall defense"
        {"hollow casemate curtain wall defense", "hazor_c22", "Hazor casemate wall construction"},

        // Query 11 -> hazor_c24: "underground rock-cut tunnel accessing water table during siege"
        {"underground rock-cut tunnel accessing water table during siege", "hazor_c24", "Hazor subterranean water shaft"},

        // Query 12 -> hazor_c26: "carved basalt feline temple guardian sculptures"
        {"carved basalt feline temple guardian sculptures", "hazor_c26", "Hazor lion orthostat temple entrance"},

        // Query 13 -> indus_c31: "ancient bitumen waterproof lining in ritual water structure"
        {"ancient bitumen waterproof lining in ritual water structure", "indus_c31", "Mohenjo-daro Great Bath waterproofing"},

        // Query 14 -> indus_c32: "covered municipal sewage drainage system with silt traps"
        {"covered municipal sewage drainage system with silt traps", "indus_c32", "Harappan corbelled street drains"},

        // Query 15 -> indus_c37: "ventilated agricultural storehouse with timber ducting"
        {"ventilated agricultural storehouse with timber ducting", "indus_c37", "Mohenjo-daro granary air ducts"},

        // Query 16 -> indus_c38: "binary cubical stone measurement metrology"
        {"binary cubical stone measurement metrology", "indus_c38", "Indus cubic balance weights"},

        // Query 17 -> method_c41: "topological directed graph representation of archaeological layers"
        {"topological directed graph representation of archaeological layers", "method_c41", "Harris Matrix DAG topology"},

        // Query 18 -> method_c44: "thermal luminescence trapped electron dating of fired pottery"
        {"thermal luminescence trapped electron dating of fired pottery", "method_c44", "Thermoluminescence quartz dating"},

        // Query 19 -> method_c48: "animal burrowing disturbance mixing diagnostic artifacts"
        {"animal burrowing disturbance mixing diagnostic artifacts", "method_c48", "Schiffer bioturbation artifacts"},

        // Query 20 -> method_c49: "soil micromorphology thin section microscopic floor analysis"
        {"soil micromorphology thin section microscopic floor analysis", "method_c49", "Courty micromorphology living surfaces"}
    };
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine — Step 4B: Semantic Retrieval Quality Benchmark             \n";
    std::cout << "  Model: nomic-embed-text-v1.5.Q4_K_M (128-dim Matryoshka + Guarded L2 Norm)    \n";
    std::cout << "================================================================================\n\n";

    std::string model_path = "models/embedding/nomic-embed-text-v1.5.Q4_K_M.gguf";

    // -------------------------------------------------------------------------
    // GATE 0: Production Safeguard Verification (Missing Model Must Throw)
    // -------------------------------------------------------------------------
    std::cout << "[GATE 0] Testing Production Safeguard Against Silent Stub Fallback...\n";
    {
#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
        // If compiled with test stub flag, ensure mock mode is explicitly FALSE
        EmbeddingEngine::instance().enable_test_mock_mode(false);
#endif
        EmbeddingEngine::instance().shutdown();

        bool threw_exception = false;
        try {
            VectorIndex::embed_text("test string without loaded weights");
        } catch (const std::runtime_error& e) {
            threw_exception = true;
            std::cout << "  ✓ Safeguard verified: unloaded model threw '" << e.what() << "'\n";
        }

        if (!threw_exception) {
            std::cerr << "  [FAIL] Critical safeguard broken: unloaded model silently returned a vector!\n";
            return 1;
        }
    }

    // -------------------------------------------------------------------------
    // STEP 1: Load Nomic GGUF Weights Into Embedded Engine
    // -------------------------------------------------------------------------
    std::cout << "\n[STEP 1] Initializing in-process Nomic GGUF Embedding Engine...\n";
    auto t_load_start = std::chrono::high_resolution_clock::now();
    bool loaded = EmbeddingEngine::instance().load_model(model_path, 4);
    auto t_load_end = std::chrono::high_resolution_clock::now();
    double load_ms = std::chrono::duration<double, std::milli>(t_load_end - t_load_start).count();

    if (!loaded) {
        std::cerr << "  [FAIL] Failed to load model weights from " << model_path << "\n";
        return 1;
    }
    std::cout << "  ✓ Model loaded successfully in " << std::fixed << std::setprecision(1) << load_ms << " ms\n";
    std::cout << "  ✓ Target Matryoshka dimension: 128 (RAM per chunk: 512 bytes)\n";

    // -------------------------------------------------------------------------
    // STEP 2: Ingest 50-Passage Ground-Truth Corpus into NativeStorage & VectorIndex
    // -------------------------------------------------------------------------
    std::string bench_dir = "test_benchmark_data";
    std::error_code ec;
    fs::remove_all(bench_dir, ec);
    fs::create_directories(bench_dir);

    NativeStorage storage(bench_dir);
    auto passages = get_50_benchmark_passages();
    std::cout << "\n[STEP 2] Vectorizing and indexing " << passages.size() << " ground-truth passages...\n";

    auto t_idx_start = std::chrono::high_resolution_clock::now();
    for (const auto& p : passages) {
        storage.store_compressed_chunk(p.chunk_id, p.text);
        auto emb = VectorIndex::embed_text(p.text, /*is_query=*/false);
        storage.vectors().insert(p.chunk_id, p.doc_id, p.page_ref, emb);
    }
    auto t_idx_end = std::chrono::high_resolution_clock::now();
    double idx_ms = std::chrono::duration<double, std::milli>(t_idx_end - t_idx_start).count();

    std::cout << "  ✓ Ingested " << storage.vectors().size() << " vectors in " 
              << std::fixed << std::setprecision(1) << idx_ms << " ms ("
              << (idx_ms / passages.size()) << " ms/passage)\n";

    // Persist to disk to verify durability integration
    storage.save_state();
    std::cout << "  ✓ Vectors atomically persisted to " << bench_dir << "/vectors.bin\n";

    // -------------------------------------------------------------------------
    // STEP 3: Execute Pre-Registered 20-Query Retrieval Benchmark
    // -------------------------------------------------------------------------
    auto queries = get_20_benchmark_queries();
    std::cout << "\n[STEP 3] Executing 20 Pre-Registered Archaeological Queries...\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(4) << "#" 
              << std::setw(40) << "Query Description" 
              << std::setw(12) << "Target" 
              << std::setw(8) << "Rank" 
              << std::setw(10) << "Score" 
              << std::setw(10) << "Latency" 
              << "Status\n";
    std::cout << "--------------------------------------------------------------------------------\n";

    int hits_at_1 = 0;
    int hits_at_3 = 0;
    int hits_at_5 = 0;
    int hits_at_10 = 0;
    double reciprocal_rank_sum = 0.0;
    double total_latency_ms = 0.0;

    for (size_t i = 0; i < queries.size(); ++i) {
        const auto& q = queries[i];
        
        auto q_start = std::chrono::high_resolution_clock::now();
        auto q_emb = VectorIndex::embed_text(q.query_text, /*is_query=*/true);
        auto matches = storage.vectors().search(q_emb, 10);
        auto q_end = std::chrono::high_resolution_clock::now();
        double q_lat = std::chrono::duration<double, std::milli>(q_end - q_start).count();
        total_latency_ms += q_lat;

        int rank = -1;
        float target_score = 0.0f;
        for (size_t r = 0; r < matches.size(); ++r) {
            if (matches[r].chunk_id == q.target_chunk_id) {
                rank = static_cast<int>(r + 1);
                target_score = matches[r].score;
                break;
            }
        }

        std::string status = "MISS";
        if (rank == 1) {
            hits_at_1++; hits_at_3++; hits_at_5++; hits_at_10++;
            reciprocal_rank_sum += 1.0;
            status = "HIT @1";
        } else if (rank > 1 && rank <= 3) {
            hits_at_3++; hits_at_5++; hits_at_10++;
            reciprocal_rank_sum += (1.0 / rank);
            status = "HIT @3";
        } else if (rank > 3 && rank <= 5) {
            hits_at_5++; hits_at_10++;
            reciprocal_rank_sum += (1.0 / rank);
            status = "HIT @5";
        } else if (rank > 5 && rank <= 10) {
            hits_at_10++;
            reciprocal_rank_sum += (1.0 / rank);
            status = "HIT @10";
        }

        std::string short_desc = q.description;
        if (short_desc.size() > 38) short_desc = short_desc.substr(0, 35) + "...";

        std::cout << std::left << std::setw(4) << (i + 1)
                  << std::setw(40) << short_desc
                  << std::setw(12) << q.target_chunk_id
                  << std::setw(8) << (rank > 0 ? std::to_string(rank) : ">10")
                  << std::setw(10) << std::fixed << std::setprecision(4) << target_score
                  << std::setw(8) << std::fixed << std::setprecision(1) << q_lat << "ms "
                  << status << "\n";
        if (rank != 1 && !matches.empty()) {
            std::cout << "     ↳ [Top 1 was " << matches[0].chunk_id << " with score " << std::fixed << std::setprecision(4) << matches[0].score << "]\n";
        }
    }

    std::cout << "--------------------------------------------------------------------------------\n\n";

    // -------------------------------------------------------------------------
    // STEP 4: Quantitative Evaluation Against Hard Pre-Registered Gates
    // -------------------------------------------------------------------------
    double recall_at_1 = (double)hits_at_1 / queries.size() * 100.0;
    double recall_at_3 = (double)hits_at_3 / queries.size() * 100.0;
    double recall_at_5 = (double)hits_at_5 / queries.size() * 100.0;
    double recall_at_10 = (double)hits_at_10 / queries.size() * 100.0;
    double mrr_at_10 = reciprocal_rank_sum / queries.size();
    double avg_latency = total_latency_ms / queries.size();

    std::cout << "================================================================================\n";
    std::cout << "  FINAL STEP 4B BENCHMARK METRICS SUMMARY                                       \n";
    std::cout << "================================================================================\n";
    std::cout << "  • Total Evaluation Queries:   " << queries.size() << "\n";
    std::cout << "  • Corpus Chunk Count:         " << passages.size() << " passages\n";
    std::cout << "  • Average Query Latency:      " << std::fixed << std::setprecision(2) << avg_latency << " ms\n";
    std::cout << "  • Recall @ 1:                 " << std::fixed << std::setprecision(1) << recall_at_1 << "% (" << hits_at_1 << "/" << queries.size() << ")\n";
    std::cout << "  • Recall @ 3:                 " << std::fixed << std::setprecision(1) << recall_at_3 << "% (" << hits_at_3 << "/" << queries.size() << ")\n";
    std::cout << "  • Recall @ 5:                 " << std::fixed << std::setprecision(1) << recall_at_5 << "% (" << hits_at_5 << "/" << queries.size() << ")\n";
    std::cout << "  • Recall @ 10:                " << std::fixed << std::setprecision(1) << recall_at_10 << "% (" << hits_at_10 << "/" << queries.size() << ")\n";
    std::cout << "  • Mean Reciprocal Rank (MRR): " << std::fixed << std::setprecision(4) << mrr_at_10 << "\n";
    std::cout << "================================================================================\n\n";

    // Hard Gate Verifications
    bool pass_recall5 = (recall_at_5 >= 85.0);
    bool pass_mrr = (mrr_at_10 >= 0.70);
    bool pass_latency = (avg_latency < 25.0);

    std::cout << "[PRE-REGISTERED HARD GATE VERIFICATION]\n";
    std::cout << "  [GATE 1] Recall@5 >= 85.0%: " << (pass_recall5 ? "[PASS]" : "[FAIL]") 
              << " (Measured: " << recall_at_5 << "%)\n";
    std::cout << "  [GATE 2] MRR@10   >= 0.70:  " << (pass_mrr ? "[PASS]" : "[FAIL]") 
              << " (Measured: " << mrr_at_10 << ")\n";
    std::cout << "  [GATE 3] Latency  < 25 ms:  " << (pass_latency ? "[PASS]" : "[FAIL]") 
              << " (Measured: " << avg_latency << " ms)\n\n";

    // Teardown benchmark data
    fs::remove_all(bench_dir, ec);

    if (pass_recall5 && pass_mrr && pass_latency) {
        std::cout << ">>> ALL STEP 4B PRE-REGISTERED RETRIEVAL BENCHMARKS PASSED! <<<\n";
        return 0;
    } else {
        std::cerr << ">>> STEP 4B BENCHMARK GATE FAILED <<<\n";
        return 1;
    }
}
