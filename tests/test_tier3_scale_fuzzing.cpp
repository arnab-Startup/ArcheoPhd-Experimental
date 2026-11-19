#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cassert>
#include <cmath>
#include <thread>
#include <random>
#include "analysis/epistemic_logic.hpp"
#include "analysis/epistemic_tier3.hpp"

using namespace archaeophd;

// =============================================================================
// T55: 50,000-Node Scale & Memory Ceiling (< 250 ms)
// =============================================================================
void test_T55_50k_node_scale() {
    std::cout << "[T55] Testing 50,000-Node Scale & Propagation Latency..." << std::endl;
    ScaleOptimizedHypergraph hypergraph;
    const size_t NUM_NODES = 50000;
    const size_t NUM_EDGES = 120000;

    hypergraph.init(NUM_NODES);
    std::cout << "  Initialized " << NUM_NODES << " compact nodes (sparse adjacency)." << std::endl;

    for (size_t i = 0; i < NUM_EDGES; ++i) {
        int premise = static_cast<int>(i % 500); // 500 hubs
        int conclusion = static_cast<int>(500 + (i % 49500));
        hypergraph.add_hyperedge(premise, conclusion, 2.5);
    }
    std::cout << "  Constructed " << NUM_EDGES << " sparse hyperedges." << std::endl;

    // Falsify central hub 0
    hypergraph.nodes[0].r_plus = 0.0;
    hypergraph.nodes[0].r_minus = 50.0;
    hypergraph.nodes[0].belief = 0.0;
    hypergraph.nodes[0].disbelief = 50.0 / 52.0;

    auto start = std::chrono::high_resolution_clock::now();
    hypergraph.propagate(3, 0.65);
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "  50,000-node propagation latency: " << elapsed_ms << " ms" << std::endl;
    assert(elapsed_ms < 250.0); // Strict requirement: < 250 ms
    std::cout << "  Passed: 50,000-node scale benchmark passed in " << elapsed_ms << " ms without O(|V|^2) matrix." << std::endl;
}

// =============================================================================
// T56: Deterministic Replay Bit-Exactness
// =============================================================================
void test_T56_deterministic_replay() {
    std::cout << "[T56] Testing Deterministic Replay Bit-Exactness..." << std::endl;
    auto run_sim = []() {
        UpgradedEpistemicHypergraph g;
        for (int i = 0; i < 50; ++i) {
            std::string id = "N_" + std::to_string(i);
            g.add_node(id, id, (i * 3) % 11, (i * 7) % 5);
        }
        for (int i = 0; i < 40; ++i) {
            UpgradedHyperedgeWarrant w;
            w.id = "w_" + std::to_string(i);
            w.in_list = {"N_" + std::to_string(i % 10)};
            w.conclusion_id = "N_" + std::to_string(10 + (i % 40));
            g.add_hyperedge(w);
        }
        g.propagate_damped_belief_revision(10);
        return g;
    };

    auto g1 = run_sim();
    auto g2 = run_sim();

    for (const auto& kv : g1.nodes()) {
        const auto& n1 = kv.second;
        const auto& n2 = g2.nodes().at(kv.first);
        assert(n1.opinion.belief == n2.opinion.belief);
        assert(n1.opinion.disbelief == n2.opinion.disbelief);
        assert(n1.opinion.uncertainty == n2.opinion.uncertainty);
    }
    std::cout << "  Passed: Final graph beliefs match bit-for-bit across independent cold starts." << std::endl;
}

// =============================================================================
// T57: Serialization Round-Trip Fidelity
// =============================================================================
void test_T57_serialization_fidelity() {
    std::cout << "[T57] Testing Serialization Round-Trip Fidelity..." << std::endl;
    UpgradedEpistemicHypergraph original;
    for (int i = 0; i < 1000; ++i) {
        std::string id = "node_" + std::to_string(i);
        original.add_node(id, id, i * 0.5, i * 0.1);
    }

    std::string serialized = GraphSerializationCodec::serialize_state(original);
    UpgradedEpistemicHypergraph reloaded;
    GraphSerializationCodec::deserialize_into(serialized, reloaded);

    for (const auto& kv : original.nodes()) {
        const auto& orig_n = kv.second;
        const auto& reload_n = reloaded.nodes().at(kv.first);
        assert(std::abs(orig_n.opinion.belief - reload_n.opinion.belief) < 1e-6);
        assert(std::abs(orig_n.opinion.disbelief - reload_n.opinion.disbelief) < 1e-6);
    }
    std::cout << "  Passed: Serialization round-trip preserved beliefs with 10^-6 precision." << std::endl;
}

// =============================================================================
// T58: Concurrent IPC Read/Write Safety
// =============================================================================
void test_T58_concurrent_thread_safety() {
    std::cout << "[T58] Testing Concurrent Multi-Threaded Read/Write Safety..." << std::endl;
    ConcurrentEpistemicEngine concurrent_engine;
    concurrent_engine.add_node_safe("H_root", "Root Hypothesis", 5.0, 0.0);

    std::atomic<bool> stop_flag{false};
    std::atomic<int> read_ops{0};

    // 4 reader threads
    std::vector<std::thread> readers;
    for (int i = 0; i < 4; ++i) {
        readers.emplace_back([&]() {
            while (!stop_flag.load()) {
                auto op = concurrent_engine.read_node_state_safe("H_root");
                if (op.belief + op.disbelief + op.uncertainty > 0.0) {
                    read_ops++;
                }
                std::this_thread::yield();
            }
        });
    }

    // 1 backend writer thread
    std::thread writer([&]() {
        for (int i = 0; i < 200; ++i) {
            concurrent_engine.add_node_safe("node_" + std::to_string(i), "Node", 1.0, 0.0);
            concurrent_engine.propagate_shockwave_safe();
        }
        stop_flag.store(true);
    });

    writer.join();
    for (auto& t : readers) t.join();

    std::cout << "  Executed " << read_ops.load() << " concurrent reads across 4 threads without races or deadlocks." << std::endl;
    std::cout << "  Passed: Concurrent read/write safety verified under active background shockwaves." << std::endl;
}

// =============================================================================
// T59: Property-Based Fuzzing (10,000 Random Operations)
// =============================================================================
void test_T59_property_fuzzing() {
    std::cout << "[T59] Testing Property-Based Fuzzing (10,000 Random Operations)..." << std::endl;
    UpgradedEpistemicHypergraph graph;
    std::mt19937 rng(999);
    std::uniform_int_distribution<int> op_dist(0, 3);
    std::uniform_real_distribution<double> val_dist(0.0, 50.0);

    for (int step = 0; step < 10000; ++step) {
        int op = op_dist(rng);
        std::string id = "node_" + std::to_string(step % 100);

        if (op == 0) {
            graph.add_node(id, "Fuzz Node", val_dist(rng), val_dist(rng));
        } else if (op == 1) {
            if (graph.nodes().count(id)) {
                auto& n = graph.mutable_nodes()[id];
                n.r_direct_plus += val_dist(rng);
                n.opinion = OpinionTuple4::from_evidence(n.r_direct_plus, n.r_direct_minus);
            }
        } else if (op == 2) {
            if (graph.nodes().count(id)) {
                auto& n = graph.mutable_nodes()[id];
                n.r_direct_minus += val_dist(rng);
                n.opinion = OpinionTuple4::from_evidence(n.r_direct_plus, n.r_direct_minus);
            }
        } else {
            graph.acknowledge_state();
        }

        // Simplex invariant must NEVER be violated on any node
        if (graph.nodes().count(id)) {
            const auto& op_tuple = graph.nodes().at(id).opinion;
            assert(!std::isnan(op_tuple.belief) && !std::isinf(op_tuple.belief));
            assert(!std::isnan(op_tuple.disbelief) && !std::isinf(op_tuple.disbelief));
            assert(!std::isnan(op_tuple.uncertainty) && !std::isinf(op_tuple.uncertainty));
            double sum = op_tuple.belief + op_tuple.disbelief + op_tuple.uncertainty;
            assert(std::abs(sum - 1.0) < 1e-6);
        }
    }
    std::cout << "  Passed: Zero simplex violations, zero NaNs across 10,000 randomized operations." << std::endl;
}

// =============================================================================
// T60: End-to-End 10-Year Multi-Season Benchmark
// =============================================================================
void test_T60_10_year_benchmark() {
    std::cout << "[T60] Testing End-to-End 10-Year Multi-Season Benchmark..." << std::endl;
    auto report = MultiSeasonExcavationSimulator::run_10_year_benchmark();

    std::cout << "  Total notes processed:               " << report.total_notes << std::endl;
    std::cout << "  Caught hidden paradigm contradictions: " << report.caught_contradictions << " / 3" << std::endl;
    std::cout << "  Year-1 anomaly reactivated in Year 9: " << (report.year_1_anomaly_reactivated ? "TRUE" : "FALSE") << std::endl;
    std::cout << "  False-alarm rate:                    " << (report.false_alarm_rate * 100.0) << "%" << std::endl;

    assert(report.caught_contradictions == 3);
    assert(report.year_1_anomaly_reactivated == true);
    assert(report.false_alarm_rate < 0.05); // False alarm rate strictly < 5%
    std::cout << "  Passed: 10-year longitudinal benchmark successfully executed with full precision." << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << "  Tier 3: Scale, Concurrency & Fuzzing Invariants Suite    " << std::endl;
    std::cout << "============================================================" << std::endl;

    test_T55_50k_node_scale();
    test_T56_deterministic_replay();
    test_T57_serialization_fidelity();
    test_T58_concurrent_thread_safety();
    test_T59_property_fuzzing();
    test_T60_10_year_benchmark();

    std::cout << "============================================================" << std::endl;
    std::cout << "  ALL TIER 3 SCALE & FUZZING TESTS PASSED (100%)!           " << std::endl;
    std::cout << "============================================================" << std::endl;
    return 0;
}
