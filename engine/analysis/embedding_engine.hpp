#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <iostream>

#ifdef HAS_LLAMA_CPP
#include "llama.h"
#endif

namespace archaeophd {

class EmbeddingEngine {
private:
    mutable std::mutex mutex_;
    bool test_mock_mode_ = false;

#ifdef HAS_LLAMA_CPP
    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
    const llama_vocab* vocab_ = nullptr;
#endif

    EmbeddingEngine() = default;

public:
    static EmbeddingEngine& instance() {
        static EmbeddingEngine inst;
        return inst;
    }

    ~EmbeddingEngine() {
        shutdown();
    }

#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
    // Compile-time segregated opt-in strictly for unit tests testing storage persistence without 80MB weights.
    // In production builds (where ARCHAEOPHD_ENABLE_TEST_STUB is undefined), this method does not exist
    // and mock hashing is completely excluded from the binary.
    void enable_test_mock_mode(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        test_mock_mode_ = enable;
    }

    bool is_test_mock_mode() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return test_mock_mode_;
    }
#endif

    bool is_ready() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return is_ready_unlocked();
    }

    bool load_model(const std::string& model_path, int n_threads = 4) {
        std::lock_guard<std::mutex> lock(mutex_);
#ifdef HAS_LLAMA_CPP
        shutdown_unlocked();

        llama_backend_init();

        // Suppress verbose internal llama.cpp info logs
        llama_log_set([](enum ggml_log_level level, const char * text, void * user_data) {
            if (level >= GGML_LOG_LEVEL_ERROR) {
                std::cerr << text;
            }
        }, nullptr);

        llama_model_params mparams = llama_model_default_params();
        model_ = llama_model_load_from_file(model_path.c_str(), mparams);
        if (!model_) {
            return false;
        }

        llama_context_params cparams = llama_context_default_params();
        cparams.embeddings = true;
        cparams.n_threads = n_threads;
        cparams.n_ctx = 2048;

        ctx_ = llama_init_from_model(model_, cparams);
        if (!ctx_) {
            llama_model_free(model_);
            model_ = nullptr;
            return false;
        }

        vocab_ = llama_model_get_vocab(model_);
        return true;
#else
        return false;
#endif
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_unlocked();
    }

private:
    void shutdown_unlocked() {
#ifdef HAS_LLAMA_CPP
        if (ctx_) {
            llama_free(ctx_);
            ctx_ = nullptr;
        }
        if (model_) {
            llama_model_free(model_);
            model_ = nullptr;
        }
        vocab_ = nullptr;
        llama_backend_free();
#endif
    }

    bool is_ready_unlocked() const {
#ifdef HAS_LLAMA_CPP
        return (model_ != nullptr && ctx_ != nullptr && vocab_ != nullptr);
#else
        return false;
#endif
    }

#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
    std::vector<float> embed_mock_hash(const std::string& text, int target_dim) const {
        std::vector<float> vec(target_dim, 0.0f);
        for (size_t i = 0; i < text.size(); ++i) {
            uint8_t c = static_cast<uint8_t>(text[i]);
            vec[c % target_dim] += 1.0f;
        }
        float norm_sq = 0.0f;
        for (float v : vec) norm_sq += v * v;
        if (norm_sq > 0.0f && std::abs(norm_sq - 1.0f) > 1e-6f) {
            float norm = std::sqrt(norm_sq);
            for (float& v : vec) v /= norm;
        }
        return vec;
    }
#endif

public:
    std::vector<float> embed(const std::string& text, bool is_query = false, int target_dim = 128) {
        std::lock_guard<std::mutex> lock(mutex_);

        // SAFEGUARD: In production builds, this path ALWAYS throws MODEL_NOT_INITIALIZED.
        // It is physically impossible to fall back to character hashing because the stub is omitted at compile time.
        if (!is_ready_unlocked()) {
#ifdef ARCHAEOPHD_ENABLE_TEST_STUB
            if (test_mock_mode_) {
                return embed_mock_hash(text, target_dim);
            }
#endif
            throw std::runtime_error("MODEL_NOT_INITIALIZED: Nomic embedding model weights not loaded. Please acquire model in Data Root.");
        }

#ifdef HAS_LLAMA_CPP
        // 1. Task prefixing per VECTOR_SEARCH.md
        std::string full_text = (is_query ? "search_query: " : "search_document: ") + text;

        // 2. Tokenization
        std::vector<llama_token> tokens(full_text.size() + 32);
        int n_tokens = llama_tokenize(vocab_, full_text.c_str(), full_text.size(), tokens.data(), tokens.size(), true, false);
        if (n_tokens < 0) {
            tokens.resize(-n_tokens);
            n_tokens = llama_tokenize(vocab_, full_text.c_str(), full_text.size(), tokens.data(), tokens.size(), true, false);
        }
        if (n_tokens <= 0) {
            return std::vector<float>(target_dim, 0.0f);
        }
        tokens.resize(n_tokens);

        // 3. Forward pass
        llama_batch batch = llama_batch_get_one(tokens.data(), n_tokens);
        if (llama_encode(ctx_, batch) != 0) {
            throw std::runtime_error("EMBEDDING_INFERENCE_ERROR: llama_encode failed during forward pass");
        }

        float* full_embd = llama_get_embeddings_seq(ctx_, 0);
        if (!full_embd) full_embd = llama_get_embeddings(ctx_);
        if (!full_embd) full_embd = llama_get_embeddings_ith(ctx_, -1);
        if (!full_embd) {
            throw std::runtime_error("EMBEDDING_INFERENCE_ERROR: failed to retrieve pooled embedding tensor");
        }

        // 4. Matryoshka truncation to target_dim (128)
        std::vector<float> vec(target_dim);
        for (int i = 0; i < target_dim; ++i) {
            vec[i] = full_embd[i];
        }

        // 5. Guarded L2 re-normalization with 1e-6f mantissa guard
        float norm_sq = 0.0f;
        for (float v : vec) norm_sq += v * v;
        if (norm_sq > 0.0f && std::abs(norm_sq - 1.0f) > 1e-6f) {
            float norm = std::sqrt(norm_sq);
            for (float& v : vec) v /= norm;
        }

        return vec;
#else
        throw std::runtime_error("MODEL_NOT_INITIALIZED: Built without HAS_LLAMA_CPP");
#endif
    }
};

} // namespace archaeophd
