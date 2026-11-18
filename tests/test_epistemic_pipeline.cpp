#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "analysis/epistemic_logic.hpp"

using namespace archaeophd;

void test_full_5year_archaeological_timeline() {
    std::cout << "============================================================" << std::endl;
    std::cout << "  5-Year Archaeological Test Case (Ground Truth Setup)" << std::endl;
    std::cout << "============================================================" << std::endl;

    // -------------------------------------------------------------------------
    // Year 1 (t = 0 months): Ingestion of O_1, O_2, A_1, L_1
    // -------------------------------------------------------------------------
    std::cout << "[Timeline: Year 1 (t = 0)] Ingestion & Initial Beliefs..." << std::endl;

    // Observation O_2: Unburnt bone tool (high certainty P_bar = 0.95)
    // Assumption A_1: Soil is acidic pH 4.2 (P_bar = 0.75, b = 0.50, u = 0.50)
    // Physical Law L_1: Unburnt bone dissolves below pH 5.5
    // Defeater D_1: "O_2 is modern rodent intrusion" (b_D1 = 0.85)
    OpinionTuple4 op_O2 = OpinionTuple4::from_evidence(19.0, 0.0, 0.5, 2.0); // P_bar = 0.95
    OpinionTuple4 op_A1_yr1 = OpinionTuple4::from_evidence(2.0, 0.0, 0.5, 2.0); // b = 0.50, P_bar = 0.75
    OpinionTuple4 op_D1_yr1;
    op_D1_yr1.belief = 0.85;
    op_D1_yr1.disbelief = 0.05;
    op_D1_yr1.uncertainty = 0.10;

    // Hypothesis H_bone ("Ancient bone-working craft area")
    // Warrant e_bone: In-List = {O_2}, Defeater Out-List = {D_1}
    UpgradedHyperedgeWarrant w_bone;
    w_bone.id = "w_bone";
    w_bone.in_list = {"O_2"};
    w_bone.out_list = {"D_1"};
    w_bone.conclusion_id = "H_bone";
    w_bone.warrant_weight = 5.0;
    w_bone.leak_tolerance = 0.02;

    std::map<std::string, OpinionTuple4> states_yr1 = {
        {"O_2", op_O2},
        {"A_1", op_A1_yr1},
        {"D_1", op_D1_yr1}
    };

    double phi_bone_yr1 = w_bone.compute_satisfaction(states_yr1);
    // (0.95) * (1 - 0.98 * 0.85) = 0.95 * (1 - 0.833) = 0.95 * 0.167 = ~0.158
    // With strict defeater veto: 0.95 * (1 - 0.85) = 0.1425
    std::cout << "  Year 1: H_bone warrant satisfaction = " << phi_bone_yr1 << " (ABANDONED)" << std::endl;
    assert(phi_bone_yr1 < 0.20);

    // -------------------------------------------------------------------------
    // Years 2–3 (t = 12..36 months): 4 duplicate notes citing O_1 and A_1
    // -------------------------------------------------------------------------
    std::cout << "[Timeline: Years 2-3 (t = 12..36)] Duplicate Notes & Root Lineage..." << std::endl;

    std::vector<RootLineageWarrant> duplicate_notes;
    for (int i = 1; i <= 4; ++i) {
        RootLineageWarrant nw;
        nw.warrant_id = "note_N" + std::to_string(i);
        nw.root_observations = {"O_1", "A_1"}; // Identical root ancestors
        nw.weight = 1.0;
        nw.satisfaction = 0.80; // Each note reports 0.80 validity
        duplicate_notes.push_back(nw);
    }

    double eff_evidence = RootLineageProjector::compute_effective_evidence(duplicate_notes);
    std::cout << "  4 duplicate notes citing {O_1, A_1} yield effective evidence = " << eff_evidence << std::endl;
    // Must be exactly 0.800 (0.0% phantom inflation)
    assert(std::abs(eff_evidence - 0.800) < 1e-4);

    // Warrant for H_wash: In-List = {O_1, A_1} with weights 0.5, 0.5
    OpinionTuple4 op_O1;
    op_O1.belief = 0.95;
    op_O1.disbelief = 0.0;
    op_O1.uncertainty = 0.0; // P_bar = 0.95
    op_O1.base_rate = 0.5;

    UpgradedHyperedgeWarrant w_wash;
    w_wash.id = "w_wash";
    w_wash.in_list = {"O_1", "A_1"};
    w_wash.in_weights = {0.5, 0.5};
    w_wash.conclusion_id = "H_wash";
    w_wash.leak_tolerance = 0.02;

    std::map<std::string, OpinionTuple4> states_wash_yr2 = {
        {"O_1", op_O1},
        {"A_1", op_A1_yr1}
    };
    double phi_wash_before = w_wash.compute_satisfaction(states_wash_yr2);
    // sqrt(0.95 * 0.75) * (1 - 0) * (1 - 0) = sqrt(0.7125) = 0.8441
    std::cout << "  Year 2-3: H_wash warrant satisfaction before lab report = " << phi_wash_before << std::endl;
    assert(std::abs(phi_wash_before - 0.8441) < 1e-2);

    // -------------------------------------------------------------------------
    // Year 5 (t = 60 months): Lab report x_60 arrives!
    // -------------------------------------------------------------------------
    std::cout << "[Timeline: Year 5 (t = 60)] Shockwave Lab Report x_60 Arrives..." << std::endl;

    // Falsification of Assumption A_1:
    // b = 0.05, d = 0.76, u = 0.19, base_rate = 0.5 -> P_bar = 0.05 + 0.5 * 0.19 = 0.145
    OpinionTuple4 op_A1_yr5;
    op_A1_yr5.belief = 0.05;
    op_A1_yr5.disbelief = 0.76;
    op_A1_yr5.uncertainty = 0.19;
    op_A1_yr5.base_rate = 0.5;

    std::map<std::string, OpinionTuple4> states_wash_yr5 = {
        {"O_1", op_O1},
        {"A_1", op_A1_yr5}
    };

    // Downward Invalidation on H_wash:
    double phi_wash_after = w_wash.compute_satisfaction(states_wash_yr5);
    // sqrt(0.95 * 0.145) * (1 - 0) * (1 - 0.98 * 0.76) = 0.371 * 0.255 = ~0.094
    std::cout << "  Downward Invalidation: H_wash satisfaction drops to = " << phi_wash_after << " (UNDERMINED)" << std::endl;
    assert(phi_wash_after < 0.11); // Collapses cleanly without cushioning!

    // Upward Exoneration on H_bone:
    // When A_1 collapses, Defeater D_1 ("rodent intrusion") collapses: b_D1 drops to 0.06
    OpinionTuple4 op_D1_yr5;
    op_D1_yr5.belief = 0.06;
    op_D1_yr5.disbelief = 0.85;
    op_D1_yr5.uncertainty = 0.09;

    std::map<std::string, OpinionTuple4> states_bone_yr5 = {
        {"O_2", op_O2},
        {"D_1", op_D1_yr5}
    };

    double phi_bone_after = w_bone.compute_satisfaction(states_bone_yr5);
    // 0.95 * (1 - 0.98 * 0.06) = 0.95 * 0.9412 = ~0.894
    std::cout << "  Upward Exoneration: H_bone satisfaction surges to = " << phi_bone_after << " (REACTIVATED)" << std::endl;
    assert(phi_bone_after > 0.85);

    // -------------------------------------------------------------------------
    // Stage 4: Shockwave-Gated Temporal Excitation across 60 Months
    // -------------------------------------------------------------------------
    std::cout << "[Timeline: Year 5 (t = 60)] Temporal Memory & Shockwave Gate..." << std::endl;
    ShockwaveHawkesTracker tracker(0.3, 0.5, 2.0); // beta_mem = 0.3, c = 0.5, eta_shock = 2.0
    tracker.record_event("H_bone", 0.0); // Logged at Month 0

    // During routine work at Month 59 (|Delta Phi| = 0.0)
    double lambda_routine = tracker.compute_intensity("H_bone", 59.0, 0.0, 0.85, 0.01);
    std::cout << "  Routine intensity at Month 59 (dormant) = " << lambda_routine << std::endl;
    assert(lambda_routine < 0.02); // Sits quietly, zero clutter

    // At Month 60 when x_60 arrives, |Delta Phi| = 0.894 - 0.142 = 0.752
    double delta_phi = phi_bone_after - phi_bone_yr1;
    double lambda_shockwave = tracker.compute_intensity("H_bone", 60.0, delta_phi, 0.85, 0.01);
    // lambda = 0.0035 + 2.0 * 0.752 * 0.85 = ~1.28
    std::cout << "  Shockwave intensity at Month 60 (lab report arrival) = " << lambda_shockwave << std::endl;
    assert(lambda_shockwave > 1.20); // Spikes straight to Rank #1

    // -------------------------------------------------------------------------
    // Stage 5: Risk-Averse CVaR Action Selection
    // -------------------------------------------------------------------------
    std::cout << "[Timeline: Year 5 (t = 60)] Risk-Averse Policy Decision..." << std::endl;
    UpgradedEpistemicHypergraph hypergraph(0.65);
    hypergraph.add_node("H_bone", "Stratum 3 Ancient Bone-Working Craft Area", 1.0, 0.0);
    hypergraph.add_hyperedge(w_bone);
    hypergraph.mutable_nodes().at("H_bone").is_exonerated = true;
    hypergraph.mutable_nodes().at("H_bone").last_shockwave_delta = delta_phi;

    DistributionalCvarPolicy policy(0.25);
    auto decision = policy.select_risk_averse_action(hypergraph, tracker, 60.0);

    assert(decision.action == EpistemicActionV2::REACTIVATE_FORGOTTEN_INSIGHT);
    assert(decision.target_node_id == "H_bone");
    assert(decision.cvar_score > 1.0);

    std::cout << "  Selected Action: " << static_cast<int>(decision.action)
              << " (" << decision.rationale << ")" << std::endl;
    std::cout << "  CVaR Score: " << decision.cvar_score << std::endl;

    std::cout << "============================================================" << std::endl;
    std::cout << "  All 5-Year Archaeological Test Benchmarks Verified 100%!" << std::endl;
    std::cout << "============================================================" << std::endl;
}

int main() {
    test_full_5year_archaeological_timeline();
    return 0;
}
