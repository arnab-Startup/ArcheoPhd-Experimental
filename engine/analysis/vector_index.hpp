#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "embedding_engine.hpp"

namespace archaeophd {

struct NativeMutex {
    CRITICAL_SECTION cs;
    NativeMutex() { InitializeCriticalSection(&cs); }
    ~NativeMutex() { DeleteCriticalSection(&cs); }
    void lock() { EnterCriticalSection(&cs); }
    void unlock() { LeaveCriticalSection(&cs); }
};

struct NativeGuard {
    NativeMutex& m;
    NativeGuard(NativeMutex& mtx) : m(mtx) { m.lock(); }
    ~NativeGuard() { m.unlock(); }
};

constexpr size_t VECTOR_DIM = 128; // Matryoshka truncated dimension

struct VectorRecord {
    std::string chunk_id;
    std::string doc_id;
    int page_ref;
    std::vector<float> embedding; // 128-dim normalized vector
};

class VectorIndex {
private:
    std::vector<VectorRecord> records_;
    mutable NativeMutex mutex_;

    static float dot_product(const std::vector<float>& a, const std::vector<float>& b) {
        float sum = 0.0f;
        size_t n = std::min({a.size(), b.size(), VECTOR_DIM});
        for (size_t i = 0; i < n; ++i) {
            sum += a[i] * b[i];
        }
        return sum;
    }

public:
    struct SearchResult {
        std::string chunk_id;
        std::string doc_id;
        int page_ref;
        float score;
    };

    static void l2_normalize(std::vector<float>& vec) {
        float norm_sq = 0.0f;
        for (float v : vec) norm_sq += v * v;
        if (norm_sq > 1e-12f && std::abs(norm_sq - 1.0f) > 1e-6f) {
            float norm = std::sqrt(norm_sq);
            for (float& v : vec) v /= norm;
        }
    }

    void insert(const std::string& chunk_id, const std::string& doc_id, int page_ref, std::vector<float> embedding) {
        NativeGuard lock(mutex_);
        if (embedding.size() > VECTOR_DIM) {
            embedding.resize(VECTOR_DIM); // Matryoshka truncation
        }
        l2_normalize(embedding);
        records_.push_back({chunk_id, doc_id, page_ref, std::move(embedding)});
    }

    std::vector<SearchResult> search(std::vector<float> query_vec, size_t top_k = 5) const {
        NativeGuard lock(mutex_);
        if (query_vec.size() > VECTOR_DIM) {
            query_vec.resize(VECTOR_DIM);
        }
        l2_normalize(query_vec);

        std::vector<SearchResult> results;
        results.reserve(records_.size());

        for (const auto& rec : records_) {
            float sim = dot_product(query_vec, rec.embedding);
            results.push_back({rec.chunk_id, rec.doc_id, rec.page_ref, sim});
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
        return results;
    }

    static std::vector<float> embed_text(const std::string& text, bool is_query = false) {
        return EmbeddingEngine::instance().embed(text, is_query, static_cast<int>(VECTOR_DIM));
    }

    void clear() {
        NativeGuard lock(mutex_);
        records_.clear();
    }

    bool save(const std::string& filepath) const {
        NativeGuard lock(mutex_);
        std::string tmp_path = filepath + ".tmp";

        HANDLE hFile = CreateFileA(tmp_path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        DWORD written = 0;
        const char magic[4] = {'A', 'P', 'V', '1'};
        if (!WriteFile(hFile, magic, 4, &written, NULL) || written != 4) {
            CloseHandle(hFile);
            DeleteFileA(tmp_path.c_str());
            return false;
        }

        uint32_t dim = static_cast<uint32_t>(VECTOR_DIM);
        if (!WriteFile(hFile, &dim, sizeof(dim), &written, NULL) || written != sizeof(dim)) {
            CloseHandle(hFile);
            DeleteFileA(tmp_path.c_str());
            return false;
        }

        uint32_t count = static_cast<uint32_t>(records_.size());
        if (!WriteFile(hFile, &count, sizeof(count), &written, NULL) || written != sizeof(count)) {
            CloseHandle(hFile);
            DeleteFileA(tmp_path.c_str());
            return false;
        }

        for (const auto& rec : records_) {
            uint16_t cid_len = static_cast<uint16_t>(rec.chunk_id.size());
            if (!WriteFile(hFile, &cid_len, sizeof(cid_len), &written, NULL) || written != sizeof(cid_len)) {
                CloseHandle(hFile); DeleteFileA(tmp_path.c_str()); return false;
            }
            if (cid_len > 0) {
                if (!WriteFile(hFile, rec.chunk_id.data(), cid_len, &written, NULL) || written != cid_len) {
                    CloseHandle(hFile); DeleteFileA(tmp_path.c_str()); return false;
                }
            }

            uint16_t did_len = static_cast<uint16_t>(rec.doc_id.size());
            if (!WriteFile(hFile, &did_len, sizeof(did_len), &written, NULL) || written != sizeof(did_len)) {
                CloseHandle(hFile); DeleteFileA(tmp_path.c_str()); return false;
            }
            if (did_len > 0) {
                if (!WriteFile(hFile, rec.doc_id.data(), did_len, &written, NULL) || written != did_len) {
                    CloseHandle(hFile); DeleteFileA(tmp_path.c_str()); return false;
                }
            }

            int32_t pref = static_cast<int32_t>(rec.page_ref);
            if (!WriteFile(hFile, &pref, sizeof(pref), &written, NULL) || written != sizeof(pref)) {
                CloseHandle(hFile); DeleteFileA(tmp_path.c_str()); return false;
            }

            DWORD emb_bytes = static_cast<DWORD>(VECTOR_DIM * sizeof(float));
            if (!WriteFile(hFile, rec.embedding.data(), emb_bytes, &written, NULL) || written != emb_bytes) {
                CloseHandle(hFile); DeleteFileA(tmp_path.c_str()); return false;
            }
        }

        // Flush OS file buffer to physical drive before promoting
        FlushFileBuffers(hFile);
        CloseHandle(hFile);

        // Atomic rename with write-through
        if (!MoveFileExA(tmp_path.c_str(), filepath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            DeleteFileA(tmp_path.c_str());
            return false;
        }

        return true;
    }

    bool load(const std::string& filepath) {
        NativeGuard lock(mutex_);
        records_.clear();

        HANDLE hFile = CreateFileA(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            // No saved index on disk yet - valid initial state
            return true;
        }

        DWORD read = 0;
        char magic[4] = {0};
        if (!ReadFile(hFile, magic, 4, &read, NULL) || read != 4 ||
            magic[0] != 'A' || magic[1] != 'P' || magic[2] != 'V' || magic[3] != '1') {
            CloseHandle(hFile);
            return false; // Invalid or corrupt header
        }

        uint32_t dim = 0;
        if (!ReadFile(hFile, &dim, sizeof(dim), &read, NULL) || read != sizeof(dim) || dim != VECTOR_DIM) {
            CloseHandle(hFile);
            return false; // Vector dimension mismatch
        }

        uint32_t count = 0;
        if (!ReadFile(hFile, &count, sizeof(count), &read, NULL) || read != sizeof(count)) {
            CloseHandle(hFile);
            return false;
        }

        records_.reserve(count);

        for (uint32_t i = 0; i < count; ++i) {
            uint16_t cid_len = 0;
            if (!ReadFile(hFile, &cid_len, sizeof(cid_len), &read, NULL) || read != sizeof(cid_len)) {
                CloseHandle(hFile); return false;
            }
            std::string chunk_id(cid_len, '\0');
            if (cid_len > 0) {
                if (!ReadFile(hFile, &chunk_id[0], cid_len, &read, NULL) || read != cid_len) {
                    CloseHandle(hFile); return false;
                }
            }

            uint16_t did_len = 0;
            if (!ReadFile(hFile, &did_len, sizeof(did_len), &read, NULL) || read != sizeof(did_len)) {
                CloseHandle(hFile); return false;
            }
            std::string doc_id(did_len, '\0');
            if (did_len > 0) {
                if (!ReadFile(hFile, &doc_id[0], did_len, &read, NULL) || read != did_len) {
                    CloseHandle(hFile); return false;
                }
            }

            int32_t page_ref = 0;
            if (!ReadFile(hFile, &page_ref, sizeof(page_ref), &read, NULL) || read != sizeof(page_ref)) {
                CloseHandle(hFile); return false;
            }

            std::vector<float> emb(VECTOR_DIM, 0.0f);
            DWORD emb_bytes = static_cast<DWORD>(VECTOR_DIM * sizeof(float));
            if (!ReadFile(hFile, emb.data(), emb_bytes, &read, NULL) || read != emb_bytes) {
                CloseHandle(hFile); return false;
            }

            records_.push_back({std::move(chunk_id), std::move(doc_id), static_cast<int>(page_ref), std::move(emb)});
        }

        CloseHandle(hFile);
        return true;
    }

    const std::vector<VectorRecord>& records() const {
        NativeGuard lock(mutex_);
        return records_;
    }

    size_t size() const {
        NativeGuard lock(mutex_);
        return records_.size();
    }
};

} // namespace archaeophd
