#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <json.hpp>
#include "models.hpp"
#include "storage.hpp"

namespace archaeophd {

// =============================================================================
// Native Evidence & Literature Graph Traversal Engine — Phase 3 Step 3
// =============================================================================
// Provides:
// 1. Heterogeneous multi-modal archaeological knowledge graph builder:
//    Nodes: SITE, STRATUM, ARTIFACT, SAMPLE, CLAIM, EVIDENCE_LINK, SOURCE
//    Edges: CONTAINS, STRATIFIED_ABOVE, EVIDENCED_BY, GROUNDED_IN, CITES, REFERENCES
// 2. Network topology analytics (degree centrality, authority hubs, epistemic grounding ratio)
// 3. Disconnected & ungrounded claim isolation (detecting claims without Layer C physical grounding)
// 4. Directed & bidirectional shortest-path causal/evidentiary chain tracing (BFS)
// 5. K-Hop ego neighborhood extraction for interactive graph exploration in ResearchGraph.jsx
// =============================================================================

struct GraphNode {
    std::string id;
    std::string label;
    std::string type; // SITE, STRATUM, ARTIFACT, SAMPLE, CLAIM, EVIDENCE_LINK, SOURCE
    json metadata;
    int degree = 0;
    int in_degree = 0;
    int out_degree = 0;
};

struct GraphEdge {
    std::string id;
    std::string source;
    std::string target;
    std::string type; // CONTAINS, STRATIFIED_ABOVE, EVIDENCED_BY, GROUNDED_IN, CITES, REFERENCES, CONTRADICTS
    std::string label;
};

struct GraphPath {
    bool found = false;
    int length = 0;
    std::vector<std::string> node_ids;
    std::vector<std::string> node_types;
    std::vector<std::string> node_labels;
    std::vector<std::string> edge_types;
};

struct GraphMetrics {
    int node_count = 0;
    int edge_count = 0;
    int site_count = 0;
    int stratum_count = 0;
    int artifact_count = 0;
    int sample_count = 0;
    int claim_count = 0;
    int evidence_link_count = 0;
    int source_count = 0;
    int grounded_claims_count = 0;
    int isolated_claims_count = 0;
    double grounding_ratio = 0.0;
    std::vector<std::string> isolated_claim_ids;
    std::vector<std::pair<std::string, int>> hub_nodes;
};

class NativeGraphEngine {
private:
    const NativeStorage& storage_;

public:
    explicit NativeGraphEngine(const NativeStorage& storage)
        : storage_(storage) {}

    // -------------------------------------------------------------
    // Build Complete Heterogeneous Adjacency Graph from Relational State
    // -------------------------------------------------------------
    struct AdjacencyGraph {
        std::map<std::string, GraphNode> nodes;
        std::vector<GraphEdge> edges;
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> adj_out; // node -> [(target, edge_type)]
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> adj_in;  // node -> [(source, edge_type)]
        std::map<std::string, std::vector<std::pair<std::string, std::string>>> adj_all; // node -> [(neighbor, edge_type)]
    };

    AdjacencyGraph build_graph(const std::string& project_id = "default") const {
        AdjacencyGraph g;

        auto sites = storage_.get_sites(project_id);
        auto strata = storage_.get_strata(project_id);
        auto artifacts = storage_.get_artifacts(project_id);
        auto samples = storage_.get_samples(project_id);
        auto claims = storage_.get_claims(project_id);
        auto evidence = storage_.get_evidence(project_id);
        auto sources = storage_.get_sources(project_id);

        // 1. Register Nodes
        for (const auto& s : sites) {
            GraphNode n;
            n.id = s.id;
            n.label = s.site_name.empty() ? s.id : s.site_name;
            n.type = "SITE";
            n.metadata = {{"region", s.region}, {"period", s.period}, {"elevation", s.elevation}};
            g.nodes[n.id] = std::move(n);
        }

        for (const auto& st : strata) {
            GraphNode n;
            n.id = st.id;
            n.label = st.stratum_name.empty() ? st.id : st.stratum_name;
            n.type = "STRATUM";
            n.metadata = {{"site_id", st.site_id}, {"phase", st.phase}, {"bounds", st.chronological_bounds}};
            g.nodes[n.id] = std::move(n);
        }

        for (const auto& a : artifacts) {
            GraphNode n;
            n.id = a.id;
            n.label = a.artifact_name.empty() ? a.id : a.artifact_name;
            n.type = "ARTIFACT";
            n.metadata = {{"category", a.category}, {"material", a.material}, {"stratum_id", a.stratum_id}};
            g.nodes[n.id] = std::move(n);
        }

        for (const auto& sm : samples) {
            GraphNode n;
            n.id = sm.id;
            n.label = sm.lab_code.empty() ? sm.id : sm.lab_code;
            n.type = "SAMPLE";
            n.metadata = {{"method", sm.method}, {"stratum_id", sm.stratum_id}, {"material", sm.material}};
            g.nodes[n.id] = std::move(n);
        }

        for (const auto& cl : claims) {
            GraphNode n;
            n.id = cl.id;
            n.label = cl.claim_text.size() > 50 ? cl.claim_text.substr(0, 47) + "..." : cl.claim_text;
            n.type = "CLAIM";
            n.metadata = {{"scholar", cl.scholar_name}, {"status", cl.status}, {"chapter", cl.chapter}};
            g.nodes[n.id] = std::move(n);
        }

        for (const auto& ev : evidence) {
            GraphNode n;
            n.id = ev.id;
            n.label = ev.evidence_text.size() > 50 ? ev.evidence_text.substr(0, 47) + "..." : ev.evidence_text;
            n.type = "EVIDENCE_LINK";
            n.metadata = {{"evidence_type", ev.evidence_type}, {"physical_entity_type", ev.physical_entity_type}};
            g.nodes[n.id] = std::move(n);
        }

        for (const auto& src : sources) {
            GraphNode n;
            n.id = src.id;
            n.label = src.title.empty() ? src.id : src.title;
            n.type = "SOURCE";
            n.metadata = {{"author", src.author}, {"year", src.year}, {"type", src.source_type}};
            g.nodes[n.id] = std::move(n);
        }

        // Helper to add edges safely
        auto add_edge = [&g](const std::string& src, const std::string& tgt, const std::string& type, const std::string& lbl) {
            if (g.nodes.find(src) == g.nodes.end() || g.nodes.find(tgt) == g.nodes.end()) return;

            std::string eid = "edge_" + src + "_" + tgt + "_" + type;
            GraphEdge e;
            e.id = eid;
            e.source = src;
            e.target = tgt;
            e.type = type;
            e.label = lbl;
            g.edges.push_back(std::move(e));

            g.adj_out[src].push_back({tgt, type});
            g.adj_in[tgt].push_back({src, type});
            g.adj_all[src].push_back({tgt, type});
            g.adj_all[tgt].push_back({src, type});

            g.nodes[src].out_degree++;
            g.nodes[src].degree++;
            g.nodes[tgt].in_degree++;
            g.nodes[tgt].degree++;
        };

        // 2. Synthesize Edges
        // A. Site -> Stratum (CONTAINS)
        for (const auto& st : strata) {
            if (!st.site_id.empty()) {
                add_edge(st.site_id, st.id, "CONTAINS", "contains stratum");
            }
        }

        // B. Stratum -> Artifact (CONTAINS)
        for (const auto& a : artifacts) {
            if (!a.stratum_id.empty()) {
                add_edge(a.stratum_id, a.id, "CONTAINS", "contains artifact");
            }
        }

        // C. Stratum -> Sample (CONTAINS)
        for (const auto& sm : samples) {
            if (!sm.stratum_id.empty()) {
                add_edge(sm.stratum_id, sm.id, "CONTAINS", "contains sample");
            }
        }

        // D. Stratum -> Stratum (STRATIFIED_ABOVE from Harris matrix)
        for (const auto& st : strata) {
            for (const auto& below_id : st.harris_below) {
                // st is directly above below_id
                add_edge(st.id, below_id, "STRATIFIED_ABOVE", "superimposed above");
            }
        }

        // E. Claim -> EvidenceLink (EVIDENCED_BY)
        for (const auto& ev : evidence) {
            if (!ev.claim_id.empty()) {
                add_edge(ev.claim_id, ev.id, "EVIDENCED_BY", "evidenced by");
            }
        }

        // F. EvidenceLink -> Physical Entity (GROUNDED_IN)
        for (const auto& ev : evidence) {
            if (!ev.physical_entity_id.empty()) {
                add_edge(ev.id, ev.physical_entity_id, "GROUNDED_IN", "grounded in observation");
            }
        }

        // G. Claim -> Source (CITES)
        for (const auto& cl : claims) {
            if (!cl.source_id.empty()) {
                add_edge(cl.id, cl.source_id, "CITES", "cites monograph");
            }
        }

        // H. EvidenceLink -> Source (CITES)
        for (const auto& ev : evidence) {
            for (const auto& sid : ev.source_ids) {
                if (!sid.empty()) {
                    add_edge(ev.id, sid, "CITES", "referenced in report");
                }
            }
        }

        // I. Claim -> Site / Stratum (REFERENCES)
        for (const auto& cl : claims) {
            for (const auto& sid : cl.site_ids) {
                if (!sid.empty()) add_edge(cl.id, sid, "REFERENCES", "references site context");
            }
            for (const auto& stid : cl.strata_ids) {
                if (!stid.empty()) add_edge(cl.id, stid, "REFERENCES", "references stratum context");
            }
        }

        return g;
    }

    // -------------------------------------------------------------
    // Topological Network Metrics & Epistemic Grounding Ratio
    // -------------------------------------------------------------
    GraphMetrics compute_metrics(const std::string& project_id = "default") const {
        auto g = build_graph(project_id);
        GraphMetrics m;

        m.node_count = static_cast<int>(g.nodes.size());
        m.edge_count = static_cast<int>(g.edges.size());

        for (const auto& [nid, node] : g.nodes) {
            if (node.type == "SITE") m.site_count++;
            else if (node.type == "STRATUM") m.stratum_count++;
            else if (node.type == "ARTIFACT") m.artifact_count++;
            else if (node.type == "SAMPLE") m.sample_count++;
            else if (node.type == "CLAIM") m.claim_count++;
            else if (node.type == "EVIDENCE_LINK") m.evidence_link_count++;
            else if (node.type == "SOURCE") m.source_count++;
        }

        // Epistemic grounding analysis: Check each CLAIM for Layer C link
        for (const auto& [nid, node] : g.nodes) {
            if (node.type != "CLAIM") continue;

            bool has_evidence = false;
            auto it_out = g.adj_out.find(nid);
            if (it_out != g.adj_out.end()) {
                for (const auto& edge : it_out->second) {
                    if (edge.second == "EVIDENCED_BY") {
                        has_evidence = true;
                        break;
                    }
                }
            }

            if (has_evidence) {
                m.grounded_claims_count++;
            } else {
                m.isolated_claims_count++;
                m.isolated_claim_ids.push_back(nid);
            }
        }

        m.grounding_ratio = (m.claim_count > 0)
            ? (static_cast<double>(m.grounded_claims_count) / static_cast<double>(m.claim_count))
            : 1.0;

        // Top hub nodes by total degree
        std::vector<std::pair<std::string, int>> degree_list;
        for (const auto& [nid, node] : g.nodes) {
            degree_list.push_back({node.label, node.degree});
        }
        std::sort(degree_list.begin(), degree_list.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        if (degree_list.size() > 5) degree_list.resize(5);
        m.hub_nodes = std::move(degree_list);

        return m;
    }

    // -------------------------------------------------------------
    // Evidentiary Path Tracing (Shortest Path via BFS)
    // -------------------------------------------------------------
    GraphPath trace_shortest_path(
        const std::string& source_id,
        const std::string& target_id,
        const std::string& project_id = "default",
        bool directed = false) const {

        GraphPath res;
        auto g = build_graph(project_id);

        if (g.nodes.find(source_id) == g.nodes.end() || g.nodes.find(target_id) == g.nodes.end()) {
            return res;
        }

        if (source_id == target_id) {
            res.found = true;
            res.length = 0;
            res.node_ids.push_back(source_id);
            res.node_types.push_back(g.nodes[source_id].type);
            res.node_labels.push_back(g.nodes[source_id].label);
            return res;
        }

        std::queue<std::string> q;
        std::map<std::string, std::pair<std::string, std::string>> parent; // curr -> (prev, edge_type)
        std::set<std::string> visited;

        q.push(source_id);
        visited.insert(source_id);

        bool reached = false;
        while (!q.empty()) {
            std::string curr = q.front();
            q.pop();

            if (curr == target_id) {
                reached = true;
                break;
            }

            const auto& adj_list = directed ? g.adj_out[curr] : g.adj_all[curr];
            for (const auto& edge : adj_list) {
                const std::string& nxt = edge.first;
                const std::string& etype = edge.second;

                if (visited.find(nxt) == visited.end()) {
                    visited.insert(nxt);
                    parent[nxt] = {curr, etype};
                    q.push(nxt);
                }
            }
        }

        if (!reached) return res;

        // Reconstruct path
        std::vector<std::string> path_nodes;
        std::vector<std::string> path_edges;
        std::string crawl = target_id;
        path_nodes.push_back(crawl);

        while (crawl != source_id) {
            auto p = parent[crawl];
            path_edges.push_back(p.second);
            crawl = p.first;
            path_nodes.push_back(crawl);
        }

        std::reverse(path_nodes.begin(), path_nodes.end());
        std::reverse(path_edges.begin(), path_edges.end());

        res.found = true;
        res.length = static_cast<int>(path_edges.size());
        res.node_ids = path_nodes;
        res.edge_types = path_edges;

        for (const auto& nid : res.node_ids) {
            res.node_types.push_back(g.nodes[nid].type);
            res.node_labels.push_back(g.nodes[nid].label);
        }

        return res;
    }

    // -------------------------------------------------------------
    // K-Hop Ego Subgraph Extraction
    // -------------------------------------------------------------
    json get_k_hop_subgraph(
        const std::string& center_id,
        int hops = 1,
        const std::string& project_id = "default") const {

        auto g = build_graph(project_id);
        json out;
        out["nodes"] = json::array();
        out["edges"] = json::array();

        if (g.nodes.find(center_id) == g.nodes.end()) return out;

        std::set<std::string> sub_nodes;
        std::queue<std::pair<std::string, int>> q;
        sub_nodes.insert(center_id);
        q.push({center_id, 0});

        while (!q.empty()) {
            auto [curr, cur_hop] = q.front();
            q.pop();

            if (cur_hop >= hops) continue;

            for (const auto& edge : g.adj_all[curr]) {
                const std::string& nxt = edge.first;
                if (sub_nodes.find(nxt) == sub_nodes.end()) {
                    sub_nodes.insert(nxt);
                    q.push({nxt, cur_hop + 1});
                }
            }
        }

        for (const auto& nid : sub_nodes) {
            const auto& n = g.nodes[nid];
            out["nodes"].push_back({
                {"id", n.id},
                {"label", n.label},
                {"type", n.type},
                {"degree", n.degree},
                {"metadata", n.metadata}
            });
        }

        for (const auto& e : g.edges) {
            if (sub_nodes.find(e.source) != sub_nodes.end() && sub_nodes.find(e.target) != sub_nodes.end()) {
                out["edges"].push_back({
                    {"id", e.id},
                    {"source", e.source},
                    {"target", e.target},
                    {"type", e.type},
                    {"label", e.label}
                });
            }
        }

        return out;
    }

    // -------------------------------------------------------------
    // Full Graph Serialization Payload for UI (ResearchGraph.jsx)
    // -------------------------------------------------------------
    json get_full_graph_data(const std::string& project_id = "default") const {
        auto g = build_graph(project_id);
        auto m = compute_metrics(project_id);

        json out;
        json nodes_arr = json::array();
        for (const auto& [nid, n] : g.nodes) {
            nodes_arr.push_back({
                {"id", n.id},
                {"label", n.label},
                {"type", n.type},
                {"degree", n.degree},
                {"in_degree", n.in_degree},
                {"out_degree", n.out_degree},
                {"metadata", n.metadata}
            });
        }

        json edges_arr = json::array();
        for (const auto& e : g.edges) {
            edges_arr.push_back({
                {"id", e.id},
                {"source", e.source},
                {"target", e.target},
                {"type", e.type},
                {"label", e.label}
            });
        }

        json hubs_arr = json::array();
        for (const auto& h : m.hub_nodes) {
            hubs_arr.push_back({{"label", h.first}, {"degree", h.second}});
        }

        out["nodes"] = nodes_arr;
        out["edges"] = edges_arr;
        out["metrics"] = {
            {"node_count", m.node_count},
            {"edge_count", m.edge_count},
            {"site_count", m.site_count},
            {"stratum_count", m.stratum_count},
            {"artifact_count", m.artifact_count},
            {"sample_count", m.sample_count},
            {"claim_count", m.claim_count},
            {"evidence_link_count", m.evidence_link_count},
            {"source_count", m.source_count},
            {"grounded_claims_count", m.grounded_claims_count},
            {"isolated_claims_count", m.isolated_claims_count},
            {"grounding_ratio", m.grounding_ratio},
            {"isolated_claim_ids", m.isolated_claim_ids},
            {"hub_authorities", hubs_arr}
        };

        return out;
    }
};

} // namespace archaeophd
