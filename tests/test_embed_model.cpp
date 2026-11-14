#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include "llama.h"

std::vector<float> embed_text_matryoshka(
    llama_context* ctx,
    const llama_vocab* vocab,
    const std::string& text,
    int target_dim = 128
) {
    std::vector<llama_token> tokens(text.size() + 32);
    int n_tokens = llama_tokenize(vocab, text.c_str(), text.size(), tokens.data(), tokens.size(), true, false);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, text.c_str(), text.size(), tokens.data(), tokens.size(), true, false);
    }
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_get_one(tokens.data(), n_tokens);
    if (llama_encode(ctx, batch) != 0) {
        std::cerr << "llama_encode failed!" << std::endl;
        return {};
    }

    float* full_embd = llama_get_embeddings_seq(ctx, 0);
    if (!full_embd) full_embd = llama_get_embeddings(ctx);
    if (!full_embd) full_embd = llama_get_embeddings_ith(ctx, -1);
    if (!full_embd) return {};

    // 1. Matryoshka truncation to target_dim (e.g. 128)
    std::vector<float> vec(target_dim);
    for (int i = 0; i < target_dim; ++i) {
        vec[i] = full_embd[i];
    }

    // 2. Guarded L2 re-normalization
    float norm_sq = 0.0f;
    for (float v : vec) norm_sq += v * v;
    if (norm_sq > 0.0f && std::abs(norm_sq - 1.0f) > 1e-6f) {
        float norm = std::sqrt(norm_sq);
        for (float& v : vec) v /= norm;
    }

    return vec;
}

float cosine_sim(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    float dot = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
    }
    return dot;
}

int main() {
    std::string model_path = "models/embedding/nomic-embed-text-v1.5.Q4_K_M.gguf";
    llama_backend_init();

    // Disable verbose logging for clean output
    llama_log_set([](enum ggml_log_level level, const char * text, void * user_data) {
        if (level >= GGML_LOG_LEVEL_ERROR) {
            std::cerr << text;
        }
    }, nullptr);

    llama_model_params mparams = llama_model_default_params();
    llama_model* model = llama_model_load_from_file(model_path.c_str(), mparams);
    if (!model) return 1;

    llama_context_params cparams = llama_context_default_params();
    cparams.embeddings = true;
    cparams.n_threads = 4;
    cparams.n_ctx = 2048;

    llama_context* ctx = llama_init_from_model(model, cparams);
    if (!ctx) return 1;

    const llama_vocab* vocab = llama_model_get_vocab(model);

    std::string doc1 = "search_document: Acheulian basalt handaxes excavated from the boulder gravel stratum at Chirki-on-Pravara.";
    std::string doc2 = "search_document: Late Bronze Age ceramic bichrome ware and destruction layer stratigraphy at Jericho Tell es-Sultan.";
    std::string query = "search_query: Chirki basalt bifaces and boulder bed tools";

    auto v_doc1 = embed_text_matryoshka(ctx, vocab, doc1, 128);
    auto v_doc2 = embed_text_matryoshka(ctx, vocab, doc2, 128);
    auto v_query = embed_text_matryoshka(ctx, vocab, query, 128);

    float sim1 = cosine_sim(v_query, v_doc1);
    float sim2 = cosine_sim(v_query, v_doc2);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Query: \"" << query << "\"\n";
    std::cout << "Sim with Chirki (Doc 1):  " << sim1 << "\n";
    std::cout << "Sim with Jericho (Doc 2): " << sim2 << "\n";
    std::cout << "Separation delta:         " << (sim1 - sim2) << "\n";

    bool pass = (sim1 > sim2 && sim1 > 0.65f);
    std::cout << "Matryoshka 128-dim Semantic Test: " << (pass ? "[PASS]" : "[FAIL]") << "\n";

    llama_free(ctx);
    llama_model_free(model);
    llama_backend_free();

    return pass ? 0 : 1;
}
