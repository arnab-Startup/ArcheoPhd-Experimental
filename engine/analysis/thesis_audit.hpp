#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <json.hpp>
#include "models.hpp"
#include "storage.hpp"
#include "contradictions.hpp"
#include "hybrid_search.hpp"

namespace archaeophd {

// =============================================================================
// Native Thesis Auditor Engine — Phase 2 Step 6
// =============================================================================
// Comprehensive pre-submission dissertation defense readiness engine.
// Evaluates thesis chapter claims against empirical evidence (Layer C),
// detected contradictions (Types 1-4), Harris matrix stratigraphy, and
// scanned OCR grounding flags.
//
// Augments unsupported claims with automated literature citation suggestions
// derived from the native hybrid vector + BM25 search index.
// =============================================================================

class NativeThesisAuditor {
private:
    const NativeStorage& storage_;
    const NativeContradictionEngine& contradiction_engine_;

public:
    NativeThesisAuditor(const NativeStorage& storage, const NativeContradictionEngine& ce)
        : storage_(storage), contradiction_engine_(ce) {}

    json run_audit(const std::string& project_id = "default") const {
        auto claims = storage_.get_claims(project_id);
        auto evidence = storage_.get_evidence(project_id);
        auto conflicts = contradiction_engine_.run_all(project_id);

        // Map claim ID to linked empirical evidence
        std::map<std::string, std::vector<EvidenceLink>> ev_map;
        for (const auto& ev : evidence) {
            ev_map[ev.claim_id].push_back(ev);
        }

        // Map claims by chapter
        std::map<std::string, std::vector<Claim>> chapter_claims;
        for (const auto& c : claims) {
            std::string ch = c.chapter.empty() ? "Unassigned Chapter" : c.chapter;
            chapter_claims[ch].push_back(c);
        }

        // Map claim ID to active contradictions
        std::map<std::string, std::vector<Contradiction>> claim_conflicts;
        for (const auto& cf : conflicts) {
            if (!cf.flagged_claim_id.empty()) {
                claim_conflicts[cf.flagged_claim_id].push_back(cf);
            }
            // Match by source citation text or entity
            for (const auto& c : claims) {
                if (c.claim_text == cf.claim_a || c.claim_text == cf.claim_b) {
                    claim_conflicts[c.id].push_back(cf);
                }
            }
        }

        std::set<std::string> affected_chapters;
        json all_findings = json::array();
        json chapter_audits = json::array();
        json remediation_plan = json::array();

        int total_unsupported = 0;
        int total_verified = 0;
        int total_contested = 0;
        int total_anomalies = 0;
        int total_contradicted_claims = 0;

        // Audit each chapter individually
        for (const auto& kv : chapter_claims) {
            const std::string& ch_name = kv.first;
            const auto& ch_claim_list = kv.second;

            int ch_unsupported = 0;
            int ch_verified = 0;
            int ch_contested = 0;
            int ch_anomalies = 0;
            int ch_contradicted = 0;
            json ch_findings = json::array();
            std::vector<std::string> viva_risks;

            for (const auto& c : ch_claim_list) {
                if (c.status == "Verified") {
                    ch_verified++;
                    total_verified++;
                } else {
                    ch_contested++;
                    total_contested++;
                }

                if (c.anomaly_flag && c.verification_status != "VERIFIED") {
                    ch_anomalies++;
                    total_anomalies++;
                    affected_chapters.insert(ch_name);
                    
                    std::string reason = c.anomaly_reason.empty() ? "Scanned OCR metric anomaly" : c.anomaly_reason;
                    viva_risks.push_back("Data reliability vulnerability: Claim contains ungrounded scan anomaly (" + reason + ").");

                    json f = {
                        {"severity", "HIGH"},
                        {"category", "Ungrounded OCR Anomaly"},
                        {"chapter", ch_name},
                        {"claim_id", c.id},
                        {"claim_text", c.claim_text},
                        {"issue", "Extracted from scanned print with anomaly flag. Primary optical crop has not been verified by researcher."},
                        {"recommendation", "Open verification queue and verify primary source optical crop before viva examination."},
                        {"optical_crop_path", c.optical_crop_path}
                    };
                    ch_findings.push_back(f);
                    all_findings.push_back(f);

                    remediation_plan.push_back({
                        {"priority", "HIGH"},
                        {"action_type", "GROUND_OCR_ANOMALY"},
                        {"chapter", ch_name},
                        {"claim_id", c.id},
                        {"description", "Verify scanned plate crop for metric: '" + c.claim_text + "'."}
                    });
                }

                const auto& c_ev = ev_map[c.id];
                int sup = 0, con = 0;
                for (const auto& e : c_ev) {
                    if (e.evidence_type == "supporting") sup++;
                    else if (e.evidence_type == "contradicting") con++;
                }

                // Check for unsupported claim (bare assertion without evidence)
                if (sup == 0 && con == 0 && !c.anomaly_flag) {
                    ch_unsupported++;
                    total_unsupported++;
                    affected_chapters.insert(ch_name);

                    viva_risks.push_back("Examiner challenge: Assertion '" + c.claim_text.substr(0, 60) + "...' has zero direct Layer C empirical citations.");

                    json f = {
                        {"severity", "HIGH"},
                        {"category", "Unsupported Claim"},
                        {"chapter", ch_name},
                        {"claim_id", c.id},
                        {"claim_text", c.claim_text},
                        {"issue", "Zero direct supporting empirical evidence recorded in Layer C."},
                        {"recommendation", "Link primary excavation records/artifacts or reframe statement as hypothesis."}
                    };

                    // Search-augmented citation suggestions via hybrid BM25 + dense vector index
                    json suggestions = json::array();
                    if (storage_.lexical().size() > 0) {
                        try {
                            auto hits = HybridSearchEngine::search(
                                storage_.vectors(),
                                storage_.lexical(),
                                c.claim_text,
                                2
                            );
                            for (const auto& hit : hits) {
                                std::string chunk_txt = storage_.read_compressed_chunk(hit.chunk_id);
                                if (!chunk_txt.empty()) {
                                    std::string snippet = chunk_txt.substr(0, std::min<size_t>(180, chunk_txt.size()));
                                    suggestions.push_back({
                                        {"doc_id", hit.doc_id},
                                        {"chunk_id", hit.chunk_id},
                                        {"page_ref", hit.page_ref},
                                        {"relevance_score", hit.score},
                                        {"excerpt", snippet + "..."}
                                    });
                                }
                            }
                        } catch (...) {}
                    }
                    f["suggested_citations"] = suggestions;
                    ch_findings.push_back(f);
                    all_findings.push_back(f);

                    remediation_plan.push_back({
                        {"priority", "MEDIUM"},
                        {"action_type", "ATTACH_CITATION"},
                        {"chapter", ch_name},
                        {"claim_id", c.id},
                        {"description", "Link primary excavation records or monograph excerpt for: '" + c.claim_text + "'."}
                    });
                }

                // Check for contradicting evidence links
                if (con > 0) {
                    ch_contradicted++;
                    total_contradicted_claims++;
                    affected_chapters.insert(ch_name);

                    std::string issue_msg = std::to_string(con) + " refuting empirical observation(s) exist in your library.";
                    viva_risks.push_back("Critical viva vulnerability: " + issue_msg + " Regarding: '" + c.claim_text.substr(0, 60) + "...'.");

                    json f = {
                        {"severity", con >= 2 ? "CRITICAL" : "HIGH"},
                        {"category", "Contradicted Claim"},
                        {"chapter", ch_name},
                        {"claim_id", c.id},
                        {"claim_text", c.claim_text},
                        {"issue", issue_msg},
                        {"recommendation", "Must acknowledge conflicting evidence in chapter text or footnotes to withstand viva examination."}
                    };
                    ch_findings.push_back(f);
                    all_findings.push_back(f);

                    remediation_plan.push_back({
                        {"priority", con >= 2 ? "CRITICAL" : "HIGH"},
                        {"action_type", "CITE_CONTRADICTION"},
                        {"chapter", ch_name},
                        {"claim_id", c.id},
                        {"description", "Acknowledge conflicting evidence in chapter footnote: '" + c.claim_text + "'."}
                    });
                }

                // Check active detected contradictions
                if (claim_conflicts.count(c.id)) {
                    for (const auto& cf : claim_conflicts[c.id]) {
                        affected_chapters.insert(ch_name);
                        viva_risks.push_back("Disputed authority: " + cf.title + " (" + cf.source_a + " vs " + cf.source_b + ").");
                    }
                }
            }

            // Entity-based contradiction mapping for this chapter
            for (const auto& cf : conflicts) {
                std::string ent = cf.entity_name;
                std::transform(ent.begin(), ent.end(), ent.begin(), ::tolower);
                for (const auto& c : ch_claim_list) {
                    std::string cl = c.claim_text;
                    std::transform(cl.begin(), cl.end(), cl.begin(), ::tolower);
                    if (!ent.empty() && cl.find(ent) != std::string::npos) {
                        affected_chapters.insert(ch_name);
                    }
                }
            }

            // Compute chapter score
            int ch_total = static_cast<int>(ch_claim_list.size());
            int ch_score = 100;
            if (ch_total > 0) {
                int ch_penalties = (ch_unsupported * 15) + (ch_contradicted * 20) + (ch_anomalies * 10);
                ch_score = std::max(20, 100 - ch_penalties);
            }

            std::string ch_status = "DEFENSE_READY";
            if (ch_score < 60) ch_status = "HIGH_RISK";
            else if (ch_score < 85) ch_status = "NEEDS_REVISION";

            chapter_audits.push_back({
                {"chapter_name", ch_name},
                {"total_claims", ch_total},
                {"verified_claims", ch_verified},
                {"unsupported_claims", ch_unsupported},
                {"contested_claims", ch_contested},
                {"contradicted_claims", ch_contradicted},
                {"anomaly_claims", ch_anomalies},
                {"chapter_score", ch_score},
                {"chapter_status", ch_status},
                {"viva_risk_factors", viva_risks},
                {"findings_count", ch_findings.size()}
            });
        }

        // Global contradiction mappings for any conflicts not caught per chapter
        for (const auto& cf : conflicts) {
            if (cf.type.find("Stratigraphic") != std::string::npos) {
                remediation_plan.push_back({
                    {"priority", "CRITICAL"},
                    {"action_type", "RESOLVE_STRATIGRAPHIC_CYCLE"},
                    {"chapter", "Stratigraphy / Methodology"},
                    {"claim_id", cf.id},
                    {"description", "Physical impossibility in Harris matrix: " + cf.title + ". Trace trench records to resolve cycle."}
                });
            }
        }

        // Calculate overall defense readiness score
        int total_claims_count = static_cast<int>(claims.size());
        int overall_score = 100;
        if (total_claims_count > 0) {
            int penalty = (total_unsupported * 10) + 
                          (static_cast<int>(conflicts.size()) * 15) + 
                          (total_anomalies * 8) +
                          (total_contradicted_claims * 12);
            overall_score = std::max(15, 100 - penalty);
        }

        std::string defense_verdict = "DEFENSE_READY";
        if (overall_score < 50) defense_verdict = "CRITICAL_DEFENSE_RISK";
        else if (overall_score < 75) defense_verdict = "VULNERABLE_TO_EXAMINATION";
        else if (overall_score < 90) defense_verdict = "SATISFACTORY_WITH_MINOR_REVISIONS";

        // Sort remediation plan by priority: CRITICAL first, then HIGH, then MEDIUM
        std::sort(remediation_plan.begin(), remediation_plan.end(), [](const json& a, const json& b) {
            auto rank = [](const std::string& p) {
                if (p == "CRITICAL") return 0;
                if (p == "HIGH") return 1;
                return 2;
            };
            return rank(a.value("priority", "MEDIUM")) < rank(b.value("priority", "MEDIUM"));
        });

        json res;
        res["project_id"] = project_id;
        res["total_claims"] = total_claims_count;
        res["verified_claims_count"] = total_verified;
        res["contested_claims_count"] = total_contested;
        res["unsupported_claims_count"] = total_unsupported;
        res["total_conflicts_detected"] = conflicts.size();
        res["affected_chapters"] = std::vector<std::string>(affected_chapters.begin(), affected_chapters.end());
        res["defense_readiness_score"] = overall_score;
        res["defense_verdict"] = defense_verdict;
        res["conflicts"] = json::array();
        for (const auto& cf : conflicts) {
            res["conflicts"].push_back(cf);
        }
        res["findings"] = all_findings;
        res["chapter_audits"] = chapter_audits;
        res["remediation_action_plan"] = remediation_plan;
        res["summary"] = "Pre-Submission Thesis Audit complete: " + std::to_string(conflicts.size()) +
                         " active conflict(s) detected across " + std::to_string(affected_chapters.size()) +
                         " chapter(s). Defense readiness: " + std::to_string(overall_score) + "% (" + defense_verdict + ").";

        return res;
    }
};

} // namespace archaeophd
