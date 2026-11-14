#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <iostream>
#include <json.hpp>

#ifdef HAS_LLAMA_CPP
#include "llama.h"
#endif

namespace archaeophd {

using json = nlohmann::json;

// ============================================================================
// ArchaeoPhD — Local LLM Engine (Qwen 2.5 7B GGUF)
//
// Dual-Role Architecture:
// 1. Phase 2 Step 4: Clausal Semantic Disambiguator (Tier 2 Background Ingestion Fallback)
// 2. Phase 2 Step 5 & Phase 3: Cross-Document Contradiction & Claim Verification
//
// Performance Optimization:
// - KV-prefix caching preserves evaluated system instructions in memory,
//   reducing per-clause inference latency from ~20s down to ~4–7s on CPU.
// ============================================================================
class LlmEngine {
private:
    mutable std::mutex mutex_;
    bool is_initialized_ = false;
    std::string model_path_;

#ifdef HAS_LLAMA_CPP
    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
    const llama_vocab* vocab_ = nullptr;
    llama_sampler* smpl_ = nullptr;
    int n_prefix_tokens_ = 0;

    static void batch_set_tokens(llama_batch_ext* batch, const llama_token* tokens, int32_t n_tokens, llama_pos pos_0) {
        llama_batch_ext_clear(batch);
        for (int32_t i = 0; i < n_tokens; ++i) {
            const int32_t idx = llama_batch_ext_add_token(batch, 0, tokens[i]);
            const llama_pos pos = pos_0 + i;
            llama_batch_ext_set_pos(batch, idx, &pos);
        }
        llama_batch_ext_set_output_logits(batch, n_tokens - 1, true);
    }
#endif

    LlmEngine() = default;

public:
    static LlmEngine& instance() {
        static LlmEngine inst;
        return inst;
    }

    ~LlmEngine() {
        shutdown();
    }

    bool initialize(const std::string& model_path = "models/llm/Qwen2.5-7B-Instruct-Q4_K_M.gguf") {
        std::lock_guard<std::mutex> lock(mutex_);
        if (is_initialized_) return true;

#ifdef HAS_LLAMA_CPP
        model_path_ = model_path;
        llama_backend_init();

        llama_model_params mparams = llama_model_default_params();
        mparams.n_gpu_layers = 0; // Native CPU inference with OpenMP

        model_ = llama_model_load_from_file(model_path_.c_str(), mparams);
        if (!model_) {
            std::cerr << "[LLM_ENGINE] Could not load GGUF model from: " << model_path_ << "\n";
            return false;
        }

        vocab_ = llama_model_get_vocab(model_);

        llama_context_params cparams = llama_context_default_params();
        cparams.n_ctx = 2048;
        cparams.n_batch = 512;
        cparams.n_threads = 4;
        cparams.n_threads_batch = 4;
        cparams.no_perf = true;

        ctx_ = llama_init_from_model(model_, cparams);
        if (!ctx_) {
            llama_model_free(model_);
            model_ = nullptr;
            return false;
        }

        auto sparams = llama_sampler_chain_default_params();
        sparams.no_perf = true;
        smpl_ = llama_sampler_chain_init(sparams);
        llama_sampler_chain_add(smpl_, llama_sampler_init_greedy());

        // Pre-evaluate system instructions and few-shot calibration into KV cache
        std::string system_prefix = 
            "<|im_start|>system\n"
            "You are a precision archaeological data extractor for research monographs.\n"
            "Rules:\n"
            "1. EXTRACT_ATTRIBUTED: If the sentence contains the specific finding requested by Target (e.g. appointment year, excavation year, stratum depth, artifact count, radiocarbon date, publication year of monograph), extract: {\"status\": \"EXTRACT_ATTRIBUTED\", \"value\": \"<val>\", \"unit\": \"<unit_or_noun>\"}\n"
            "2. REJECT_NON_FINDING: If Target refers to background noise (scholar lifespans like 1820-1903, journal footnote imprints, page numbers) or is absent: {\"status\": \"REJECT_NON_FINDING\"}\n"
            "3. FLAG_AMBIGUOUS_MULTI_CANDIDATE: If multiple conflicting candidates exist without resolution: {\"status\": \"FLAG_AMBIGUOUS_MULTI_CANDIDATE\", \"candidates\": [...]}\n"
            "Return valid JSON only.\n"
            "<|im_end|>\n"
            "<|im_start|>user\nSentence: Continental Daily Mail, Paris, 14.6.1947 ... reporting brain surgery 10,000 years ago\nTarget: newspaper_report_date\n<|im_end|>\n"
            "<|im_start|>assistant\n{\"status\": \"REJECT_NON_FINDING\"}\n<|im_end|>\n"
            "<|im_start|>user\nSentence: reached a basal gravel depth of about 6 m below surface\nTarget: stratum_depth\n<|im_end|>\n"
            "<|im_start|>assistant\n{\"status\": \"EXTRACT_ATTRIBUTED\", \"value\": \"6\", \"unit\": \"m\"}\n<|im_end|>\n";

        n_prefix_tokens_ = -llama_tokenize(vocab_, system_prefix.c_str(), system_prefix.size(), NULL, 0, true, true);
        std::vector<llama_token> p_tokens(n_prefix_tokens_);
        llama_tokenize(vocab_, system_prefix.c_str(), system_prefix.size(), p_tokens.data(), p_tokens.size(), true, true);

        llama_batch_ext* p_batch = llama_batch_ext_init(ctx_);
        batch_set_tokens(p_batch, p_tokens.data(), n_prefix_tokens_, 0);
        llama_process(ctx_, LLAMA_PROCESS_TYPE_DECODE, p_batch);
        llama_batch_ext_free(p_batch);

        is_initialized_ = true;
        return true;
#else
        (void)model_path;
        return false;
#endif
    }

    bool is_loaded() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return is_initialized_;
    }

    // Step 4: Disambiguate difficult clausal candidate in background ingestion
    std::string disambiguate_candidate(const std::string& sentence, const std::string& target_query, int max_tokens = 30) {
        std::lock_guard<std::mutex> lock(mutex_);
#ifdef HAS_LLAMA_CPP
        if (!is_initialized_ || !ctx_) return "";

        std::string suffix = 
            "<|im_start|>user\nSentence: " + sentence + "\nTarget: " + target_query + "\n<|im_end|>\n<|im_start|>assistant\n";

        const int n_suffix = -llama_tokenize(vocab_, suffix.c_str(), suffix.size(), NULL, 0, false, true);
        std::vector<llama_token> suffix_tokens(n_suffix);
        llama_tokenize(vocab_, suffix.c_str(), suffix.size(), suffix_tokens.data(), suffix_tokens.size(), false, true);

        llama_batch_ext* batch = llama_batch_ext_init(ctx_);
        batch_set_tokens(batch, suffix_tokens.data(), n_suffix, n_prefix_tokens_);
        llama_process(ctx_, LLAMA_PROCESS_TYPE_DECODE, batch);

        std::string response = "";
        int cur_pos = n_prefix_tokens_ + n_suffix;
        for (int step = 0; step < max_tokens; ++step) {
            llama_token tok = llama_sampler_sample(smpl_, ctx_, -1);
            if (llama_vocab_is_eog(vocab_, tok)) break;

            char buf[128];
            int n = llama_token_to_piece(vocab_, tok, buf, sizeof(buf), 0, true);
            if (n > 0) response.append(buf, n);

            batch_set_tokens(batch, &tok, 1, cur_pos);
            llama_process(ctx_, LLAMA_PROCESS_TYPE_DECODE, batch);
            cur_pos++;
        }

        llama_batch_ext_free(batch);

        // Remove suffix and generation tokens from KV cache, retaining prefix [0, n_prefix_tokens_)
        llama_memory_t mem = llama_get_memory(ctx_);
        llama_memory_seq_rm(mem, 0, n_prefix_tokens_, -1);

        return response;
#else
        (void)sentence; (void)target_query; (void)max_tokens;
        return "";
#endif
    }

    // Phase 2 Step 5 & Phase 3: Semantic Contradiction Analysis
    std::string analyze_contradiction(const std::string& claim_a, const std::string& claim_b, int max_tokens = 60) {
        std::lock_guard<std::mutex> lock(mutex_);
#ifdef HAS_LLAMA_CPP
        if (!is_initialized_ || !ctx_) return "";

        std::string prompt = 
            "<|im_start|>system\n"
            "You are an archaeological contradiction analyst. Compare Claim A and Claim B.\n"
            "Determine if they CONTRADICT, COMPLEMENT, or are UNRELATED.\n"
            "Output JSON: {\"relation\": \"CONTRADICTION\"|\"COMPATIBLE\"|\"UNRELATED\", \"reason\": \"...\"}\n"
            "<|im_end|>\n"
            "<|im_start|>user\n"
            "Claim A: " + claim_a + "\n"
            "Claim B: " + claim_b + "\n"
            "<|im_end|>\n"
            "<|im_start|>assistant\n";

        const int n_prompt = -llama_tokenize(vocab_, prompt.c_str(), prompt.size(), NULL, 0, true, true);
        std::vector<llama_token> tokens(n_prompt);
        llama_tokenize(vocab_, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);

        // Clear existing cache for standalone contradiction prompt
        llama_memory_t mem = llama_get_memory(ctx_);
        llama_memory_clear(mem, true);

        llama_batch_ext* batch = llama_batch_ext_init(ctx_);
        batch_set_tokens(batch, tokens.data(), n_prompt, 0);
        llama_process(ctx_, LLAMA_PROCESS_TYPE_DECODE, batch);

        std::string response = "";
        int cur_pos = n_prompt;
        for (int step = 0; step < max_tokens; ++step) {
            llama_token tok = llama_sampler_sample(smpl_, ctx_, -1);
            if (llama_vocab_is_eog(vocab_, tok)) break;

            char buf[128];
            int n = llama_token_to_piece(vocab_, tok, buf, sizeof(buf), 0, true);
            if (n > 0) response.append(buf, n);

            batch_set_tokens(batch, &tok, 1, cur_pos);
            llama_process(ctx_, LLAMA_PROCESS_TYPE_DECODE, batch);
            cur_pos++;
        }

        llama_batch_ext_free(batch);
        llama_memory_clear(mem, true);

        return response;
#else
        (void)claim_a; (void)claim_b; (void)max_tokens;
        return "";
#endif
    }

    // Phase 3 & 4: General Scholarly Synthesis & Conversational Reasoning
    std::string generate(const std::string& user_prompt, const std::string& system_prompt = "You are ArchaeoPhD AI, an expert archaeological research assistant. Provide concise, grounded, academically rigorous analysis based strictly on archaeological evidence.", int max_tokens = 512) {
        std::lock_guard<std::mutex> lock(mutex_);
#ifdef HAS_LLAMA_CPP
        if (!is_initialized_ || !ctx_) return "";

        std::string prompt = 
            "<|im_start|>system\n" + system_prompt + "\n<|im_end|>\n"
            "<|im_start|>user\n" + user_prompt + "\n<|im_end|>\n"
            "<|im_start|>assistant\n";

        const int n_prompt = -llama_tokenize(vocab_, prompt.c_str(), prompt.size(), NULL, 0, true, true);
        if (n_prompt <= 0) return "";
        std::vector<llama_token> tokens(n_prompt);
        llama_tokenize(vocab_, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);

        // Clear existing cache for standalone generation
        llama_memory_t mem = llama_get_memory(ctx_);
        llama_memory_clear(mem, true);

        llama_batch_ext* batch = llama_batch_ext_init(ctx_);
        batch_set_tokens(batch, tokens.data(), n_prompt, 0);
        llama_process(ctx_, LLAMA_PROCESS_TYPE_DECODE, batch);

        std::string response = "";
        int cur_pos = n_prompt;
        for (int step = 0; step < max_tokens; ++step) {
            llama_token tok = llama_sampler_sample(smpl_, ctx_, -1);
            if (llama_vocab_is_eog(vocab_, tok)) break;

            char buf[128];
            int n = llama_token_to_piece(vocab_, tok, buf, sizeof(buf), 0, true);
            if (n > 0) response.append(buf, n);

            batch_set_tokens(batch, &tok, 1, cur_pos);
            llama_process(ctx_, LLAMA_PROCESS_TYPE_DECODE, batch);
            cur_pos++;
        }

        llama_batch_ext_free(batch);
        llama_memory_clear(mem, true);

        return response;
#else
        (void)user_prompt; (void)system_prompt; (void)max_tokens;
        return "";
#endif
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
#ifdef HAS_LLAMA_CPP
        if (smpl_) {
            llama_sampler_free(smpl_);
            smpl_ = nullptr;
        }
        if (ctx_) {
            llama_free(ctx_);
            ctx_ = nullptr;
        }
        if (model_) {
            llama_model_free(model_);
            model_ = nullptr;
        }
        if (is_initialized_) {
            llama_backend_free();
        }
#endif
        is_initialized_ = false;
    }
};

} // namespace archaeophd
