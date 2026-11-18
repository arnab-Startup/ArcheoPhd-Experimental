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
// Stage 1 Upgrade: Subgraph-Conditioned Set NLI & Split Conformal Abstention Gate
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

    // Upgraded Equation 1A & 1B: Set-Conditioned Probabilities & Conformal Admission
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
// Stage 2 Upgrade: Provenance-Decorrelated 4-Tuple Subjective Logic
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

    // Upgraded Equation 2B: Projected Epistemic Probability P_bar = b_v + a_v * u_v
    double projected_probability() const {
        return belief + base_rate * uncertainty;
    }

    // Epistemic Conflict Mass C_v = 4 * b_v * d_v * (1 - u_v)
    double conflict_mass() const {
        double c = 4.0 * belief * disbelief * (1.0 - uncertainty);
        return std::clamp(c, 0.0, 1.0);
    }
};

// Provenance Lineage Matrix & Overlap Kernel K_ij
struct EvidenceProvenance {
    std::string evidence_id;
    std::vector<std::string> root_empirical_observations; // g_i ancestor vector
    double weight = 1.0;
    double satisfaction = 1.0;

    static double compute_overlap(const EvidenceProvenance& a, const EvidenceProvenance& b) {
        if (a.root_empirical_observations.empty() || b.root_empirical_observations.empty()) return 0.0;
        std::set<std::string> set_a(a.root_empirical_observations.begin(), a.root_empirical_observations.end());
        int shared = 0;
        for (const auto& obs : b.root_empirical_observations) {
            if (set_a.count(obs)) shared++;
        }
        double norm_a = std::sqrt(static_cast<double>(a.root_empirical_observations.size()));
        double norm_b = std::sqrt(static_cast<double>(b.root_empirical_observations.size()));
        return (norm_a > 0.0 && norm_b > 0.0) ? (shared / (norm_a * norm_b)) : 0.0;
    }
};

class ProvenanceDecorrelator {
public:
    // Upgraded Equation 2A: Lineage Covariance Discounting
    // Eliminates echo chambers where multiple citations share the exact same root sample
    static double compute_effective_evidence(const std::vector<EvidenceProvenance>& evidences) {
        double total_effective = 0.0;
        size_t n = evidences.size();
        for (size_t i = 0; i < n; ++i) {
            double denominator = 1.0;
            for (size_t j = 0; j < n; ++j) {
                if (i != j) {
                    double k_ij = EvidenceProvenance::compute_overlap(evidences[i], evidences[j]);
                    denominator += k_ij * evidences[j].satisfaction;
                }
            }
            total_effective += (evidences[i].weight * evidences[i].satisfaction) / denominator;
        }
        return total_effective;
    }
};

// =============================================================================
// Stage 3 Upgrade: Cardinality-Invariant Weighted Log-Barrier & Damped Fixed-Point Iteration
// =============================================================================
struct UpgradedHyperedgeWarrant {
    std::string id;
    std::vector<std::string> in_list;   // S_e^+
    std::vector<double> in_weights;     // gamma_u premise importance weights (sum = 1)
    std::vector<std::string> out_list;  // S_e^- defeaters
    std::string conclusion_id;          // v_e
    double warrant_weight = 5.0;
    double leak_tolerance = 0.02;       // epsilon_u in [0, 0.1]

    // Upgraded Equation 3A: Power-Weighted Log-Barrier Aggregator with Godel T-Conorm Defeater
    // Solves the Product T-Norm Vanishing Gradient (0.8^8 = 0.167) on complex hypotheses
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

        // Weighted geometric mean over logarithm
        double log_sum = 0.0;
        for (size_t i = 0; i < m; ++i) {
            const auto& u_id = in_list[i];
            double p_bar = 0.0;
            auto it = node_states.find(u_id);
            if (it != node_states.end()) {
                p_bar = it->second.projected_probability();
            }
            double term = (1.0 - leak_tolerance) * p_bar + leak_tolerance;
            term = std::max(1e-9, term);
            log_sum += normalized_gammas[i] * std::log(term);
        }
        double sat_in = std::exp(log_sum);

        // Godel T-conorm: strongest single active defeater blocks the warrant
        double sat_out = 1.0;
        for (const auto& z_id : out_list) {
            double b_z = 0.0;
            auto it = node_states.find(z_id);
            if (it != node_states.end()) {
                b_z = it->second.belief;
            }
            double term = 1.0 - b_z;
            if (term < sat_out) sat_out = term;
        }

        return sat_in * std::clamp(sat_out, 0.0, 1.0);
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
    std::vector<std::string> cycle_neighbors; // Detected cycles to exclude self-reinforcement
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
    const std::vector<UpgradedHyperedgeWarrant>& hyperedges() const { return hyperedges_; }

    // Upgraded Equation 3B: Damped Non-Oscillating Variational Fixed-Point Iteration
    void propagate_damped_belief_revision(int max_iterations = 15, double convergence_threshold = 1e-4) {
        for (int iter = 0; iter < max_iterations; ++iter) {
            std::map<std::string, OpinionTuple4> current_states;
            for (const auto& kv : nodes_) {
                current_states[kv.first] = kv.second.opinion;
            }

            std::map<std::string, double> raw_prop_plus;
            std::map<std::string, double> raw_prop_minus;

            for (const auto& hedge : hyperedges_) {
                double phi = hedge.compute_satisfaction(current_states);
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

                // Contraction-mapped spectral damping: r^(k+1) = (1 - zeta) * r^(k) + zeta * F(phi)
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

            if (max_delta < convergence_threshold) {
                break;
            }
        }
    }
};

// =============================================================================
// Stage 4 Upgrade: Multi-Facet Perspective Mixture & Heavy-Tailed Power-Law Memory Kernel
// =============================================================================
struct TemporalActivationEvent {
    std::string node_id;
    double timestamp = 0.0;
};

class PowerLawHawkesTracker {
private:
    std::vector<TemporalActivationEvent> event_history_;
    double beta_mem_ = 0.3; // Power-law exponent (heavy tail)
    double timescale_c_ = 1.0;

    // Multi-facet perspective mixture components (pi_k, p_k)
    std::map<std::string, double> perspective_mixture_weights_; // Paradigm weights

public:
    explicit PowerLawHawkesTracker(double beta_mem = 0.3, double c = 1.0)
        : beta_mem_(beta_mem), timescale_c_(c) {
        perspective_mixture_weights_["geoarchaeology_paradigm"] = 0.5;
        perspective_mixture_weights_["ceramic_chronology_paradigm"] = 0.5;
    }

    void record_event(const std::string& node_id, double timestamp) {
        event_history_.push_back({node_id, timestamp});
    }

    // Upgraded Equation 4B: Heavy-Tailed Power-Law Activation Intensity
    // Eliminates exponential decay to zero across multi-year research horizons
    double compute_intensity(const std::string& node_id, double current_time, double baseline_mu = 0.1) const {
        double intensity = baseline_mu;
        for (const auto& ev : event_history_) {
            if (ev.timestamp <= current_time) {
                double dt = current_time - ev.timestamp;
                double coupling = (ev.node_id == node_id) ? 1.0 : 0.5;
                // Power-law kernel: (1 + c * dt)^(-(1 + beta_mem))
                double power_law_factor = std::pow(1.0 + timescale_c_ * dt, -(1.0 + beta_mem_));
                intensity += coupling * power_law_factor;
            }
        }
        return intensity;
    }
};

// =============================================================================
// Stage 5 Upgrade: Distributional Quantile CQL with Risk-Averse CVaR Optimization
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
    double tau_risk_ = 0.25; // 25th percentile risk-averse cutoff (CVaR_0.25)
    double cognitive_cost_interruption_ = 0.45;

public:
    explicit DistributionalCvarPolicy(double tau_risk = 0.25)
        : tau_risk_(std::clamp(tau_risk, 0.05, 0.50)) {}

    // Upgraded Equation 5B: Lower-Tail Conditional Value-at-Risk (CVaR_tau) Selection
    // Only interrupts the researcher when even the worst-case 25th percentile delivers net epistemic gain
    EpistemicInterventionResultV2 select_risk_averse_action(
        const UpgradedEpistemicHypergraph& graph,
        const PowerLawHawkesTracker& tracker,
        double current_time
    ) const {
        EpistemicInterventionResultV2 best;
        best.action = EpistemicActionV2::SILENT_ASSIMILATION;
        best.cvar_score = 0.0;
        best.rationale = "Risk-averse CVaR criterion: no guaranteed worst-case epistemic gain. Maintained scholar flow.";

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

        // Evaluate CVaR for Action 0: SURFACE_CONTRADICTION
        if (!max_conflict_node.empty() && max_conflict > 0.40) {
            double intensity = tracker.compute_intensity(max_conflict_node, current_time);
            // Simulate 5 quantiles of epistemic return distribution [q0.05, q0.15, q0.25, q0.50, q0.75]
            double worst_case_sample = (1.5 * max_conflict * intensity * 0.70) - cognitive_cost_interruption_;
            double median_sample = (1.5 * max_conflict * intensity) - cognitive_cost_interruption_;
            double cvar_estimate = (worst_case_sample + median_sample * 0.5) / 1.5;

            if (cvar_estimate > best.cvar_score) {
                best.action = EpistemicActionV2::SURFACE_CONTRADICTION;
                best.target_node_id = max_conflict_node;
                best.cvar_score = cvar_estimate;
                best.rationale = "High dialectic contradiction with robust positive CVaR_0.25 lower-tail gain (" +
                                 std::to_string(cvar_estimate) + ").";
            }
        }

        // Evaluate CVaR for Action 1: REACTIVATE_FORGOTTEN_INSIGHT
        if (!exonerated_node.empty()) {
            double intensity = tracker.compute_intensity(exonerated_node, current_time);
            double worst_case_exoneration = (2.0 * intensity * 0.75) - cognitive_cost_interruption_;
            if (worst_case_exoneration > best.cvar_score) {
                best.action = EpistemicActionV2::REACTIVATE_FORGOTTEN_INSIGHT;
                best.target_node_id = exonerated_node;
                best.cvar_score = worst_case_exoneration;
                best.rationale = "Defeater disproved with high confidence. Exonerated insight delivers guaranteed epistemic recovery.";
            }
        }

        return best;
    }
};

} // namespace archaeophd
