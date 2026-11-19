#pragma once

#include "epistemic_logic.hpp"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <chrono>

namespace archaeophd {

// =============================================================================
// Tier 2: System, Policy & Archaeological Robustness Extensions
// =============================================================================

// T33: Boundary Oscillation & Hysteresis Dampener
class OscillationHysteresisGuard {
private:
    std::vector<double> value_history_;
    int max_window_ = 10;
    int flip_threshold_ = 4;

public:
    void record_value(double val) {
        value_history_.push_back(val);
        if (value_history_.size() > static_cast<size_t>(max_window_)) {
            value_history_.erase(value_history_.begin());
        }
    }

    bool is_unstable_oscillation() const {
        if (value_history_.size() < 4) return false;
        int flips = 0;
        for (size_t i = 1; i + 1 < value_history_.size(); ++i) {
            double d1 = value_history_[i] - value_history_[i - 1];
            double d2 = value_history_[i + 1] - value_history_[i];
            if ((d1 > 0.1 && d2 < -0.1) || (d1 < -0.1 && d2 > 0.1)) {
                flips++;
            }
        }
        return (flips >= flip_threshold_);
    }
};

// T44: Stubborn User Dismissal Backoff Tracker
class UserDismissalTracker {
private:
    std::map<std::string, int> dismissal_counts_;
    std::map<std::string, double> evidence_at_last_dismissal_;

public:
    void record_dismissal(const std::string& hypothesis_id, double current_evidence) {
        dismissal_counts_[hypothesis_id]++;
        evidence_at_last_dismissal_[hypothesis_id] = current_evidence;
    }

    bool should_backoff(const std::string& hypothesis_id, double current_evidence) const {
        auto it_cnt = dismissal_counts_.find(hypothesis_id);
        if (it_cnt == dismissal_counts_.end() || it_cnt->second < 3) return false;

        // If user dismissed 3+ times and no new evidence has arrived, back off (suppress)
        auto it_ev = evidence_at_last_dismissal_.find(hypothesis_id);
        if (it_ev != evidence_at_last_dismissal_.end()) {
            double delta_ev = std::abs(current_evidence - it_ev->second);
            if (delta_ev < 1.0) return true; // Suppress
        }
        return false;
    }
};

// T47: Multi-Author Divergence Disambiguation
enum class DivergenceType {
    NONE = 0,
    SELF_CONTRADICTION = 1,
    COLLABORATOR_DIVERGENCE = 2
};

class AuthorProvenanceDisambiguator {
public:
    static DivergenceType classify_conflict(
        const std::string& author_1,
        const std::string& author_2,
        double conflict_mass
    ) {
        if (conflict_mass < 0.30) return DivergenceType::NONE;
        if (author_1 == author_2) return DivergenceType::SELF_CONTRADICTION;
        return DivergenceType::COLLABORATOR_DIVERGENCE;
    }
};

// T48: Top-K Action Space Pruning (< 10 ms for 5M pairs)
struct CandidateEpistemicAction {
    std::string node_id;
    double priority_score = 0.0;
};

class EpistemicActionSpacePruner {
public:
    static std::vector<CandidateEpistemicAction> prune_top_k(
        const std::vector<CandidateEpistemicAction>& candidates,
        size_t k = 50
    ) {
        if (candidates.size() <= k) return candidates;
        std::vector<CandidateEpistemicAction> top = candidates;
        std::partial_sort(top.begin(), top.begin() + k, top.end(),
            [](const CandidateEpistemicAction& a, const CandidateEpistemicAction& b) {
                return a.priority_score > b.priority_score;
            });
        top.resize(k);
        return top;
    }
};

// T41: Distributional CVaR_alpha Evaluation
class DistributionalCvarEvaluator {
public:
    // Calculates CVaR_alpha: expected value in the worst alpha quantile tail
    static double compute_cvar(const std::vector<double>& payoff_samples, double alpha = 0.25) {
        if (payoff_samples.empty()) return 0.0;
        std::vector<double> sorted = payoff_samples;
        std::sort(sorted.begin(), sorted.end());

        size_t tail_count = std::max<size_t>(1, static_cast<size_t>(sorted.size() * alpha));
        double sum = 0.0;
        for (size_t i = 0; i < tail_count; ++i) {
            sum += sorted[i];
        }
        return sum / static_cast<double>(tail_count);
    }
};

// T51: Harris Matrix Superposition Validator
class StratigraphicSuperpositionValidator {
public:
    static bool check_stratigraphic_inversion(
        double stratum_1_year, // Higher layer (L1, above)
        double stratum_2_year  // Lower layer (L2, below)
    ) {
        // Law of Superposition: An undisturbed layer above (L1) cannot be older than the layer below (L2).
        // In astronomical years, older means more negative (e.g. -1500 is older than -1000).
        // If stratum 1 is older than stratum 2 by > 100 years, violation!
        return (stratum_1_year < stratum_2_year - 100.0);
    }
};

// T53: BCE/CE Astronomical Calendar Calculator
class AstronomicalYearCalculator {
public:
    static double parse_historical_year(int year, bool is_bce) {
        // Astronomical convention:
        // 1 CE = +1
        // 1 BCE = 0 (Astronomical year 0)
        // 2 BCE = -1
        if (is_bce) {
            return -static_cast<double>(year - 1);
        }
        return static_cast<double>(year);
    }

    static double calculate_span_years(int start_year, bool start_bce, int end_year, bool end_bce) {
        double y_start = parse_historical_year(start_year, start_bce);
        double y_end = parse_historical_year(end_year, end_bce);
        return std::abs(y_end - y_start);
    }
};

// T54: Taphonomic Preservation Discounter
class TaphonomicPreservationModel {
public:
    static double apply_preservation_discount(
        double raw_disbelief,
        double taphonomic_preservation_factor // rho_taph in [0, 1]
    ) {
        // Absence of evidence is not evidence of absence in poor preservation contexts
        return raw_disbelief * std::clamp(taphonomic_preservation_factor, 0.0, 1.0);
    }
};

} // namespace archaeophd
