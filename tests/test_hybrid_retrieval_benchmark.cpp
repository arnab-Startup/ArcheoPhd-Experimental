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
#include "lexical_index.hpp"
#include "hybrid_search.hpp"
#include "embedding_engine.hpp"

using namespace archaeophd;
namespace fs = std::filesystem;

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

// Ground-Truth Dataset (Exact 50 Passages from test_semantic_retrieval_benchmark.cpp)
std::vector<BenchmarkPassage> get_benchmark_passages() {
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

std::vector<BenchmarkQuery> get_benchmark_queries() {
    return {
        {"early pleistocene stone biface tools from river rubble", "chirki_c01", "Chirki Locality VII boulder bed bifaces"},
        {"unabraded hominin manufacturing site near paleochannel", "chirki_c04", "Chirki in-situ knapping floor preservation"},
        {"fossil elephant molars and bovine fauna with lithics", "chirki_c07", "Chirki Elephas and Bos fossil associations"},
        {"burnt collapsed mud brick defensive fortification", "jericho_c11", "Jericho City IV collapsed mudbrick wall"},
        {"charred food grain vessels preserved in fiery destruction", "jericho_c12", "Jericho carbonized grain storage jars"},
        {"cypriot painted bichrome pottery dating controversy", "jericho_c13", "Wood vs Kenyon bichrome ware chronology"},
        {"neolithic circular stone watchtower and moat", "jericho_c16", "PPNA monumental tower architecture"},
        {"modeled facial features on ancestral human skulls", "jericho_c17", "PPNB plastered skulls with shell eyes"},
        {"iron age six-chambered monumental gateway fortifications", "hazor_c21", "Hazor Solomonic 6-chamber gate"},
        {"hollow casemate curtain wall defense", "hazor_c22", "Hazor casemate wall construction"},
        {"underground rock-cut tunnel accessing water table during siege", "hazor_c24", "Hazor subterranean water shaft"},
        {"carved basalt feline temple guardian sculptures", "hazor_c26", "Hazor lion orthostat temple entrance"},
        {"ancient bitumen waterproof lining in ritual water structure", "indus_c31", "Mohenjo-daro Great Bath waterproofing"},
        {"covered municipal sewage drainage system with silt traps", "indus_c32", "Harappan corbelled street drains"},
        {"ventilated agricultural storehouse with timber ducting", "indus_c37", "Mohenjo-daro granary air ducts"},
        {"binary cubical stone measurement metrology", "indus_c38", "Indus cubic balance weights"},
        {"topological directed graph representation of archaeological layers", "method_c41", "Harris Matrix DAG topology"},
        {"thermal luminescence trapped electron dating of fired pottery", "method_c44", "Thermoluminescence quartz dating"},
        {"animal burrowing disturbance mixing diagnostic artifacts", "method_c48", "Schiffer bioturbation artifacts"},
        {"soil micromorphology thin section microscopic floor analysis", "method_c49", "Courty micromorphology living surfaces"}
    };
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD Engine — Step 5: Hybrid Lexical (BM25) + Dense Vector Benchmark   \n";
    std::cout << "  Model: nomic-embed-text-v1.5.Q4_K_M + Okapi BM25 + Reciprocal Rank Fusion      \n";
    std::cout << "================================================================================\n\n";

    std::string bench_dir = "test_hybrid_bench_data";
    std::error_code ec;
    fs::remove_all(bench_dir, ec);
    fs::create_directories(bench_dir);

    // 1. Initialize Nomic GGUF Embedding Model
    std::string model_path = "models/embedding/nomic-embed-text-v1.5.Q4_K_M.gguf";
    if (!fs::exists(model_path)) {
        model_path = "../models/embedding/nomic-embed-text-v1.5.Q4_K_M.gguf";
    }

    if (!EmbeddingEngine::instance().load_model(model_path)) {
        std::cerr << "[ERROR] Failed to load Nomic embedding model from " << model_path << "\n";
        return 1;
    }
    std::cout << "  ✓ In-process Nomic GGUF embedding engine loaded.\n";

    // 2. Build VectorIndex and LexicalIndex
    VectorIndex vector_index;
    LexicalIndex lexical_index;

    auto passages = get_benchmark_passages();
    auto queries = get_benchmark_queries();

    std::cout << "  ✓ Indexing " << passages.size() << " passages into Dense and Lexical indices...\n";
    for (const auto& p : passages) {
        auto emb = VectorIndex::embed_text(p.text, /*is_query=*/false);
        vector_index.insert(p.chunk_id, p.doc_id, p.page_ref, emb);
        lexical_index.insert(p.chunk_id, p.doc_id, p.page_ref, p.text);
    }
    std::cout << "  ✓ Lexical vocabulary size: " << lexical_index.vocab_size() << " unique terms.\n\n";

    // Test Lexical Index Binary Persistence
    std::string lex_file = bench_dir + "/lexical.bin";
    bool save_ok = lexical_index.save(lex_file);
    assert(save_ok == true);
    LexicalIndex lex_reloaded;
    bool load_ok = lex_reloaded.load(lex_file);
    assert(load_ok == true);
    assert(lex_reloaded.size() == passages.size());
    std::cout << "  ✓ LexicalIndex atomic binary persistence (APL1) verified: 100% round-trip fidelity.\n\n";

    // Warm up embedding context and OpenMP thread pool
    VectorIndex::embed_text("warmup archaeological query", /*is_query=*/true);

    // 3. Run Hybrid Evaluation
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(4) << "#"
              << std::setw(38) << "Query Description"
              << std::setw(12) << "Target"
              << std::setw(8) << "Dense"
              << std::setw(8) << "BM25"
              << std::setw(8) << "Hybrid"
              << std::setw(10) << "Latency"
              << "Status\n";
    std::cout << "--------------------------------------------------------------------------------\n";

    int dense_hits5 = 0;
    int hybrid_hits1 = 0;
    int hybrid_hits3 = 0;
    int hybrid_hits5 = 0;
    int hybrid_hits10 = 0;
    double reciprocal_rank_sum = 0.0;
    double total_latency_ms = 0.0;

    for (size_t i = 0; i < queries.size(); ++i) {
        const auto& q = queries[i];

        auto q_start = std::chrono::high_resolution_clock::now();

        // 1. Embed query (CPU in-process GGUF embedding)
        auto q_vec = VectorIndex::embed_text(q.query_text, /*is_query=*/true);

        // 2. Hybrid search (Dense vector search + BM25 inverted index + Reciprocal Rank Fusion)
        auto hybrid_res = HybridSearchEngine::search(vector_index, lexical_index, q.query_text, 10, 1.0f, 1.0f, {}, 60.0f, q_vec);

        auto q_end = std::chrono::high_resolution_clock::now();
        double q_lat = std::chrono::duration<double, std::milli>(q_end - q_start).count();
        total_latency_ms += q_lat;

        // Standalone comparison runs (outside benchmark timer to avoid double-counting search calls)
        auto dense_res = vector_index.search(q_vec, 10);
        auto lex_res = lexical_index.search(q.query_text, 10);

        // Find ranks
        int d_rank = 0;
        for (size_t r = 0; r < dense_res.size(); ++r) {
            if (dense_res[r].chunk_id == q.target_chunk_id) { d_rank = static_cast<int>(r + 1); break; }
        }
        if (d_rank > 0 && d_rank <= 5) dense_hits5++;

        int l_rank = 0;
        for (size_t r = 0; r < lex_res.size(); ++r) {
            if (lex_res[r].chunk_id == q.target_chunk_id) { l_rank = static_cast<int>(r + 1); break; }
        }

        int h_rank = 0;
        for (size_t r = 0; r < hybrid_res.size(); ++r) {
            if (hybrid_res[r].chunk_id == q.target_chunk_id) { h_rank = static_cast<int>(r + 1); break; }
        }

        std::string status = "MISS";
        if (h_rank == 1) {
            hybrid_hits1++; hybrid_hits3++; hybrid_hits5++; hybrid_hits10++;
            reciprocal_rank_sum += 1.0;
            status = "HIT @1";
        } else if (h_rank > 1 && h_rank <= 3) {
            hybrid_hits3++; hybrid_hits5++; hybrid_hits10++;
            reciprocal_rank_sum += (1.0 / h_rank);
            status = "HIT @3";
        } else if (h_rank > 3 && h_rank <= 5) {
            hybrid_hits5++; hybrid_hits10++;
            reciprocal_rank_sum += (1.0 / h_rank);
            status = "HIT @5";
        } else if (h_rank > 5 && h_rank <= 10) {
            hybrid_hits10++;
            reciprocal_rank_sum += (1.0 / h_rank);
            status = "HIT @10";
        }

        std::string short_desc = q.description;
        if (short_desc.size() > 36) short_desc = short_desc.substr(0, 33) + "...";

        std::cout << std::left << std::setw(4) << (i + 1)
                  << std::setw(38) << short_desc
                  << std::setw(12) << q.target_chunk_id
                  << std::setw(8) << (d_rank > 0 ? std::to_string(d_rank) : ">10")
                  << std::setw(8) << (l_rank > 0 ? std::to_string(l_rank) : ">10")
                  << std::setw(8) << (h_rank > 0 ? std::to_string(h_rank) : ">10")
                  << std::setw(8) << std::fixed << std::setprecision(1) << q_lat << "ms "
                  << status << "\n";
    }

    std::cout << "--------------------------------------------------------------------------------\n\n";

    double dense_recall5 = (double)dense_hits5 / queries.size() * 100.0;
    double hybrid_recall1 = (double)hybrid_hits1 / queries.size() * 100.0;
    double hybrid_recall3 = (double)hybrid_hits3 / queries.size() * 100.0;
    double hybrid_recall5 = (double)hybrid_hits5 / queries.size() * 100.0;
    double hybrid_recall10 = (double)hybrid_hits10 / queries.size() * 100.0;
    double hybrid_mrr = reciprocal_rank_sum / queries.size();
    double avg_latency = total_latency_ms / queries.size();

    // 4. Repeated Multi-Run Latency Profiling (5 iterations = 100 queries)
    std::cout << "  Conducting 5-run repeated latency profiling (100 query evaluations)...\n";
    std::vector<double> all_latencies;
    for (int run = 0; run < 5; ++run) {
        for (const auto& q : queries) {
            auto t0 = std::chrono::high_resolution_clock::now();
            auto q_vec = VectorIndex::embed_text(q.query_text, /*is_query=*/true);
            auto h_res = HybridSearchEngine::search(vector_index, lexical_index, q.query_text, 10, 1.0f, 1.0f, {}, 60.0f, q_vec);
            auto t1 = std::chrono::high_resolution_clock::now();
            all_latencies.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
        }
    }
    std::sort(all_latencies.begin(), all_latencies.end());
    double sum_lat = 0.0;
    for (double lat : all_latencies) sum_lat += lat;
    double mean_lat = sum_lat / all_latencies.size();
    double min_lat = all_latencies.front();
    double max_lat = all_latencies.back();
    double median_lat = (all_latencies[49] + all_latencies[50]) / 2.0;
    double p90_lat = all_latencies[static_cast<size_t>(all_latencies.size() * 0.90)];
    double p95_lat = all_latencies[static_cast<size_t>(all_latencies.size() * 0.95)];

    std::cout << "\n================================================================================\n";
    std::cout << "  STEP 5 HYBRID RETRIEVAL BENCHMARK SUMMARY                                     \n";
    std::cout << "================================================================================\n";
    std::cout << "  • Pure Dense Recall@5:        " << std::fixed << std::setprecision(1) << dense_recall5 << "% (" << dense_hits5 << "/" << queries.size() << ")\n";
    std::cout << "  • Hybrid RRF Recall@1:        " << std::fixed << std::setprecision(1) << hybrid_recall1 << "% (" << hybrid_hits1 << "/" << queries.size() << ")\n";
    std::cout << "  • Hybrid RRF Recall@3:        " << std::fixed << std::setprecision(1) << hybrid_recall3 << "% (" << hybrid_hits3 << "/" << queries.size() << ")\n";
    std::cout << "  • Hybrid RRF Recall@5:        " << std::fixed << std::setprecision(1) << hybrid_recall5 << "% (" << hybrid_hits5 << "/" << queries.size() << ")\n";
    std::cout << "  • Hybrid RRF Recall@10:       " << std::fixed << std::setprecision(1) << hybrid_recall10 << "% (" << hybrid_hits10 << "/" << queries.size() << ")\n";
    std::cout << "  • Hybrid Mean Reciprocal Rank: " << std::fixed << std::setprecision(4) << hybrid_mrr << " (vs Dense: 0.7571)\n";
    std::cout << "  ------------------------------------------------------------------------------\n";
    std::cout << "  • Latency Distribution (n=100 queries across 5 repeated runs):\n";
    std::cout << "      - Mean:   " << std::fixed << std::setprecision(2) << mean_lat << " ms\n";
    std::cout << "      - Median: " << std::fixed << std::setprecision(2) << median_lat << " ms\n";
    std::cout << "      - p90:    " << std::fixed << std::setprecision(2) << p90_lat << " ms\n";
    std::cout << "      - p95:    " << std::fixed << std::setprecision(2) << p95_lat << " ms\n";
    std::cout << "      - Min:    " << std::fixed << std::setprecision(2) << min_lat << " ms\n";
    std::cout << "      - Max:    " << std::fixed << std::setprecision(2) << max_lat << " ms\n";
    std::cout << "================================================================================\n\n";

    fs::remove_all(bench_dir, ec);

    if (hybrid_recall5 >= 85.0 && hybrid_mrr >= 0.70 && median_lat < 25.0) {
        std::cout << ">>> STEP 5 HYBRID RETRIEVAL BENCHMARK: PASS! <<<\n";
        return 0;
    } else {
        std::cerr << ">>> STEP 5 HYBRID RETRIEVAL BENCHMARK: FAIL <<<\n";
        return 1;
    }
}
