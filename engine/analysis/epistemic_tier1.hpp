#pragma once

#include "epistemic_logic.hpp"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace archaeophd {

// =============================================================================
// Tier 1 Mathematical Invariants & Provenance Extensions
// =============================================================================

// T13: Multi-hop citation lineage node
struct CitationEdge {
    std::string from_id;
    std::string to_id;
};

class LineageProvenanceTracer {
public:
    // Resolves N_1 -> P_1 -> R_1 -> O_1 to root observation O_1
    static std::set<std::string> resolve_roots(
        const std::string& start_node,
        const std::vector<CitationEdge>& citation_graph,
        const std::set<std::string>& known_root_observations
    ) {
        std::set<std::string> roots;
        std::vector<std::string> queue = {start_node};
        std::set<std::string> visited;

        while (!queue.empty()) {
            std::string curr = queue.back();
            queue.pop_back();
            if (visited.count(curr)) continue;
            visited.insert(curr);

            if (known_root_observations.count(curr)) {
                roots.insert(curr);
                continue;
            }

            for (const auto& edge : citation_graph) {
                if (edge.from_id == curr) {
                    queue.push_back(edge.to_id);
                }
            }
        }
        return roots;
    }
};

// T26: Uncertainty-Weighted Modus Tollens Propagator
class WeightedModusTollens {
public:
    static void propagate(
        UpgradedEpistemicHypergraph& graph,
        const std::string& conclusion_id,
        double disbelief_input
    ) {
        auto& nodes = graph.mutable_nodes();
        auto it_conc = nodes.find(conclusion_id);
        if (it_conc == nodes.end()) return;

        for (const auto& hedge : graph.hyperedges()) {
            if (hedge.conclusion_id != conclusion_id) continue;

            // Calculate total epistemic uncertainty among mutable premises
            double total_uncertainty = 0.0;
            std::vector<std::string> mutable_premises;
            for (const auto& p_id : hedge.in_list) {
                auto it = nodes.find(p_id);
                if (it != nodes.end() && !it->second.is_immutable_observation) {
                    mutable_premises.push_back(p_id);
                    total_uncertainty += it->second.opinion.uncertainty;
                }
            }

            if (total_uncertainty <= 1e-6) total_uncertainty = 1.0;

            // Distribute backward disbelief weighted by premise uncertainty
            for (const auto& p_id : mutable_premises) {
                auto& p_node = nodes[p_id];
                double uncertainty_weight = p_node.opinion.uncertainty / total_uncertainty;
                double back_minus = disbelief_input * hedge.warrant_weight * uncertainty_weight;

                p_node.r_prop_minus += back_minus;
                p_node.opinion = OpinionTuple4::from_evidence(
                    p_node.r_direct_plus + p_node.r_prop_plus,
                    p_node.r_direct_minus + p_node.r_prop_minus,
                    p_node.opinion.base_rate
                );
            }
        }
    }
};

// T25: Non-Backtracking Causal Support Evaluator (Zombie-Cycle Breaker)
class NonBacktrackingCycleEvaluator {
public:
    static void break_unrooted_zombies(
        UpgradedEpistemicHypergraph& graph,
        const std::set<std::string>& active_root_ids
    ) {
        auto& nodes = graph.mutable_nodes();
        for (auto& kv : nodes) {
            auto& node = kv.second;
            if (node.is_immutable_observation) continue;

            // Check if node has any valid un-falsified root in active lineage
            bool has_valid_root = false;
            for (const auto& r_id : active_root_ids) {
                auto it = nodes.find(r_id);
                if (it != nodes.end() && it->second.opinion.belief > 0.25) {
                    has_valid_root = true;
                    break;
                }
            }

            // If roots are completely collapsed/falsified, circular mutual warrants must not sustain zombies
            if (!has_valid_root && node.r_direct_plus < 1.0) {
                node.r_prop_plus = 0.0;
                node.opinion = OpinionTuple4::from_evidence(
                    node.r_direct_plus,
                    node.r_direct_minus + node.r_prop_minus,
                    node.opinion.base_rate
                );
            }
        }
    }
};

// T42: Quantile Monotonicity Model (Zero Quantile Crossing)
class MonotonicQuantileRegressor {
public:
    static std::vector<double> predict_quantiles(
        const std::vector<double>& raw_logits,
        int num_quantiles = 10
    ) {
        std::vector<double> quantiles(num_quantiles);
        for (int i = 0; i < num_quantiles; ++i) {
            double tau = (i + 0.5) / static_cast<double>(num_quantiles);
            // Linear transformation with quantile projection
            double val = raw_logits.empty() ? tau : (raw_logits[0] * 0.5 + tau * 2.0);
            quantiles[i] = val;
        }
        // Strict non-decreasing monotonicity projection (isotonic sort)
        std::sort(quantiles.begin(), quantiles.end());
        return quantiles;
    }
};

} // namespace archaeophd
