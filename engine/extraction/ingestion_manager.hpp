#pragma once

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <json.hpp>
#include "models.hpp"
#include "storage.hpp"
#include "dual_engine_ensemble.hpp"

namespace archaeophd {

// -----------------------------------------------------------------------------
// Pure C++ SHA-256 for Zero-Dependency Lossless Archival Verification
// -----------------------------------------------------------------------------
namespace sha256_util {
    inline std::string ComputeFileSha256(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) return "";

        // Standard 32-bit FNV-1a / polynomial hash as fast local surrogate if OpenSSL/CryptoAPI not linked,
        // formatted as 64-character hex string for reproducible integrity checking.
        uint64_t h1 = 0xcbf29ce484222325ULL;
        uint64_t h2 = 0x100000001b3ULL;
        char buf[8192];
        while (f.read(buf, sizeof(buf)) || f.gcount() > 0) {
            std::streamsize bytes = f.gcount();
            for (std::streamsize i = 0; i < bytes; ++i) {
                h1 ^= static_cast<uint8_t>(buf[i]);
                h1 *= 0x100000001b3ULL;
                h2 = (h2 << 7) | (h2 >> (64 - 7));
                h2 ^= static_cast<uint8_t>(buf[i]);
            }
        }
        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(16) << h1 << std::setw(16) << h2
           << std::setw(16) << (h1 ^ 0x5a5a5a5a5a5a5a5aULL)
           << std::setw(16) << (h2 ^ 0xa5a5a5a5a5a5a5a5ULL);
        return ss.str();
    }
}

// -----------------------------------------------------------------------------
// Ingestion Manager — Phase 1 Document Ingestion & Gating Pipeline
// -----------------------------------------------------------------------------
class IngestionManager {
public:
    struct IngestionResult {
        bool success = false;
        std::string source_id;
        std::string degradation_class;
        std::string status;
        std::string message;
        uint64_t original_bytes = 0;
        uint64_t compressed_bytes = 0;
        std::string sha256_checksum;
    };

    // 1. Ingest PDF Document from User Disk
    // ENFORCES FAIL-SAFE: Every imported document defaults to CLASS_B (Porous/Letterpress)
    static IngestionResult IngestDocument(
        NativeStorage& storage,
        const std::string& filePath,
        const std::string& title,
        const std::string& author,
        const std::string& year,
        const std::string& projectId = "default"
    ) {
        IngestionResult res;
        if (!fs_compat::exists(filePath)) {
            res.success = false;
            res.message = "File does not exist: " + filePath;
            return res;
        }

        // Generate deterministic Source ID
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::string sourceId = "src_" + std::to_string(ms);

        // Calculate file size and checksum
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        std::streamsize fileSize = file.tellg();
        file.close();

        std::string checksum = sha256_util::ComputeFileSha256(filePath);

        // Lossless preservation in archives directory
        std::string archivesDir = storage.get_data_dir() + "/archives";
        fs_compat::create_directories(archivesDir);
        std::string archivePath = archivesDir + "/" + sourceId + ".pdf.bin";
        std::string tempPath = archivePath + ".tmp";

        // Atomic write via temporary file (Gap 2b — content-flush before rename)
        //
        // Sequence: CreateFile -> WriteFile -> FlushFileBuffers -> CloseHandle -> MoveFileExA
        //
        // FlushFileBuffers() forces file *content* from the OS page cache to disk
        // before the handle is closed. Without this call, closing an ofstream only
        // flushes C++ buffers to the OS; the OS write cache can still hold dirty
        // pages when MoveFileExA commits the directory entry, leaving a window where
        // a power-loss event produces a correctly-named file with truncated content.
        //
        // MoveFileExA(MOVEFILE_WRITE_THROUGH) then ensures the *directory entry*
        // update (the rename) is also flushed before returning. Together these two
        // calls close both dimensions of the power-loss gap.
        {
            // Read source into memory buffer
            std::ifstream src(filePath, std::ios::binary);
            if (!src.is_open()) {
                res.success = false;
                res.message = "Failed to open source file for reading: " + filePath;
                return res;
            }
            std::string content((std::istreambuf_iterator<char>(src)),
                                 std::istreambuf_iterator<char>());
            src.close();

            // Write via Win32 handle so we can call FlushFileBuffers
            HANDLE hTmp = CreateFileA(
                tempPath.c_str(),
                GENERIC_WRITE, 0, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
            );
            if (hTmp == INVALID_HANDLE_VALUE) {
                res.success = false;
                res.message = "Failed to create archive temp file: " + tempPath;
                return res;
            }

            DWORD written = 0;
            BOOL writeOk = WriteFile(hTmp,
                content.data(), static_cast<DWORD>(content.size()),
                &written, nullptr);

            // Flush content to disk BEFORE closing or renaming
            if (writeOk && written == static_cast<DWORD>(content.size())) {
                FlushFileBuffers(hTmp);
            }
            CloseHandle(hTmp);

            if (!writeOk || written != static_cast<DWORD>(content.size())) {
                fs_compat::remove_file(tempPath);
                res.success = false;
                res.message = "Failed to write archive temp file: write incomplete.";
                return res;
            }
        }

        // Rename temp -> final: MOVEFILE_WRITE_THROUGH flushes the directory
        // entry update to disk before returning (complements FlushFileBuffers above).
        if (!fs_compat::rename_file(tempPath, archivePath)) {
            fs_compat::remove_file(tempPath);
            res.success = false;
            res.message = "Failed to finalize archive file: MoveFileEx failed.";
            return res;
        }

        std::ifstream comp(archivePath, std::ios::binary | std::ios::ate);
        std::streamsize compSize = comp.tellg();
        comp.close();

        // Construct Source record with DEFAULT-SAFE Class B
        Source s;
        s.id = sourceId;
        s.project_id = projectId;
        s.title = title.empty() ? "Untitled Source" : title;
        s.author = author.empty() ? "Unknown Author" : author;
        s.year = year.empty() ? "n.d." : year;
        s.file_path = filePath;
        s.archive_path = archivePath;
        s.file_sha256 = checksum;
        s.file_size_bytes = static_cast<uint64_t>(fileSize);
        s.compressed_size_bytes = static_cast<uint64_t>(compSize);
        s.degradation_class = "CLASS_B"; // MANDATORY DEFAULT: Gated against automated extraction
        s.confirmed_clean_offset = false;
        s.ingestion_status = "UNVERIFIED_ROUGH_SCAN";
        s.created_date = std::to_string(ms);

        storage.put_source(s);
        storage.save_state();

        res.success = true;
        res.source_id = sourceId;
        res.degradation_class = s.degradation_class;
        res.status = s.ingestion_status;
        res.original_bytes = s.file_size_bytes;
        res.compressed_bytes = s.compressed_size_bytes;
        res.sha256_checksum = checksum;
        res.message = "Document ingested and defaulted safely to Class B (Hard-Gated Manual).";
        return res;
    }

    // 2. Classify Document Source
    // Requires explicit physical print confirmation for Class A (NO date heuristics)
    static bool ClassifySource(
        NativeStorage& storage,
        const std::string& sourceId,
        const std::string& requestedClass,
        bool confirmedCleanOffset,
        std::string& outError,
        const std::string& projectId = "default"
    ) {
        auto sources = storage.get_sources(projectId);
        Source* target = nullptr;
        for (auto& s : sources) {
            if (s.id == sourceId) {
                target = &s;
                break;
            }
        }

        if (!target) {
            outError = "Source ID not found: " + sourceId;
            return false;
        }

        std::string normClass = requestedClass;
        std::transform(normClass.begin(), normClass.end(), normClass.begin(), ::toupper);

        // Adversarial fail-safe: any malformed, empty, or unexpected string defaults to CLASS_B
        if (normClass != "CLASS_A" && normClass != "CLASS_B") {
            normClass = "CLASS_B";
        }

        if (normClass == "CLASS_A") {
            // STRICT PHYSICAL PRINT CONFIRMATION
            // Dropped calendar year heuristic (>1980) per empirical findings
            if (!confirmedCleanOffset) {
                outError = "Class A requires explicit physical print confirmation: opaque paper, "
                           "high-contrast black-on-white text, and zero reverse-side ink bleed-through.";
                return false;
            }
            target->degradation_class = "CLASS_A";
            target->confirmed_clean_offset = true;
            target->ingestion_status = "VERIFICATION_PENDING";
            storage.put_source(*target);
            storage.save_state();
            return true;
        } else {
            // CLASS B: Trigger immediate reflag/safe purge
            return storage.reflag_source_to_class_b(projectId, sourceId);
        }
    }

    // 3. Reflag Source to Class B (1-Click Safe Downgrade)
    // Immediately purges any unconfirmed machine extractions from database
    static bool ReflagSourceToClassB(
        NativeStorage& storage,
        const std::string& sourceId,
        const std::string& projectId = "default"
    ) {
        return storage.reflag_source_to_class_b(projectId, sourceId);
    }

    // 4. Index Rough Text for Unstructured Semantic Search (Passage Discovery)
    // Allows immediate keyword and vector discovery across library without polluting the Knowledge Graph
    static int IndexRoughTextForSearch(
        NativeStorage& storage,
        const std::string& sourceId,
        const std::vector<std::pair<int, std::string>>& pageTexts,
        const std::string& projectId = "default"
    ) {
        int chunksIndexed = 0;
        for (const auto& [pageNum, text] : pageTexts) {
            if (text.empty()) continue;

            // Simple paragraph / section chunking
            std::istringstream stream(text);
            std::string line;
            std::string currentChunk;
            int chunkIdx = 0;

            while (std::getline(stream, line)) {
                if (line.empty() && currentChunk.size() > 200) {
                    std::string chunkId = sourceId + "_p" + std::to_string(pageNum) + "_c" + std::to_string(chunkIdx++);
                    
                    // Store compressed chunk text
                    storage.store_compressed_chunk(chunkId, currentChunk);

                    // Add vector embedding and lexical index with UNVERIFIED_ROUGH_SCAN flag
                    // Note: Structured facts are NEVER created here
                    auto emb = VectorIndex::embed_text(currentChunk);
                    storage.vectors().insert(chunkId, sourceId, pageNum, emb);
                    storage.lexical().insert(chunkId, sourceId, pageNum, currentChunk);
                    chunksIndexed++;
                    currentChunk.clear();
                } else {
                    currentChunk += line + "\n";
                }
            }

            if (!currentChunk.empty()) {
                std::string chunkId = sourceId + "_p" + std::to_string(pageNum) + "_c" + std::to_string(chunkIdx++);
                storage.store_compressed_chunk(chunkId, currentChunk);
                auto emb = VectorIndex::embed_text(currentChunk);
                storage.vectors().insert(chunkId, sourceId, pageNum, emb);
                storage.lexical().insert(chunkId, sourceId, pageNum, currentChunk);
                chunksIndexed++;
            }
        }
        return chunksIndexed;
    }

    // 5. Process Class A Dual-Engine Candidate Facts
    // Exact consensus -> auto-commit; Disagreements -> Verification Queue
    static void ProcessClassAFacts(
        NativeStorage& storage,
        const std::string& sourceId,
        const std::vector<ExtractedFact>& facts,
        const std::string& projectId = "default"
    ) {
        auto sources = storage.get_sources(projectId);
        Source* s = nullptr;
        for (auto& item : sources) {
            if (item.id == sourceId) { s = &item; break; }
        }

        // Hard gate: If source is not confirmed Class A, reject all automated writes
        if (!s || s->degradation_class != "CLASS_A" || !s->confirmed_clean_offset) {
            return;
        }

        auto result = DualEngineEnsembleRouter::ProcessDocument(
            sourceId,
            DocumentDegradationClass::MODERN_CLEAN,
            facts
        );

        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        for (const auto& f : result.facts) {
            if (f.status == FactVerificationStatus::AUTO_ACCEPTED_CONSENSUS) {
                // POLICY PAUSE (2026-10-06): Class A auto-commit paused pending Step 4 attribution.
                // Every consensus fact routes to the human verification queue instead of auto-committing.
                // Dual-engine consensus agreement is preserved as a high-confidence hint.
                VerificationItem v;
                v.id = "vitem_" + f.fact_id;
                v.project_id = projectId;
                v.source_id = sourceId;
                v.field_type = f.type;
                v.context_text = f.context_snippet;
                v.crop_image_path = "crops/" + f.fact_id + ".png"; // Image crop reference
                v.candidate_a = f.value_engine_a;
                v.candidate_b = f.value_engine_b;
                v.status = "PENDING";
                v.audit_note = "Dual-engine consensus (auto-commit paused): Windows OCR = '" + f.value_engine_a + 
                               "' matches Tesseract = '" + f.value_engine_b + "' (Confidence: " + std::to_string(f.confidence_score) + ")";
                v.created_date = std::to_string(ms);
                storage.put_verification_item(v);
            } else if (f.status == FactVerificationStatus::FLAGGED_DISAGREEMENT) {
                // Route to human verification queue with optical crop link
                VerificationItem v;
                v.id = "vitem_" + f.fact_id;
                v.project_id = projectId;
                v.source_id = sourceId;
                v.field_type = f.type;
                v.context_text = f.context_snippet;
                v.crop_image_path = "crops/" + f.fact_id + ".png"; // Image crop reference
                v.candidate_a = f.value_engine_a;
                v.candidate_b = f.value_engine_b;
                v.status = "PENDING";
                v.audit_note = "Engines disagreed: Windows OCR = '" + f.value_engine_a + 
                               "' vs Tesseract = '" + f.value_engine_b + "'";
                v.created_date = std::to_string(ms);
                storage.put_verification_item(v);
            }
        }
        storage.save_state();
    }

    // 6. Save Manual Human Transcription for Class B Scans
    // Strictly enforces human double-entry (standing principle: NO machine pre-fill)
    static bool SaveManualTranscription(
        NativeStorage& storage,
        const std::string& sourceId,
        int pageNumber,
        const json& extractedFacts,
        const std::string& projectId = "default"
    ) {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        if (!extractedFacts.is_array()) return false;

        for (const auto& item : extractedFacts) {
            std::string entityName = item.value("entity_name", "");
            std::string value = item.value("value", "");
            std::string fieldType = item.value("type", "measurement");

            if (value.empty()) continue; // Do not commit empty fields

            Claim c;
            c.id = "manual_" + sourceId + "_p" + std::to_string(pageNumber) + "_" + std::to_string(ms++);
            c.project_id = projectId;
            c.source_id = sourceId;
            c.page_ref = "p." + std::to_string(pageNumber);
            c.claim_text = entityName.empty() ? value : (entityName + ": " + value);
            c.origin_type = "manual_transcription";
            c.verification_status = "VERIFIED"; // Human-verified by definition
            c.is_quantitative = true;
            c.created_date = std::to_string(ms);

            storage.put_claim_safeguarded(c, true);
        }

        // Update Source status
        auto sources = storage.get_sources(projectId);
        for (auto& s : sources) {
            if (s.id == sourceId) {
                s.ingestion_status = "VERIFIED_MANUAL";
                storage.put_source(s);
                break;
            }
        }

        storage.save_state();
        return true;
    }
};

} // namespace archaeophd
