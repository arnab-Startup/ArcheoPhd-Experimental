/**
 * NOTE: The native C++ engine is now compiled directly into release/ArchaeoPhD.exe
 * via direct in-memory WebView2 IPC (desktop/src/main.cpp).
 * 
 * An HTTP server (port 8000) is NO LONGER needed or used in the desktop workstation.
 * This standalone runner is preserved for headless command-line benchmarking only.
 */

#include <iostream>
#include <string>
#include <filesystem>
#include <json.hpp>

#include "models.hpp"
#include "storage.hpp"
#include "vector_index.hpp"
#include "contradictions.hpp"
#include "thesis_audit.hpp"
#include "system_inspector.hpp"
#include "benchmark_seed.hpp"

using json = nlohmann::json;
using namespace archaeophd;

int main() {
    std::string data_dir = std::getenv("ARCHAEOPHD_DATA_DIR") ? std::getenv("ARCHAEOPHD_DATA_DIR") : "/data";
    std::cout << "[ArchaeoPhD Native C++ Engine] Initializing storage at: " << data_dir << std::endl;

    NativeStorage storage(data_dir);
    NativeContradictionEngine contradiction_engine(storage);
    NativeThesisAuditor thesis_auditor(storage, contradiction_engine);

    // Clean release: Storage initializes clean without static test data

    httplib::Server svr;

    // Enable CORS for local desktop UI
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "*"}
    });

    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // -------------------------------------------------------------
    // Health & System Hardware
    // -------------------------------------------------------------
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        json j = {
            {"status", "online"},
            {"engine", "ArchaeoPhD Native C++20 Core"},
            {"runtime", "Bare-Metal Compiled Binary (<20MB)"},
            {"zero_cloud_leakage", true}
        };
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/system/hardware", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(NativeSystemInspector::get_hardware_info().dump(), "application/json");
    });

    svr.Get("/api/system/storage", [&data_dir](const httplib::Request&, httplib::Response& res) {
        res.set_content(NativeSystemInspector::get_storage_breakdown(data_dir).dump(), "application/json");
    });

    // -------------------------------------------------------------
    // Entities API (Sites, Strata, Artifacts, Claims, Evidence, etc.)
    // -------------------------------------------------------------
    svr.Get("/api/entities/sites", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "";
        auto sites = storage.get_sites(pid);
        json j = json::array();
        for (const auto& s : sites) j.push_back(s);
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/entities/claims", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "";
        auto claims = storage.get_claims(pid);
        json j = json::array();
        for (const auto& c : claims) j.push_back(c);
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/entities/strata", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "";
        auto strata = storage.get_strata(pid);
        json j = json::array();
        for (const auto& s : strata) j.push_back(s);
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/entities/artifacts", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "";
        auto artifacts = storage.get_artifacts(pid);
        json j = json::array();
        for (const auto& a : artifacts) j.push_back(a);
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/entities/evidence", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "";
        auto ev = storage.get_evidence(pid);
        json j = json::array();
        for (const auto& e : ev) j.push_back(e);
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/entities/sources", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "";
        auto sources = storage.get_sources(pid);
        json j = json::array();
        for (const auto& s : sources) j.push_back(s);
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/entities/notes", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "";
        auto notes = storage.get_notes(pid);
        json j = json::array();
        for (const auto& n : notes) j.push_back(n);
        res.set_content(j.dump(), "application/json");
    });

    // -------------------------------------------------------------
    // Contradiction Engine (The 4 Types)
    // -------------------------------------------------------------
    svr.Get("/api/contradictions", [&contradiction_engine](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "default";
        auto conflicts = contradiction_engine.run_all(pid);
        json j = json::array();
        for (const auto& c : conflicts) j.push_back(c);
        res.set_content(j.dump(), "application/json");
    });

    // -------------------------------------------------------------
    // Pre-Submission Thesis Audit & Defense-Proofing
    // -------------------------------------------------------------
    svr.Get("/api/thesis/audit", [&thesis_auditor](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "default";
        res.set_content(thesis_auditor.run_audit(pid).dump(), "application/json");
    });

    // -------------------------------------------------------------
    // Benchmark Seeder
    // -------------------------------------------------------------
    svr.Post("/api/benchmark/load-demo", [&storage](const httplib::Request& req, httplib::Response& res) {
        std::string pid = req.has_param("project_id") ? req.get_param_value("project_id") : "default";
        seed_benchmark_corpus(storage, pid);
        json j = {{"status", "success"}, {"message", "Benchmark corpus loaded."}};
        res.set_content(j.dump(), "application/json");
    });

    std::cout << "[ArchaeoPhD Native C++ Engine] Listening on http://0.0.0.0:8000" << std::endl;
    svr.listen("0.0.0.0", 8000);

    return 0;
}
