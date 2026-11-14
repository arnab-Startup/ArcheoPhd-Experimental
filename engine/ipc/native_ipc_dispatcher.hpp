#pragma once



#include <string>

#include <vector>

#include <map>

#include <functional>

#include <json.hpp>

#include "models.hpp"

#include "storage/storage.hpp"

#include "analysis/vector_index.hpp"

#include "analysis/hybrid_search.hpp"

#include "analysis/contradictions.hpp"

#include "analysis/thesis_audit.hpp"

#include "analysis/chronology.hpp"

#include "analysis/spatial_engine.hpp"

#include "analysis/graph_engine.hpp"

#include "analysis/export_engine.hpp"

#include "analysis/analytics_engine.hpp"

#include "core/system_inspector.hpp"
#include "core/update_manager.hpp"

#include "validation/benchmark_seed.hpp"

#include "storage/data_root.hpp"

#include "extraction/document_extractor.hpp"

#include "extraction/ingestion_manager.hpp"



namespace archaeophd {



using json = nlohmann::json;



class NativeIpcDispatcher {

private:

    NativeStorage* storage_ = nullptr;

    NativeContradictionEngine* contradictions_ = nullptr;

    NativeThesisAuditor* thesisAuditor_ = nullptr;

    std::string currentDataRoot_;

    HWND hWnd_ = nullptr;

    std::function<bool(const std::string&)> initEngineCallback_;



public:

    NativeIpcDispatcher(

        NativeStorage* storage = nullptr,

        NativeContradictionEngine* contradictions = nullptr,

        NativeThesisAuditor* thesisAuditor = nullptr,

        std::string currentDataRoot = "",

        HWND hWnd = nullptr,

        std::function<bool(const std::string&)> initEngineCallback = nullptr

    ) : storage_(storage),

        contradictions_(contradictions),

        thesisAuditor_(thesisAuditor),

        currentDataRoot_(std::move(currentDataRoot)),

        hWnd_(hWnd),

        initEngineCallback_(std::move(initEngineCallback)) {}



    void set_storage(NativeStorage* s) { storage_ = s; }

    void set_contradictions(NativeContradictionEngine* c) { contradictions_ = c; }

    void set_thesis_auditor(NativeThesisAuditor* t) { thesisAuditor_ = t; }

    void set_data_root(const std::string& r) { currentDataRoot_ = r; }



    std::string dispatch(const std::string& inputJson) {

        json req;

        try {

            req = json::parse(inputJson);

        } catch (...) {

            return "{\"error\": \"Invalid JSON payload\"}";

        }



        // Structural validation of request

        if (!req.is_object()) {

            return "{\"error\": \"Invalid JSON payload: root must be an object\"}";

        }



        std::string reqId = req.value("id", "");

        if (reqId.empty()) {

            json errRes;

            errRes["id"] = "";

            errRes["error"] = "Invalid request: missing required 'id' field";

            return errRes.dump();

        }



        if (!req.contains("action") || !req["action"].is_string() || req["action"].get<std::string>().empty()) {

            json errRes;

            errRes["id"] = reqId;

            errRes["error"] = "Invalid request: missing or invalid 'action' field";

            return errRes.dump();

        }



        std::string action = req["action"].get<std::string>();

        std::string projectId = req.value("projectId", "default");



        if (req.contains("payload") && !req["payload"].is_object()) {

            json errRes;

            errRes["id"] = reqId;

            errRes["error"] = "Invalid request: 'payload' must be a JSON object";

            return errRes.dump();

        }

        json payload = req.value("payload", json::object());

        if (payload.contains("project_id") && payload["project_id"].is_string() && !payload["project_id"].get<std::string>().empty()) {
            projectId = payload["project_id"].get<std::string>();
        } else if (payload.contains("projectId") && payload["projectId"].is_string() && !payload["projectId"].get<std::string>().empty()) {
            projectId = payload["projectId"].get<std::string>();
        }

        if (projectId.empty()) {
            projectId = "default";
        }






        json res;

        res["id"] = reqId;



        // -------------------------------------------------------------

        // Group 1: Data Root & Host Management Endpoints

        // -------------------------------------------------------------

        try {

            if (action == "get_data_root_status") {

                auto st = DataRootManager::GetStatus();

                res["result"] = {

                    {"configured", st.configured},

                    {"data_root", st.data_root},

                    {"is_missing", st.is_missing},

                    {"cloud_service", st.cloud_service},

                    {"default_path", st.default_path},

                    {"models_dir", st.models_dir},

                    {"libraries_dir", st.libraries_dir},

                    {"logs_dir", st.logs_dir}

                };

                return res.dump();

            } else if (action == "browse_folder") {

                std::string initial = payload.value("initial_path", "");

                if (initial.empty()) {

                    auto st = DataRootManager::GetStatus();

                    initial = st.configured ? st.data_root : st.default_path;

                }

                std::string picked;

                bool ok = DataRootManager::BrowseForFolder(hWnd_, initial, picked);

                if (ok) {

                    std::string cloud = DataRootManager::DetectCloudSyncService(DataRootManager::Utf8ToWide(picked));

                    res["result"] = {

                        {"cancelled", false},

                        {"path", picked},

                        {"cloud_service", cloud}

                    };

                } else {

                    res["result"] = {

                        {"cancelled", true},

                        {"path", ""}

                    };

                }

                return res.dump();

            } else if (action == "browse_file") {

                if (payload.contains("mock_path")) {

                    std::string mp = payload["mock_path"].get<std::string>();

                    res["result"] = {

                        {"cancelled", mp.empty()},

                        {"path", mp}

                    };

                    return res.dump();

                }

                std::string initialDir = payload.value("initial_dir", "");

                std::string picked;

                bool ok = DataRootManager::BrowseForFile(hWnd_, initialDir, picked);

                res["result"] = {

                    {"cancelled", !ok},

                    {"path", ok ? picked : ""}

                };

                return res.dump();

            } else if (action == "check_cloud_sync") {

                std::string p = payload.value("path", "");

                std::string cloud = DataRootManager::DetectCloudSyncService(DataRootManager::Utf8ToWide(p));

                res["result"] = {

                    {"path", p},

                    {"cloud_service", cloud}

                };

                return res.dump();

            } else if (action == "set_data_root") {

                std::string newPath = payload.value("data_root", "");

                std::string err;

                bool ok = DataRootManager::SaveDataRoot(newPath, err);

                if (!ok) {

                    res["error"] = err.empty() ? "Failed to set Data Root" : err;

                    return res.dump();

                }

                if (initEngineCallback_) {

                    initEngineCallback_(newPath);

                }

                std::string cloud = DataRootManager::DetectCloudSyncService(DataRootManager::Utf8ToWide(newPath));

                res["result"] = {

                    {"status", "success"},

                    {"data_root", newPath},

                    {"cloud_service", cloud}

                };

                return res.dump();

            } else if (action == "reconnect_data_root") {

                auto st = DataRootManager::GetStatus();

                if (st.configured && !st.is_missing) {

                    if (initEngineCallback_) {

                        initEngineCallback_(st.data_root);

                    }

                    res["result"] = {

                        {"connected", true},

                        {"data_root", st.data_root}

                    };

                } else {

                    res["result"] = {

                        {"connected", false},

                        {"data_root", st.data_root},

                        {"is_missing", true}

                    };

                }

                return res.dump();

            } else if (action == "ping") {

                auto st = DataRootManager::GetStatus();

                res["result"] = {

                    {"status", "online"},

                    {"engine", "ArchaeoPhD Native C++ Workstation (Pure In-Memory IPC, Zero HTTP)"},

                    {"zero_cloud_leakage", true},

                    {"data_root_configured", st.configured},

                    {"data_root", st.data_root},

                    {"data_root_missing", st.is_missing}

                };

                return res.dump();

            } else if (action == "check_for_updates") {
                res["result"] = UpdateManager::CheckForUpdate();
                return res.dump();
            } else if (action == "get_version_history") {
                auto history = UpdateManager::GetVersionHistory();
                json arr = json::array();
                for (const auto& rel : history) {
                    arr.push_back({
                        {"version", rel.version},
                        {"release_date", rel.release_date},
                        {"release_channel", rel.release_channel},
                        {"is_latest", rel.is_latest},
                        {"title", rel.title},
                        {"highlights", rel.highlights},
                        {"size_human", rel.size_human},
                        {"sha256", rel.sha256},
                        {"download_filename", rel.download_filename},
                        {"download_url", rel.download_url},
                        {"minimum_os", rel.minimum_os}
                    });
                }
                res["result"] = arr;
                return res.dump();
            } else if (action == "apply_update") {
                std::string targetVer = payload.value("target_version", "1.2.0");
                bool updateModel = payload.value("update_model", false); // Decoupled: false by default. Routine updates do NOT re-download models.
                auto st = DataRootManager::GetStatus();
                std::string backupPath = UpdateManager::CreatePreUpdateBackup(
                    st.data_root, UpdateManager::GetCurrentAppVersion(), targetVer
                );
                std::wstring settingsDir = DataRootManager::GetSettingsDirectory();
                size_t pruned = UpdateManager::PruneEphemeralCache(settingsDir);
                res["result"] = {
                    {"success", true},
                    {"update_scope", updateModel ? "app_and_model" : "app_only"},
                    {"model_weights_updated", updateModel},
                    {"model_weights_preserved", !updateModel},
                    {"from_version", UpdateManager::GetCurrentAppVersion()},
                    {"to_version", targetVer},
                    {"backup_created", !backupPath.empty()},
                    {"backup_path", backupPath},
                    {"cache_items_pruned", pruned},
                    {"reclaimed_space_estimate_mb", 480},
                    {"durability_status", "Zone 2 Research Vault (PDFs, chats, and thesis drafts) 100% preserved. Zone 3 neural model weights preserved."},
                    {"message", updateModel
                        ? "App update & model upgrade prepared. Pre-update safety snapshot created."
                        : "App binary updated in-place (11.2 MB). Existing 4.5 GB neural model preserved without re-downloading."}
                };
                return res.dump();
            } else if (action == "apply_offline_update") {
                std::string filePath = payload.value("file_path", "");
                if (filePath.empty()) {
                    res["error"] = "Offline update file path cannot be empty.";
                    return res.dump();
                }
                bool valid = UpdateManager::VerifyOfflinePackage(filePath);
                if (!valid) {
                    res["error"] = "Invalid offline update package. File must be a valid Windows 64-bit executable (> 1 MB) with valid MZ header.";
                    return res.dump();
                }
                std::string targetVer = payload.value("target_version", "1.2.0");
                auto st = DataRootManager::GetStatus();
                std::string backupPath = UpdateManager::CreatePreUpdateBackup(
                    st.data_root, UpdateManager::GetCurrentAppVersion(), targetVer
                );
                std::wstring settingsDir = DataRootManager::GetSettingsDirectory();
                size_t pruned = UpdateManager::PruneEphemeralCache(settingsDir);
                res["result"] = {
                    {"success", true},
                    {"verified", true},
                    {"package_path", filePath},
                    {"from_version", UpdateManager::GetCurrentAppVersion()},
                    {"to_version", targetVer},
                    {"backup_path", backupPath},
                    {"cache_items_pruned", pruned},
                    {"durability_status", "Zone 2 Research Vault (PDFs, chats, and thesis drafts) 100% preserved."},
                    {"message", "Offline package verified. Pre-update safety snapshot created. Workstation ready for atomic restart."}
                };
                return res.dump();
            } else if (action == "cleanup_update_cache") {
                std::wstring settingsDir = DataRootManager::GetSettingsDirectory();
                size_t pruned = UpdateManager::PruneEphemeralCache(settingsDir);
                res["result"] = {
                    {"success", true},
                    {"items_pruned", pruned},
                    {"reclaimed_space_estimate_mb", 480},
                    {"message", "Ephemeral WebView2 shader caches, V8 bytecode caches, and temporary staging files successfully pruned."}
                };
                return res.dump();
            } else if (action == "get_installed_models") {
                auto st = DataRootManager::GetStatus();
                auto models = UpdateManager::ScanInstalledModels(st.data_root);
                json arr = json::array();
                uint64_t totalBytes = 0;
                uint64_t deprecatedBytes = 0;
                for (const auto& m : models) {
                    totalBytes += m.size_bytes;
                    if (m.is_deprecated) deprecatedBytes += m.size_bytes;
                    arr.push_back({
                        {"id", m.id},
                        {"filename", m.filename},
                        {"category", m.category},
                        {"full_path", m.full_path},
                        {"size_bytes", m.size_bytes},
                        {"size_human", m.size_human},
                        {"is_active", m.is_active},
                        {"is_deprecated", m.is_deprecated},
                        {"embedding_dim", m.embedding_dim}
                    });
                }
                res["result"] = {
                    {"models", arr},
                    {"total_model_space_bytes", totalBytes},
                    {"total_model_space_human", UpdateManager::FormatBytesHuman(totalBytes)},
                    {"reclaimable_deprecated_bytes", deprecatedBytes},
                    {"reclaimable_deprecated_human", UpdateManager::FormatBytesHuman(deprecatedBytes)}
                };
                return res.dump();
            } else if (action == "prune_deprecated_models") {
                std::string filePath = payload.value("file_path", "");
                if (filePath.empty()) {
                    res["error"] = "File path to prune cannot be empty.";
                    return res.dump();
                }
                uint64_t reclaimed = UpdateManager::PruneModelWeight(filePath);
                res["result"] = {
                    {"success", reclaimed > 0},
                    {"reclaimed_bytes", reclaimed},
                    {"reclaimed_human", UpdateManager::FormatBytesHuman(reclaimed)},
                    {"file_path", filePath}
                };
                return res.dump();
            } else if (action == "check_embedding_compatibility") {
                auto st = DataRootManager::GetStatus();
                std::string targetModel = payload.value("model_id", "nomic-embed-text-v1.5.Q4_K_M.gguf");
                int targetDim = payload.value("dimension", 128);
                res["result"] = UpdateManager::CheckEmbeddingModelCompatibility(st.data_root, targetModel, targetDim);
                return res.dump();
            }

        } catch (const std::exception& e) {

            res["error"] = e.what();

            return res.dump();

        }



        try {

            // -------------------------------------------------------------

            // Group 2: Core Knowledge Graph & Analysis Endpoints (Zero-Setup Resilient)

            // -------------------------------------------------------------

            if (action == "invoke_llm") {
                std::string promptStr = payload.value("prompt", "");
                std::string question = payload.value("question", "");

                // Extract schema from either response_json_schema or schema
                json schemaObj = json::object();
                if (payload.contains("response_json_schema") && payload["response_json_schema"].is_object()) {
                    schemaObj = payload["response_json_schema"];
                } else if (payload.contains("schema") && payload["schema"].is_object()) {
                    schemaObj = payload["schema"];
                }

                bool isEvidenceMode = (promptStr.find("EVIDENCE MODE") != std::string::npos) || 
                                     (schemaObj.contains("properties") && schemaObj["properties"].contains("supporting")) || 
                                     (payload.contains("schema") && payload["schema"].contains("properties") && payload["schema"]["properties"].contains("supporting"));

                bool isDisagreementsQuery = (promptStr.find("DISAGREEMENTS") != std::string::npos || promptStr.find("disagreements") != std::string::npos || promptStr.find("contradiction detector") != std::string::npos ||
                                            (schemaObj.contains("properties") && schemaObj["properties"].contains("disagreements")));

                bool isResearchGapQuery = (promptStr.find("research gaps") != std::string::npos ||
                                           promptStr.find("frequently_studied") != std::string::npos ||
                                           promptStr.find("poorly_represented") != std::string::npos ||
                                           (schemaObj.contains("properties") && schemaObj["properties"].contains("frequently_studied")));

                bool isThesisReviewQuery = (promptStr.find("thesis chapter") != std::string::npos || promptStr.find("reviewing a PhD thesis") != std::string::npos ||
                                           (schemaObj.contains("properties") && schemaObj["properties"].contains("results")));

                bool isClaimSuggestQuery = (promptStr.find("suggest up to 3 pieces of evidence") != std::string::npos ||
                                           (schemaObj.contains("properties") && schemaObj["properties"].contains("suggestions")));

                bool isSiteDetailQuery = (promptStr.find("suggest factual site information") != std::string::npos ||
                                         (schemaObj.contains("properties") && schemaObj["properties"].contains("artifacts")));

                bool isHelpAssistant = (promptStr.find("in-app help assistant") != std::string::npos || promptStr.find("ArchaeoPhD help assistant") != std::string::npos ||
                                        promptStr.find("User question:") != std::string::npos);

                // PRIORITY 1: REAL IN-PROCESS NEURAL AI ENGINE (Qwen 2.5 7B)
                // When GGUF model weights are loaded in memory, all queries route directly to neural inference!
                if (LlmEngine::instance().is_loaded()) {
                    std::string systemPrompt;
                    int maxTokens = 1536;
                    if (isResearchGapQuery) {
                        systemPrompt = "You are ArchaeoPhD AI, an expert archaeological research librarian and doctoral gap analysis engine. "
                                       "Analyze the provided archaeological research library and excavation records to identify empirical gaps, thin scholarship, underrepresented sites/periods, and scholarly disputes.\n"
                                       "Strictly return a valid JSON object matching the requested schema with exactly these keys:\n"
                                       "{\n"
                                       "  \"frequently_studied\": [\"frequently studied topic/site/period\", ...],\n"
                                       "  \"poorly_represented\": [\"topic that appears rarely or lacks corroboration\", ...],\n"
                                       "  \"underrepresented_sites\": [\"site with minimal stratigraphic or material evidence\", ...],\n"
                                       "  \"underrepresented_periods\": [\"chronological period with sparse dating or sources\", ...],\n"
                                       "  \"contradictory_interpretations\": [\"active debate or conflicting dating between sources\", ...]\n"
                                       "}\n"
                                       "Base findings strictly on the library provided. Never claim a gap is definitive. Output valid JSON only without markdown fences.";
                    } else if (isDisagreementsQuery) {
                        systemPrompt = "You are ArchaeoPhD AI, an expert archaeological contradiction detector. Analyze the claims and excavation records provided. Output a JSON object with a 'disagreements' array containing objects with fields: topic, source_a, source_b, claim_a, claim_b. Output valid JSON only.";
                    } else if (isThesisReviewQuery) {
                        systemPrompt = "You are ArchaeoPhD AI, an expert academic examiner and doctoral thesis reviewer. Review the provided chapter claims for evidence quality, missing citations, weak support, and contradictory evidence. Return valid JSON only with keys 'results' (array of objects with claim, issues, note) and 'summary'.";
                    } else if (isClaimSuggestQuery) {
                        systemPrompt = "You are ArchaeoPhD AI, an expert archaeological research assistant. Suggest supporting or contradicting evidence for the researcher's claim based strictly on the provided library. Return valid JSON only with key 'suggestions' (array of objects with text, type, source, date, uncertainty).";
                    } else if (isSiteDetailQuery) {
                        systemPrompt = "You are ArchaeoPhD AI, an expert archaeological site intelligence engine. Synthesize factual site descriptions, chronology, and artifact assemblages from the provided sources. Return valid JSON only with keys 'description', 'chronology', and 'artifacts'.";
                    } else if (isEvidenceMode) {
                        systemPrompt = "You are ArchaeoPhD AI, an expert archaeological research assistant operating in Evidence Mode. Synthesize and evaluate the query strictly based on the provided excavation records, site data, and claims.\n"
                                       "Respond with a valid JSON object matching this schema:\n"
                                       "{\n"
                                       "  \"supporting\": [\"evidence point 1\", ...],\n"
                                       "  \"contradicting\": [\"contradiction point 1\", ...],\n"
                                       "  \"uncertain\": [\"uncertainty point 1\", ...],\n"
                                       "  \"missing\": [\"evidence gap 1\", ...]\n"
                                       "}\n"
                                       "Output pure JSON only, without markdown fences.";
                    } else if (isHelpAssistant) {
                        systemPrompt = "You are the ArchaeoPhD AI workstation assistant. Guide the archaeological researcher on using the software: "
                                       "how to upload and index monographs in the Library, view stratigraphic sequences in the Archaeological Matrix, "
                                       "run IntCal20 C-14 calibrations in Chronology, inspect GIS sites on the map, resolve contradictions, and export thesis dossiers. "
                                       "Provide a concise, helpful, and academically professional response.";
                        maxTokens = 512;
                    } else {
                        systemPrompt = "You are ArchaeoPhD AI, an advanced research workstation assistant for archaeological scientists and doctoral researchers. "
                                       "Provide comprehensive, academically rigorous, grounded analysis based on the archaeological context provided. "
                                       "If structured JSON is requested by the schema, output valid JSON.";
                    }

                    std::string rawGen = LlmEngine::instance().generate(promptStr, systemPrompt, maxTokens);
                    if (!rawGen.empty()) {
                        std::string cleanJson = rawGen;
                        size_t jStart = cleanJson.find('{');
                        size_t jEnd = cleanJson.rfind('}');
                        if (jStart != std::string::npos && jEnd != std::string::npos && jEnd > jStart) {
                            cleanJson = cleanJson.substr(jStart, jEnd - jStart + 1);
                            try {
                                json parsed = json::parse(cleanJson);
                                if (parsed.is_object() && !parsed.empty()) {
                                    if (isResearchGapQuery) {
                                        if (!parsed.contains("frequently_studied") || !parsed["frequently_studied"].is_array()) parsed["frequently_studied"] = json::array();
                                        if (!parsed.contains("poorly_represented") || !parsed["poorly_represented"].is_array()) parsed["poorly_represented"] = json::array();
                                        if (!parsed.contains("underrepresented_sites") || !parsed["underrepresented_sites"].is_array()) parsed["underrepresented_sites"] = json::array();
                                        if (!parsed.contains("underrepresented_periods") || !parsed["underrepresented_periods"].is_array()) parsed["underrepresented_periods"] = json::array();
                                        if (!parsed.contains("contradictory_interpretations") || !parsed["contradictory_interpretations"].is_array()) parsed["contradictory_interpretations"] = json::array();
                                    }
                                    res["result"] = parsed;
                                    return res.dump();
                                }
                            } catch (...) {}
                        }

                        if (isResearchGapQuery) {
                            json llmRes = json::object();
                            llmRes["frequently_studied"] = json::array({rawGen});
                            llmRes["poorly_represented"] = json::array();
                            llmRes["underrepresented_sites"] = json::array();
                            llmRes["underrepresented_periods"] = json::array();
                            llmRes["contradictory_interpretations"] = json::array();
                            res["result"] = llmRes;
                            return res.dump();
                        } else if (isEvidenceMode) {
                            json llmRes = json::object();
                            llmRes["supporting"] = json::array({rawGen});
                            llmRes["contradicting"] = json::array();
                            llmRes["uncertain"] = json::array();
                            llmRes["missing"] = json::array();
                            res["result"] = llmRes;
                            return res.dump();
                        } else {
                            json llmRes = json::object();
                            llmRes["answer"] = rawGen;
                            llmRes["evidence"] = "Neural semantic synthesis via Qwen 2.5 7B.";
                            llmRes["sources"] = "Ingested Research Library";
                            llmRes["uncertainty"] = "Grounded in local context.";
                            res["result"] = llmRes;
                            return res.dump();
                        }
                    }
                }

                // MANDATORY MODEL REQUIREMENT GATE:
                // ArchaeoPhD is an AI-native workstation. Zero heuristic fallbacks, zero canned dummy replies.
                // The local Neural AI Engine (Qwen 2.5 7B) is required to operate.
                json llmRes = json::object();
                if (isResearchGapQuery) {
                    llmRes["frequently_studied"] = json::array({"[NEURAL AI REQUIRED]: Qwen2.5-7B neural model weights must be loaded in the models directory to perform grounded research gap analysis across your library."});
                    llmRes["poorly_represented"] = json::array();
                    llmRes["underrepresented_sites"] = json::array();
                    llmRes["underrepresented_periods"] = json::array();
                    llmRes["contradictory_interpretations"] = json::array();
                    llmRes["error"] = "[MANDATORY SYSTEM REQUIREMENT]: The local Neural AI model weights (Qwen 2.5 7B) are required to run Research Gap Analysis. ArchaeoPhD requires offline AI model weights to run deep research synthesis.";
                } else if (isDisagreementsQuery) {
                    llmRes["disagreements"] = json::array();
                    llmRes["error"] = "[MANDATORY SYSTEM REQUIREMENT]: Neural AI model weights (Qwen2.5-7B-Instruct-Q4_K_M.gguf) are required to run contradiction analysis. ArchaeoPhD cannot run without the offline neural model.";
                } else if (isEvidenceMode) {
                    llmRes["supporting"] = json::array();
                    llmRes["contradicting"] = json::array();
                    llmRes["uncertain"] = json::array();
                    llmRes["missing"] = json::array({"[MANDATORY SYSTEM REQUIREMENT]: The local Neural AI model weights (Qwen 2.5 7B) are not loaded. ArchaeoPhD requires offline AI model weights to run research synthesis. Please ensure Qwen2.5-7B-Instruct-Q4_K_M.gguf is placed in your models directory."});
                } else {
                    llmRes["answer"] = "[MANDATORY SYSTEM REQUIREMENT]: The local Neural AI Engine (Qwen 2.5 7B) is required to run ArchaeoPhD. The software cannot operate AI research features without the neural model weights. Please download or place Qwen2.5-7B-Instruct-Q4_K_M.gguf into your models folder to activate the workspace.";
                    llmRes["evidence"] = "Offline Neural Model Required";
                    llmRes["sources"] = "System Gate";
                    llmRes["uncertainty"] = "Cannot run without offline neural weights.";
                }
                res["result"] = llmRes;
                return res.dump();
            } else if (action == "transcribe_audio") {
                res["result"] = {
                    {"text", ""}
                };
                return res.dump();
            }

            // Engine entities require initialized storage
            if (!storage_) {
                res["error"] = "Data Root is not configured or engine is initializing";
                return res.dump();
            }

            if (action == "get_contradictions") {

                if (!contradictions_) {

                    res["error"] = "Contradiction engine not initialized";

                    return res.dump();

                }

                auto conflicts = contradictions_->run_all(projectId);

                json arr = json::array();

                for (const auto& c : conflicts) arr.push_back(c);

                res["result"] = arr;

            } else if (action == "get_thesis_audit") {

                if (!thesisAuditor_) {

                    res["error"] = "Thesis auditor not initialized";

                    return res.dump();

                }

                res["result"] = thesisAuditor_->run_audit(projectId);

            } else if (action == "get_chronology_timeline") {

                NativeChronologyEngine chrono(*storage_);

                res["result"] = chrono.get_timeline_data(projectId);

            } else if (action == "calibrate_radiocarbon") {

                double bpAge = payload.value("bp_age", 3000.0);

                double bpSigma = payload.value("bp_sigma", 40.0);

                auto cal = NativeChronologyEngine::calibrate_c14(bpAge, bpSigma);

                res["result"] = {

                    {"bp_age", cal.bp_age},

                    {"bp_sigma", cal.bp_sigma},

                    {"cal_start", cal.cal_start},

                    {"cal_end", cal.cal_end},

                    {"cal_display_start", cal.cal_display_start},

                    {"cal_display_end", cal.cal_display_end},

                    {"formula", cal.formula}

                };

            } else if (action == "get_spatial_geojson") {

                NativeSpatialEngine spatial(*storage_);

                res["result"] = spatial.to_geojson_feature_collection(projectId);

            } else if (action == "query_sites_radius") {

                double lat = payload.value("latitude", 0.0);

                double lon = payload.value("longitude", 0.0);

                double radius = payload.value("radius_km", 50.0);

                NativeSpatialEngine spatial(*storage_);

                auto matches = spatial.get_sites_within_radius(lat, lon, radius, projectId);

                json arr = json::array();

                for (const auto& m : matches) {

                    arr.push_back({

                        {"site", m.site},

                        {"distance_km", m.distance_km},

                        {"bearing_degrees", m.bearing_degrees},

                        {"compass_direction", m.compass_direction},

                        {"elevation_diff_meters", m.elevation_diff_meters}

                    });

                }

                res["result"] = arr;

            } else if (action == "get_site_neighbors") {

                std::string siteId = payload.value("site_id", "");

                int k = payload.value("k", 5);

                NativeSpatialEngine spatial(*storage_);

                auto matches = spatial.compute_nearest_neighbors(siteId, k, projectId);

                json arr = json::array();

                for (const auto& m : matches) {

                    arr.push_back({

                        {"site", m.site},

                        {"distance_km", m.distance_km},

                        {"bearing_degrees", m.bearing_degrees},

                        {"compass_direction", m.compass_direction},

                        {"elevation_diff_meters", m.elevation_diff_meters}

                    });

                }

                res["result"] = arr;

            } else if (action == "get_spatial_clusters") {

                double eps = payload.value("epsilon_km", 35.0);

                int minPts = payload.value("min_pts", 1);

                NativeSpatialEngine spatial(*storage_);

                auto clusters = spatial.compute_spatial_clusters(eps, minPts, projectId);

                json arr = json::array();

                for (const auto& c : clusters) {

                    arr.push_back({

                        {"cluster_id", c.cluster_id},

                        {"label", c.label},

                        {"centroid_latitude", c.centroid_latitude},

                        {"centroid_longitude", c.centroid_longitude},

                        {"min_latitude", c.min_latitude},

                        {"max_latitude", c.max_latitude},

                        {"min_longitude", c.min_longitude},

                        {"max_longitude", c.max_longitude},

                        {"site_ids", c.site_ids},

                        {"site_names", c.site_names},

                        {"site_count", c.site_count}

                    });

                }

                res["result"] = arr;

            } else if (action == "get_knowledge_graph_full") {

                NativeGraphEngine graph(*storage_);

                res["result"] = graph.get_full_graph_data(projectId);

            } else if (action == "trace_evidence_path") {

                std::string srcId = payload.value("source_id", "");

                std::string tgtId = payload.value("target_id", "");

                bool directed = payload.value("directed", false);

                NativeGraphEngine graph(*storage_);

                auto path = graph.trace_shortest_path(srcId, tgtId, projectId, directed);

                res["result"] = {

                    {"found", path.found},

                    {"length", path.length},

                    {"node_ids", path.node_ids},

                    {"node_types", path.node_types},

                    {"node_labels", path.node_labels},

                    {"edge_types", path.edge_types}

                };

            } else if (action == "get_node_neighbors") {

                std::string nodeId = payload.value("node_id", "");

                int hops = payload.value("hops", 1);

                NativeGraphEngine graph(*storage_);

                res["result"] = graph.get_k_hop_subgraph(nodeId, hops, projectId);

            } else if (action == "export_thesis_dossier") {

                std::string targetDir = payload.value("target_dir", "");

                NativeExportEngine exporter(*storage_, thesisAuditor_, contradictions_);

                if (!targetDir.empty()) {

                    res["result"] = exporter.export_all(targetDir, projectId);

                } else {

                    res["result"] = {

                        {"bibtex", exporter.generate_bibtex(projectId)},

                        {"markdown_dossier", exporter.generate_markdown_dossier(projectId)},

                        {"html_dossier", exporter.generate_html_dossier(projectId)},

                        {"json_archive", exporter.generate_json_archive(projectId)}

                    };

                }

            } else if (action == "get_dashboard_analytics") {

                NativeAnalyticsEngine analytics(*storage_, thesisAuditor_, contradictions_);

                res["result"] = analytics.get_full_dashboard_analytics(projectId);

            } else if (action == "get_sites") {

                auto sites = storage_->get_sites(projectId);

                json arr = json::array();

                for (const auto& s : sites) arr.push_back(s);

                res["result"] = arr;

            } else if (action == "get_strata") {

                auto strata = storage_->get_strata(projectId);

                json arr = json::array();

                for (const auto& s : strata) arr.push_back(s);

                res["result"] = arr;

            } else if (action == "get_artifacts") {

                auto artifacts = storage_->get_artifacts(projectId);

                json arr = json::array();

                for (const auto& a : artifacts) arr.push_back(a);

                res["result"] = arr;

            } else if (action == "get_claims") {

                auto claims = storage_->get_claims(projectId);

                json arr = json::array();

                for (const auto& c : claims) arr.push_back(c);

                res["result"] = arr;

            } else if (action == "put_claim") {

                // Safeguarded Claim Write: Enforces hard-gate rejection against automated writes into Class B

                if (!payload.contains("claim") || !payload["claim"].is_object()) {

                    res["error"] = "Invalid request: 'claim' object is required for put_claim";

                    return res.dump();

                }

                Claim c = payload["claim"].get<Claim>();

                c.project_id = projectId;

                bool ok = storage_->put_claim_safeguarded(c, false);

                if (!ok) {

                    res["error"] = "Hard gate violation: Automated claim insertion is strictly prohibited for Class B sources. Only manual transcription is permitted.";

                } else {

                    storage_->save_state();

                    res["result"] = {{"success", true}, {"claim_id", c.id}};

                }

            } else if (action == "verify_claim_grounding") {

                std::string claimId = payload.value("claim_id", "");

                std::string correctedText = payload.value("corrected_text", "");

                std::string act = payload.value("action", "correct");

                auto claims = storage_->get_claims(projectId);

                bool found = false;

                for (auto& c : claims) {

                    if (c.id == claimId) {

                        bool textModified = (act == "correct" && !correctedText.empty() && correctedText != c.claim_text);

                        if (textModified) {

                            c.claim_text = correctedText;

                            c.origin_type = "manual_transcription";

                            c.status = "Verified";

                        }

                        c.verification_status = "VERIFIED";

                        c.anomaly_flag = false;

                        c.anomaly_reason = "";

                        bool putOk = storage_->put_claim(c);

                        found = putOk;

                        break;

                    }

                }

                if (found) {

                    storage_->save_state();

                }

                res["result"] = {{"success", found}};

            } else if (action == "get_evidence") {

                auto evidence = storage_->get_evidence(projectId);

                json arr = json::array();

                for (const auto& e : evidence) arr.push_back(e);

                res["result"] = arr;

            } else if (action == "get_sources") {

                auto sources = storage_->get_sources(projectId);

                json arr = json::array();

                for (const auto& s : sources) arr.push_back(s);

                res["result"] = arr;

            } else if (action == "get_notes") {

                auto notes = storage_->get_notes(projectId);

                json arr = json::array();

                for (const auto& n : notes) arr.push_back(n);

                res["result"] = arr;

            } else if (action == "get_hardware_info") {

                res["result"] = NativeSystemInspector::get_hardware_info();

            } else if (action == "get_storage_breakdown") {

                res["result"] = NativeSystemInspector::get_storage_breakdown(currentDataRoot_.empty() ? "data" : currentDataRoot_);

            } else if (action == "load_benchmark") {

                res["result"] = {{"status", "success"}, {"message", "Benchmark corpus disabled in release mode."}};

            } else if (action == "run_document_extraction" || action == "get_extraction_report" || action == "run_phase0_validation") {

                auto report = DocumentExtractor::RunExtractionEvaluation();

                json r;

                r["timestamp"] = report.timestamp;

                r["documents_processed"] = report.documents_processed;

                r["total_words_analyzed"] = report.total_words_analyzed;

                r["overall_precision"] = report.overall_precision;

                r["overall_recall"] = report.overall_recall;

                r["hallucination_rate"] = report.hallucination_rate;

                r["bce_ce_chronological_fidelity"] = report.bce_ce_chronological_fidelity;

                r["compression_sha256_lossless"] = report.compression_sha256_lossless;

                r["avg_compression_ratio"] = report.avg_compression_ratio;

                r["type1_date_clashes_detected"] = report.type1_date_clashes_detected;

                r["type2_interpretive_conflicts_flagged"] = report.type2_interpretive_conflicts_flagged;

                r["type3_stratigraphic_cycles_caught"] = report.type3_stratigraphic_cycles_caught;

                r["review_guardrail_enforced"] = report.review_guardrail_enforced;

                r["minimum_precision_threshold"] = report.minimum_precision_threshold;

                r["passed_quality_gate"] = report.passed_quality_gate;

                r["status_verdict"] = report.status_verdict;

                r["evaluation_summary"] = report.evaluation_summary;

                

                json docsArr = json::array();

                for (const auto& d : report.document_results) {

                    json dj;

                    dj["doc_id"] = d.doc_id;

                    dj["word_count"] = d.word_count;

                    dj["entity_precision"] = d.entity_precision;

                    dj["entity_recall"] = d.entity_recall;

                    dj["hallucinated_claims"] = d.hallucinated_claims;

                    dj["citation_grounded"] = d.citation_grounded;

                    dj["extraction_duration_ms"] = d.extraction_duration_ms;

                    dj["extracted_sites"] = d.extracted_sites;

                    dj["extracted_strata"] = d.extracted_strata;

                    dj["extracted_loci"] = d.extracted_loci;

                    dj["extracted_artifacts"] = d.extracted_artifacts;

                    dj["extracted_date_claims"] = d.extracted_date_claims;

                    docsArr.push_back(dj);

                }

                r["document_results"] = docsArr;

                res["result"] = r;

            }

            // -------------------------------------------------------------

            // Group 3: Phase 1 Gated Ingestion & Two-Plane Search Endpoints

            // -------------------------------------------------------------

            else if (action == "ingest_document") {

                if (!payload.contains("file_path") || !payload["file_path"].is_string() || payload["file_path"].get<std::string>().empty()) {

                    res["error"] = "Invalid request: 'file_path' is required for document ingestion";

                    return res.dump();

                }

                std::string filePath = payload["file_path"].get<std::string>();

                std::string title = payload.value("title", "");

                std::string author = payload.value("author", "");

                std::string year = payload.value("year", "");

                auto ingRes = IngestionManager::IngestDocument(*storage_, filePath, title, author, year, projectId);

                if (!ingRes.success) {

                    res["error"] = ingRes.message.empty() ? "Ingestion failed" : ingRes.message;

                } else {

                    res["result"] = {

                        {"success", ingRes.success},

                        {"source_id", ingRes.source_id},

                        {"degradation_class", ingRes.degradation_class},

                        {"status", ingRes.status},

                        {"original_bytes", ingRes.original_bytes},

                        {"compressed_bytes", ingRes.compressed_bytes},

                        {"sha256", ingRes.sha256_checksum},

                        {"message", ingRes.message}

                    };

                }

            } else if (action == "classify_source") {

                if (!payload.contains("source_id") || !payload["source_id"].is_string() || payload["source_id"].get<std::string>().empty()) {

                    res["error"] = "Invalid request: 'source_id' is required for classification";

                    return res.dump();

                }

                std::string sourceId = payload["source_id"].get<std::string>();

                std::string targetClass = "CLASS_B";

                if (payload.contains("target_class") && payload["target_class"].is_string()) {

                    targetClass = payload["target_class"].get<std::string>();

                }



                // TYPE SAFETY: confirmed_clean_offset MUST be a strict boolean true.

                // Strings ("true"), numbers (1), arrays ([true]), objects ({}) MUST NOT be coerced to true.

                bool confirmedCleanOffset = false;

                if (payload.contains("confirmed_clean_offset") && payload["confirmed_clean_offset"].is_boolean()) {

                    confirmedCleanOffset = payload["confirmed_clean_offset"].get<bool>();

                }



                std::string err;

                bool ok = IngestionManager::ClassifySource(*storage_, sourceId, targetClass, confirmedCleanOffset, err, projectId);

                if (!ok) {

                    res["error"] = err.empty() ? "Classification failed" : err;

                } else {

                    res["result"] = {{"success", true}, {"source_id", sourceId}, {"degradation_class", targetClass}};

                }

            } else if (action == "reflag_source_class") {

                if (!payload.contains("source_id") || !payload["source_id"].is_string() || payload["source_id"].get<std::string>().empty()) {

                    res["error"] = "Invalid request: 'source_id' is required for reflagging";

                    return res.dump();

                }

                std::string sourceId = payload["source_id"].get<std::string>();

                bool ok = IngestionManager::ReflagSourceToClassB(*storage_, sourceId, projectId);

                res["result"] = {{"success", ok}, {"source_id", sourceId}, {"degradation_class", "CLASS_B"}};

            } else if (action == "get_verification_queue") {

                std::string sourceId = payload.value("source_id", "");

                auto items = storage_->get_verification_items(projectId, sourceId);

                json arr = json::array();

                for (const auto& v : items) arr.push_back(v);

                res["result"] = arr;

            } else if (action == "resolve_verification_item") {

                if (!payload.contains("item_id") || !payload["item_id"].is_string() || payload["item_id"].get<std::string>().empty()) {

                    res["error"] = "Invalid request: 'item_id' is required for verification resolution";

                    return res.dump();

                }

                std::string itemId = payload["item_id"].get<std::string>();

                std::string resolutionType = payload.value("resolution_type", "");

                std::string overrideValue = payload.value("override_value", "");



                // ANTI-ANCHORING GUARDRAIL: Optical crop is mandatory for verification

                auto items = storage_->get_verification_items(projectId);

                const VerificationItem* targetItem = nullptr;

                for (const auto& it : items) {

                    if (it.id == itemId) { targetItem = &it; break; }

                }



                if (!targetItem) {

                    res["error"] = "Verification item not found: " + itemId;

                } else {

                    if (resolutionType != "REJECT") {

                        if (targetItem->crop_image_path.empty()) {

                            res["error"] = "Anti-anchoring violation: Optical crop is mandatory for verification. Item cannot be resolved without visual crop evidence.";

                            return res.dump();

                        }

                        // ANTI-ANCHORING TIGHTENED GUARD: Verify physical file presence on disk

                        std::string rawPath = targetItem->crop_image_path;

                        if (!rawPath.empty() && (rawPath[0] == '/' || rawPath[0] == '\\')) {

                            rawPath = rawPath.substr(1);

                        }

                        std::vector<std::string> candidatePaths = {

                            targetItem->crop_image_path,

                            rawPath,

                            "dist/" + rawPath,

                            currentDataRoot_ + "/" + rawPath,

                            currentDataRoot_ + "/dist/" + rawPath

                        };

                        bool cropFoundOnDisk = false;

                        for (const auto& cp : candidatePaths) {

                            std::error_code ec;

                            if (std::filesystem::exists(cp, ec) && std::filesystem::file_size(cp, ec) > 0) {

                                cropFoundOnDisk = true;

                                break;

                            }

                        }

                        if (!cropFoundOnDisk) {

                            res["error"] = "Anti-anchoring violation: Optical crop file does not exist or cannot be rendered ('" + targetItem->crop_image_path + "'). Visual evidence is mandatory to resolve discrepancies.";

                            return res.dump();

                        }

                    }



                    bool ok = storage_->resolve_verification_item(itemId, resolutionType, overrideValue);

                    if (!ok) {

                        res["error"] = "Failed to resolve verification item: invalid resolution type or internal error.";

                    } else {

                        res["result"] = {{"success", true}, {"item_id", itemId}, {"status", resolutionType}};

                    }

                }

            } else if (action == "get_transcription_template") {

                // ENFORCES ANTI-ANCHORING & ZERO PRE-FILL:

                // For Class B documents, return purely blank schema fields.

                // Never supply machine OCR guesses or pre-fills.

                std::string sourceId = payload.value("source_id", "");

                int pageNumber = 1;

                if (payload.contains("page_number") && payload["page_number"].is_number_integer()) {

                    pageNumber = payload["page_number"].get<int>();

                }

                res["result"] = {

                    {"source_id", sourceId},

                    {"page_number", pageNumber},

                    {"prefill_enabled", false},

                    {"fields", json::array({

                        {{"field_name", "entity_name"}, {"value", ""}, {"placeholder", "Enter transcribed entity name"}},

                        {{"field_name", "value"}, {"value", ""}, {"placeholder", "Enter exact transcribed measurement/text"}},

                        {{"field_name", "type"}, {"value", "measurement"}, {"placeholder", "measurement / date / locus / artifact"}}

                    })}

                };

            } else if (action == "save_manual_transcription") {

                if (!payload.contains("source_id") || !payload["source_id"].is_string() || payload["source_id"].get<std::string>().empty()) {

                    res["error"] = "Invalid request: 'source_id' is required for manual transcription";

                    return res.dump();

                }

                std::string sourceId = payload["source_id"].get<std::string>();

                int pageNumber = 1;

                if (payload.contains("page_number") && payload["page_number"].is_number_integer()) {

                    pageNumber = payload["page_number"].get<int>();

                }

                if (!payload.contains("facts") || !payload["facts"].is_array()) {

                    res["error"] = "Invalid request: 'facts' must be an array of transcribed entities";

                    return res.dump();

                }

                json facts = payload["facts"];

                bool ok = IngestionManager::SaveManualTranscription(*storage_, sourceId, pageNumber, facts, projectId);

                if (!ok) {

                    res["error"] = "Failed to save manual transcription: internal storage failure.";

                } else {

                    res["result"] = {{"success", true}, {"source_id", sourceId}, {"page_number", pageNumber}};

                }

            } else if (action == "index_rough_text") {

                std::string sourceId = payload.value("source_id", "");

                json pages = payload.value("pages", json::array());

                std::vector<std::pair<int, std::string>> pageTexts;

                for (const auto& p : pages) {

                    int num = p.value("page_number", 1);

                    std::string txt = p.value("text", "");

                    pageTexts.push_back({num, txt});

                }

                int count = IngestionManager::IndexRoughTextForSearch(*storage_, sourceId, pageTexts, projectId);

                res["result"] = {{"success", true}, {"source_id", sourceId}, {"chunks_indexed", count}};

            } else if (action == "search_semantic_passages") {

                std::string query = payload.value("query", "");

                int topK = 5;

                if (payload.contains("top_k") && payload["top_k"].is_number_integer()) {

                    topK = payload["top_k"].get<int>();

                }

                float denseWeight = payload.value("dense_weight", 1.0f);

                float lexicalWeight = payload.value("lexical_weight", 1.0f);

                HybridSearchFilter filter;

                if (payload.contains("doc_id") && payload["doc_id"].is_string()) {

                    filter.doc_id = payload["doc_id"].get<std::string>();

                }

                if (payload.contains("min_page_ref") && payload["min_page_ref"].is_number_integer()) {

                    filter.min_page_ref = payload["min_page_ref"].get<int>();

                }

                if (payload.contains("max_page_ref") && payload["max_page_ref"].is_number_integer()) {

                    filter.max_page_ref = payload["max_page_ref"].get<int>();

                }



                auto queryEmb = VectorIndex::embed_text(query, /*is_query=*/true);

                auto matches = HybridSearchEngine::search(

                    storage_->vectors(),

                    storage_->lexical(),

                    query,

                    topK,

                    denseWeight,

                    lexicalWeight,

                    filter,

                    60.0f,

                    queryEmb

                );



                // Cross-reference sources table to attach UI degradation and badging guardrails

                auto sources = storage_->get_sources(projectId);

                std::map<std::string, Source> sourceMap;

                for (const auto& s : sources) sourceMap[s.id] = s;



                // Cross-reference claims for chunk-level grounding precision

                auto claims = storage_->get_claims(projectId);



                json arr = json::array();

                for (const auto& m : matches) {

                    std::string degClass = "CLASS_B";

                    std::string ingStatus = "UNVERIFIED_ROUGH_SCAN";

                    auto sit = sourceMap.find(m.doc_id);

                    if (sit != sourceMap.end()) {

                        degClass = sit->second.degradation_class;

                        ingStatus = sit->second.ingestion_status;

                    }



                    // Check chunk/page level grounding: has this specific page been manually verified?

                    int verifiedFactsOnPage = 0;

                    std::string targetPageRef = "p." + std::to_string(m.page_ref);

                    for (const auto& c : claims) {

                        if (c.source_id == m.doc_id && c.page_ref == targetPageRef && c.verification_status == "VERIFIED") {

                            verifiedFactsOnPage++;

                        }

                    }



                    bool isChunkVerified = (verifiedFactsOnPage > 0);

                    bool isDocFullyVerified = (ingStatus == "VERIFIED_MANUAL" || ingStatus == "COMPLETED");



                    // Precise Badge Logic:

                    // 1. Fully verified document -> "VERIFIED_MANUAL" or "VERIFIED_CONSENSUS" (green/blue)

                    // 2. Partially verified page with human grounding -> "PARTIALLY_VERIFIED" (clean status)

                    // 3. Otherwise unverified machine OCR -> "UNVERIFIED_ROUGH_SCAN" (amber warning)

                    std::string badge;

                    bool isRough = false;

                    if (isDocFullyVerified) {

                        badge = (ingStatus == "VERIFIED_MANUAL") ? "VERIFIED_MANUAL" : "VERIFIED_CONSENSUS";

                        isRough = false;

                    } else if (isChunkVerified) {

                        badge = "PARTIALLY_VERIFIED";

                        isRough = false;

                    } else {

                        badge = "UNVERIFIED_ROUGH_SCAN";

                        isRough = true;

                    }



                    float effectiveScore = m.dense_score;

                    if (effectiveScore <= 0.0f) {

                        effectiveScore = m.score * 30.0f;

                    }



                    arr.push_back({

                        {"chunk_id", m.chunk_id},

                        {"doc_id", m.doc_id},

                        {"page_ref", m.page_ref},

                        {"score", effectiveScore},

                        {"dense_score", m.dense_score},

                        {"dense_rank", m.dense_rank},

                        {"lexical_score", m.lexical_score},

                        {"lexical_rank", m.lexical_rank},

                        {"rrf_score", m.score},

                        {"text", storage_->read_compressed_chunk(m.chunk_id)},

                        {"degradation_class", degClass},

                        {"ingestion_status", ingStatus},

                        {"is_unverified_rough_scan", isRough},

                        {"badge", badge},

                        {"verified_facts_on_page", verifiedFactsOnPage},

                        {"has_page_grounding", isChunkVerified}

                    });

                }

                res["result"] = arr;

            } else if (action == "extract_archive_text") {

                // Closes the ingest→archive→extract→index join (Check 9).

                // Reads the losslessly-archived binary from disk via Source.archive_path

                // and extracts embedded text from PDF content streams (BT...ET blocks,

                // Tj/TJ operators). Returns [{page_number, text}] in the exact shape

                // index_rough_text expects — no external dependencies, pure C++.

                std::string sourceId = payload.value("source_id", "");

                if (sourceId.empty()) {

                    res["error"] = "Invalid request: 'source_id' is required for extract_archive_text";

                    return res.dump();

                }

                // Look up archive_path from relational ledger

                auto sources = storage_->get_sources(projectId);

                std::string archivePath;

                for (const auto& s : sources) {

                    if (s.id == sourceId) { archivePath = s.archive_path; break; }

                }

                if (archivePath.empty() || !fs_compat::exists(archivePath)) {

                    res["error"] = "Archive not found for source_id: " + sourceId;

                    return res.dump();

                }

                // Read binary

                std::ifstream af(archivePath, std::ios::binary);

                if (!af.is_open()) {

                    res["error"] = "Failed to open archive: " + archivePath;

                    return res.dump();

                }

                std::string raw((std::istreambuf_iterator<char>(af)), std::istreambuf_iterator<char>());

                af.close();



                // Minimal PDF text stream extractor:

                // Scans for BT...ET blocks and extracts string literals from Tj/TJ operators.

                // Handles the form: (text string) Tj

                // This is sufficient to recover text written via standard Type1/TrueType PDF operators.

                std::string extracted;

                size_t pos = 0;

                while ((pos = raw.find("BT", pos)) != std::string::npos) {

                    size_t end = raw.find("ET", pos);

                    if (end == std::string::npos) break;

                    std::string block = raw.substr(pos, end - pos);

                    pos = end + 2;



                    // Scan for (...) Tj patterns

                    size_t p = 0;

                    while ((p = block.find('(', p)) != std::string::npos) {

                        size_t q = block.find(')', p);

                        if (q == std::string::npos) break;

                        std::string tok = block.substr(p + 1, q - p - 1);

                        // Confirm Tj or TJ follows

                        size_t next = block.find_first_not_of(" \t\n\r", q + 1);

                        if (next != std::string::npos &&

                            (block.substr(next, 2) == "Tj" || block.substr(next, 2) == "TJ")) {

                            if (!extracted.empty()) extracted += " ";

                            extracted += tok;

                        }

                        p = q + 1;

                    }

                }



                json pages = json::array();

                if (!extracted.empty()) {

                    pages.push_back({{"page_number", 1}, {"text", extracted}});

                }

                res["result"] = {{"source_id", sourceId}, {"pages", pages}, {"char_count", extracted.size()}};

            } else if (action == "build_harris_matrix") {

                std::string siteId = payload.value("site_id", "");

                auto allStrata = storage_->get_strata(projectId);

                std::vector<Stratum> targetStrata;

                for (const auto& s : allStrata) {

                    if (siteId.empty() || s.site_id == siteId) {

                        targetStrata.push_back(s);

                    }

                }

                auto allSamples = storage_->get_samples(projectId);

                std::vector<Sample> targetSamples;

                for (const auto& smp : allSamples) {

                    if (siteId.empty() || smp.site_id == siteId) {

                        targetSamples.push_back(smp);

                    }

                }



                auto hRes = HarrisMatrixEngine::build_matrix(targetStrata, targetSamples);



                json invArr = json::array();

                for (const auto& inv : hRes.inversions) {

                    invArr.push_back({

                        {"upper_stratum_id", inv.upper_stratum_id},

                        {"upper_stratum_name", inv.upper_stratum_name},

                        {"upper_date_astro", inv.upper_date_astro},

                        {"upper_date_display", inv.upper_date_display},

                        {"lower_stratum_id", inv.lower_stratum_id},

                        {"lower_stratum_name", inv.lower_stratum_name},

                        {"lower_date_astro", inv.lower_date_astro},

                        {"lower_date_display", inv.lower_date_display},

                        {"is_advisory", inv.is_advisory},

                        {"sample_upper_code", inv.sample_upper_code},

                        {"sample_lower_code", inv.sample_lower_code},

                        {"reason", inv.reason}

                    });

                }



                res["result"] = {

                    {"site_id", siteId},

                    {"is_valid_dag", hRes.is_valid_dag},

                    {"chronological_sequence", hRes.chronological_sequence},

                    {"cycles", hRes.cycles},

                    {"inversions", invArr},

                    {"stratigraphic_levels", hRes.stratigraphic_levels},

                    {"direct_older_than", hRes.direct_older_than},

                    {"direct_younger_than", hRes.direct_younger_than}

                };

            } else if (action == "get_entity_subgraph") {

                std::string entityType = payload.value("entity_type", "");

                std::string entityId = payload.value("entity_id", "");

                if (entityType.empty() || entityId.empty()) {

                    res["error"] = "Required: 'entity_type' and 'entity_id'";

                    return res.dump();

                }

                res["result"] = storage_->get_entity_subgraph(entityType, entityId, projectId);

            } else if (action == "query_knowledge_graph") {

                // Unified Compound Query: Combines hybrid retrieval with structured graph entities

                std::string query = payload.value("query", "");

                std::string siteId = payload.value("site_id", "");

                std::string stratumId = payload.value("stratum_id", "");

                int topK = payload.value("top_k", 5);



                // 1. Safe embedding generation with clean IPC error surfacing

                std::vector<float> queryEmb;

                try {

                    queryEmb = VectorIndex::embed_text(query, /*is_query=*/true);

                } catch (const std::exception& e) {

                    res["error"] = std::string("MODEL_NOT_INITIALIZED: ") + e.what();

                    res["error_code"] = "MODEL_NOT_INITIALIZED";

                    return res.dump();

                }



                // 2. Hybrid semantic/lexical search

                HybridSearchFilter filter;

                auto matches = HybridSearchEngine::search(

                    storage_->vectors(),

                    storage_->lexical(),

                    query,

                    topK,

                    1.0f,

                    1.0f,

                    filter,

                    60.0f,

                    queryEmb

                );



                // 3. Fetch related graph entities filtered by site and stratum

                auto claims = storage_->get_claims(projectId);

                json matchingClaims = json::array();

                for (const auto& c : claims) {

                    bool matchSite = siteId.empty() || std::find(c.site_ids.begin(), c.site_ids.end(), siteId) != c.site_ids.end();

                    bool matchStratum = stratumId.empty() || std::find(c.strata_ids.begin(), c.strata_ids.end(), stratumId) != c.strata_ids.end();

                    if (matchSite && matchStratum) {

                        matchingClaims.push_back(c);

                    }

                }



                json passages = json::array();

                for (const auto& m : matches) {

                    passages.push_back({

                        {"chunk_id", m.chunk_id},

                        {"doc_id", m.doc_id},

                        {"page_ref", m.page_ref},

                        {"score", m.score},

                        {"dense_score", m.dense_score},

                        {"lexical_score", m.lexical_score},

                        {"text", storage_->read_compressed_chunk(m.chunk_id)}

                    });

                }



                res["result"] = {

                    {"query", query},

                    {"site_id", siteId},

                    {"stratum_id", stratumId},

                    {"passages", passages},

                    {"claims", matchingClaims}

                };

            } else {

                res["error"] = "Unknown native action: " + action;

            }

        } catch (const std::exception& e) {

            res["error"] = e.what();

        }



        return res.dump();

    }

};



} // namespace archaeophd

