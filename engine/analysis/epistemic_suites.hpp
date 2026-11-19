#pragma once

#include "epistemic_logic.hpp"
#include "epistemic_tier1.hpp"
#include "epistemic_tier2.hpp"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace archaeophd {

// =============================================================================
// Suite 1 Support: Conformal Hygiene, Calibration Guard & Ingestion
// =============================================================================
class AdvancedConformalGate {
private:
    double fallback_threshold_ = 0.85;

public:
    static bool validate_probabilities(double p_sup, double p_con, double p_unr) {
        if (std::isnan(p_sup) || std::isnan(p_con) || std::isnan(p_unr)) return false;
        if (std::isinf(p_sup) || std::isinf(p_con) || std::isinf(p_unr)) return false;
        if (p_sup < 0.0 || p_con < 0.0 || p_unr < 0.0) return false;
        if (p_sup > 1.0 || p_con > 1.0 || p_unr > 1.0) return false;
        return (std::abs(p_sup + p_con + p_unr - 1.0) < 0.05);
    }

    static ConformalPredictionSet evaluate_with_tiebreak(
        double p_sup, double p_con, double p_unr, double threshold = 0.65
    ) {
        ConformalPredictionSet res;
        res.calibrated_support = p_sup;
        res.calibrated_contradiction = p_con;
        res.signed_weight = p_sup - p_con;

        if ((1.0 - p_sup) <= threshold) res.relations.push_back(NliRelation::SUPPORT);
        if ((1.0 - p_con) <= threshold) res.relations.push_back(NliRelation::CONTRADICTION);
        if ((1.0 - p_unr) <= threshold) res.relations.push_back(NliRelation::UNRELATED);

        res.is_singleton = (res.relations.size() == 1);
        res.is_ambiguous = (res.relations.size() > 1);
        return res;
    }

    double get_effective_threshold(size_t calibration_set_size, double empirical_q) const {
        // Cold-start guard: If N < 20, fall back to conservative threshold
        if (calibration_set_size < 20) return fallback_threshold_;
        return empirical_q;
    }
};

// T08: Instrument Miscalibration Recall Processor
class InstrumentRecallProcessor {
public:
    static void recall_batch(
        UpgradedEpistemicHypergraph& graph,
        const std::set<std::string>& batch_root_ids
    ) {
        auto& nodes = graph.mutable_nodes();
        for (const auto& id : batch_root_ids) {
            auto it = nodes.find(id);
            if (it != nodes.end()) {
                it->second.r_direct_plus = 0.0;
                it->second.r_direct_minus = 100.0; // Mark retracted
                it->second.opinion = OpinionTuple4::from_evidence(0.0, 100.0, 0.5);
            }
        }
        graph.propagate_damped_belief_revision(5);
    }
};

// T09: Entity Homonymy Disambiguator
class EntityHomonymyDisambiguator {
public:
    static std::string build_unique_id(const std::string& name, const std::string& epoch) {
        return name + "_" + epoch;
    }
};

// =============================================================================
// Suite 2 Support: Provenance, External Literature & Simplex Transfer
// =============================================================================
class SimplexPlausibilityDistributor {
public:
    // T19: Redistributes plausibility mass among mutually exclusive hypotheses
    static void transfer_disbelief(
        const std::string& falsified_id,
        const std::vector<std::string>& group_ids,
        UpgradedEpistemicHypergraph& graph
    ) {
        auto& nodes = graph.mutable_nodes();
        auto it = nodes.find(falsified_id);
        if (it == nodes.end()) return;

        double d_mass = it->second.opinion.disbelief;
        std::vector<std::string> remaining;
        for (const auto& gid : group_ids) {
            if (gid != falsified_id && nodes.count(gid)) remaining.push_back(gid);
        }
        if (remaining.empty()) return;

        double share = (d_mass * 2.0) / static_cast<double>(remaining.size());
        for (const auto& rem_id : remaining) {
            nodes[rem_id].r_prop_plus += share;
            nodes[rem_id].opinion = OpinionTuple4::from_evidence(
                nodes[rem_id].r_direct_plus + nodes[rem_id].r_prop_plus,
                nodes[rem_id].r_direct_minus + nodes[rem_id].r_prop_minus,
                nodes[rem_id].opinion.base_rate
            );
        }
    }
};

// =============================================================================
// Suite 3 Support: Deep Chains, Domino Waves & Garbage Collection
// =============================================================================
class DeepChainPropagator {
public:
    static void remove_dangling_defeater(
        UpgradedEpistemicHypergraph& graph,
        const std::string& defeater_id
    ) {
        for (auto& hedge : graph.mutable_hyperedges()) {
            auto it = std::find(hedge.out_list.begin(), hedge.out_list.end(), defeater_id);
            if (it != hedge.out_list.end()) {
                hedge.out_list.erase(it);
            }
        }
        graph.propagate_damped_belief_revision(3);
    }
};

// =============================================================================
// Suite 4 Support: Perspective Clusters & Embedding Fallback
// =============================================================================
class PerspectiveClusterManager {
public:
    static double cosine_similarity(const std::vector<double>& a, const std::vector<double>& b) {
        if (a.empty() || a.size() != b.size()) return 0.0;
        double dot = 0.0, norm_a = 0.0, norm_b = 0.0;
        for (size_t i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            norm_a += a[i] * a[i];
            norm_b += b[i] * b[i];
        }
        if (norm_a <= 1e-9 || norm_b <= 1e-9) return 0.0;
        return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
    }
};

// =============================================================================
// Suite 5 Support: Privacy Redaction
// =============================================================================
class EpistemicPrivacyRedactor {
public:
    static void redact_site_coordinates(
        UpgradedEpistemicHypergraph& graph,
        const std::set<std::string>& restricted_nodes
    ) {
        auto& nodes = graph.mutable_nodes();
        for (const auto& id : restricted_nodes) {
            auto it = nodes.find(id);
            if (it != nodes.end()) {
                it->second.label = "[REDACTED_LOCATION_PROTECTED]";
            }
        }
    }
};

} // namespace archaeophd
