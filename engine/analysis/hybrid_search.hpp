#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include "vector_index.hpp"
#include "lexical_index.hpp"

namespace archaeophd {

// =============================================================================
// Hybrid Search Engine (Dense Vector + BM25 Lexical + Reciprocal Rank Fusion)
// =============================================================================
// Combines dense semantic vector retrieval (Nomic 128-dim Matryoshka) with
// exact-term Okapi BM25 keyword matching via Reciprocal Rank Fusion (RRF).
//
// Solves intra-document near-neighbor collisions where shared monograph vocabulary
// dilutes pure cosine similarity, while preserving semantic generalization for
// paraphrased concept queries.
// =============================================================================

struct HybridSearchResult {
    std::string chunk_id;
    std::string doc_id;
    int page_ref;
    float score;        // RRF fused score in [0.0, ~0.033]
    float dense_score;  // Cosine similarity in [-1.0, 1.0]
    int dense_rank;     // 1-based rank (or 0 if not in dense top candidates)
    float lexical_score;// Raw BM25 score >= 0.0
    int lexical_rank;   // 1-based rank (or 0 if not in lexical top candidates)
};

struct HybridSearchFilter {
    std::string doc_id;         // Filter to specific document (empty = any)
    int min_page_ref = -1;      // Filter to >= page number (-1 = any)
    int max_page_ref = -1;      // Filter to <= page number (-1 = any)
};

class HybridSearchEngine {
public:
    static std::vector<HybridSearchResult> search(
        const VectorIndex& vector_index,
        const LexicalIndex& lexical_index,
        const std::string& query,
        size_t top_k = 5,
        float dense_weight = 1.0f,
        float lexical_weight = 1.0f,
        const HybridSearchFilter& filter = {},
        float rrf_k = 60.0f,
        const std::vector<float>& precomputed_query_vec = {}
    ) {
        size_t candidate_pool = std::max(top_k * 4, static_cast<size_t>(20));

        // 1. Dense Vector Search
        std::vector<VectorIndex::SearchResult> dense_candidates;
        try {
            std::vector<float> query_vec = precomputed_query_vec;
            if (query_vec.empty()) {
                query_vec = VectorIndex::embed_text(query, /*is_query=*/true);
            }
            dense_candidates = vector_index.search(query_vec, candidate_pool);
        } catch (...) {
            // If embedding model is uninitialized, dense candidates remain empty fail-safe
        }

        // 2. Lexical Okapi BM25 Search
        auto lexical_candidates = lexical_index.search(query, candidate_pool);

        // 3. Map candidates and compute Reciprocal Rank Fusion (RRF)
        struct CandidateAccum {
            std::string chunk_id;
            std::string doc_id;
            int page_ref;
            float dense_score = 0.0f;
            int dense_rank = 0;
            float lexical_score = 0.0f;
            int lexical_rank = 0;
            float rrf_score = 0.0f;
        };

        std::unordered_map<std::string, CandidateAccum> pool;

        // Process dense ranks
        for (size_t r = 0; r < dense_candidates.size(); ++r) {
            const auto& dc = dense_candidates[r];
            // Apply document filter if requested
            if (!filter.doc_id.empty() && dc.doc_id != filter.doc_id) continue;
            if (filter.min_page_ref >= 0 && dc.page_ref < filter.min_page_ref) continue;
            if (filter.max_page_ref >= 0 && dc.page_ref > filter.max_page_ref) continue;

            auto& entry = pool[dc.chunk_id];
            entry.chunk_id = dc.chunk_id;
            entry.doc_id = dc.doc_id;
            entry.page_ref = dc.page_ref;
            entry.dense_score = dc.score;
            entry.dense_rank = static_cast<int>(r + 1);
            entry.rrf_score += dense_weight / (rrf_k + static_cast<float>(r + 1));
        }

        // Process lexical ranks
        for (size_t r = 0; r < lexical_candidates.size(); ++r) {
            const auto& lc = lexical_candidates[r];
            // Apply document filter if requested
            if (!filter.doc_id.empty() && lc.doc_id != filter.doc_id) continue;
            if (filter.min_page_ref >= 0 && lc.page_ref < filter.min_page_ref) continue;
            if (filter.max_page_ref >= 0 && lc.page_ref > filter.max_page_ref) continue;

            auto& entry = pool[lc.chunk_id];
            entry.chunk_id = lc.chunk_id;
            entry.doc_id = lc.doc_id;
            entry.page_ref = lc.page_ref;
            entry.lexical_score = lc.score;
            entry.lexical_rank = static_cast<int>(r + 1);
            entry.rrf_score += lexical_weight / (rrf_k + static_cast<float>(r + 1));
        }

        // 4. Sort fused candidates by RRF score descending
        std::vector<HybridSearchResult> results;
        results.reserve(pool.size());

        for (const auto& [cid, cand] : pool) {
            results.push_back({
                cand.chunk_id,
                cand.doc_id,
                cand.page_ref,
                cand.rrf_score,
                cand.dense_score,
                cand.dense_rank,
                cand.lexical_score,
                cand.lexical_rank
            });
        }

        std::partial_sort(
            results.begin(),
            results.begin() + std::min(top_k, results.size()),
            results.end(),
            [](const HybridSearchResult& a, const HybridSearchResult& b) {
                return a.score > b.score;
            }
        );

        if (results.size() > top_k) {
            results.resize(top_k);
        }

        return results;
    }
};

} // namespace archaeophd
