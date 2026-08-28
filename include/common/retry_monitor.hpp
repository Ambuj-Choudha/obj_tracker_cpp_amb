#pragma once

#include <format>
#include <string_view>

#include "status.hpp"

class RetryMonitor {
    public:
        RetryMonitor(Status::Stage stage, int budget) noexcept
            : stage_{stage}, budget_{budget} {}

        Status::Error record_failure(const char* message, std::string_view operation) {
            ++consecutive_failures_;

            if (consecutive_failures_ > budget_) {
              return Status::Fatal{
                  .origin = stage_,
                  .cause = std::format(
                      "{} unrecoverable after {} consecutive failures",
                      operation, consecutive_failures_)};
            }

            return Status::Recoverable{.origin = stage_,
                                       .cause = message,
                                       .attempt_count = consecutive_failures_};
        }

        // Reset failure count
        void record_success() noexcept { consecutive_failures_ = 0; }

        [[nodiscard]] int consecutive_failures() const noexcept {
          return consecutive_failures_;
        }
        [[nodiscard]] bool exhausted() const noexcept {
          return consecutive_failures_ > budget_;
        }

    private:
        Status::Stage stage_;
        int budget_;
        int consecutive_failures_{0};
};
