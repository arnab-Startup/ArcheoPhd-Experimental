#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <cmath>
#include <json.hpp>
#include "models.hpp"
#include "storage.hpp"
#include "contradictions.hpp"
#include "thesis_audit.hpp"

namespace archaeophd {

using json = nlohmann::json;

struct DashboardSummaryCounts {
    int total_sites = 0;
    int total_strata = 0;
    int total_artifacts = 0;
    int total_samples = 0;
    int total_claims = 0;
    int total_evidence = 0;
    int total_sources = 0;
    int total_notes = 0;
    int total_contradictions = 0;
    int pending_class_b_verifications = 0;
    int ungrounded_claims = 0;
};

struct ClaimsBreakdown {
    int total_claims = 0;
    int supported = 0;
    int contradicted = 0;
    int unsupported = 0;
    double grounding_percentage = 0.0;
};

struct ResearchGapItem {
    std::string gap_type; // "UNSUPPORTED_CLAIM", "SPARSE_SOURCE_TOPIC", "STRATIGRAPHIC_DATA_GAP"
    std::string title;
    std::string description;
    std::string severity; // "HIGH", "MEDIUM", "LOW"
    std::string entity_id;
};

struct VivaDefenseAlertItem {
    std::string alert_type; // "CONTRADICTION", "CLASS_B_UNVERIFIED", "CHRONOLOGY_CONFLICT", "UNGROUNDED_CLAIM"
    std::string severity;   // "CRITICAL", "HIGH", "MEDIUM"
    std::string title;
    std::string message;
    std::string resolution_guidance;
};

class NativeAnalyticsEngine {
private:
    NativeStorage& storage_;
    NativeThesisAuditor* thesisAuditor_ = nullptr;
    NativeContradictionEngine* contradictionEngine_ = nullptr;

public:
    NativeAnalyticsEngine(
        NativeStorage& storage,
        NativeThesisAuditor* thesisAuditor = nullptr,
        NativeContradictionEngine* contradictionEngine = nullptr
    ) : storage_(storage),
        thesisAuditor_(thesisAuditor),
        contradictionEngine_(contradictionEngine) {}

    DashboardSummaryCounts compute_summary_counts(const std::string& project_id) const {
        DashboardSummaryCounts c;
        c.total_sites = static_cast<int>(storage_.get_sites(project_id).size());
        c.total_strata = static_cast<int>(storage_.get_strata(project_id).size());
        c.total_artifacts = static_cast<int>(storage_.get_artifacts(project_id).size());
        c.total_samples = static_cast<int>(storage_.get_samples(project_id).size());
        c.total_evidence = static_cast<int>(storage_.get_evidence(project_id).size());
        c.total_sources = static_cast<int>(storage_.get_sources(project_id).size());
        c.total_notes = static_cast<int>(storage_.get_notes(project_id).size());

        auto claims = storage_.get_claims(project_id);
        c.total_claims = static_cast<int>(claims.size());

        auto evidence = storage_.get_evidence(project_id);
        std::set<std::string> groundedClaimIds;
        for (const auto& ev : evidence) {
            if (!ev.claim_id.empty() && ev.evidence_type == "supporting") {
                groundedClaimIds.insert(ev.claim_id);
            }
        }

        for (const auto& cl : claims) {
            if (cl.verification_status == "PENDING_VERIFICATION" || cl.anomaly_flag) {
                c.pending_class_b_verifications++;
            }
            if (groundedClaimIds.find(cl.id) == groundedClaimIds.end()) {
                c.ungrounded_claims++;
            }
        }

        if (contradictionEngine_) {
            c.total_contradictions = static_cast<int>(contradictionEngine_->run_all(project_id).size());
        }

        return c;
    }

    ClaimsBreakdown compute_claims_breakdown(const std::string& project_id) const {
        ClaimsBreakdown cb;
        auto claims = storage_.get_claims(project_id);
        auto evidence = storage_.get_evidence(project_id);
        cb.total_claims = static_cast<int>(claims.size());

        std::map<std::string, bool> hasSupporting;
        std::map<std::string, bool> hasContradicting;

        for (const auto& ev : evidence) {
            if (ev.claim_id.empty()) continue;
            if (ev.evidence_type == "supporting") {
                hasSupporting[ev.claim_id] = true;
            } else if (ev.evidence_type == "contradicting") {
                hasContradicting[ev.claim_id] = true;
            }
        }

        for (const auto& cl : claims) {
            bool sup = hasSupporting[cl.id];
            bool con = hasContradicting[cl.id];

            if (con) {
                cb.contradicted++;
            }
            if (sup) {
                cb.supported++;
            }
            if (!sup && !con) {
                cb.unsupported++;
            }
        }

        if (cb.total_claims > 0) {
            cb.grounding_percentage = (static_cast<double>(cb.supported) / cb.total_claims) * 100.0;
        }

        return cb;
    }

    json compute_temporal_coverage_heatmap(const std::string& project_id) const {
        auto sites = storage_.get_sites(project_id);
        auto artifacts = storage_.get_artifacts(project_id);

        std::map<std::string, int> sitePeriodCounts;
        for (const auto& s : sites) {
            std::string p = s.period.empty() ? "Unassigned Period" : s.period;
            sitePeriodCounts[p]++;
        }

        std::map<std::string, int> artifactPeriodCounts;
        for (const auto& a : artifacts) {
            std::string p = a.period.empty() ? "Unassigned Period" : a.period;
            artifactPeriodCounts[p]++;
        }

        std::set<std::string> allPeriods;
        for (const auto& [p, _] : sitePeriodCounts) allPeriods.insert(p);
        for (const auto& [p, _] : artifactPeriodCounts) allPeriods.insert(p);

        json heatmap = json::array();
        for (const auto& p : allPeriods) {
            int sCount = sitePeriodCounts.count(p) ? sitePeriodCounts[p] : 0;
            int aCount = artifactPeriodCounts.count(p) ? artifactPeriodCounts[p] : 0;
            int total = sCount + aCount;
            std::string density = "Sparse";
            if (total >= 5) density = "High Density";
            else if (total >= 2) density = "Moderate";

            heatmap.push_back({
                {"period", p},
                {"site_count", sCount},
                {"artifact_count", aCount},
                {"total_entities", total},
                {"coverage_density", density}
            });
        }

        return heatmap;
    }

    json compute_sources_breakdown(const std::string& project_id) const {
        auto sources = storage_.get_sources(project_id);

        std::map<std::string, int> byYear;
        std::map<std::string, int> byType;
        std::map<std::string, int> byClass;

        for (const auto& s : sources) {
            std::string y = s.year.empty() ? "Undated" : s.year;
            byYear[y]++;

            std::string t = s.source_type.empty() ? "Other" : s.source_type;
            byType[t]++;

            std::string c = s.degradation_class.empty() ? "CLASS_B" : s.degradation_class;
            byClass[c]++;
        }

        json sourcesByYear = json::array();
        for (const auto& [year, count] : byYear) {
            sourcesByYear.push_back({{"year", year}, {"count", count}});
        }

        json sourcesByType = json::array();
        for (const auto& [type, count] : byType) {
            sourcesByType.push_back({{"name", type}, {"value", count}});
        }

        json sourcesByClass = json::array();
        for (const auto& [cls, count] : byClass) {
            sourcesByClass.push_back({{"classification", cls}, {"count", count}});
        }

        return {
            {"sources_by_year", sourcesByYear},
            {"sources_by_type", sourcesByType},
            {"sources_by_class", sourcesByClass}
        };
    }

    std::vector<ResearchGapItem> detect_research_gaps(const std::string& project_id) const {
        std::vector<ResearchGapItem> gaps;
        auto claims = storage_.get_claims(project_id);
        auto evidence = storage_.get_evidence(project_id);
        auto sources = storage_.get_sources(project_id);
        auto sites = storage_.get_sites(project_id);
        auto strata = storage_.get_strata(project_id);

        std::set<std::string> evidenceClaimIds;
        for (const auto& ev : evidence) {
            if (!ev.claim_id.empty()) evidenceClaimIds.insert(ev.claim_id);
        }

        // 1. Detect claims lacking empirical evidentiary linkage
        for (const auto& cl : claims) {
            if (evidenceClaimIds.find(cl.id) == evidenceClaimIds.end()) {
                ResearchGapItem item;
                item.gap_type = "UNSUPPORTED_CLAIM";
                item.title = "Unsupported Assertion: " + cl.claim_text.substr(0, std::min<size_t>(cl.claim_text.size(), 45)) + "...";
                item.description = "Claim ID '" + cl.id + "' by " + (cl.scholar_name.empty() ? "Scholar" : cl.scholar_name) + " possesses 0 supporting empirical evidence links in the Knowledge Graph.";
                item.severity = "HIGH";
                item.entity_id = cl.id;
                gaps.push_back(item);
            }
        }

        // 2. Detect topic clusters with assertions but minimal bibliographic groundings
        std::map<std::string, int> claimsPerTopic;
        for (const auto& cl : claims) {
            std::string top = cl.topic.empty() ? "General" : cl.topic;
            claimsPerTopic[top]++;
        }
        for (const auto& [top, count] : claimsPerTopic) {
            if (count >= 2 && sources.size() < 2) {
                ResearchGapItem item;
                item.gap_type = "SPARSE_SOURCE_TOPIC";
                item.title = "Bibliographic Gap in Topic: " + top;
                item.description = "Topic '" + top + "' has " + std::to_string(count) + " claims but fewer than 2 primary literature sources in the catalog.";
                item.severity = "MEDIUM";
                item.entity_id = top;
                gaps.push_back(item);
            }
        }

        // 3. Detect sites lacking published stratigraphic profiles
        for (const auto& site : sites) {
            bool hasStratum = false;
            for (const auto& st : strata) {
                if (st.site_id == site.id) {
                    hasStratum = true;
                    break;
                }
            }
            if (!hasStratum) {
                ResearchGapItem item;
                item.gap_type = "STRATIGRAPHIC_DATA_GAP";
                item.title = "Unstratified Excavation Site: " + site.site_name;
                item.description = "Site '" + site.site_name + "' has geographic coordinates recorded but 0 stratigraphic horizons linked.";
                item.severity = "LOW";
                item.entity_id = site.id;
                gaps.push_back(item);
            }
        }

        return gaps;
    }

    std::vector<VivaDefenseAlertItem> derive_viva_defense_alerts(const std::string& project_id) const {
        std::vector<VivaDefenseAlertItem> alerts;

        // 1. Contradictions detected by rule & neural engine
        if (contradictionEngine_) {
            auto contradictions = contradictionEngine_->run_all(project_id);
            for (const auto& c : contradictions) {
                VivaDefenseAlertItem item;
                item.alert_type = "CONTRADICTION";
                item.severity = c.severity;
                item.title = "Scholarly Contradiction: " + c.title;
                item.message = "Discrepancy between " + c.source_a + " and " + c.source_b + " regarding " + c.entity_name + ".";
                item.resolution_guidance = c.resolution_guidance.empty() ? "Review in dissertation footnote before defense." : c.resolution_guidance;
                alerts.push_back(item);
            }
        }

        // 2. Class B unverified scans / physical outliers
        auto claims = storage_.get_claims(project_id);
        for (const auto& cl : claims) {
            if (cl.anomaly_flag || cl.verification_status == "PENDING_VERIFICATION") {
                VivaDefenseAlertItem item;
                item.alert_type = "CLASS_B_UNVERIFIED";
                item.severity = "HIGH";
                item.title = "Unverified Scanned OCR Outlier: " + cl.id;
                item.message = cl.anomaly_reason.empty() ? "Claim flagged for manual verification before viva submission." : cl.anomaly_reason;
                item.resolution_guidance = "Inspect optical scan crop and manually verify or correct transcription.";
                alerts.push_back(item);
            }
        }

        // 3. Chronological Discrepancies across Multi-Site Horizons
        auto evidence = storage_.get_evidence(project_id);
        std::map<std::string, std::set<std::string>> siteDates;
        for (const auto& ev : evidence) {
            if (!ev.date_info.empty()) {
                for (const auto& sid : ev.source_ids) {
                    siteDates[sid].insert(ev.date_info);
                }
            }
        }
        for (const auto& [sid, dateSet] : siteDates) {
            if (dateSet.size() > 1) {
                std::string summaryDates;
                for (const auto& d : dateSet) {
                    if (!summaryDates.empty()) summaryDates += ", ";
                    summaryDates += d;
                }
                VivaDefenseAlertItem item;
                item.alert_type = "CHRONOLOGY_CONFLICT";
                item.severity = "MEDIUM";
                item.title = "Chronological Disagreement for Source: " + sid;
                item.message = "Multiple divergent absolute dates asserted (" + summaryDates + ").";
                item.resolution_guidance = "Reconcile radiocarbon determinations or qualify chronological boundaries.";
                alerts.push_back(item);
            }
        }

        return alerts;
    }

    json get_full_dashboard_analytics(const std::string& project_id) const {
        auto counts = compute_summary_counts(project_id);
        auto claimsBreakdown = compute_claims_breakdown(project_id);
        auto temporalHeatmap = compute_temporal_coverage_heatmap(project_id);
        auto sourcesBreakdown = compute_sources_breakdown(project_id);
        auto gaps = detect_research_gaps(project_id);
        auto alerts = derive_viva_defense_alerts(project_id);

        int defenseScore = 70;
        json affectedChapters = json::array();
        if (thesisAuditor_) {
            json audit = thesisAuditor_->run_audit(project_id);
            if (audit.contains("defense_readiness_score")) {
                defenseScore = audit["defense_readiness_score"].get<int>();
            }
            if (audit.contains("affected_chapters")) {
                affectedChapters = audit["affected_chapters"];
            }
        }

        json gapsArr = json::array();
        for (const auto& g : gaps) {
            gapsArr.push_back({
                {"gap_type", g.gap_type},
                {"title", g.title},
                {"description", g.description},
                {"severity", g.severity},
                {"entity_id", g.entity_id}
            });
        }

        json alertsArr = json::array();
        for (const auto& a : alerts) {
            alertsArr.push_back({
                {"alert_type", a.alert_type},
                {"severity", a.severity},
                {"title", a.title},
                {"message", a.message},
                {"resolution_guidance", a.resolution_guidance}
            });
        }

        json result;
        result["counts"] = {
            {"sites", counts.total_sites},
            {"strata", counts.total_strata},
            {"artifacts", counts.total_artifacts},
            {"samples", counts.total_samples},
            {"claims", counts.total_claims},
            {"evidence", counts.total_evidence},
            {"sources", counts.total_sources},
            {"notes", counts.total_notes},
            {"contradictions", counts.total_contradictions},
            {"pending_verifications", counts.pending_class_b_verifications},
            {"ungrounded_claims", counts.ungrounded_claims}
        };

        result["claims_breakdown"] = {
            {"total", claimsBreakdown.total_claims},
            {"supported", claimsBreakdown.supported},
            {"contradicted", claimsBreakdown.contradicted},
            {"unsupported", claimsBreakdown.unsupported},
            {"grounding_percentage", claimsBreakdown.grounding_percentage}
        };

        result["thesis_readiness"] = {
            {"defense_readiness_score", defenseScore},
            {"affected_chapters", affectedChapters}
        };

        result["temporal_coverage_heatmap"] = temporalHeatmap;
        result["sources_breakdown"] = sourcesBreakdown;
        result["research_gaps"] = gapsArr;
        result["viva_defense_alerts"] = alertsArr;

        return result;
    }
};

} // namespace archaeophd
