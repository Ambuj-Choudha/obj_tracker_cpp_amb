#pragma once

#include <chrono>

// Adds the elapsed wall time of its scope into `accumulator` on destruction.
// Used to build cumulative per-stage timing totals (Phase 2 baseline
// instrumentation) without threading a profiler object through call sites -
// each stage just times itself into a duration it already owns.
namespace Timing {

class ScopedAccumulator {
    public:
        explicit ScopedAccumulator(std::chrono::steady_clock::duration& accumulator) noexcept
            : accumulator_{accumulator}, start_{std::chrono::steady_clock::now()} {}

        ~ScopedAccumulator() { accumulator_ += std::chrono::steady_clock::now() - start_; }

        ScopedAccumulator(const ScopedAccumulator&) = delete;
        ScopedAccumulator& operator=(const ScopedAccumulator&) = delete;

    private:
        std::chrono::steady_clock::duration& accumulator_;
        std::chrono::steady_clock::time_point start_;
};

}
