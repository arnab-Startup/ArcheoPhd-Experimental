#pragma once

#include "storage.hpp"

namespace archaeophd {

// Production Clean Invariant: No static test or benchmark data embedded in released builds.
inline void seed_benchmark_corpus(NativeStorage& storage, const std::string& project_id = "default") {
    (void)storage;
    (void)project_id;
    // No-op in production release: workspace starts completely clean.
}

} // namespace archaeophd
