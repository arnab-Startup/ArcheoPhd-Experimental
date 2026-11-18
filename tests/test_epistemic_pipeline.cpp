#include <iostream>
#include <cassert>
#include <cmath>
#include "analysis/epistemic_logic.hpp"

using namespace archaeophd;

void test_stage1_calibrated_nli() {
    std::cout << "[Test 1] Calibrated NLI Cross-Encoder Scoring..." << std::endl;
    // Test neutral logits with temperature calibration
    auto p1 = NliProbabilities::from_logits(2.5, -1.0, 0.5, 1.2);
    assert(p1.p_support > p1.p_contradiction);
    assert(p1.signed_weight > 0.0);
    assert(p1.signed_weight <= 1.0);

    // Test strong contradiction logits
    auto p2 = NliProbabilities::from_logits(-2.0, 3.5, 0.0, 1.0);
    assert(p2.p_contradiction > p2.p_support);
    assert(p2.signed_weight < 0.0);
    assert(p2.signed_weight >= -1.0);

    std::cout << "  Passed. Signed weights: p1=" << p1.signed_weight << ", p2=" << p2.signed_weight << std::endl;
}

void test_stage2_subjective_logic() {
    std::cout << "[Test 2] Subjective Logic & Conflict Mass..." << std::endl;
    // Case A: Total ignorance (0 evidence)
    auto op_empty = OpinionTuple::from_evidence(0.0, 0.0, 2.0);
    assert(std::abs(op_empty.uncertainty - 1.0) < 1e-5);
    assert(std::abs(op_empty.belief - 0.0) < 1e-5);
    assert(std::abs(op_empty.conflict_mass() - 0.0) < 1e-5);

    // Case B: High consensus corroboration (10 positive, 0 negative)
    auto op_corrob = OpinionTuple::from_evidence(10.0, 0.0, 2.0);
    assert(op_corrob.belief > 0.8);
    assert(op_corrob.uncertainty < 0.2);
    assert(op_corrob.conflict_mass() < 0.05);

    // Case C: Intense dialectic contradiction (10 positive, 10 negative)
    auto op_conflict = OpinionTuple::from_evidence(10.0, 10.0, 2.0);
    assert(std::abs(op_conflict.belief - op_conflict.disbelief) < 1e-5);
    assert(op_conflict.uncertainty < 0.1);
    double c_mass = op_conflict.conflict_mass();
    // 4 * (10/22) * (10/22) * (1 - 2/22) = 4 * 0.4545 * 0.4545 * 0.909 = ~0.751
    assert(c_mass > 0.70);

    std::cout << "  Passed. Conflict Mass: empty=" << op_empty.conflict_mass()
              << ", corrob=" << op_corrob.conflict_mass()
              << ", conflict=" << c_mass << std::endl;
}

void test_stage3_hypergraph_belief_revision() {
    std::cout << "[Test 3] Directed Hypergraph Belief Revision..." << std::endl;
    EpistemicHypergraph graph;

    // Node 1: Primary sealed floor (in-situ evidence)
    graph.add_node("stratum_c4", "Sealed Layer 4", 8.0, 0.5);
    // Node 2: Radiocarbon assay
    graph.add_node("c14_assay", "OxA-1234 Charcoal Date", 9.0, 0.2);
    // Node 3: Defeater / Intrusion (initially believed false)
    graph.add_node("intrusive_pit", "Intrusive Hellenistic Pit", 0.0, 5.0);
    // Target Hypothesis: Foundation Date 500 BCE (starts with 0 direct evidence, depends on warrant)
    graph.add_node("hypo_date", "Site Founded 500 BCE", 0.0, 0.0);

    // Hyperedge: (stratum_c4 AND c14_assay AND NOT intrusive_pit) => hypo_date
    HyperedgeWarrant warrant;
    warrant.id = "warrant_chronology";
    warrant.in_list = {"stratum_c4", "c14_assay"};
    warrant.out_list = {"intrusive_pit"};
    warrant.conclusion_id = "hypo_date";
    warrant.warrant_weight = 5.0; // Strong scientific warrant providing up to 5.0 evidence mass
    warrant.leak_tolerance = 0.02;

    graph.add_hyperedge(warrant);

    // Step 3A: Initial propagation -> Hypothesis should be strongly corroborated
    graph.propagate_belief_revision();
    double b_initial = graph.nodes().at("hypo_date").opinion.belief;
    assert(b_initial > 0.4);

    // Step 3B: Downward Invalidation:
    // A new micromorphology study proves stratum_c4 was NOT sealed but bioturbated!
    graph.add_node("stratum_c4", "Sealed Layer 4 (Disproved)", 0.0, 10.0);
    graph.propagate_belief_revision();
    double b_invalidated = graph.nodes().at("hypo_date").opinion.belief;
    assert(b_invalidated < b_initial);

    // Step 3C: Upward Exoneration:
    // Restore stratum_c4, but activate defeater intrusive_pit
    graph.add_node("stratum_c4", "Sealed Layer 4", 8.0, 0.5);
    graph.add_node("intrusive_pit", "Intrusive Pit Confirmed", 10.0, 0.0);
    graph.propagate_belief_revision();
    double b_blocked = graph.nodes().at("hypo_date").opinion.belief;
    assert(b_blocked < 0.25);

    // Now GPR survey disproves the intrusive pit completely (Upward Exoneration)
    graph.add_node("intrusive_pit", "Intrusive Pit Disproved by GPR", 0.0, 10.0);
    graph.propagate_belief_revision();
    double b_exonerated = graph.nodes().at("hypo_date").opinion.belief;
    assert(b_exonerated > b_blocked);
    assert(graph.nodes().at("hypo_date").is_exonerated);

    std::cout << "  Passed. Initial=" << b_initial
              << ", Invalidated=" << b_invalidated
              << ", Blocked=" << b_blocked
              << ", Exonerated=" << b_exonerated << std::endl;
}

void test_stage4_hawkes_activation() {
    std::cout << "[Test 4] Neural Hawkes Intensity Tracker..." << std::endl;
    HawkesPerspectiveTracker tracker(0.2); // Decay delta = 0.2

    // Record an event 4 years ago (t = 1.0)
    tracker.record_event("harappan_seal_1", 1.0);

    // Check intensity at year 5 (t = 5.0) before any new discovery
    double lambda_dormant = tracker.compute_intensity("harappan_seal_1", 5.0);

    // Now a new discovery occurs today (t = 5.0) on a related seal
    tracker.record_event("harappan_seal_2", 5.0);
    double lambda_spiked = tracker.compute_intensity("harappan_seal_1", 5.0);

    assert(lambda_spiked > lambda_dormant);
    std::cout << "  Passed. Dormant intensity=" << lambda_dormant
              << ", Spiked intensity=" << lambda_spiked << std::endl;
}

void test_stage5_conservative_q_policy() {
    std::cout << "[Test 5] Conservative Q-Learning Epistemic Intervention Policy..." << std::endl;
    EpistemicHypergraph graph;
    HawkesPerspectiveTracker tracker(0.2);
    ConservativeQPolicy policy;

    // Case 1: Quiet state with no conflicts -> Action should be SILENT_ASSIMILATION
    graph.add_node("calm_finding", "Standard Brick Dimensions", 5.0, 0.0);
    auto res_quiet = policy.select_action(graph, tracker, 2.0);
    assert(res_quiet.action == EpistemicAction::SILENT_ASSIMILATION);

    // Case 2: Severe contradiction on active theme -> Action should be SURFACE_CONTRADICTION
    graph.add_node("disputed_chronology", "Urbanization Horizon", 12.0, 12.0);
    tracker.record_event("disputed_chronology", 2.0);
    auto res_conflict = policy.select_action(graph, tracker, 2.0);
    assert(res_conflict.action == EpistemicAction::SURFACE_CONTRADICTION);
    assert(res_conflict.target_node_id == "disputed_chronology");

    std::cout << "  Passed. Quiet action=" << static_cast<int>(res_quiet.action)
              << ", Conflict action=" << static_cast<int>(res_conflict.action)
              << " (" << res_conflict.rationale << ")" << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "  Running Epistemic Neuro-Symbolic & RL Pipeline Test Suite" << std::endl;
    std::cout << "============================================================" << std::endl;

    test_stage1_calibrated_nli();
    test_stage2_subjective_logic();
    test_stage3_hypergraph_belief_revision();
    test_stage4_hawkes_activation();
    test_stage5_conservative_q_policy();

    std::cout << "============================================================" << std::endl;
    std::cout << "  All 5 Epistemic Pipeline Stages Passed with 100% Precision!" << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
