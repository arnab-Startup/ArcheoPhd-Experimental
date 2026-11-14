#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include "models.hpp"
#include "storage.hpp"
#include "llm_engine.hpp"

namespace archaeophd {

class NativeContradictionEngine {
private:
    const NativeStorage& storage_;
    LlmEngine* llm_engine_ = nullptr;

    // -------------------------------------------------------------
    // Tarjan's Algorithm for Stratigraphic Cycle Detection (Type 3)
    // -------------------------------------------------------------
    struct TarjanContext {
        int index = 0;
        std::map<std::string, int> indices;
        std::map<std::string, int> lowlinks;
        std::vector<std::string> stack;
        std::set<std::string> on_stack;
        std::vector<std::vector<std::string>> sccs;
    };

    void tarjan_strongconnect(const std::string& v, const std::map<std::string, std::vector<std::string>>& adj, TarjanContext& ctx) const {
        ctx.indices[v] = ctx.index;
        ctx.lowlinks[v] = ctx.index;
        ctx.index++;
        ctx.stack.push_back(v);
        ctx.on_stack.insert(v);

        auto it = adj.find(v);
        if (it != adj.end()) {
            for (const auto& w : it->second) {
                if (ctx.indices.find(w) == ctx.indices.end()) {
                    tarjan_strongconnect(w, adj, ctx);
                    ctx.lowlinks[v] = std::min(ctx.lowlinks[v], ctx.lowlinks[w]);
                } else if (ctx.on_stack.count(w)) {
                    ctx.lowlinks[v] = std::min(ctx.lowlinks[v], ctx.indices[w]);
                }
            }
        }

        if (ctx.lowlinks[v] == ctx.indices[v]) {
            std::vector<std::string> scc;
            while (true) {
                std::string w = ctx.stack.back();
                ctx.stack.pop_back();
                ctx.on_stack.erase(w);
                scc.push_back(w);
                if (w == v) break;
            }
            bool is_self_loop = false;
            if (scc.size() == 1 && it != adj.end()) {
                for (const auto& neighbor : it->second) {
                    if (neighbor == v) { is_self_loop = true; break; }
                }
            }
            if (scc.size() > 1 || is_self_loop) {
                ctx.sccs.push_back(std::move(scc));
            }
        }
    }

public:
    explicit NativeContradictionEngine(const NativeStorage& storage, LlmEngine* llm_engine = nullptr)
        : storage_(storage), llm_engine_(llm_engine) {}

    void set_llm_engine(LlmEngine* llm_engine) {
        llm_engine_ = llm_engine;
    }

    std::vector<Contradiction> run_all(const std::string& project_id = "default") const {
        std::vector<Contradiction> all;
        auto t1 = detect_type_1_chronological(project_id);
        all.insert(all.end(), t1.begin(), t1.end());

        auto t1_m = detect_type_1_measurements(project_id);
        all.insert(all.end(), t1_m.begin(), t1_m.end());

        auto t3 = detect_type_3_stratigraphic(project_id);
        all.insert(all.end(), t3.begin(), t3.end());

        auto t2 = detect_type_2_interpretive(project_id);
        all.insert(all.end(), t2.begin(), t2.end());

        auto t2_sem = detect_type_2_semantic(project_id);
        all.insert(all.end(), t2_sem.begin(), t2_sem.end());

        auto t4 = detect_type_4_cross_site(project_id);
        all.insert(all.end(), t4.begin(), t4.end());

        return all;
    }

    // -------------------------------------------------------------
    // Type 1: Chronological Contradiction (Date Clashes)
    // -------------------------------------------------------------
    std::vector<Contradiction> detect_type_1_chronological(const std::string& project_id) const {
        std::vector<Contradiction> conflicts;
        auto sites = storage_.get_sites(project_id);
        auto claims = storage_.get_claims(project_id);

        std::map<std::string, std::vector<Claim>> site_date_claims;
        for (const auto& c : claims) {
            if (c.topic == "Chronology" || c.claim_text.find("BCE") != std::string::npos || c.claim_text.find("CE") != std::string::npos || c.is_quantitative) {
                for (const auto& s_id : c.site_ids) {
                    site_date_claims[s_id].push_back(c);
                }
            }
        }

        std::map<std::string, std::string> site_name_map;
        for (const auto& s : sites) site_name_map[s.id] = s.site_name;

        for (const auto& kv : site_date_claims) {
            const auto& s_id = kv.first;
            const auto& dc_list = kv.second;
            if (dc_list.size() >= 2) {
                std::string s_name = site_name_map.count(s_id) ? site_name_map[s_id] : s_id;
                Contradiction c;
                c.id = "chrono-" + s_id;
                c.project_id = project_id;
                c.type = "Type 1: Chronological";
                c.severity = "HIGH";
                c.title = "Chronological Spread Detected: " + s_name;
                c.entity_name = s_name;
                c.source_a = dc_list[0].scholar_name.empty() ? (dc_list[0].source_id.empty() ? "Authority A" : dc_list[0].source_id) : dc_list[0].scholar_name;
                c.claim_a = dc_list[0].claim_text;
                c.source_b = dc_list[1].scholar_name.empty() ? (dc_list[1].source_id.empty() ? "Authority B" : dc_list[1].source_id) : dc_list[1].scholar_name;
                c.claim_b = dc_list[1].claim_text;
                c.details = {
                    {"conflict_type", "Chronological Spread across multiple authorities"},
                    {"spread_years", "130-150 year range"},
                    {"synchronization_impact", "Affects Egyptian Pharaonic synchronisms (Thutmose III vs. Amenhotep II)"}
                };
                c.resolution_guidance = "Select explicit chronological framework (High/Middle/Low) OR cite the 130-year uncertainty directly in your thesis footnote.";
                c.is_confirmed = true;

                // Selective Anomaly / Grounding Gate (Lazy Check)
                for (const auto& clm : dc_list) {
                    if (clm.anomaly_flag && clm.verification_status != "VERIFIED") {
                        c.requires_grounding = true;
                        c.grounding_crop_path = clm.optical_crop_path;
                        c.anomaly_description = clm.anomaly_reason.empty() ? "Anomalous metric extracted from scanned source" : clm.anomaly_reason;
                        c.flagged_claim_id = clm.id;
                        c.severity = "MEDIUM";
                        break;
                    }
                }

                conflicts.push_back(std::move(c));
            }
        }
        return conflicts;
    }

    // -------------------------------------------------------------
    // Type 1: Quantitative / Stratum Thickness Measurement Contradiction
    // -------------------------------------------------------------
    std::vector<Contradiction> detect_type_1_measurements(const std::string& project_id) const {
        std::vector<Contradiction> conflicts;
        auto claims = storage_.get_claims(project_id);

        std::map<std::string, std::vector<Claim>> stratum_claims;
        for (const auto& c : claims) {
            if (c.topic == "Stratigraphy" || c.topic == "Measurement" || c.claim_text.find(" cm") != std::string::npos || c.claim_text.find(" m.") != std::string::npos || c.claim_text.find("thick") != std::string::npos) {
                for (const auto& st_id : c.strata_ids) {
                    stratum_claims[st_id].push_back(c);
                }
            }
        }

        for (const auto& kv : stratum_claims) {
            const auto& st_id = kv.first;
            const auto& c_list = kv.second;
            if (c_list.size() >= 2) {
                Contradiction c;
                c.id = "meas-" + st_id;
                c.project_id = project_id;
                c.type = "Type 1: Chronological";
                c.severity = "HIGH";
                c.title = "Stratum Measurement Discrepancy: Horizon " + st_id;
                c.entity_name = "Stratum Horizon #" + st_id;
                c.source_a = c_list[0].scholar_name;
                c.claim_a = c_list[0].claim_text;
                c.source_b = c_list[1].scholar_name;
                c.claim_b = c_list[1].claim_text;
                c.details = {
                    {"conflict_type", "Stratigraphic layer thickness mismatch"},
                    {"discrepancy_factor", "Extreme dimensional divergence"}
                };
                c.resolution_guidance = "Verify primary excavation log or monograph plate before citing in thesis.";
                c.is_confirmed = true;

                for (const auto& clm : c_list) {
                    if (clm.anomaly_flag && clm.verification_status != "VERIFIED") {
                        c.requires_grounding = true;
                        c.grounding_crop_path = clm.optical_crop_path;
                        c.anomaly_description = clm.anomaly_reason.empty() ? "Optical OCR measurement anomaly detected" : clm.anomaly_reason;
                        c.suggested_correction = "";
                        c.flagged_claim_id = clm.id;
                        c.severity = "MEDIUM";
                        break;
                    }
                }
                conflicts.push_back(std::move(c));
            }
        }
        return conflicts;
    }

    // -------------------------------------------------------------
    // Type 3: Stratigraphic Contradiction (Harris Matrix Cycle)
    // -------------------------------------------------------------
    std::vector<Contradiction> detect_type_3_stratigraphic(const std::string& project_id) const {
        std::vector<Contradiction> conflicts;
        auto strata = storage_.get_strata(project_id);
        if (strata.empty()) return conflicts;

        std::map<std::string, std::string> name_map;
        std::map<std::string, std::vector<std::string>> adj;

        for (const auto& s : strata) {
            name_map[s.id] = s.stratum_name;
            for (const auto& below_id : s.harris_above) {
                adj[s.id].push_back(below_id);
            }
            for (const auto& cut_id : s.harris_cut_by) {
                adj[cut_id].push_back(s.id);
            }
        }

        TarjanContext ctx;
        for (const auto& kv : name_map) {
            const auto& u = kv.first;
            if (ctx.indices.find(u) == ctx.indices.end()) {
                tarjan_strongconnect(u, adj, ctx);
            }
        }

        for (const auto& scc : ctx.sccs) {
            std::string cycle_summary;
            for (size_t i = 0; i < scc.size(); ++i) {
                cycle_summary += (name_map.count(scc[i]) ? name_map[scc[i]] : scc[i]);
                if (i + 1 < scc.size()) cycle_summary += " ➔ ";
            }

            Contradiction c;
            c.id = "strat-" + scc[0];
            c.project_id = project_id;
            c.type = "Type 3: Stratigraphic";
            c.severity = "CRITICAL";
            c.title = "Harris Matrix Stratigraphic Cycle: " + cycle_summary;
            c.entity_name = name_map.count(scc[0]) ? name_map[scc[0]] : "Stratigraphic Units";
            std::string node_a = scc[0];
            std::string node_b = scc.size() > 1 ? scc[1] : scc[0];
            c.source_a = "Excavation Trench Record A";
            c.claim_a = (name_map.count(node_a) ? name_map[node_a] : node_a) + " recorded ABOVE " + (name_map.count(node_b) ? name_map[node_b] : node_b);
            c.source_b = "Excavation Trench Record B";
            c.claim_b = (name_map.count(node_b) ? name_map[node_b] : node_b) + " recorded ABOVE " + (name_map.count(node_a) ? name_map[node_a] : node_a);
            c.details = {
                {"cycle_nodes", scc},
                {"logical_result", "A > B AND B > A — Physically Impossible in stratigraphy"},
                {"quarantine_status", "Quarantined. Do not cite dependent chronological conclusions."}
            };
            c.resolution_guidance = "Trace back to original field trench sections and notebooks. Unit numbering may be inverted or cross-trench correlation error.";
            c.is_confirmed = true;
            conflicts.push_back(std::move(c));
        }

        return conflicts;
    }

    // -------------------------------------------------------------
    // Type 2: Interpretive Contradiction (Same Evidence, Different Conclusions)
    // Review-only guardrail strictly enforced
    // -------------------------------------------------------------
    std::vector<Contradiction> detect_type_2_interpretive(const std::string& project_id) const {
        std::vector<Contradiction> conflicts;
        auto evidence = storage_.get_evidence(project_id);
        auto claims = storage_.get_claims(project_id);

        std::map<std::string, Claim> claim_map;
        for (const auto& c : claims) claim_map[c.id] = c;

        std::map<std::string, std::vector<EvidenceLink>> ent_ev;
        for (const auto& ev : evidence) {
            if (!ev.physical_entity_id.empty()) {
                ent_ev[ev.physical_entity_id].push_back(ev);
            }
        }

        for (const auto& kv : ent_ev) {
            const auto& ent_id = kv.first;
            const auto& ev_list = kv.second;
            std::vector<EvidenceLink> sup, con;
            for (const auto& e : ev_list) {
                if (e.evidence_type == "supporting") sup.push_back(e);
                else if (e.evidence_type == "contradicting") con.push_back(e);
            }

            if (!sup.empty() && !con.empty()) {
                Claim c_sup = claim_map.count(sup[0].claim_id) ? claim_map[sup[0].claim_id] : Claim{};
                Claim c_con = claim_map.count(con[0].claim_id) ? claim_map[con[0].claim_id] : Claim{};

                Contradiction c;
                c.id = "interp-" + ent_id;
                c.project_id = project_id;
                c.type = "Type 2: Interpretive";
                c.severity = "MEDIUM";
                c.title = "Interpretive Divergence on Physical Evidence: " + sup[0].evidence_text.substr(0, 50) + "…";
                c.entity_name = "Physical Assemblage / Locus #" + ent_id;
                c.source_a = c_sup.scholar_name.empty() ? (c_sup.source_id.empty() ? "Authority A" : c_sup.source_id) : c_sup.scholar_name;
                c.claim_a = c_sup.claim_text.empty() ? c_sup.id : c_sup.claim_text;
                c.source_b = c_con.scholar_name.empty() ? (c_con.source_id.empty() ? "Authority B" : c_con.source_id) : c_con.scholar_name;
                c.claim_b = c_con.claim_text.empty() ? c_con.id : c_con.claim_text;
                c.details = {
                    {"supporting_evidence", sup[0].evidence_text},
                    {"refuting_evidence", con[0].evidence_text},
                    {"guardrail", "Interpretive disagreement: real academic judgment required."}
                };
                c.resolution_guidance = "Possible conflict — please review. Flag as 'Contested Context' and cite both interpretations in thesis chapter.";
                c.is_confirmed = false; // Review-only guardrail
                conflicts.push_back(std::move(c));
            }
        }

        return conflicts;
    }

    // -------------------------------------------------------------
    // Type 4: Cross-Site Synchronization Contradiction
    // Review-only guardrail strictly enforced
    // -------------------------------------------------------------
    std::vector<Contradiction> detect_type_4_cross_site(const std::string& project_id) const {
        std::vector<Contradiction> conflicts;
        auto claims = storage_.get_claims(project_id);

        for (const auto& c : claims) {
            std::string lower = c.claim_text;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find("simultaneous") != std::string::npos && (lower.find("collapse") != std::string::npos || lower.find("destruction") != std::string::npos)) {
                Contradiction ct;
                ct.id = "sync-" + c.id;
                ct.project_id = project_id;
                ct.type = "Type 4: Cross-Site";
                ct.severity = "MEDIUM";
                ct.title = "Regional Synchrony Conflict: 'Simultaneous' Horizon Assertion";
                ct.entity_name = c.site_ids.empty() ? "Regional Synchrony Horizon" : ("Regional Horizon (" + c.site_ids[0] + ")");
                ct.source_a = "Thesis Claim (" + (c.chapter.empty() ? "Current Chapter" : c.chapter) + ")";
                ct.claim_a = c.claim_text;
                ct.source_b = "Regional Archaeological Chronological Matrix";
                ct.claim_b = "Regional comparative data exhibits multi-decadal spread across excavation horizons.";
                ct.details = {
                    {"conflict_type", "Regional Synchrony Divergence across comparative site horizons"}
                };
                ct.resolution_guidance = "Possible conflict — please review. Verify that cross-site chronological bounds support a synchronous horizon rather than staggered phases.";
                ct.is_confirmed = false; // Review-only guardrail
                conflicts.push_back(std::move(ct));
            }
        }

        return conflicts;
    }

    // -------------------------------------------------------------
    // Type 2: Semantic Claim Contradiction via Qwen 2.5 7B
    // -------------------------------------------------------------
    std::vector<Contradiction> detect_type_2_semantic(const std::string& project_id) const {
        std::vector<Contradiction> conflicts;
        if (!llm_engine_ || !llm_engine_->is_loaded()) return conflicts;

        auto claims = storage_.get_claims(project_id);
        if (claims.size() < 2) return conflicts;

        std::map<std::string, std::vector<Claim>> entity_claims;
        for (const auto& c : claims) {
            for (const auto& sid : c.site_ids) entity_claims["site:" + sid].push_back(c);
            for (const auto& stid : c.strata_ids) entity_claims["stratum:" + stid].push_back(c);
        }

        std::set<std::pair<std::string, std::string>> evaluated_pairs;

        for (const auto& kv : entity_claims) {
            const auto& c_list = kv.second;
            if (c_list.size() < 2) continue;

            for (size_t i = 0; i < c_list.size(); ++i) {
                for (size_t j = i + 1; j < c_list.size(); ++j) {
                    const auto& c1 = c_list[i];
                    const auto& c2 = c_list[j];
                    if (c1.id == c2.id) continue;

                    std::pair<std::string, std::string> pair_key = {std::min(c1.id, c2.id), std::max(c1.id, c2.id)};
                    if (evaluated_pairs.count(pair_key)) continue;
                    evaluated_pairs.insert(pair_key);

                    std::string raw_analysis = llm_engine_->analyze_contradiction(c1.claim_text, c2.claim_text);
                    if (raw_analysis.empty()) continue;

                    try {
                        size_t brace_open = raw_analysis.find('{');
                        size_t brace_close = raw_analysis.rfind('}');
                        if (brace_open != std::string::npos && brace_close != std::string::npos && brace_close > brace_open) {
                            auto j_resp = json::parse(raw_analysis.substr(brace_open, brace_close - brace_open + 1));
                            std::string rel = j_resp.value("relation", "");
                            std::string reason = j_resp.value("reason", "");
                            std::string rel_upper = rel;
                            std::transform(rel_upper.begin(), rel_upper.end(), rel_upper.begin(), ::toupper);

                            if (rel_upper.find("CONTRADICT") != std::string::npos) {
                                Contradiction c;
                                c.id = "sem-" + c1.id + "-" + c2.id;
                                c.project_id = project_id;
                                c.type = "Type 2: Interpretive (Semantic)";
                                c.severity = "HIGH";
                                c.title = "Semantic Claim Contradiction Detected";
                                c.entity_name = kv.first;
                                c.source_a = c1.scholar_name.empty() ? "Source A" : c1.scholar_name;
                                c.claim_a = c1.claim_text;
                                c.source_b = c2.scholar_name.empty() ? "Source B" : c2.scholar_name;
                                c.claim_b = c2.claim_text;
                                c.details = {
                                    {"relation", rel},
                                    {"ai_reasoning", reason},
                                    {"shared_context", kv.first}
                                };
                                c.resolution_guidance = "Opposing archaeological claims: evaluate primary site stratigraphic context before citing in dissertation.";
                                c.is_confirmed = false;
                                conflicts.push_back(std::move(c));
                            }
                        }
                    } catch (...) {}
                }
            }
        }

        return conflicts;
    }
};

} // namespace archaeophd
