#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace archaeophd {

// =============================================================================
// Lexical / BM25 Search Index for Archaeological Literature
// =============================================================================
// Provides pure C++ term-frequency and Okapi BM25 scoring with:
// - Exact diagnostic keyword matching (site names, stratum IDs, ceramic types)
// - Robertson-Spärck Jones non-negative IDF formulation
// - Thread-safe insertion and query operations (NativeMutex)
// - Binary atomic disk serialization (APL1 magic format)
// =============================================================================

class LexicalIndex {
public:
    struct SearchResult {
        std::string chunk_id;
        std::string doc_id;
        int page_ref;
        float score;
    };

    struct DocumentMeta {
        std::string chunk_id;
        std::string doc_id;
        int page_ref;
        uint32_t doc_len;
    };

    struct Posting {
        std::string chunk_id;
        uint32_t tf;
    };

private:
    std::unordered_map<std::string, DocumentMeta> docs_;
    std::unordered_map<std::string, std::vector<Posting>> inverted_index_;
    uint64_t total_token_count_ = 0;
    mutable CRITICAL_SECTION cs_;

    void lock() const { EnterCriticalSection(&cs_); }
    void unlock() const { LeaveCriticalSection(&cs_); }

    static bool is_stop_word(const std::string& w) {
        static const std::unordered_set<std::string> stop_words = {
            "a", "an", "the", "and", "or", "in", "on", "at", "to", "for",
            "of", "with", "by", "from", "is", "are", "was", "were", "be",
            "been", "being", "that", "this", "it", "as", "into", "their",
            "its", "within", "which", "over", "these", "those"
        };
        return stop_words.find(w) != stop_words.end();
    }

public:
    LexicalIndex() {
        InitializeCriticalSection(&cs_);
    }

    ~LexicalIndex() {
        DeleteCriticalSection(&cs_);
    }

    // Pure C++ alphanumeric tokenizer with lowercase normalization
    static std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        std::string current;
        current.reserve(32);

        for (size_t i = 0; i <= text.size(); ++i) {
            char c = (i < text.size()) ? text[i] : ' ';
            unsigned char uc = static_cast<unsigned char>(c);

            if (std::isalnum(uc)) {
                current += static_cast<char>(std::tolower(uc));
            } else {
                if (current.size() >= 2) {
                    if (!is_stop_word(current)) {
                        tokens.push_back(current);
                    }
                }
                current.clear();
            }
        }
        return tokens;
    }

    // Index chunk text into the inverted index
    void insert(const std::string& chunk_id, const std::string& doc_id, int page_ref, const std::string& text) {
        lock();
        auto tokens = tokenize(text);
        uint32_t doc_len = static_cast<uint32_t>(tokens.size());

        // Remove existing postings for chunk_id if re-indexing
        auto doc_it = docs_.find(chunk_id);
        if (doc_it != docs_.end()) {
            total_token_count_ -= doc_it->second.doc_len;
            docs_.erase(doc_it);
            for (auto& [term, postings] : inverted_index_) {
                postings.erase(
                    std::remove_if(postings.begin(), postings.end(),
                        [&chunk_id](const Posting& p) { return p.chunk_id == chunk_id; }),
                    postings.end()
                );
            }
        }

        docs_[chunk_id] = {chunk_id, doc_id, page_ref, doc_len};
        total_token_count_ += doc_len;

        // Compute term frequencies for this chunk
        std::unordered_map<std::string, uint32_t> tf_map;
        for (const auto& t : tokens) {
            tf_map[t]++;
        }

        for (const auto& [term, count] : tf_map) {
            inverted_index_[term].push_back({chunk_id, count});
        }
        unlock();
    }

    // Okapi BM25 scoring across inverted index
    std::vector<SearchResult> search(const std::string& query, size_t top_k = 5, float k1 = 1.2f, float b = 0.75f) const {
        lock();
        std::vector<SearchResult> results;
        if (docs_.empty()) {
            unlock();
            return results;
        }

        auto query_tokens = tokenize(query);
        if (query_tokens.empty()) {
            unlock();
            return results;
        }

        size_t N = docs_.size();
        float avgdl = static_cast<float>(total_token_count_) / static_cast<float>(N);
        if (avgdl < 1.0f) avgdl = 1.0f;

        // Accumulate BM25 score per candidate chunk
        std::unordered_map<std::string, float> chunk_scores;

        for (const auto& q_term : query_tokens) {
            auto it = inverted_index_.find(q_term);
            if (it == inverted_index_.end() || it->second.empty()) continue;

            size_t n_q = it->second.size();
            // Robertson-Spärck Jones non-negative IDF
            float idf = std::log(1.0f + (static_cast<float>(N) - static_cast<float>(n_q) + 0.5f) /
                                       (static_cast<float>(n_q) + 0.5f));
            if (idf < 0.0f) idf = 0.0f;

            for (const auto& posting : it->second) {
                auto doc_meta_it = docs_.find(posting.chunk_id);
                if (doc_meta_it == docs_.end()) continue;

                float tf = static_cast<float>(posting.tf);
                float dl = static_cast<float>(doc_meta_it->second.doc_len);

                float denom = tf + k1 * (1.0f - b + b * (dl / avgdl));
                float tf_component = (tf * (k1 + 1.0f)) / (denom > 1e-6f ? denom : 1e-6f);

                chunk_scores[posting.chunk_id] += idf * tf_component;
            }
        }

        results.reserve(chunk_scores.size());
        for (const auto& [cid, score] : chunk_scores) {
            const auto& meta = docs_.at(cid);
            results.push_back({cid, meta.doc_id, meta.page_ref, score});
        }

        std::partial_sort(
            results.begin(),
            results.begin() + std::min(top_k, results.size()),
            results.end(),
            [](const SearchResult& a, const SearchResult& b) {
                return a.score > b.score;
            }
        );

        if (results.size() > top_k) {
            results.resize(top_k);
        }

        unlock();
        return results;
    }

    void clear() {
        lock();
        docs_.clear();
        inverted_index_.clear();
        total_token_count_ = 0;
        unlock();
    }

    size_t size() const {
        lock();
        size_t s = docs_.size();
        unlock();
        return s;
    }

    size_t vocab_size() const {
        lock();
        size_t s = inverted_index_.size();
        unlock();
        return s;
    }

    // Binary atomic persistence (APL1 magic format)
    bool save(const std::string& filepath) const {
        lock();
        std::string tmp_path = filepath + ".tmp";

        HANDLE hFile = CreateFileA(tmp_path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            unlock();
            return false;
        }

        DWORD written = 0;
        const char magic[4] = {'A', 'P', 'L', '1'};
        if (!WriteFile(hFile, magic, 4, &written, NULL) || written != 4) {
            CloseHandle(hFile);
            DeleteFileA(tmp_path.c_str());
            unlock();
            return false;
        }

        uint32_t doc_count = static_cast<uint32_t>(docs_.size());
        WriteFile(hFile, &doc_count, sizeof(doc_count), &written, NULL);

        for (const auto& [cid, meta] : docs_) {
            uint32_t cid_len = static_cast<uint32_t>(meta.chunk_id.size());
            WriteFile(hFile, &cid_len, sizeof(cid_len), &written, NULL);
            WriteFile(hFile, meta.chunk_id.data(), cid_len, &written, NULL);

            uint32_t doc_id_len = static_cast<uint32_t>(meta.doc_id.size());
            WriteFile(hFile, &doc_id_len, sizeof(doc_id_len), &written, NULL);
            WriteFile(hFile, meta.doc_id.data(), doc_id_len, &written, NULL);

            int32_t page_ref = static_cast<int32_t>(meta.page_ref);
            WriteFile(hFile, &page_ref, sizeof(page_ref), &written, NULL);

            uint32_t doc_len = meta.doc_len;
            WriteFile(hFile, &doc_len, sizeof(doc_len), &written, NULL);
        }

        uint32_t term_count = static_cast<uint32_t>(inverted_index_.size());
        WriteFile(hFile, &term_count, sizeof(term_count), &written, NULL);

        for (const auto& [term, postings] : inverted_index_) {
            uint32_t t_len = static_cast<uint32_t>(term.size());
            WriteFile(hFile, &t_len, sizeof(t_len), &written, NULL);
            WriteFile(hFile, term.data(), t_len, &written, NULL);

            uint32_t p_count = static_cast<uint32_t>(postings.size());
            WriteFile(hFile, &p_count, sizeof(p_count), &written, NULL);

            for (const auto& p : postings) {
                uint32_t p_cid_len = static_cast<uint32_t>(p.chunk_id.size());
                WriteFile(hFile, &p_cid_len, sizeof(p_cid_len), &written, NULL);
                WriteFile(hFile, p.chunk_id.data(), p_cid_len, &written, NULL);

                uint32_t tf = p.tf;
                WriteFile(hFile, &tf, sizeof(tf), &written, NULL);
            }
        }

        FlushFileBuffers(hFile);
        CloseHandle(hFile);

        BOOL ren_ok = MoveFileExA(tmp_path.c_str(), filepath.c_str(), MOVEFILE_REPLACE_EXISTING);
        if (!ren_ok) {
            DeleteFileA(tmp_path.c_str());
            unlock();
            return false;
        }

        unlock();
        return true;
    }

    bool load(const std::string& filepath) {
        lock();
        docs_.clear();
        inverted_index_.clear();
        total_token_count_ = 0;

        HANDLE hFile = CreateFileA(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            unlock();
            return false;
        }

        DWORD read = 0;
        char magic[4] = {0};
        if (!ReadFile(hFile, magic, 4, &read, NULL) || read != 4 ||
            magic[0] != 'A' || magic[1] != 'P' || magic[2] != 'L' || magic[3] != '1') {
            CloseHandle(hFile);
            unlock();
            return false;
        }

        uint32_t doc_count = 0;
        if (!ReadFile(hFile, &doc_count, sizeof(doc_count), &read, NULL) || read != sizeof(doc_count)) {
            CloseHandle(hFile);
            unlock();
            return false;
        }

        for (uint32_t i = 0; i < doc_count; ++i) {
            uint32_t cid_len = 0;
            ReadFile(hFile, &cid_len, sizeof(cid_len), &read, NULL);
            std::string cid(cid_len, '\0');
            ReadFile(hFile, &cid[0], cid_len, &read, NULL);

            uint32_t doc_id_len = 0;
            ReadFile(hFile, &doc_id_len, sizeof(doc_id_len), &read, NULL);
            std::string doc_id(doc_id_len, '\0');
            ReadFile(hFile, &doc_id[0], doc_id_len, &read, NULL);

            int32_t page_ref = 0;
            ReadFile(hFile, &page_ref, sizeof(page_ref), &read, NULL);

            uint32_t doc_len = 0;
            ReadFile(hFile, &doc_len, sizeof(doc_len), &read, NULL);

            docs_[cid] = {cid, doc_id, static_cast<int>(page_ref), doc_len};
            total_token_count_ += doc_len;
        }

        uint32_t term_count = 0;
        if (!ReadFile(hFile, &term_count, sizeof(term_count), &read, NULL) || read != sizeof(term_count)) {
            CloseHandle(hFile);
            unlock();
            return false;
        }

        for (uint32_t i = 0; i < term_count; ++i) {
            uint32_t t_len = 0;
            ReadFile(hFile, &t_len, sizeof(t_len), &read, NULL);
            std::string term(t_len, '\0');
            ReadFile(hFile, &term[0], t_len, &read, NULL);

            uint32_t p_count = 0;
            ReadFile(hFile, &p_count, sizeof(p_count), &read, NULL);

            std::vector<Posting> postings;
            postings.reserve(p_count);

            for (uint32_t p = 0; p < p_count; ++p) {
                uint32_t p_cid_len = 0;
                ReadFile(hFile, &p_cid_len, sizeof(p_cid_len), &read, NULL);
                std::string p_cid(p_cid_len, '\0');
                ReadFile(hFile, &p_cid[0], p_cid_len, &read, NULL);

                uint32_t tf = 0;
                ReadFile(hFile, &tf, sizeof(tf), &read, NULL);

                postings.push_back({p_cid, tf});
            }
            inverted_index_[term] = std::move(postings);
        }

        CloseHandle(hFile);
        unlock();
        return true;
    }
};

} // namespace archaeophd
