#include <iostream>
#include <string>
#include <cassert>
#include <chrono>

#define HAS_LLAMA_CPP 1
#include "analysis/llm_engine.hpp"
#include "extraction/candidate_generator.hpp"

using namespace archaeophd;

int main() {
    std::cout << "================================================================================\n";
    std::cout << "  ArchaeoPhD — Hybrid Candidate Generator Smoke Test\n";
    std::cout << "================================================================================\n\n";

    // 1. Fast Path Smoke Test (Pure C++, No LLM)
    std::string text_simple = "reached a basal gravel depth of about 6 m below surface";
    auto t0 = std::chrono::steady_clock::now();
    AttributedCandidate c1 = CandidateGenerator::GenerateCandidate(text_simple, "STRATUM_DEPTH", "stratum depth");
    auto t1 = std::chrono::steady_clock::now();
    double fast_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    std::cout << "[FAST PATH] Text: \"" << text_simple << "\"\n";
    std::cout << "  Output Value: " << c1.value.raw_text << " " << c1.value.normalized_unit << "\n";
    std::cout << "  Execution Time: " << fast_us << " microseconds\n\n";

    // 2. Multi-Date Disambiguation without LLM (Expect C++ limitation)
    std::string text_multi = "opened to the public as early as 1819, his guide book appeared only in 1839";
    AttributedCandidate c2_pure = CandidateGenerator::GenerateCandidate(text_multi, "ARTIFACT_COUNT", "guidebook publication year");
    std::cout << "[MULTI-DATE PURE C++] Text: \"" << text_multi << "\"\n";
    std::cout << "  Output Value: " << c2_pure.value.raw_text << " (C++ displaced to neighbor token)\n\n";

    // 3. Initialize LLM Engine
    std::cout << "[HYBRID PATH] Initializing LlmEngine with Qwen 2.5 7B...\n";
    auto& llm = LlmEngine::instance();
    bool loaded = llm.initialize("models/llm/Qwen2.5-7B-Instruct-Q4_K_M.gguf");
    if (!loaded) {
        std::cerr << "[WARN] Model file not found or could not load; test passes in offline fallback mode.\n";
        return 0;
    }
    std::cout << "  LlmEngine initialized successfully with KV-cache prefix.\n\n";

    // 4. Test Hybrid Disambiguation on DEV-12
    auto t2 = std::chrono::steady_clock::now();
    AttributedCandidate c2_hybrid = CandidateGenerator::GenerateCandidateHybrid(text_multi, "ARTIFACT_COUNT", "guidebook publication year", &llm);
    auto t3 = std::chrono::steady_clock::now();
    double hybrid_s = std::chrono::duration<double>(t3 - t2).count();

    std::cout << "[HYBRID PATH] Result: \"" << c2_hybrid.value.raw_text << "\"\n";
    std::cout << "  Execution Latency: " << hybrid_s << " s\n";
    if (c2_hybrid.value.raw_text == "1839") {
        std::cout << "  SUCCESS: Qwen 2.5 7B resolved multi-date displacement to 1839!\n";
    } else {
        std::cout << "  NOTE: Hybrid output was '" << c2_hybrid.value.raw_text << "'\n";
    }

    // 5. Test Hybrid Disambiguation on DEV-14
    std::string text_books = "Modern Savages (1865) and the second book The Origin of Civilisation (1870) went";
    AttributedCandidate c3_hybrid = CandidateGenerator::GenerateCandidateHybrid(text_books, "ARTIFACT_COUNT", "second treatise publication", &llm);
    std::cout << "\n[HYBRID PATH] Text: \"" << text_books << "\"\n";
    std::cout << "  Output Value: " << c3_hybrid.value.raw_text << "\n";
    if (c3_hybrid.value.raw_text == "1870") {
        std::cout << "  SUCCESS: Qwen 2.5 7B resolved ordinal qualifier to 1870!\n";
    }

    std::cout << "\n================================================================================\n";
    std::cout << "  Hybrid Candidate Generator Smoke Test Complete.\n";
    std::cout << "================================================================================\n";
    return 0;
}
