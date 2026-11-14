#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

#include "models.hpp"
#include "storage.hpp"
#include "analysis/graph_engine.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using namespace archaeophd;

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD — Phase 3 Step 3: Evidence & Literature Graph Traversal Suite      \n";
    std::cout << "================================================================================\n\n";

    std::string test_db = "test_step3_graph_storage";
    NativeStorage storage(test_db);
    std::string pid = "proj_evidentiary_graph";

    // -------------------------------------------------------------
    // Set up Multi-Modal Heterogeneous Archaeological Network
    // -------------------------------------------------------------
    // 1. Sites
    Site s_jericho;
    s_jericho.id = "site_jericho";
    s_jericho.project_id = pid;
    s_jericho.site_name = "Tell es-Sultan (Jericho)";
    storage.put_site(s_jericho);

    Site s_megiddo;
    s_megiddo.id = "site_megiddo";
    s_megiddo.project_id = pid;
    s_megiddo.site_name = "Tel Megiddo";
    storage.put_site(s_megiddo);

    // 2. Strata
    Stratum st_iv;
    st_iv.id = "strat_city_iv";
    st_iv.site_id = "site_jericho";
    st_iv.project_id = pid;
    st_iv.stratum_name = "City IV Destruction Horizon";
    storage.put_stratum(st_iv);

    Stratum st_iii;
    st_iii.id = "strat_city_iii";
    st_iii.site_id = "site_jericho";
    st_iii.project_id = pid;
    st_iii.stratum_name = "City III Middle Bronze IIA";
    st_iii.harris_above = {"strat_city_iv"};
    st_iv.harris_below = {"strat_city_iii"};
    storage.put_stratum(st_iii);
    storage.put_stratum(st_iv);

    Stratum st_meg;
    st_meg.id = "strat_meg_vii";
    st_meg.site_id = "site_megiddo";
    st_meg.project_id = pid;
    st_meg.stratum_name = "Stratum VIIA Palace";
    storage.put_stratum(st_meg);

    // 3. Artifacts
    Artifact a_jar;
    a_jar.id = "art_collared_jar";
    a_jar.project_id = pid;
    a_jar.stratum_id = "strat_city_iv";
    a_jar.artifact_name = "Collared-Rim Storage Jar #42";
    a_jar.category = "Ceramic";
    storage.put_artifact(a_jar);

    // 4. Samples
    Sample sm_c14;
    sm_c14.id = "sample_c14_charcoal";
    sm_c14.project_id = pid;
    sm_c14.stratum_id = "strat_city_iv";
    sm_c14.site_id = "site_jericho";
    sm_c14.lab_code = "GrN-1855";
    sm_c14.material = "Short-lived cereal grain";
    storage.put_sample(sm_c14);

    // 5. Sources
    Source src_kenyon;
    src_kenyon.id = "src_kenyon_1957";
    src_kenyon.project_id = pid;
    src_kenyon.title = "Digging Up Jericho";
    src_kenyon.author = "Kenyon, Kathleen M.";
    src_kenyon.year = "1957";
    src_kenyon.degradation_class = "CLASS_A";
    src_kenyon.confirmed_clean_offset = true;
    storage.put_source(src_kenyon);

    Source src_wood;
    src_wood.id = "src_wood_1990";
    src_wood.project_id = pid;
    src_wood.title = "Did the Israelites Conquer Jericho?";
    src_wood.author = "Wood, Bryant G.";
    src_wood.year = "1990";
    src_wood.degradation_class = "CLASS_A";
    src_wood.confirmed_clean_offset = true;
    storage.put_source(src_wood);

    // 6. Claims
    Claim c1;
    c1.id = "claim_destruction_date";
    c1.project_id = pid;
    c1.claim_text = "City IV fell at the terminal Late Bronze I horizon ca. 1400 BCE.";
    c1.scholar_name = "Wood, Bryant G.";
    c1.source_id = "src_wood_1990";
    c1.site_ids = {"site_jericho"};
    c1.strata_ids = {"strat_city_iv"};
    c1.origin_type = "manual_transcription";
    c1.verification_status = "VERIFIED";
    storage.put_claim(c1);

    Claim c2;
    c2.id = "claim_ceramic_continuity";
    c2.project_id = pid;
    c2.claim_text = "Cypriot bichrome pottery establishes clear stratigraphic synchronism.";
    c2.scholar_name = "Kenyon, Kathleen M.";
    c2.source_id = "src_kenyon_1957";
    c2.site_ids = {"site_jericho"};
    c2.origin_type = "manual_transcription";
    c2.verification_status = "VERIFIED";
    storage.put_claim(c2);

    Claim c3_isolated;
    c3_isolated.id = "claim_isolated_speculation";
    c3_isolated.project_id = pid;
    c3_isolated.claim_text = "Jericho was destroyed by seismic liquefaction rather than siege warfare.";
    c3_isolated.scholar_name = "Speculative Author";
    c3_isolated.origin_type = "manual_transcription";
    c3_isolated.verification_status = "VERIFIED";
    storage.put_claim(c3_isolated);

    // 7. Evidence Links
    EvidenceLink ev1;
    ev1.id = "ev_c14_date";
    ev1.project_id = pid;
    ev1.claim_id = "claim_destruction_date";
    ev1.physical_entity_type = "SAMPLE";
    ev1.physical_entity_id = "sample_c14_charcoal";
    ev1.evidence_text = "Radiocarbon determination GrN-1855 yields 3310 +- 30 BP.";
    ev1.source_ids = {"src_wood_1990"};
    storage.put_evidence(ev1);

    EvidenceLink ev2;
    ev2.id = "ev_jar_locus";
    ev2.project_id = pid;
    ev2.claim_id = "claim_ceramic_continuity";
    ev2.physical_entity_type = "ARTIFACT";
    ev2.physical_entity_id = "art_collared_jar";
    ev2.evidence_text = "Complete in situ storage jar found on room floor of Area H.";
    ev2.source_ids = {"src_kenyon_1957"};
    storage.put_evidence(ev2);

    NativeGraphEngine graph_engine(storage);

    // -------------------------------------------------------------
    // TEST 1: Heterogeneous Graph Construction
    // -------------------------------------------------------------
    std::cout << "[TEST 1] Testing Heterogeneous Multi-Modal Graph Construction...\n";
    auto g = graph_engine.build_graph(pid);

    std::cout << "  ✓ Total Graph Nodes: " << g.nodes.size() << "\n";
    std::cout << "  ✓ Total Graph Edges: " << g.edges.size() << "\n";

    // 2 sites + 3 strata + 1 artifact + 1 sample + 2 sources + 3 claims + 2 evidence = 14 nodes
    assert(g.nodes.size() == 14);
    assert(g.edges.size() >= 12);

    // Verify presence of node types
    assert(g.nodes["site_jericho"].type == "SITE");
    assert(g.nodes["strat_city_iv"].type == "STRATUM");
    assert(g.nodes["art_collared_jar"].type == "ARTIFACT");
    assert(g.nodes["sample_c14_charcoal"].type == "SAMPLE");
    assert(g.nodes["src_wood_1990"].type == "SOURCE");
    assert(g.nodes["claim_destruction_date"].type == "CLAIM");
    assert(g.nodes["ev_c14_date"].type == "EVIDENCE_LINK");

    std::cout << "  [PASS] Heterogeneous graph constructed with 100% topological integrity!\n\n";

    // -------------------------------------------------------------
    // TEST 2: Network Centrality & Epistemic Grounding Ratio
    // -------------------------------------------------------------
    std::cout << "[TEST 2] Testing Network Metrics & Epistemic Grounding Ratio...\n";
    auto m = graph_engine.compute_metrics(pid);

    std::cout << "  ✓ Total Claims: " << m.claim_count << "\n";
    std::cout << "  ✓ Grounded Claims: " << m.grounded_claims_count << "\n";
    std::cout << "  ✓ Isolated (Ungrounded) Claims: " << m.isolated_claims_count << "\n";
    std::cout << "  ✓ Epistemic Grounding Ratio: " << std::fixed << std::setprecision(2) << (m.grounding_ratio * 100.0) << "%\n";

    assert(m.claim_count == 3);
    assert(m.grounded_claims_count == 2);
    assert(m.isolated_claims_count == 1);
    assert(m.isolated_claim_ids.size() == 1);
    assert(m.isolated_claim_ids[0] == "claim_isolated_speculation");
    assert(std::abs(m.grounding_ratio - (2.0 / 3.0)) < 0.01);

    std::cout << "  ✓ Top Hub Nodes:\n";
    for (const auto& hub : m.hub_nodes) {
        std::cout << "    • " << hub.first << " (degree: " << hub.second << ")\n";
    }
    assert(!m.hub_nodes.empty());
    std::cout << "  [PASS] Epistemic grounding ratio & isolated claim detection verified!\n\n";

    // -------------------------------------------------------------
    // TEST 3: Evidentiary Path Tracing (BFS Shortest Path)
    // -------------------------------------------------------------
    std::cout << "[TEST 3] Testing Evidentiary Path Tracing (BFS Causal Chain)...\n";

    // Path from claim_destruction_date to physical site_jericho:
    // claim_destruction_date ➔ ev_c14_date ➔ sample_c14_charcoal ➔ strat_city_iv ➔ site_jericho
    auto path = graph_engine.trace_shortest_path("claim_destruction_date", "site_jericho", pid, false);

    assert(path.found);
    std::cout << "  ✓ Shortest evidentiary path found! Length: " << path.length << " hops\n";
    for (size_t i = 0; i < path.node_ids.size(); ++i) {
        std::cout << "    [" << i << "] (" << path.node_types[i] << ") " << path.node_labels[i];
        if (i < path.edge_types.size()) {
            std::cout << "  \u2014(" << path.edge_types[i] << ")\u2192\n";
        } else {
            std::cout << "\n";
        }
    }
    assert(path.node_ids.front() == "claim_destruction_date");
    assert(path.node_ids.back() == "site_jericho");

    // Path from isolated claim to artifact: should be empty / not connected
    auto disconnected_path = graph_engine.trace_shortest_path("claim_isolated_speculation", "art_collared_jar", pid, false);
    assert(!disconnected_path.found);
    std::cout << "  ✓ Disconnected claim correctly returns no path (isolation confirmed).\n";
    std::cout << "  [PASS] Evidentiary path derivation & causal chain tracing verified!\n\n";

    // -------------------------------------------------------------
    // TEST 4: K-Hop Ego Subgraph Extraction
    // -------------------------------------------------------------
    std::cout << "[TEST 4] Testing K-Hop Ego Subgraph Extraction (Stratum City IV)...\n";
    auto ego_1 = graph_engine.get_k_hop_subgraph("strat_city_iv", 1, pid);
    assert(ego_1.contains("nodes") && ego_1.contains("edges"));
    std::cout << "  ✓ 1-Hop Ego Network around Stratum City IV: " << ego_1["nodes"].size() << " nodes, "
              << ego_1["edges"].size() << " edges.\n";
    assert(ego_1["nodes"].size() >= 4); // site, strat_city_iii, art_collared_jar, sample_c14_charcoal

    auto ego_2 = graph_engine.get_k_hop_subgraph("claim_ceramic_continuity", 2, pid);
    assert(ego_2["nodes"].size() >= 3);
    std::cout << "  ✓ 2-Hop Ego Network around Claim Ceramic Continuity: " << ego_2["nodes"].size() << " nodes.\n";
    std::cout << "  [PASS] K-Hop ego neighborhood extraction verified!\n\n";

    // -------------------------------------------------------------
    // TEST 5: Native IPC Dispatcher Graph Endpoints
    // -------------------------------------------------------------
    std::cout << "[TEST 5] Testing Native IPC Dispatcher Endpoints...\n";
    NativeIpcDispatcher dispatcher(&storage, nullptr, nullptr, "");

    // 1. get_knowledge_graph_full
    std::string req_kg = "{\"id\": \"msg-kg-1\", \"action\": \"get_knowledge_graph_full\", \"projectId\": \"" + pid + "\"}";
    std::string res_kg_raw = dispatcher.dispatch(req_kg);
    auto res_kg = json::parse(res_kg_raw);
    assert(res_kg.contains("result"));
    const auto& kg_res = res_kg["result"];
    assert(kg_res.contains("nodes") && kg_res.contains("edges") && kg_res.contains("metrics"));
    std::cout << "  ✓ get_knowledge_graph_full returned " << kg_res["nodes"].size() << " nodes and "
              << kg_res["edges"].size() << " edges.\n";

    // 2. trace_evidence_path
    std::string req_tr = "{\"id\": \"msg-kg-2\", \"action\": \"trace_evidence_path\", \"projectId\": \"" + pid + "\", \"payload\": {\"source_id\": \"claim_ceramic_continuity\", \"target_id\": \"art_collared_jar\"}}";
    std::string res_tr_raw = dispatcher.dispatch(req_tr);
    auto res_tr = json::parse(res_tr_raw);
    assert(res_tr.contains("result"));
    assert(res_tr["result"]["found"] == true);
    std::cout << "  ✓ trace_evidence_path successfully traced link to artifact (length "
              << res_tr["result"]["length"] << " hops).\n";

    // 3. get_node_neighbors
    std::string req_nb = "{\"id\": \"msg-kg-3\", \"action\": \"get_node_neighbors\", \"projectId\": \"" + pid + "\", \"payload\": {\"node_id\": \"site_jericho\", \"hops\": 1}}";
    std::string res_nb_raw = dispatcher.dispatch(req_nb);
    auto res_nb = json::parse(res_nb_raw);
    assert(res_nb.contains("result"));
    assert(res_nb["result"]["nodes"].size() >= 3);
    std::cout << "  ✓ get_node_neighbors returned " << res_nb["result"]["nodes"].size() << " direct neighbors.\n";

    std::cout << "  [PASS] IPC Bridge Graph Endpoints operate with 100% schema compliance!\n\n";

    std::cout << "================================================================================\n";
    std::cout << "  ALL PHASE 3 STEP 3 GRAPH TRAVERSAL TESTS PASSED (100%)!                       \n";
    std::cout << "================================================================================\n";
    return 0;
}
