#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <json.hpp>
#include "models.hpp"
#include "storage/storage.hpp"
#include "analysis/contradictions.hpp"
#include "analysis/thesis_audit.hpp"
#include "ipc/native_ipc_dispatcher.hpp"

using json = nlohmann::json;
using namespace archaeophd;

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "  Testing Upgraded v2 Native Epistemic IPC Endpoints" << std::endl;
    std::cout << "============================================================" << std::endl;

    std::string testStorageDir = "experimental/tests/test_sandbox_storage_v2";
    fs_compat::create_directories(testStorageDir);

    NativeStorage storage(testStorageDir);
    NativeContradictionEngine contradictions(storage);
    NativeThesisAuditor thesisAuditor(storage, contradictions);

    NativeIpcDispatcher dispatcher(&storage, &contradictions, &thesisAuditor);

    // -------------------------------------------------------------
    // Test 1: score_calibrated_nli with Split Conformal Gate
    // -------------------------------------------------------------
    std::cout << "[IPC Test 1] Invoking score_calibrated_nli..." << std::endl;
    json req1 = {
        {"id", "req-201"},
        {"action", "score_calibrated_nli"},
        {"payload", {
            {"p_support", 0.92},
            {"p_contradiction", 0.04},
            {"p_unrelated", 0.04},
            {"quantile_threshold", 0.65}
        }}
    };

    std::string resp1_str = dispatcher.dispatch(req1.dump());
    json resp1 = json::parse(resp1_str);

    assert(resp1.value("id", "") == "req-201");
    assert(resp1.contains("result"));
    auto res1 = resp1["result"];
    assert(res1["accepted_for_hypergraph"] == true);
    assert(res1["routing"] == "ADMITTED_TO_HYPERGRAPH");
    std::cout << "  Passed. Admitted to hypergraph with signed weight=" << res1["signed_weight"] << std::endl;

    // -------------------------------------------------------------
    // Test 2: Seed repeated provenance citations, invoke get_epistemic_graph_state
    // -------------------------------------------------------------
    std::cout << "[IPC Test 2] Invoking get_epistemic_graph_state with provenance overlap..." << std::endl;
    Claim c1;
    c1.id = "claim_site_chronology";
    c1.project_id = "test_v2_proj";
    c1.claim_text = "Chandraketugarh Rampart constructed in 2nd Century BCE";
    storage.put_claim(c1);

    // Seed 4 evidence links citing the SAME root sample "c14_charcoal_layer3"
    for (int i = 1; i <= 4; ++i) {
        EvidenceLink ev;
        ev.id = "ev_citation_" + std::to_string(i);
        ev.project_id = "test_v2_proj";
        ev.claim_id = "claim_site_chronology";
        ev.evidence_type = "supporting";
        ev.source_ids = {"c14_charcoal_layer3"};
        storage.put_evidence(ev);
    }

    json req2 = {
        {"id", "req-202"},
        {"action", "get_epistemic_graph_state"},
        {"projectId", "test_v2_proj"},
        {"payload", json::object()}
    };

    std::string resp2_str = dispatcher.dispatch(req2.dump());
    json resp2 = json::parse(resp2_str);

    assert(resp2.value("id", "") == "req-202");
    auto nodes = resp2["result"]["nodes"];
    assert(nodes.size() >= 1);
    auto node = nodes[0];
    // In naive sum: r_plus would be 4.0, belief would be 4/6 = 0.667
    // In provenance overlap discount: 4 citations of exact same sample discount to ~1.0, belief is ~1/3 = 0.333!
    double belief = node["belief"];
    assert(belief < 0.40); // Provenance discount successfully prevented echo-chamber inflation!
    std::cout << "  Passed. Provenance-discounted node: b=" << belief
              << ", u=" << node["uncertainty"]
              << ", projected_prob=" << node["projected_probability"] << std::endl;

    // -------------------------------------------------------------
    // Test 3: get_optimal_epistemic_intervention via CVaR_0.25
    // -------------------------------------------------------------
    std::cout << "[IPC Test 3] Invoking get_optimal_epistemic_intervention via CVaR..." << std::endl;
    json req3 = {
        {"id", "req-203"},
        {"action", "get_optimal_epistemic_intervention"},
        {"projectId", "test_v2_proj"},
        {"payload", {
            {"current_time", 2.0}
        }}
    };

    std::string resp3_str = dispatcher.dispatch(req3.dump());
    json resp3 = json::parse(resp3_str);

    assert(resp3.value("id", "") == "req-203");
    auto decision = resp3["result"];
    assert(decision.contains("action_name"));
    assert(decision.contains("cvar_score"));
    std::cout << "  Passed. Selected Action: " << decision["action_name"]
              << " (" << decision["policy"] << ")" << std::endl;

    // Clean up test sandbox storage
    try { std::filesystem::remove_all(testStorageDir); } catch (...) {}

    std::cout << "============================================================" << std::endl;
    std::cout << "  All Upgraded v2 Epistemic IPC Endpoints Functioning 100%!" << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
