#include "reporting.hpp"

#include <iostream>
#include <variant>

bool ConsoleReporter::start_new_warning_interval_(Status::Stage origin) {
    const auto now = Clock::now();
    auto& last_warned = last_recoverable_.at(static_cast<std::size_t>(origin));

    const bool first_warning = (last_warned == Clock::time_point{});
    const bool interval_elapsed = ((now - last_warned) >= ReporterDefaults::recoverable_min_interval);

    if (!first_warning && !interval_elapsed) {
        return false;
    }

    last_warned = now;
    return true;
}

void ConsoleReporter::report(const Status::Error& error) {
    if (const auto* fatal = std::get_if<Status::Fatal>(&error)) {  // get_if() returns a nullptr if the error is not Fatal
        std::cerr << "[FATAL] [" << Status::stage_name(fatal->origin) << "] " << fatal->cause << '\n';
        return;
    }

    // Dropped error report if the minimal interval is not elapsed, otherwise print
    const auto& recoverable = std::get<Status::Recoverable>(error);
    if (!start_new_warning_interval_(recoverable.origin)) {
        return;
    }
    std::cerr << "[WARN] [" << Status::stage_name(recoverable.origin) << "] " << recoverable.cause
              << " (attempt " << recoverable.attempt_count << ")\n";
}
