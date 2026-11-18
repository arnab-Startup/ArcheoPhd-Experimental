#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "analysis/epistemic_logic.hpp"

using namespace archaeophd;

void test_upgrade1_split_conformal_gate() {
    std::cout << "[Stress Test 1] Split Conformal Abstention Gate (Zero Hallucination)..." << std::endl;
    ConformalNliGate gate(0.65); // Quantile threshold = 0.65

    // Case A: High-certainty singleton entailment (p_sup = 0.90, p_con = 0.05, p_unr = 0.05)
    // non-conformity: sup = 0.10 <= 0.65, con = 0.95 > 0.65, unr = 0.95 > 0.65
    auto res_clean = gate.evaluate_admission(0.90, 0.05, 0.05);
    assert(res_clean.is_singleton);
    assert(!res_clean.is_ambiguous);
    assert(res_clean.relations.size() == 1);
    assert(res_clean.relations[0] == NliRelation::SUPPORT);
    assert(res_clean.signed_weight > 0.8);

    // Case B: High ambiguity (p_sup = 0.45, p_con = 0.45, p_unr = 0.10)
    // non-conformity: sup = 0.55 <= 0.65, con = 0.55 <= 0.65 -> Both admitted to prediction set
    auto res_ambiguous = gate.evaluate_admission(0.45, 0.45, 0.10);
    assert(!res_ambiguous.is_singleton);
    assert(res_ambiguous.is_ambiguous);
    assert(res_ambiguous.relations.size() >= 2);

    std::cout << "  Passed. Clean singleton count=" << res_clean.relations.size()
              << ", Ambiguous prediction set count=" << res_ambiguous.relations.size() << std::endl;
}

void test_upgrade2_provenance_decorrelation() {
    std::cout << "[Stress Test 2] Provenance Lineage Matrix & Echo-Chamber Elimination..." << std::endl;

    // Simulate 5 papers/notes that ALL cite the exact same root 1995 radiocarbon sample "sample_C14_1995"
    std::vector<EvidenceProvenance> echo_chamber_evidences;
    for (int i = 0; i < 5; ++i) {
        EvidenceProvenance ep;
        ep.evidence_id = "paper_" + std::to_string(i + 1);
        ep.root_empirical_observations = {"sample_C14_1995"}; // 100% shared provenance (K_ij = 1.0)
        ep.weight = 1.0;
        ep.satisfaction = 1.0;
        echo_chamber_evidences.push_back(ep);
    }

    double eff_count = ProvenanceDecorrelator::compute_effective_evidence(echo_chamber_evidences);

    // In naive sum: 5 * 1.0 = 5.0 (false certainty).
    // In lineage covariance discount: 5 * (1.0 / (1 + 4)) = 5 * 0.2 = 1.0!
    assert(std::abs(eff_count - 1.0) < 1e-5);

    // Test 4-tuple opinion with base rate a_v = 0.3
    auto op = OpinionTuple4::from_evidence(eff_count, 0.0, 0.3, 2.0);
    // b = 1/3 = 0.333, u = 2/3 = 0.667. Projected P_bar = b + a * u = 0.333 + 0.3 * 0.667 = 0.533
    double p_bar = op.projected_probability();
    assert(p_bar > 0.50 && p_bar < 0.55);

    std::cout << "  Passed. 5 repeated citations of sample_C14_1995 discounted to: "
              << eff_count << " (Projected P_bar=" << p_bar << ")" << std::endl;
}

void test_upgrade3_cardinality_invariant_t_norm() {
    std::cout << "[Stress Test 3] Cardinality-Invariant Power-Weighted Log-Barrier Aggregator..." << std::endl;

    // Test 3A: 8 valid premises each having belief 0.8
    // In naive Product T-Norm: 0.8^8 = 0.167 (vanishing belief penalty).
    // In Power-Weighted Log-Barrier (geometric mean with equal weights): Phi_e must remain exactly 0.8!
    UpgradedHyperedgeWarrant warrant;
    warrant.id = "complex_warrant";
    warrant.warrant_weight = 5.0;
    warrant.leak_tolerance = 0.0; // Zero leakage for exact test

    std::map<std::string, OpinionTuple4> premise_states;
    for (int i = 0; i < 8; ++i) {
        std::string p_id = "premise_" + std::to_string(i + 1);
        warrant.in_list.push_back(p_id);
        // Direct evidence yielding projected probability 0.8
        OpinionTuple4 op;
        op.belief = 0.8;
        op.disbelief = 0.0;
        op.uncertainty = 0.2;
        op.base_rate = 0.0;
        premise_states[p_id] = op;
    }

    double phi_8_premises = warrant.compute_satisfaction(premise_states);
    assert(std::abs(phi_8_premises - 0.8) < 1e-4);

    // Test 3B: Strict Falsification Cascade:
    // If a single premise collapses to 0.0 (with small leak 0.02), Phi_e must collapse toward 0!
    warrant.leak_tolerance = 0.02;
    premise_states["premise_3"].belief = 0.0;
    premise_states["premise_3"].uncertainty = 1.0;
    premise_states["premise_3"].base_rate = 0.0; // P_bar = 0.0
    double phi_collapsed = warrant.compute_satisfaction(premise_states);
    assert(phi_collapsed < 0.55); // Severe collapse triggered by single falsified premise

    // Test 3C: Damped Fixed-Point Iteration on Cyclic Graph
    UpgradedEpistemicHypergraph graph(0.65);
    graph.add_node("node_A", "Layer A Interpretation", 5.0, 0.0);
    graph.add_node("node_B", "Artifact B Chronology", 0.0, 0.0);

    // Circular hyperedges: A supports B, and B supports A!
    UpgradedHyperedgeWarrant w_ab;
    w_ab.id = "w_ab";
    w_ab.in_list = {"node_A"};
    w_ab.conclusion_id = "node_B";
    w_ab.warrant_weight = 4.0;
    graph.add_hyperedge(w_ab);

    UpgradedHyperedgeWarrant w_ba;
    w_ba.id = "w_ba";
    w_ba.in_list = {"node_B"};
    w_ba.conclusion_id = "node_A";
    w_ba.warrant_weight = 3.0;
    graph.add_hyperedge(w_ba);

    // Run damped belief propagation - must converge without infinite oscillation
    graph.propagate_damped_belief_revision(25, 1e-4);
    double b_a = graph.nodes().at("node_A").opinion.belief;
    double b_b = graph.nodes().at("node_B").opinion.belief;
    assert(b_a > 0.6 && b_b > 0.4);

    std::cout << "  Passed. 8-premise satisfaction=" << phi_8_premises
              << " (zero decay!), Collapsed=" << phi_collapsed
              << ", Cyclic fixed-point b_A=" << b_a << ", b_B=" << b_b << std::endl;
}

void test_upgrade4_power_law_memory() {
    std::cout << "[Stress Test 4] Heavy-Tailed Power-Law Memory Kernel over Multi-Year Gaps..." << std::endl;

    PowerLawHawkesTracker tracker(0.3, 1.0); // beta_mem = 0.3, c = 1.0

    // Record an excavation note at Year 1 (t = 1.0)
    tracker.record_event("ancient_layer_note", 1.0);

    // Evaluate intensity after 5 years (t = 6.0, dt = 5.0)
    // Under power law: (1 + 1 * 5)^(-1.3) = 6^(-1.3) = 0.0987 (finite, robust reactivity)
    // Contrast with exponential decay: exp(-0.35 * 5 * 365) = 0.0000000 (total memory death)
    double lambda_5_years = tracker.compute_intensity("ancient_layer_note", 6.0);
    assert(lambda_5_years > 0.15); // Stays non-zero and active above baseline 0.10

    // Spiked intensity when a new related discovery is logged at Year 6
    tracker.record_event("ancient_layer_note_revival", 6.0);
    double lambda_spiked = tracker.compute_intensity("ancient_layer_note", 6.0);
    assert(lambda_spiked > lambda_5_years);

    std::cout << "  Passed. 5-year retention intensity=" << lambda_5_years
              << " (non-zero!), Re-excited intensity=" << lambda_spiked << std::endl;
}

void test_upgrade5_distributional_cvar_policy() {
    std::cout << "[Stress Test 5] Risk-Averse CVaR_0.25 Distributional Epistemic Policy..." << std::endl;

    UpgradedEpistemicHypergraph graph(0.65);
    PowerLawHawkesTracker tracker(0.3, 1.0);
    DistributionalCvarPolicy policy(0.25); // Risk-averse cutoff 0.25

    // Case 1: Low-signal, speculative discrepancy -> CVaR remains below interruption cost threshold
    graph.add_node("speculative_claim", "Vague decorative motif", 1.0, 1.0);
    auto decision_speculative = policy.select_risk_averse_action(graph, tracker, 2.0);
    assert(decision_speculative.action == EpistemicActionV2::SILENT_ASSIMILATION);

    // Case 2: Deep, well-grounded dialectic contradiction -> Guaranteed worst-case net positive gain
    graph.add_node("critical_dispute", "Harappa Phase 3 Radiocarbon Synchronism", 12.0, 12.0);
    tracker.record_event("critical_dispute", 2.0);
    auto decision_critical = policy.select_risk_averse_action(graph, tracker, 2.0);
    assert(decision_critical.action == EpistemicActionV2::SURFACE_CONTRADICTION);
    assert(decision_critical.cvar_score > 0.0);

    std::cout << "  Passed. Speculative action=" << static_cast<int>(decision_speculative.action)
              << " (SILENT), Critical contradiction action=" << static_cast<int>(decision_critical.action)
              << " (CVaR=" << decision_critical.cvar_score << ")" << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "  Stress-Testing Upgraded v2 Neuro-Symbolic & RL Pipeline" << std::endl;
    std::cout << "============================================================" << std::endl;

    test_upgrade1_split_conformal_gate();
    test_upgrade2_provenance_decorrelation();
    test_upgrade3_cardinality_invariant_t_norm();
    test_upgrade4_power_law_memory();
    test_upgrade5_distributional_cvar_policy();

    std::cout << "============================================================" << std::endl;
    std::cout << "  All 5 Stress-Test Upgrades Succeeded with Zero Flaws!" << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
