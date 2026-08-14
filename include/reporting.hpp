#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <string_view>

#include "common/status.hpp"

struct ReporterDefaults {
    // Minimum gap between warnings from the same stage.
    static constexpr std::chrono::milliseconds recoverable_min_interval{1000};
};

// The interface exists so a DiagnosticsReporter can replace ConsoleReporter later on
class IErrorReporter {
    public:
        IErrorReporter() = default;
        IErrorReporter(const IErrorReporter&) = delete;
        IErrorReporter& operator=(const IErrorReporter&) = delete;
        IErrorReporter(IErrorReporter&&) = delete;
        IErrorReporter& operator=(IErrorReporter&&) = delete;
        virtual ~IErrorReporter() = default;

        virtual void report(const Status::Error& error) = 0;
};

class ConsoleReporter : public IErrorReporter {
    public:
        void report(const Status::Error& error) override;

    private:
        using Clock = std::chrono::steady_clock;

        // Per-stage, not global, so a noisy camera doesn't hide a tracker warning in the same second.
        std::array<Clock::time_point, Status::stage_count> last_recoverable_{};

        bool start_new_warning_interval_(Status::Stage origin);
};
