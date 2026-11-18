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
    std::cout << "  Testing Native Epistemic IPC Endpoints via Dispatcher" << std::endl;
    std::cout << "============================================================" << std::endl;

    // Use current working directory or test sandbox storage
    std::string testStorageDir = "experimental/tests/test_sandbox_storage";
    fs_compat::create_directories(testStorageDir);

    NativeStorage storage(testStorageDir);
    NativeContradictionEngine contradictions(storage);
    NativeThesisAuditor thesisAuditor(storage, contradictions);

    NativeIpcDispatcher dispatcher(&storage, &contradictions, &thesisAuditor);

    // -------------------------------------------------------------
    // Test 1: score_calibrated_nli
    // -------------------------------------------------------------
    std::cout << "[IPC Test 1] Invoking score_calibrated_nli..." << std::endl;
    json req1 = {
        {"id", "req-101"},
        {"action", "score_calibrated_nli"},
        {"payload", {
            {"logit_support", 3.0},
            {"logit_contradiction", -1.0},
            {"logit_unrelated", 0.0},
            {"temperature", 1.2}
        }}
    };

    std::string resp1_str = dispatcher.dispatch(req1.dump());
    json resp1 = json::parse(resp1_str);

    assert(resp1.value("id", "") == "req-101");
    assert(resp1.contains("result"));
    auto res1 = resp1["result"];
    assert(res1.contains("p_support"));
    assert(res1.contains("p_contradiction"));
    assert(res1.contains("signed_weight"));
    double w1 = res1.value("signed_weight", 0.0);
    assert(w1 > 0.8);
    std::cout << "  Passed. p_support=" << res1["p_support"]
              << ", p_contradiction=" << res1["p_contradiction"]
              << ", signed_weight=" << w1 << std::endl;

    // -------------------------------------------------------------
    // Test 2: Seed test claims & evidence, then get_epistemic_graph_state
    // -------------------------------------------------------------
    std::cout << "[IPC Test 2] Invoking get_epistemic_graph_state..." << std::endl;
    // Create test claims
    Claim c1;
    c1.id = "claim_phase_a";
    c1.project_id = "test_epistemic_proj";
    c1.claim_text = "Chandraketugarh Phase II begins in 300 BCE";
    storage.put_claim(c1);

    Claim c2;
    c2.id = "claim_phase_b";
    c2.project_id = "test_epistemic_proj";
    c2.claim_text = "Terracotta seal dates Layer 3 to Mauryan horizon";
    storage.put_claim(c2);

    // Create supporting & contradicting evidence
    EvidenceLink ev1;
    ev1.id = "ev_sup_1";
    ev1.project_id = "test_epistemic_proj";
    ev1.claim_id = "claim_phase_a";
    ev1.evidence_type = "supporting";
    storage.put_evidence(ev1);

    EvidenceLink ev2;
    ev2.id = "ev_con_1";
    ev2.project_id = "test_epistemic_proj";
    ev2.claim_id = "claim_phase_a";
    ev2.evidence_type = "contradicting";
    storage.put_evidence(ev2);

    json req2 = {
        {"id", "req-102"},
        {"action", "get_epistemic_graph_state"},
        {"projectId", "test_epistemic_proj"},
        {"payload", json::object()}
    };

    std::string resp2_str = dispatcher.dispatch(req2.dump());
    json resp2 = json::parse(resp2_str);

    assert(resp2.value("id", "") == "req-102");
    assert(resp2["result"]["total_nodes"] >= 2);
    auto nodes = resp2["result"]["nodes"];
    assert(nodes.is_array());
    std::cout << "  Passed. Computed states for " << nodes.size() << " epistemic nodes." << std::endl;
    for (const auto& node : nodes) {
        std::cout << "    Node " << node["id"]
                  << " -> b=" << node["belief"]
                  << ", d=" << node["disbelief"]
                  << ", u=" << node["uncertainty"]
                  << ", conflict_mass=" << node["conflict_mass"] << std::endl;
    }

    // -------------------------------------------------------------
    // Test 3: get_optimal_epistemic_intervention
    // -------------------------------------------------------------
    std::cout << "[IPC Test 3] Invoking get_optimal_epistemic_intervention..." << std::endl;
    json req3 = {
        {"id", "req-103"},
        {"action", "get_optimal_epistemic_intervention"},
        {"projectId", "test_epistemic_proj"},
        {"payload", {
            {"current_time", 2.0}
        }}
    };

    std::string resp3_str = dispatcher.dispatch(req3.dump());
    json resp3 = json::parse(resp3_str);

    assert(resp3.value("id", "") == "req-103");
    assert(resp3.contains("result"));
    auto decision = resp3["result"];
    assert(decision.contains("action_name"));
    assert(decision.contains("rationale"));
    std::cout << "  Passed. Selected Action: " << decision["action_name"]
              << " (" << decision["rationale"] << ")" << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << "  All Native Epistemic IPC Endpoints Functioning Perfectly!" << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
