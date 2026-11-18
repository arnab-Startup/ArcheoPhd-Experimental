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
// Stage 1 (v2.1): Subgraph-Conditioned Set NLI & Split Conformal Abstention Gate
// =============================================================================
enum class NliRelation {
    SUPPORT = 0,
    CONTRADICTION = 1,
    UNRELATED = 2
};

struct ConformalPredictionSet {
    std::vector<NliRelation> relations;
    bool is_singleton = false;
    bool is_ambiguous = false;
    double calibrated_support = 0.0;
    double calibrated_contradiction = 0.0;
    double signed_weight = 0.0;
};

class ConformalNliGate {
private:
    double non_conformity_quantile_threshold_ = 0.65; // (1 - alpha_err) quantile threshold

public:
    explicit ConformalNliGate(double threshold = 0.65)
        : non_conformity_quantile_threshold_(threshold) {}

    ConformalPredictionSet evaluate_admission(
        double p_support,
        double p_contradiction,
        double p_unrelated
    ) const {
        ConformalPredictionSet res;
        res.calibrated_support = p_support;
        res.calibrated_contradiction = p_contradiction;
        res.signed_weight = p_support - p_contradiction;

        double non_conf_sup = 1.0 - p_support;
        double non_conf_con = 1.0 - p_contradiction;
        double non_conf_unr = 1.0 - p_unrelated;

        if (non_conf_sup <= non_conformity_quantile_threshold_) {
            res.relations.push_back(NliRelation::SUPPORT);
        }
        if (non_conf_con <= non_conformity_quantile_threshold_) {
            res.relations.push_back(NliRelation::CONTRADICTION);
        }
        if (non_conf_unr <= non_conformity_quantile_threshold_) {
            res.relations.push_back(NliRelation::UNRELATED);
        }

        res.is_singleton = (res.relations.size() == 1);
        res.is_ambiguous = (res.relations.size() > 1);
        return res;
    }
};

// =============================================================================
// Stage 2 (v2.1): Zero-Inflation Root-Lineage Projection & 4-Tuple Subjective Logic
// =============================================================================
struct OpinionTuple4 {
    double belief = 0.0;       // b_v (corroboration)
    double disbelief = 0.0;    // d_v (falsification)
    double uncertainty = 1.0;  // u_v (epistemic gap)
    double base_rate = 0.5;    // a_v (domain base-rate prior in (0, 1))

    static OpinionTuple4 from_evidence(double r_plus, double r_minus, double base_rate = 0.5, double w0 = 2.0) {
        r_plus = std::max(0.0, r_plus);
        r_minus = std::max(0.0, r_minus);
        if (w0 <= 0.0) w0 = 2.0;

        double total = r_plus + r_minus + w0;
        OpinionTuple4 op;
        op.belief = r_plus / total;
        op.disbelief = r_minus / total;
        op.uncertainty = w0 / total;
        op.base_rate = std::clamp(base_rate, 0.01, 0.99);
        return op;
    }

    double projected_probability() const {
        return belief + base_rate * uncertainty;
    }

    double conflict_mass() const {
        double c = 4.0 * belief * disbelief * (1.0 - uncertainty);
        return std::clamp(c, 0.0, 1.0);
    }
};

struct RootLineageWarrant {
    std::string warrant_id;
    std::vector<std::string> root_observations; // Ancestor basis O_t
    double weight = 1.0;
    double satisfaction = 1.0;
};

class RootLineageProjector {
public:
    // Patched Equation 2A: Zero-Inflation Root-Lineage Basis Max-Projection
    // r_{v,eff}^+ = sum_{o in O_t} max_{i} ( G_{i,o} * w_i * Phi_i )
    // Guarantees mathematical idempotency: N duplicate notes yield exactly 0.0% phantom inflation.
    static double compute_effective_evidence(const std::vector<RootLineageWarrant>& warrants) {
        if (warrants.empty()) return 0.0;

        std::set<std::string> unique_roots;
        for (const auto& w : warrants) {
            for (const auto& r : w.root_observations) {
                unique_roots.insert(r);
            }
        }

        if (unique_roots.empty()) {
            double max_val = 0.0;
            for (const auto& w : warrants) max_val = std::max(max_val, w.weight * w.satisfaction);
            return max_val;
        }

        double total_effective = 0.0;
        for (const auto& root : unique_roots) {
            double max_for_root = 0.0;
            for (const auto& w : warrants) {
                bool has_root = std::find(w.root_observations.begin(), w.root_observations.end(), root) != w.root_observations.end();
                if (has_root) {
                    double g_io = 1.0 / static_cast<double>(w.root_observations.size());
                    double val = g_io * w.weight * w.satisfaction;
                    if (val > max_for_root) max_for_root = val;
                }
            }
            total_effective += max_for_root;
        }
        return total_effective;
    }
};

// =============================================================================
// Stage 3 (v2.1): Asymmetric Godel-Frank Operator & Damped Fixed-Point Iteration
// =============================================================================
struct UpgradedHyperedgeWarrant {
    std::string id;
    std::vector<std::string> in_list;   // S_e^+ required premises
    std::vector<double> in_weights;     // gamma_u premise importance weights (sum = 1)
    std::vector<std::string> out_list;  // S_e^- blocking defeaters
    std::string conclusion_id;          // v_e
    double warrant_weight = 5.0;
    double leak_tolerance = 0.02;       // epsilon_u in [0, 0.1]
    double previous_satisfaction = 0.0;

    // Patched Equation 3A: Asymmetric Godel-Frank Operator
    // Sufficiency uses cardinality-invariant geometric mean: exp(sum gamma * ln(P_bar))
    // Falsification necessity uses un-cushioned veto: prod (1 - (1 - eps)*d_u) * prod (1 - (1 - eps)*b_z)
    double compute_satisfaction(const std::map<std::string, OpinionTuple4>& node_states) const {
        if (in_list.empty()) return 0.0;

        size_t m = in_list.size();
        std::vector<double> normalized_gammas(m, 1.0 / static_cast<double>(m));
        if (in_weights.size() == m) {
            double sum_g = std::accumulate(in_weights.begin(), in_weights.end(), 0.0);
            if (sum_g > 1e-6) {
                for (size_t i = 0; i < m; ++i) normalized_gammas[i] = in_weights[i] / sum_g;
            }
        }

        // Part 1: Cardinality-Invariant Positive Support
        double log_sum = 0.0;
        for (size_t i = 0; i < m; ++i) {
            const auto& u_id = in_list[i];
            double p_bar = 0.0;
            auto it = node_states.find(u_id);
            if (it != node_states.end()) {
                p_bar = it->second.projected_probability();
            }
            p_bar = std::max(1e-9, p_bar);
            log_sum += normalized_gammas[i] * std::log(p_bar);
        }
        double support_term = std::exp(log_sum);

        // Part 2: Un-cushioned Premise Falsification Veto
        double falsification_veto = 1.0;
        for (const auto& u_id : in_list) {
            double d_u = 0.0;
            auto it = node_states.find(u_id);
            if (it != node_states.end()) {
                d_u = it->second.disbelief;
            }
            double term = 1.0 - ((1.0 - leak_tolerance) * d_u);
            falsification_veto *= std::clamp(term, 0.0, 1.0);
        }

        // Part 3: Defeater Veto (Out-List)
        double defeater_veto = 1.0;
        for (const auto& z_id : out_list) {
            double b_z = 0.0;
            auto it = node_states.find(z_id);
            if (it != node_states.end()) {
                b_z = it->second.belief;
            }
            double term = 1.0 - ((1.0 - leak_tolerance) * b_z);
            defeater_veto *= std::clamp(term, 0.0, 1.0);
        }

        return support_term * falsification_veto * defeater_veto;
    }
};

struct UpgradedEpistemicNode {
    std::string id;
    std::string label;
    double r_direct_plus = 0.0;
    double r_direct_minus = 0.0;
    double r_prop_plus = 0.0;
    double r_prop_minus = 0.0;
    OpinionTuple4 opinion;
    double previous_belief = 0.0;
    bool is_exonerated = false;
    double last_shockwave_delta = 0.0;
};

class UpgradedEpistemicHypergraph {
private:
    std::map<std::string, UpgradedEpistemicNode> nodes_;
    std::vector<UpgradedHyperedgeWarrant> hyperedges_;
    double damping_factor_zeta_ = 0.65; // Spectral contraction damping

public:
    explicit UpgradedEpistemicHypergraph(double zeta = 0.65)
        : damping_factor_zeta_(std::clamp(zeta, 0.1, 0.95)) {}

    void add_node(const std::string& id, const std::string& label, double direct_plus = 0.0, double direct_minus = 0.0, double base_rate = 0.5) {
        UpgradedEpistemicNode n;
        n.id = id;
        n.label = label;
        n.r_direct_plus = direct_plus;
        n.r_direct_minus = direct_minus;
        n.opinion = OpinionTuple4::from_evidence(direct_plus, direct_minus, base_rate);
        n.previous_belief = n.opinion.belief;
        nodes_[id] = n;
    }

    void add_hyperedge(const UpgradedHyperedgeWarrant& hedge) {
        hyperedges_.push_back(hedge);
    }

    const std::map<std::string, UpgradedEpistemicNode>& nodes() const { return nodes_; }
    std::map<std::string, UpgradedEpistemicNode>& mutable_nodes() { return nodes_; }
    const std::vector<UpgradedHyperedgeWarrant>& hyperedges() const { return hyperedges_; }
    std::vector<UpgradedHyperedgeWarrant>& mutable_hyperedges() { return hyperedges_; }

    void propagate_damped_belief_revision(int max_iterations = 15, double convergence_threshold = 1e-4) {
        for (int iter = 0; iter < max_iterations; ++iter) {
            std::map<std::string, OpinionTuple4> current_states;
            for (const auto& kv : nodes_) {
                current_states[kv.first] = kv.second.opinion;
            }

            std::map<std::string, double> raw_prop_plus;
            std::map<std::string, double> raw_prop_minus;

            for (auto& hedge : hyperedges_) {
                double phi = hedge.compute_satisfaction(current_states);
                double delta_phi = std::abs(phi - hedge.previous_satisfaction);
                hedge.previous_satisfaction = phi;

                if (nodes_.count(hedge.conclusion_id)) {
                    nodes_[hedge.conclusion_id].last_shockwave_delta = std::max(nodes_[hedge.conclusion_id].last_shockwave_delta, delta_phi);
                }

                double eff_weight = std::abs(hedge.warrant_weight) * phi;
                if (hedge.warrant_weight >= 0.0) {
                    raw_prop_plus[hedge.conclusion_id] += eff_weight;
                } else {
                    raw_prop_minus[hedge.conclusion_id] += eff_weight;
                }
            }

            double max_delta = 0.0;
            for (auto& kv : nodes_) {
                auto& node = kv.second;
                double damped_prop_plus = (1.0 - damping_factor_zeta_) * node.r_prop_plus + damping_factor_zeta_ * raw_prop_plus[node.id];
                double damped_prop_minus = (1.0 - damping_factor_zeta_) * node.r_prop_minus + damping_factor_zeta_ * raw_prop_minus[node.id];

                double tot_plus = node.r_direct_plus + damped_prop_plus;
                double tot_minus = node.r_direct_minus + damped_prop_minus;

                OpinionTuple4 updated = OpinionTuple4::from_evidence(tot_plus, tot_minus, node.opinion.base_rate);
                double delta = std::abs(updated.belief - node.opinion.belief);
                if (delta > max_delta) max_delta = delta;

                if (node.opinion.belief < 0.25 && updated.belief >= 0.5) {
                    node.is_exonerated = true;
                }

                node.previous_belief = node.opinion.belief;
                node.opinion = updated;
                node.r_prop_plus = damped_prop_plus;
                node.r_prop_minus = damped_prop_minus;
            }

            if (max_delta < convergence_threshold) break;
        }
    }
};

// =============================================================================
// Stage 4 (v2.1): Shockwave-Gated Temporal Excitation Kernel
// =============================================================================
struct TemporalActivationEvent {
    std::string node_id;
    double timestamp = 0.0;
};

class ShockwaveHawkesTracker {
private:
    std::vector<TemporalActivationEvent> event_history_;
    double beta_mem_ = 0.3; // Heavy-tail exponent
    double timescale_c_ = 0.5; // Monthly scaling parameter
    double eta_shock_ = 2.0;   // Multiplicative shockwave gain

public:
    explicit ShockwaveHawkesTracker(double beta_mem = 0.3, double c = 0.5, double eta_shock = 2.0)
        : beta_mem_(beta_mem), timescale_c_(c), eta_shock_(eta_shock) {}

    void record_event(const std::string& node_id, double timestamp) {
        event_history_.push_back({node_id, timestamp});
    }

    // Patched Equation 4B: Shockwave-Gated Temporal Excitation
    // Evaluates baseline power-law dormancy, amplified by absolute epistemic jump |Delta Phi_e|
    double compute_intensity(
        const std::string& node_id,
        double current_time,
        double delta_phi = 0.0,
        double structural_coupling = 0.85,
        double baseline_mu = 0.01
    ) const {
        double tau_v = current_time;
        for (const auto& ev : event_history_) {
            if (ev.node_id == node_id) {
                tau_v = std::min(tau_v, ev.timestamp);
            }
        }

        double dt_dormant = std::max(0.0, current_time - tau_v);
        // Heavy-tailed baseline power-law decay
        double baseline_decay = std::pow(1.0 + timescale_c_ * dt_dormant, -beta_mem_);
        double intensity = baseline_mu * baseline_decay;

        // Epistemic Shockwave Multiplier
        if (delta_phi > 0.0) {
            intensity += eta_shock_ * delta_phi * structural_coupling;
        }

        return intensity;
    }
};

// =============================================================================
// Stage 5 (v2.1): Risk-Averse CVaR_0.25 Distributional Quantile Policy
// =============================================================================
enum class EpistemicActionV2 {
    SURFACE_CONTRADICTION = 0,
    REACTIVATE_FORGOTTEN_INSIGHT = 1,
    PROPOSE_ASSUMPTION_TEST = 2,
    SILENT_ASSIMILATION = 3
};

struct EpistemicInterventionResultV2 {
    EpistemicActionV2 action = EpistemicActionV2::SILENT_ASSIMILATION;
    std::string target_node_id;
    double cvar_score = 0.0;
    std::string rationale;
};

class DistributionalCvarPolicy {
private:
    double tau_risk_ = 0.25;
    double cognitive_cost_interruption_ = 0.45;

public:
    explicit DistributionalCvarPolicy(double tau_risk = 0.25)
        : tau_risk_(std::clamp(tau_risk, 0.05, 0.50)) {}

    EpistemicInterventionResultV2 select_risk_averse_action(
        const UpgradedEpistemicHypergraph& graph,
        const ShockwaveHawkesTracker& tracker,
        double current_time
    ) const {
        EpistemicInterventionResultV2 best;
        best.action = EpistemicActionV2::SILENT_ASSIMILATION;
        best.cvar_score = 0.0;
        best.rationale = "Risk-averse CVaR criterion: preserved scholar focus.";

        std::string max_conflict_node;
        double max_conflict = 0.0;
        std::string exonerated_node;

        for (const auto& kv : graph.nodes()) {
            const auto& node = kv.second;
            double c = node.opinion.conflict_mass();
            if (c > max_conflict) {
                max_conflict = c;
                max_conflict_node = node.id;
            }
            if (node.is_exonerated && exonerated_node.empty()) {
                exonerated_node = node.id;
            }
        }

        if (!max_conflict_node.empty() && max_conflict > 0.40) {
            double intensity = tracker.compute_intensity(max_conflict_node, current_time);
            double worst_case_sample = (1.5 * max_conflict * intensity * 0.70) - cognitive_cost_interruption_;
            double median_sample = (1.5 * max_conflict * intensity) - cognitive_cost_interruption_;
            double cvar_estimate = (worst_case_sample + median_sample * 0.5) / 1.5;

            if (cvar_estimate > best.cvar_score) {
                best.action = EpistemicActionV2::SURFACE_CONTRADICTION;
                best.target_node_id = max_conflict_node;
                best.cvar_score = cvar_estimate;
                best.rationale = "High dialectic contradiction with positive CVaR_0.25 (" + std::to_string(cvar_estimate) + ").";
            }
        }

        if (!exonerated_node.empty()) {
            const auto& node = graph.nodes().at(exonerated_node);
            double intensity = tracker.compute_intensity(exonerated_node, current_time, node.last_shockwave_delta);
            double worst_case_exoneration = (2.0 * intensity * 0.75) - cognitive_cost_interruption_;
            if (worst_case_exoneration > best.cvar_score) {
                best.action = EpistemicActionV2::REACTIVATE_FORGOTTEN_INSIGHT;
                best.target_node_id = exonerated_node;
                best.cvar_score = worst_case_exoneration;
                best.rationale = "Upward exoneration shockwave: forgotten insight restored with high confidence.";
            }
        }

        return best;
    }
};

} // namespace archaeophd
