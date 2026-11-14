#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <iostream>
#include "models.hpp"

namespace archaeophd {

// =============================================================================
// Stratigraphic DAG & Harris Matrix Engine
// =============================================================================
// Implements archaeological stratigraphic logic based on Edward C. Harris (1979):
// - "Principles of Archaeological Stratigraphy"
// 
// Mathematical & Chronological Foundations:
// 1. Stratigraphic Sequence as Directed Acyclic Graph (DAG)
//    - Vertices: Stratigraphic Units / Strata
//    - Directed Edges: Temporal order (Older -> Younger)
//      * u is below v  => u is older than v (u -> v)
//      * v is above u  => u is older than v (u -> v)
//      * v cuts u      => u is older than v (u -> v)
// 2. Cycle Detection:
//    - Physical impossibility loops (self-loops u -> u, 2-cycles A <-> B, n-cycles)
//      detected via Tarjan's Strongly Connected Components algorithm.
// 3. Topological Sort (Kahn's Algorithm):
//    - Computes valid chronological sequence from oldest bedrock to youngest surface.
//    - Double-guarded: cycle detection clears sequence; Kahn size mismatch aborts.
// 4. Stratigraphic Level Assignment:
//    - Computes vertical phase level depth for Harris Matrix diagram layout.
// 5. Astronomical Year Normalization & 1 BCE / 1 CE Boundary:
//    - Civil calendar has NO Year 0 (1 BCE is followed immediately by 1 CE).
//    - Astronomical year mapping:
//        1 CE  -> +1, Y CE -> +Y
//        1 BCE ->  0, Y BCE -> 1 - Y
//    - Normalizes temporal comparisons across BCE/CE boundaries without off-by-one errors.
// 6. Law of Superposition & Radiometric C-14 Advisory Inversion Validator:
//    - Physical strata: strict non-overlapping inversion flags impossible superposition.
//    - Radiocarbon samples: overlapping 2-sigma ranges are valid under Bayesian priors.
//      Strict non-overlap flags an advisory review citing both lab codes and ranges.
// =============================================================================

struct StratigraphicInversion {
    std::string upper_stratum_id;
    std::string upper_stratum_name;
    int upper_date_astro = 0;          // Astronomical year
    std::string upper_date_display;    // Human-readable (e.g. "1800 BCE" or "50 CE")
    std::string lower_stratum_id;
    std::string lower_stratum_name;
    int lower_date_astro = 0;          // Astronomical year
    std::string lower_date_display;    // Human-readable (e.g. "1400 BCE")
    std::string reason;
    bool is_advisory = false;          // True for C-14 2-sigma non-overlap (needs review, not blocking)
    std::string sample_upper_code;     // Lab code of upper sample (if radiometric)
    std::string sample_lower_code;     // Lab code of lower sample (if radiometric)
};

struct HarrisMatrixResult {
    bool is_valid_dag = true;
    std::vector<std::string> chronological_sequence; // From oldest (earliest) to newest (latest)
    std::vector<std::vector<std::string>> cycles;     // Impossible paradox cycles (if any)
    std::vector<StratigraphicInversion> inversions;   // Law of Superposition date inversions
    std::map<std::string, int> stratigraphic_levels;  // Vertical phase depth (0 = earliest basal deposit)
    std::map<std::string, std::vector<std::string>> direct_older_than; // Edges: u -> {younger units}
    std::map<std::string, std::vector<std::string>> direct_younger_than;// Edges: v -> {older units}
};

class HarrisMatrixEngine {
public:
    // Astronomical Year Normalization:
    // In historical chronology, there is NO Year 0.
    // 1 BCE was followed directly by 1 CE.
    // In Astronomical year numbering:
    // 1 CE  ->  +1
    // Y CE  ->  +Y  (for Y >= 1)
    // 1 BCE ->   0
    // 2 BCE ->  -1
    // Y BCE ->  1 - Y (for Y >= 1)
    static std::pair<bool, int> to_astro_year(int date_bce, int date_ce) {
        if (date_ce > 0) {
            return {true, date_ce};
        } else if (date_bce > 0) {
            return {true, 1 - date_bce};
        }
        return {false, 0};
    }

    static std::string format_astro_year(int y_astro) {
        if (y_astro > 0) {
            return std::to_string(y_astro) + " CE";
        } else {
            int bce = 1 - y_astro;
            return std::to_string(bce) + " BCE";
        }
    }

    struct DateRangeAstro {
        bool has_date = false;
        int t_earliest = 0; // Smaller number = older/earlier in astronomical chronology
        int t_latest = 0;   // Larger number = younger/later
        std::string display;
    };

    static DateRangeAstro get_stratum_astro_range(const Stratum& s) {
        auto d_start = to_astro_year(s.date_start_bce, s.date_start_ce);
        auto d_end = to_astro_year(s.date_end_bce, s.date_end_ce);
        if (!d_start.first && !d_end.first) {
            return {false, 0, 0, ""};
        }
        int y1 = d_start.first ? d_start.second : d_end.second;
        int y2 = d_end.first ? d_end.second : d_start.second;
        int t_min = std::min(y1, y2);
        int t_max = std::max(y1, y2);
        std::string disp = format_astro_year(t_min) + " - " + format_astro_year(t_max);
        return {true, t_min, t_max, disp};
    }

    static DateRangeAstro get_sample_astro_range(const Sample& smp) {
        auto d_start = to_astro_year(smp.date_cal_start_bce, smp.date_cal_start_ce);
        auto d_end = to_astro_year(smp.date_cal_end_bce, smp.date_cal_end_ce);
        if (!d_start.first && !d_end.first) {
            return {false, 0, 0, ""};
        }
        int y1 = d_start.first ? d_start.second : d_end.second;
        int y2 = d_end.first ? d_end.second : d_start.second;
        int t_min = std::min(y1, y2);
        int t_max = std::max(y1, y2);
        std::string disp = smp.cal_range_2sigma.empty() ? 
            (format_astro_year(t_min) + " - " + format_astro_year(t_max) + " cal") : 
            smp.cal_range_2sigma;
        return {true, t_min, t_max, disp};
    }

private:
    // Tarjan's Strongly Connected Components algorithm for DAG cycle detection
    struct TarjanState {
        int index = 0;
        std::map<std::string, int> indices;
        std::map<std::string, int> lowlinks;
        std::vector<std::string> stack;
        std::set<std::string> on_stack;
        std::vector<std::vector<std::string>> sccs;
    };

    static void tarjan_connect(
        const std::string& u,
        const std::map<std::string, std::set<std::string>>& adj,
        TarjanState& state
    ) {
        state.indices[u] = state.index;
        state.lowlinks[u] = state.index;
        state.index++;
        state.stack.push_back(u);
        state.on_stack.insert(u);

        auto it = adj.find(u);
        if (it != adj.end()) {
            for (const auto& v : it->second) {
                if (state.indices.find(v) == state.indices.end()) {
                    tarjan_connect(v, adj, state);
                    state.lowlinks[u] = std::min(state.lowlinks[u], state.lowlinks[v]);
                } else if (state.on_stack.count(v)) {
                    state.lowlinks[u] = std::min(state.lowlinks[u], state.indices[v]);
                }
            }
        }

        if (state.lowlinks[u] == state.indices[u]) {
            std::vector<std::string> component;
            while (true) {
                std::string w = state.stack.back();
                state.stack.pop_back();
                state.on_stack.erase(w);
                component.push_back(w);
                if (w == u) break;
            }

            // CRITICAL FIX: A component represents a cycle if it contains more than 1 vertex,
            // OR if it contains exactly 1 vertex with a self-loop (u -> u).
            bool has_self_loop = (component.size() == 1 && it != adj.end() && it->second.count(u) > 0);
            if (component.size() > 1 || has_self_loop) {
                state.sccs.push_back(std::move(component));
            }
        }
    }

public:
    static HarrisMatrixResult build_matrix(
        const std::vector<Stratum>& strata,
        const std::vector<Sample>& samples = {}
    ) {
        HarrisMatrixResult result;
        if (strata.empty()) return result;

        std::map<std::string, Stratum> stratum_map;
        for (const auto& s : strata) stratum_map[s.id] = s;

        // Build directed adjacency graph: Older -> Younger
        std::map<std::string, std::set<std::string>> older_to_younger;
        std::map<std::string, std::set<std::string>> younger_to_older;
        std::set<std::string> all_nodes;

        for (const auto& s : strata) {
            all_nodes.insert(s.id);

            // If unit B is listed in s.harris_above, then s is below B, so s is older than B: s -> B
            for (const auto& above_id : s.harris_above) {
                if (stratum_map.find(above_id) != stratum_map.end()) {
                    older_to_younger[s.id].insert(above_id);
                    younger_to_older[above_id].insert(s.id);
                }
            }

            // If unit C is listed in s.harris_below, then C is below s, so C is older than s: C -> s
            for (const auto& below_id : s.harris_below) {
                if (stratum_map.find(below_id) != stratum_map.end()) {
                    older_to_younger[below_id].insert(s.id);
                    younger_to_older[s.id].insert(below_id);
                }
            }

            // If unit D is listed in s.harris_cut_by, then D cuts s, so s was deposited before D: s -> D
            for (const auto& cut_id : s.harris_cut_by) {
                if (stratum_map.find(cut_id) != stratum_map.end()) {
                    older_to_younger[s.id].insert(cut_id);
                    younger_to_older[cut_id].insert(s.id);
                }
            }
        }

        // 1. Detect Stratigraphic Cycles (Physical Impossibility Loops: self-loops, 2-cycles, n-cycles)
        TarjanState t_state;
        for (const auto& node : all_nodes) {
            if (t_state.indices.find(node) == t_state.indices.end()) {
                tarjan_connect(node, older_to_younger, t_state);
            }
        }

        if (!t_state.sccs.empty()) {
            result.is_valid_dag = false;
            result.cycles = std::move(t_state.sccs);
            result.chronological_sequence.clear();
            // Cycle detected: impossible stratigraphic loop, abort topological sort
            return result;
        }

        result.is_valid_dag = true;

        // 2. Kahn's Algorithm for Topological Sorting (Oldest -> Youngest)
        std::map<std::string, int> in_degree;
        for (const auto& node : all_nodes) in_degree[node] = 0;

        for (const auto& [u, targets] : older_to_younger) {
            for (const auto& v : targets) {
                in_degree[v]++;
            }
        }

        std::queue<std::string> q;
        for (const auto& node : all_nodes) {
            if (in_degree[node] == 0) {
                q.push(node);
                result.stratigraphic_levels[node] = 0; // Basal layer (level 0)
            }
        }

        while (!q.empty()) {
            std::string curr = q.front();
            q.pop();
            result.chronological_sequence.push_back(curr);

            int current_level = result.stratigraphic_levels[curr];

            auto it = older_to_younger.find(curr);
            if (it != older_to_younger.end()) {
                for (const auto& nxt : it->second) {
                    // Stratigraphic phase level is max depth from basal deposits
                    result.stratigraphic_levels[nxt] = std::max(
                        result.stratigraphic_levels[nxt],
                        current_level + 1
                    );

                    in_degree[nxt]--;
                    if (in_degree[nxt] == 0) {
                        q.push(nxt);
                    }
                }
            }
        }

        // Safeguard: verify that Kahn's algorithm sequenced every single vertex
        if (result.chronological_sequence.size() != all_nodes.size()) {
            result.is_valid_dag = false;
            result.chronological_sequence.clear();
            return result;
        }

        // Store direct adjacency for diagram rendering
        for (const auto& [u, targets] : older_to_younger) {
            result.direct_older_than[u] = std::vector<std::string>(targets.begin(), targets.end());
        }
        for (const auto& [v, sources] : younger_to_older) {
            result.direct_younger_than[v] = std::vector<std::string>(sources.begin(), sources.end());
        }

        // 3. Law of Superposition & Chronological Date Inversion Validation
        // For edge u -> v (u is older/below v; v is younger/above u):
        // In astronomical years: smaller = earlier/older, larger = later/younger.
        // If lower unit u was deposited strictly AFTER upper unit v:
        // That is: T_earliest(u) > T_latest(v)
        // Then upper unit v is dated older than the underlying stratum u -> Inversion!
        for (const auto& [u_id, targets] : older_to_younger) {
            const auto& u = stratum_map[u_id];
            auto r_lower = get_stratum_astro_range(u);
            if (!r_lower.has_date) continue;

            for (const auto& v_id : targets) {
                const auto& v = stratum_map[v_id];
                auto r_upper = get_stratum_astro_range(v);
                if (!r_upper.has_date) continue;

                // Strict non-overlapping chronological contradiction:
                // Lower stratum earliest date is strictly later (larger) than upper stratum latest date.
                if (r_lower.t_earliest > r_upper.t_latest) {
                    StratigraphicInversion inv;
                    inv.upper_stratum_id = v.id;
                    inv.upper_stratum_name = v.stratum_name;
                    inv.upper_date_astro = r_upper.t_latest;
                    inv.upper_date_display = r_upper.display;
                    inv.lower_stratum_id = u.id;
                    inv.lower_stratum_name = u.stratum_name;
                    inv.lower_date_astro = r_lower.t_earliest;
                    inv.lower_date_display = r_lower.display;
                    inv.is_advisory = true; // Stratum dates are scholarly interpretations, flagged for researcher review
                    inv.reason = "Stratigraphic date discrepancy — please review: Upper stratum '" + v.stratum_name + 
                                 "' (" + r_upper.display + ") is assigned an older date range than " +
                                 "underlying stratum '" + u.stratum_name + "' (" + 
                                 r_lower.display + ").";
                    result.inversions.push_back(inv);
                }
            }
        }

        // 4. Sample C-14 Radiometric Inversion Cross-Check
        // Evaluates 2-sigma calibrated radiocarbon intervals:
        // - Overlapping intervals: VALID under stratigraphic Bayesian priors (NOT an inversion).
        // - Strict non-overlap where lower sample is younger than upper sample:
        //   Flags an ADVISORY REVIEW entry ("Possible redeposition or context error — please review")
        //   citing both lab codes and ranges, without blocking DAG validity.
        if (!samples.empty()) {
            std::map<std::string, std::vector<Sample>> stratum_samples;
            for (const auto& smp : samples) {
                if (!smp.stratum_id.empty()) {
                    stratum_samples[smp.stratum_id].push_back(smp);
                }
            }

            for (const auto& [u_id, targets] : older_to_younger) {
                const auto& u = stratum_map[u_id];
                auto u_samples_it = stratum_samples.find(u_id);
                if (u_samples_it == stratum_samples.end()) continue;

                for (const auto& v_id : targets) {
                    const auto& v = stratum_map[v_id];
                    auto v_samples_it = stratum_samples.find(v_id);
                    if (v_samples_it == stratum_samples.end()) continue;

                    for (const auto& s_lower : u_samples_it->second) {
                        auto r_lower = get_sample_astro_range(s_lower);
                        if (!r_lower.has_date) continue;

                        for (const auto& s_upper : v_samples_it->second) {
                            auto r_upper = get_sample_astro_range(s_upper);
                            if (!r_upper.has_date) continue;

                            // Check 2-sigma interval overlap:
                            // Two intervals [A1, A2] and [B1, B2] overlap iff max(A1, B1) <= min(A2, B2).
                            bool intervals_overlap = std::max(r_lower.t_earliest, r_upper.t_earliest) <= 
                                                     std::min(r_lower.t_latest, r_upper.t_latest);

                            // If calibrated ranges overlap, stratigraphic sequence is valid (no inversion).
                            if (intervals_overlap) {
                                continue;
                            }

                            // Strict non-overlap: check if lower sample is strictly younger than upper sample
                            // (i.e. Lower sample earliest date is strictly after upper sample latest date)
                            if (r_lower.t_earliest > r_upper.t_latest) {
                                StratigraphicInversion inv;
                                inv.upper_stratum_id = v.id;
                                inv.upper_stratum_name = v.stratum_name;
                                inv.upper_date_astro = r_upper.t_latest;
                                inv.upper_date_display = r_upper.display;
                                inv.lower_stratum_id = u.id;
                                inv.lower_stratum_name = u.stratum_name;
                                inv.lower_date_astro = r_lower.t_earliest;
                                inv.lower_date_display = r_lower.display;
                                inv.is_advisory = true; // Advisory review flag per project locked principles
                                inv.sample_upper_code = s_upper.lab_code;
                                inv.sample_lower_code = s_lower.lab_code;
                                inv.reason = "Possible redeposition or context error — please review: Sample " + 
                                             s_upper.lab_code + " (" + r_upper.display + ") in upper stratum '" + v.stratum_name + 
                                             "' appears older than Sample " + s_lower.lab_code + " (" + r_lower.display + 
                                             ") in underlying stratum '" + u.stratum_name + "'.";
                                result.inversions.push_back(inv);
                            }
                        }
                    }
                }
            }
        }

        return result;
    }
};

} // namespace archaeophd
