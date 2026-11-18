#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <json.hpp>

namespace archaeophd {

// =============================================================================
// Stage 1: Calibrated NLI Relation Scoring
// =============================================================================
enum class NliRelation {
    SUPPORT = 0,
    CONTRADICTION = 1,
    UNRELATED = 2
};

struct NliProbabilities {
    double p_support = 0.0;
    double p_contradiction = 0.0;
    double p_unrelated = 0.0;
    double signed_weight = 0.0; // w_e = P(+) - P(-) in [-1, 1]

    static NliProbabilities from_logits(double z_sup, double z_con, double z_unrel, double t_cal = 1.0) {
        if (t_cal <= 0.0) t_cal = 1.0;
        double max_z = std::max({z_sup / t_cal, z_con / t_cal, z_unrel / t_cal});
        double e_sup = std::exp((z_sup / t_cal) - max_z);
        double e_con = std::exp((z_con / t_cal) - max_z);
        double e_unr = std::exp((z_unrel / t_cal) - max_z);
        double denom = e_sup + e_con + e_unr;

        NliProbabilities res;
        res.p_support = e_sup / denom;
        res.p_contradiction = e_con / denom;
        res.p_unrelated = e_unr / denom;
        res.signed_weight = res.p_support - res.p_contradiction;
        return res;
    }
};

// =============================================================================
// Stage 2: Subjective Logic & Beta-Binomial Algebra
// =============================================================================
struct OpinionTuple {
    double belief = 0.0;       // b_v (corroboration)
    double disbelief = 0.0;    // d_v (falsification)
    double uncertainty = 1.0;  // u_v (epistemic gap)
    double base_rate = 0.5;    // a_v

    static OpinionTuple from_evidence(double r_plus, double r_minus, double w0 = 2.0) {
        r_plus = std::max(0.0, r_plus);
        r_minus = std::max(0.0, r_minus);
        if (w0 <= 0.0) w0 = 2.0;

        double total = r_plus + r_minus + w0;
        OpinionTuple op;
        op.belief = r_plus / total;
        op.disbelief = r_minus / total;
        op.uncertainty = w0 / total;
        return op;
    }

    // Epistemic Conflict Mass C_v = 4 * b_v * d_v * (1 - u_v) in [0, 1]
    double conflict_mass() const {
        double c = 4.0 * belief * disbelief * (1.0 - uncertainty);
        return std::clamp(c, 0.0, 1.0);
    }

    // Expected scalar probability P(v) = b_v + a_v * u_v
    double expected_probability() const {
        return belief + base_rate * uncertainty;
    }
};

// =============================================================================
// Stage 3: Directed Hypergraph Neural Network & Differentiable TMS
// =============================================================================
struct HyperedgeWarrant {
    std::string id;
    std::vector<std::string> in_list;   // S_e^+ required supporting premises
    std::vector<std::string> out_list;  // S_e^- blocking defeaters / assumptions
    std::string conclusion_id;          // v_e target conclusion node
    double warrant_weight = 1.0;        // w_e in [-1, 1]
    double leak_tolerance = 0.02;       // epsilon_u in [0, 0.1] (taphonomic noise)

    // Premise satisfaction: Phi_e = prod(S_e^+) * prod(1 - S_e^-)
    double compute_satisfaction(const std::map<std::string, OpinionTuple>& node_states) const {
        double sat_in = 1.0;
        for (const auto& u_id : in_list) {
            double b_u = 0.0;
            auto it = node_states.find(u_id);
            if (it != node_states.end()) {
                b_u = it->second.belief;
            }
            double term = (1.0 - leak_tolerance) * b_u + leak_tolerance;
            sat_in *= term;
        }

        double sat_out = 1.0;
        for (const auto& z_id : out_list) {
            double b_z = 0.0;
            auto it = node_states.find(z_id);
            if (it != node_states.end()) {
                b_z = it->second.belief;
            }
            double term = 1.0 - ((1.0 - leak_tolerance) * b_z);
            sat_out *= std::clamp(term, 0.0, 1.0);
        }

        return sat_in * sat_out;
    }
};

struct EpistemicNode {
    std::string id;
    std::string label;
    double r_direct_plus = 0.0;
    double r_direct_minus = 0.0;
    double r_prop_plus = 0.0;
    double r_prop_minus = 0.0;
    OpinionTuple opinion;
    double previous_belief = 0.0;
    bool is_exonerated = false;
};

class EpistemicHypergraph {
private:
    std::map<std::string, EpistemicNode> nodes_;
    std::vector<HyperedgeWarrant> hyperedges_;

public:
    void add_node(const std::string& id, const std::string& label, double direct_plus = 0.0, double direct_minus = 0.0) {
        EpistemicNode n;
        n.id = id;
        n.label = label;
        n.r_direct_plus = direct_plus;
        n.r_direct_minus = direct_minus;
        n.opinion = OpinionTuple::from_evidence(direct_plus, direct_minus);
        n.previous_belief = n.opinion.belief;
        nodes_[id] = n;
    }

    void add_hyperedge(const HyperedgeWarrant& hedge) {
        hyperedges_.push_back(hedge);
    }

    const std::map<std::string, EpistemicNode>& nodes() const { return nodes_; }
    const std::vector<HyperedgeWarrant>& hyperedges() const { return hyperedges_; }

    // Multi-Hop Belief Revision with Downward Invalidation & Upward Exoneration
    void propagate_belief_revision(int max_iterations = 10, double convergence_threshold = 1e-4) {
        for (int iter = 0; iter < max_iterations; ++iter) {
            std::map<std::string, OpinionTuple> current_states;
            for (const auto& kv : nodes_) {
                current_states[kv.first] = kv.second.opinion;
            }

            std::map<std::string, double> new_prop_plus;
            std::map<std::string, double> new_prop_minus;

            for (const auto& hedge : hyperedges_) {
                double phi = hedge.compute_satisfaction(current_states);
                double eff_weight = std::abs(hedge.warrant_weight) * phi;
                if (hedge.warrant_weight >= 0.0) {
                    new_prop_plus[hedge.conclusion_id] += eff_weight;
                } else {
                    new_prop_minus[hedge.conclusion_id] += eff_weight;
                }
            }

            double max_delta = 0.0;
            for (auto& kv : nodes_) {
                auto& node = kv.second;
                double tot_plus = node.r_direct_plus + new_prop_plus[node.id];
                double tot_minus = node.r_direct_minus + new_prop_minus[node.id];

                OpinionTuple updated = OpinionTuple::from_evidence(tot_plus, tot_minus);
                double delta = std::abs(updated.belief - node.opinion.belief);
                if (delta > max_delta) max_delta = delta;

                // Check for Upward Exoneration: formerly suppressed hypothesis regaining belief
                if (node.opinion.belief < 0.2 && updated.belief >= 0.5) {
                    node.is_exonerated = true;
                }

                node.previous_belief = node.opinion.belief;
                node.opinion = updated;
                node.r_prop_plus = new_prop_plus[node.id];
                node.r_prop_minus = new_prop_minus[node.id];
            }

            if (max_delta < convergence_threshold) {
                break;
            }
        }
    }
};

// =============================================================================
// Stage 4: Temporal Graph Attention & Neural Hawkes Process
// =============================================================================
struct HistoricalEvent {
    std::string node_id;
    double timestamp = 0.0; // Normalized time (e.g., years or elapsed days)
};

class HawkesPerspectiveTracker {
private:
    std::vector<HistoricalEvent> event_history_;
    double decay_rate_delta_ = 0.35; // Exponential memory decay

public:
    explicit HawkesPerspectiveTracker(double delta = 0.35) : decay_rate_delta_(delta) {}

    void record_event(const std::string& node_id, double timestamp) {
        event_history_.push_back({node_id, timestamp});
    }

    // Structural Excitation Kernel Gamma_theta(v, v_j):
    // Higher coupling when nodes share hyperedges or thematic ontology
    double compute_coupling(const std::string& v, const std::string& v_j) const {
        if (v == v_j) return 1.0;
        // Simple structural overlap heuristic for standalone engine
        return 0.5;
    }

    // Relevance Intensity lambda_v(t) = mu_v(p_t) + sum Gamma * exp(-delta * dt)
    double compute_intensity(const std::string& node_id, double current_time, double baseline_mu = 0.1) const {
        double intensity = baseline_mu;
        for (const auto& ev : event_history_) {
            if (ev.timestamp <= current_time) {
                double dt = current_time - ev.timestamp;
                double gamma = compute_coupling(node_id, ev.node_id);
                intensity += gamma * std::exp(-decay_rate_delta_ * dt);
            }
        }
        return intensity;
    }
};

// =============================================================================
// Stage 5: Conservative Q-Learning (CQL) / Double DQN Decision Policy
// =============================================================================
enum class EpistemicAction {
    SURFACE_CONTRADICTION = 0,
    REACTIVATE_FORGOTTEN_INSIGHT = 1,
    PROPOSE_ASSUMPTION_TEST = 2,
    SILENT_ASSIMILATION = 3
};

struct EpistemicInterventionResult {
    EpistemicAction action = EpistemicAction::SILENT_ASSIMILATION;
    std::string target_node_id;
    double expected_reward = 0.0;
    std::string rationale;
};

class ConservativeQPolicy {
private:
    double beta_conflict_ = 1.5;
    double beta_exoneration_ = 2.0;
    double cognitive_cost_interruption_ = 0.4; // Cost penalty for user interruption
    double cognitive_cost_silent_ = 0.0;

public:
    EpistemicInterventionResult select_action(
        const EpistemicHypergraph& graph,
        const HawkesPerspectiveTracker& tracker,
        double current_time
    ) const {
        EpistemicInterventionResult best;
        best.action = EpistemicAction::SILENT_ASSIMILATION;
        best.expected_reward = 0.0;
        best.rationale = "No critical epistemic conflict or breakthrough detected. Preserved scholar focus.";

        // Find highest conflict node (Contradiction Intervention)
        std::string max_conflict_node;
        double max_conflict = 0.0;

        // Find exonerated node (Forgotten Insight Intervention)
        std::string exonerated_node;

        for (const auto& kv : graph.nodes()) {
            const auto& node = kv.second;
            double c = node.opinion.conflict_mass();
            double lambda = tracker.compute_intensity(node.id, current_time);

            if (c > max_conflict) {
                max_conflict = c;
                max_conflict_node = node.id;
            }
            if (node.is_exonerated && exonerated_node.empty()) {
                exonerated_node = node.id;
            }
        }

        // Evaluate Q-Value for Action 0: SURFACE_CONTRADICTION
        if (!max_conflict_node.empty() && max_conflict > 0.45) {
            double intensity = tracker.compute_intensity(max_conflict_node, current_time);
            double reward = beta_conflict_ * max_conflict * intensity - cognitive_cost_interruption_;
            if (reward > best.expected_reward) {
                best.action = EpistemicAction::SURFACE_CONTRADICTION;
                best.target_node_id = max_conflict_node;
                best.expected_reward = reward;
                best.rationale = "High internal conflict mass (" + std::to_string(max_conflict) + ") on active theme. Surfacing dialectic debate.";
            }
        }

        // Evaluate Q-Value for Action 1: REACTIVATE_FORGOTTEN_INSIGHT
        if (!exonerated_node.empty()) {
            double intensity = tracker.compute_intensity(exonerated_node, current_time);
            double reward = beta_exoneration_ * intensity - cognitive_cost_interruption_;
            if (reward > best.expected_reward) {
                best.action = EpistemicAction::REACTIVATE_FORGOTTEN_INSIGHT;
                best.target_node_id = exonerated_node;
                best.expected_reward = reward;
                best.rationale = "Defeater disproved: previously abandoned hypothesis now exonerated by upstream evidence.";
            }
        }

        return best;
    }
};

} // namespace archaeophd
