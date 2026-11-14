#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <optional>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "models.hpp"
#include "vector_index.hpp"
#include "lexical_index.hpp"
#include "hybrid_search.hpp"
#include "harris_matrix.hpp"

#ifdef HAS_ZSTD
#include <zstd.h>
#endif

namespace archaeophd {

namespace fs_compat {
    inline bool exists(const std::string& path) {
        DWORD dw = GetFileAttributesA(path.c_str());
        return (dw != INVALID_FILE_ATTRIBUTES);
    }

    inline bool create_directories(const std::string& path) {
        if (exists(path)) return true;
        char tmp[MAX_PATH];
        if (path.length() >= MAX_PATH) return false;
        strncpy(tmp, path.c_str(), sizeof(tmp) - 1);
        for (char* p = tmp + 1; *p; p++) {
            if (*p == '/' || *p == '\\') {
                *p = '\0';
                CreateDirectoryA(tmp, NULL);
                *p = '/';
            }
        }
        CreateDirectoryA(tmp, NULL);
        return true;
    }

    inline bool remove_file(const std::string& path) {
        return DeleteFileA(path.c_str()) != 0;
    }

    inline bool rename_file(const std::string& from, const std::string& to) {
        // MOVEFILE_REPLACE_EXISTING: overwrite destination if it already exists.
        // MOVEFILE_WRITE_THROUGH: flush the rename to disk before returning,
        // so a power-loss event cannot leave a half-written .tmp as the canonical archive.
        return MoveFileExA(from.c_str(), to.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    }

    inline void cleanup_tmp_files(const std::string& dir) {
        if (!exists(dir)) return;
        std::string pattern = dir + "/*.tmp";
        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::string full_path = dir + "/" + fd.cFileName;
                    DeleteFileA(full_path.c_str());
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    }
}

class NativeStorage {
private:
    std::string data_dir_;
    mutable NativeMutex mutex_;

    // In-memory relational tables
    std::map<std::string, Site> sites_;
    std::map<std::string, Stratum> strata_;
    std::map<std::string, Artifact> artifacts_;
    std::map<std::string, Sample> samples_;
    std::map<std::string, Claim> claims_;
    std::map<std::string, EvidenceLink> evidence_;
    std::map<std::string, Source> sources_;
    std::map<std::string, Note> notes_;
    std::map<std::string, VerificationItem> verification_items_;

    VectorIndex vector_index_;
    LexicalIndex lexical_index_;

    std::string state_path() const {
        return data_dir_ + "/relational_state.json";
    }

    std::string vectors_path() const {
        return data_dir_ + "/vectors.bin";
    }

    std::string lexical_path() const {
        return data_dir_ + "/lexical.bin";
    }

    std::string chunks_dir() const {
        return data_dir_ + "/chunks";
    }

public:
    explicit NativeStorage(std::string data_dir = "data")
        : data_dir_(std::move(data_dir)) {
        fs_compat::create_directories(data_dir_);
        fs_compat::create_directories(chunks_dir());
        load_state();
    }

    VectorIndex& vectors() { return vector_index_; }
    const VectorIndex& vectors() const { return vector_index_; }
    LexicalIndex& lexical() { return lexical_index_; }
    const LexicalIndex& lexical() const { return lexical_index_; }
    const std::string& get_data_dir() const { return data_dir_; }

    void save_state() {
        NativeGuard lock(mutex_);
        json state;
        state["sites"] = json::array();
        for (const auto& kv : sites_) state["sites"].push_back(kv.second);

        state["strata"] = json::array();
        for (const auto& kv : strata_) state["strata"].push_back(kv.second);

        state["artifacts"] = json::array();
        for (const auto& kv : artifacts_) state["artifacts"].push_back(kv.second);

        state["samples"] = json::array();
        for (const auto& kv : samples_) state["samples"].push_back(kv.second);

        state["claims"] = json::array();
        for (const auto& kv : claims_) state["claims"].push_back(kv.second);

        state["evidence"] = json::array();
        for (const auto& kv : evidence_) state["evidence"].push_back(kv.second);

        state["sources"] = json::array();
        for (const auto& kv : sources_) state["sources"].push_back(kv.second);

        state["notes"] = json::array();
        for (const auto& kv : notes_) state["notes"].push_back(kv.second);

        state["verification_items"] = json::array();
        for (const auto& kv : verification_items_) state["verification_items"].push_back(kv.second);

        // 1. Atomic write-through for contiguous binary vectors.bin and lexical.bin first
        // Ordering rationale: Vectors and lexical index must be committed before promoting the authoritative
        // relational state ledger. If a crash occurs between (1) and (2), relational state
        // remains at transaction N-1, and any orphaned vectors or lexical postings are safely dropped
        // by the query joiner. Inverting this would risk claims pointing to missing index entries.
        vector_index_.save(vectors_path());
        lexical_index_.save(lexical_path());

        // 2. Atomic write-through for authoritative relational state JSON
        std::string tmp_state = state_path() + ".tmp";
        std::string payload = state.dump(2);
        HANDLE hFile = CreateFileA(tmp_state.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(hFile, payload.data(), static_cast<DWORD>(payload.size()), &written, NULL);
            FlushFileBuffers(hFile);
            CloseHandle(hFile);
            fs_compat::rename_file(tmp_state, state_path());
        }
    }

    void purge_legacy_test_benchmark_data() {
        bool changed = false;
        std::vector<std::string> testSiteIds = {
            "site-jericho", "site-megiddo", "site-hazor"
        };
        for (const auto& sid : testSiteIds) {
            if (sites_.erase(sid) > 0) changed = true;
        }
        std::vector<std::string> testStratumIds = {
            "stratum-jericho-ivb", "unit-402", "unit-317", "stratum-chirki-rubble"
        };
        for (const auto& stid : testStratumIds) {
            if (strata_.erase(stid) > 0) changed = true;
        }
        std::vector<std::string> testClaimIds = {
            "claim-kenyon-date", "claim-wood-date", "claim-seeds-domestic", "claim-seeds-siege",
            "claim-regional-collapse", "claim-chirki-corvinus", "claim-chirki-sankalia",
            "cl-1", "cl-2", "cl-3"
        };
        for (const auto& cid : testClaimIds) {
            if (claims_.erase(cid) > 0) changed = true;
        }
        std::vector<std::string> testArtifactIds = {
            "artifact-mb2b-pottery", "art-1"
        };
        for (const auto& aid : testArtifactIds) {
            if (artifacts_.erase(aid) > 0) changed = true;
        }
        std::vector<std::string> testSampleIds = {
            "sample-charred-seeds"
        };
        for (const auto& smid : testSampleIds) {
            if (samples_.erase(smid) > 0) changed = true;
        }
        std::vector<std::string> testEvidenceIds = {
            "ev-ash-layer", "ev-seeds-sup", "ev-seeds-con", "ev-1", "ev-2", "ev-3"
        };
        for (const auto& eid : testEvidenceIds) {
            if (evidence_.erase(eid) > 0) changed = true;
        }
        std::vector<std::string> testSourceIds = {
            "src-kenyon-1978", "src-wood-1990", "src-corvinus-1968", "src-sankalia-1974",
            "src-1", "src-2"
        };
        for (const auto& scid : testSourceIds) {
            if (sources_.erase(scid) > 0) changed = true;
        }
        std::vector<std::string> testNoteIds = {
            "n-1"
        };
        for (const auto& nid : testNoteIds) {
            if (notes_.erase(nid) > 0) changed = true;
        }

        for (auto it = claims_.begin(); it != claims_.end(); ) {
            if (it->second.claim_text.find("Tell es-Sultan") != std::string::npos ||
                it->second.claim_text.find("Stratum IV was destroyed") != std::string::npos ||
                it->second.claim_text.find("Southern Levant ca. 1200 BCE") != std::string::npos) {
                it = claims_.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }
        for (auto it = sites_.begin(); it != sites_.end(); ) {
            if (it->second.site_name.find("Tell es-Sultan") != std::string::npos) {
                it = sites_.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }

        if (changed) {
            save_state();
        }
    }

    void load_state() {
        NativeGuard lock(mutex_);

        // Clean up any orphaned .tmp files left by an interrupted write or power failure
        fs_compat::cleanup_tmp_files(data_dir_);
        fs_compat::cleanup_tmp_files(data_dir_ + "/archives");
        fs_compat::cleanup_tmp_files(chunks_dir());

        if (fs_compat::exists(state_path())) {
            std::ifstream in(state_path().c_str());
            if (in.is_open()) {
                try {
                    json state;
                    in >> state;

                    if (state.contains("sites")) {
                        for (const auto& it : state["sites"]) {
                            Site s = it.get<Site>();
                            sites_[s.id] = s;
                        }
                    }
                    if (state.contains("strata")) {
                        for (const auto& it : state["strata"]) {
                            Stratum s = it.get<Stratum>();
                            strata_[s.id] = s;
                        }
                    }
                    if (state.contains("artifacts")) {
                        for (const auto& it : state["artifacts"]) {
                            Artifact a = it.get<Artifact>();
                            artifacts_[a.id] = a;
                        }
                    }
                    if (state.contains("samples")) {
                        for (const auto& it : state["samples"]) {
                            Sample s = it.get<Sample>();
                            samples_[s.id] = s;
                        }
                    }
                    if (state.contains("claims")) {
                        for (const auto& it : state["claims"]) {
                            Claim c = it.get<Claim>();
                            claims_[c.id] = c;
                        }
                    }
                    if (state.contains("evidence")) {
                        for (const auto& it : state["evidence"]) {
                            EvidenceLink e = it.get<EvidenceLink>();
                            evidence_[e.id] = e;
                        }
                    }
                    if (state.contains("sources")) {
                        for (const auto& it : state["sources"]) {
                            Source s = it.get<Source>();
                            sources_[s.id] = s;
                        }
                    }
                    if (state.contains("notes")) {
                        for (const auto& it : state["notes"]) {
                            Note n = it.get<Note>();
                            notes_[n.id] = n;
                        }
                    }
                    if (state.contains("verification_items")) {
                        for (const auto& it : state["verification_items"]) {
                            VerificationItem v = it.get<VerificationItem>();
                            verification_items_[v.id] = v;
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error loading relational state: " << e.what() << std::endl;
                }
            }
        }

        // Restore contiguous binary vectors from vectors.bin
        vector_index_.load(vectors_path());

        // Restore Okapi BM25 inverted index from lexical.bin
        lexical_index_.load(lexical_path());
        purge_legacy_test_benchmark_data();
    }

    // -------------------------------------------------------------
    // Chunk Storage
    // -------------------------------------------------------------
    bool store_compressed_chunk(const std::string& chunk_id, const std::string& text) {
#ifdef HAS_ZSTD
        size_t bound = ZSTD_compressBound(text.size());
        std::vector<char> comp(bound);

        size_t c_size = ZSTD_compress(comp.data(), comp.size(), text.data(), text.size(), 3);
        if (ZSTD_isError(c_size)) return false;

        std::string filepath = chunks_dir() + "/" + chunk_id + ".zst";
        std::ofstream out(filepath.c_str(), std::ios::binary);
        if (!out.is_open()) return false;

        out.write(comp.data(), static_cast<std::streamsize>(c_size));
        return true;
#else
        std::string filepath = chunks_dir() + "/" + chunk_id + ".txt";
        std::ofstream out(filepath.c_str(), std::ios::binary);
        if (!out.is_open()) return false;
        out.write(text.data(), static_cast<std::streamsize>(text.size()));
        return true;
#endif
    }

    std::string read_compressed_chunk(const std::string& chunk_id) const {
#ifdef HAS_ZSTD
        std::string filepath = chunks_dir() + "/" + chunk_id + ".zst";
        if (!fs_compat::exists(filepath)) return "";

        std::ifstream in(filepath.c_str(), std::ios::binary | std::ios::ate);
        if (!in.is_open()) return "";

        std::streamsize size = in.tellg();
        in.seekg(0, std::ios::beg);

        std::vector<char> comp(size);
        in.read(comp.data(), size);

        unsigned long long r_size = ZSTD_getFrameContentSize(comp.data(), size);
        if (r_size == ZSTD_CONTENTSIZE_ERROR || r_size == ZSTD_CONTENTSIZE_UNKNOWN) {
            return "";
        }

        std::string decomp;
        decomp.resize(r_size);

        size_t d_res = ZSTD_decompress(&decomp[0], r_size, comp.data(), size);
        if (ZSTD_isError(d_res)) return "";

        return decomp;
#else
        std::string filepath = chunks_dir() + "/" + chunk_id + ".txt";
        if (!fs_compat::exists(filepath)) return "";
        std::ifstream in(filepath.c_str(), std::ios::binary);
        if (!in.is_open()) return "";
        return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
#endif
    }

    // -------------------------------------------------------------
    // Generic Entity Getters & Mutators
    // -------------------------------------------------------------
    std::vector<Site> get_sites(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Site> res;
        for (const auto& kv : sites_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    std::optional<Site> get_site(const std::string& id) const {
        NativeGuard lock(mutex_);
        auto it = sites_.find(id);
        if (it != sites_.end()) return it->second;
        return std::nullopt;
    }

    std::vector<Stratum> get_strata(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Stratum> res;
        for (const auto& kv : strata_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    std::vector<Artifact> get_artifacts(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Artifact> res;
        for (const auto& kv : artifacts_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    std::vector<Sample> get_samples(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Sample> res;
        for (const auto& kv : samples_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    std::vector<Claim> get_claims(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Claim> res;
        for (const auto& kv : claims_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    std::vector<EvidenceLink> get_evidence(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<EvidenceLink> res;
        for (const auto& kv : evidence_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    std::vector<Source> get_sources(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Source> res;
        for (const auto& kv : sources_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    std::vector<Note> get_notes(const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Note> res;
        for (const auto& kv : notes_) {
            if (project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default")
                res.push_back(kv.second);
        }
        return res;
    }

    void put_site(Site s) {
        NativeGuard lock(mutex_);
        sites_[s.id] = std::move(s);
    }
    void put_stratum(Stratum s) {
        NativeGuard lock(mutex_);
        strata_[s.id] = std::move(s);
    }
    void put_artifact(Artifact a) {
        NativeGuard lock(mutex_);
        artifacts_[a.id] = std::move(a);
    }
    void put_sample(Sample s) {
        NativeGuard lock(mutex_);
        samples_[s.id] = std::move(s);
    }
    bool put_claim(Claim c) {
        NativeGuard lock(mutex_);
        // POLICY PAUSE GUARD: Direct insertion of unverified dual-consensus claims is strictly forbidden.
        if (c.origin_type == "scanned_ocr_dual_consensus" && c.verification_status != "VERIFIED") {
            return false;
        }
        // CLASS B HARD GATE: Automated claims are strictly prohibited on Class B sources.
        auto s_it = sources_.find(c.source_id);
        if (s_it != sources_.end() && s_it->second.degradation_class == "CLASS_B") {
            if (c.origin_type != "manual_transcription") {
                return false;
            }
        }
        claims_[c.id] = std::move(c);
        return true;
    }
    void put_evidence(EvidenceLink e) {
        NativeGuard lock(mutex_);
        evidence_[e.id] = std::move(e);
    }
    void put_source(Source s) {
        NativeGuard lock(mutex_);
        sources_[s.id] = std::move(s);
    }
    void put_note(Note n) {
        NativeGuard lock(mutex_);
        notes_[n.id] = std::move(n);
    }

    // -------------------------------------------------------------
    // Phase 2 Knowledge Graph Relational Cross-Referencing
    // -------------------------------------------------------------
    std::vector<Claim> get_claims_by_site(const std::string& site_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Claim> res;
        for (const auto& kv : claims_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                std::find(kv.second.site_ids.begin(), kv.second.site_ids.end(), site_id) != kv.second.site_ids.end()) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    std::vector<Claim> get_claims_by_stratum(const std::string& stratum_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Claim> res;
        for (const auto& kv : claims_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                std::find(kv.second.strata_ids.begin(), kv.second.strata_ids.end(), stratum_id) != kv.second.strata_ids.end()) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    std::vector<Stratum> get_strata_by_site(const std::string& site_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Stratum> res;
        for (const auto& kv : strata_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                kv.second.site_id == site_id) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    std::vector<Artifact> get_artifacts_by_stratum(const std::string& stratum_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Artifact> res;
        for (const auto& kv : artifacts_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                kv.second.stratum_id == stratum_id) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    std::vector<Sample> get_samples_by_stratum(const std::string& stratum_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Sample> res;
        for (const auto& kv : samples_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                kv.second.stratum_id == stratum_id) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    std::vector<EvidenceLink> get_evidence_by_claim(const std::string& claim_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<EvidenceLink> res;
        for (const auto& kv : evidence_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                kv.second.claim_id == claim_id) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    std::vector<Claim> get_claims_by_source(const std::string& source_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<Claim> res;
        for (const auto& kv : claims_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                kv.second.source_id == source_id) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    json get_entity_subgraph(const std::string& entity_type, const std::string& entity_id, const std::string& project_id = "") const {
        NativeGuard lock(mutex_);
        json sub = json::object();
        sub["entity_type"] = entity_type;
        sub["entity_id"] = entity_id;

        if (entity_type == "site") {
            auto it = sites_.find(entity_id);
            if (it != sites_.end()) sub["site"] = it->second;
            sub["strata"] = json::array();
            for (const auto& kv : strata_) {
                if (kv.second.site_id == entity_id) sub["strata"].push_back(kv.second);
            }
            sub["claims"] = json::array();
            for (const auto& kv : claims_) {
                if (std::find(kv.second.site_ids.begin(), kv.second.site_ids.end(), entity_id) != kv.second.site_ids.end()) {
                    sub["claims"].push_back(kv.second);
                }
            }
        } else if (entity_type == "stratum") {
            auto it = strata_.find(entity_id);
            if (it != strata_.end()) {
                sub["stratum"] = it->second;
                auto sit = sites_.find(it->second.site_id);
                if (sit != sites_.end()) sub["site"] = sit->second;
            }
            sub["artifacts"] = json::array();
            for (const auto& kv : artifacts_) {
                if (kv.second.stratum_id == entity_id) sub["artifacts"].push_back(kv.second);
            }
            sub["samples"] = json::array();
            for (const auto& kv : samples_) {
                if (kv.second.stratum_id == entity_id) sub["samples"].push_back(kv.second);
            }
            sub["claims"] = json::array();
            for (const auto& kv : claims_) {
                if (std::find(kv.second.strata_ids.begin(), kv.second.strata_ids.end(), entity_id) != kv.second.strata_ids.end()) {
                    sub["claims"].push_back(kv.second);
                }
            }
        }
        return sub;
    }

    std::vector<VerificationItem> get_verification_items(const std::string& project_id = "", const std::string& source_id = "") const {
        NativeGuard lock(mutex_);
        std::vector<VerificationItem> res;
        for (const auto& kv : verification_items_) {
            if ((project_id.empty() || kv.second.project_id == project_id || kv.second.project_id == "default") &&
                (source_id.empty() || kv.second.source_id == source_id)) {
                res.push_back(kv.second);
            }
        }
        return res;
    }

    void put_verification_item(VerificationItem v) {
        NativeGuard lock(mutex_);
        verification_items_[v.id] = std::move(v);
    }

    bool resolve_verification_item(const std::string& item_id, const std::string& resolution_type, const std::string& override_value) {
        NativeGuard lock(mutex_);
        auto it = verification_items_.find(item_id);
        if (it == verification_items_.end()) return false;

        if (resolution_type == "CANDIDATE_A") {
            it->second.resolved_value = it->second.candidate_a;
            it->second.status = "RESOLVED_A";
        } else if (resolution_type == "CANDIDATE_B") {
            it->second.resolved_value = it->second.candidate_b;
            it->second.status = "RESOLVED_B";
        } else if (resolution_type == "MANUAL_OVERRIDE") {
            it->second.resolved_value = override_value;
            it->second.status = "RESOLVED_MANUAL_OVERRIDE";
        } else if (resolution_type == "REJECT") {
            it->second.status = "REJECTED";
        } else {
            return false;
        }

        save_state();
        return true;
    }

    // ADVERSARIAL PROTECTION: Reflagging source to Class B retroactively purges
    // ALL automated extractions (even if previously accepted by dual-engine consensus).
    // ONLY human manual double-entry (origin_type == "manual_transcription") is preserved.
    bool reflag_source_to_class_b(const std::string& project_id, const std::string& source_id) {
        NativeGuard lock(mutex_);
        auto s_it = sources_.find(source_id);
        if (s_it != sources_.end()) {
            s_it->second.degradation_class = "CLASS_B";
            s_it->second.confirmed_clean_offset = false;
            s_it->second.ingestion_status = "MANUAL_TRANSCRIPTION_PENDING";
        }

        // RETROACTIVE PURGE: Purge any machine-extracted claim for this source.
        // If a document was mistakenly classified as Class A and consensus facts were committed,
        // reflagging to Class B invalidates those automated extractions immediately.
        for (auto it = claims_.begin(); it != claims_.end(); ) {
            if (it->second.source_id == source_id) {
                if (it->second.origin_type != "manual_transcription") {
                    it = claims_.erase(it);
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }

        // Reject pending verification items for this source
        for (auto& kv : verification_items_) {
            if (kv.second.source_id == source_id && kv.second.status == "PENDING") {
                kv.second.status = "REJECTED_DUE_TO_RECLASSIFICATION";
                kv.second.audit_note = "Purged: source was reflagged to Class B (Hard-Gated Manual).";
            }
        }

        save_state();
        return true;
    }

    // ADVERSARIAL PROTECTION: Put claim with strict Class B and Class A auto-commit prevention
    bool put_claim_safeguarded(Claim c, bool is_human_verified = false) {
        NativeGuard lock(mutex_);
        // POLICY PAUSE (2026-10-06): Direct automated ingestion on Class A dual-consensus is paused.
        // Direct auto-commits of unverified OCR consensus are strictly rejected across all sources.
        if (c.origin_type == "scanned_ocr_dual_consensus" || !is_human_verified) {
            return false;
        }
        auto s_it = sources_.find(c.source_id);
        if (s_it != sources_.end()) {
            if (s_it->second.degradation_class == "CLASS_B") {
                // For Class B sources, ONLY explicit manual transcription or verified human entry is permitted
                if (c.origin_type != "manual_transcription") {
                    // HARD-GATE ENFORCED: Refuse automated write to Class B
                    return false;
                }
            }
        }
        claims_[c.id] = std::move(c);
        return true;
    }

    bool delete_entity(const std::string& entity_type, const std::string& id) {
        NativeGuard lock(mutex_);
        if (entity_type == "sites") return sites_.erase(id) > 0;
        if (entity_type == "strata") return strata_.erase(id) > 0;
        if (entity_type == "artifacts") return artifacts_.erase(id) > 0;
        if (entity_type == "samples") return samples_.erase(id) > 0;
        if (entity_type == "claims") return claims_.erase(id) > 0;
        if (entity_type == "evidence") return evidence_.erase(id) > 0;
        if (entity_type == "sources") return sources_.erase(id) > 0;
        if (entity_type == "notes") return notes_.erase(id) > 0;
        return false;
    }

    size_t count_sites() const { NativeGuard lock(mutex_); return sites_.size(); }
};

} // namespace archaeophd
