// Tests RetryMonitor: the Recoverable -> Fatal escalation rule.
//
// This is the one piece of the error model every converted component shares,
// so an off-by-one here silently changes how long the camera, the tracker and
// anything converted later keep retrying. Needs no hardware and no model.
//
// Run from repo root: ./build-ninja/test_retry_monitor

#include "common/retry_monitor.hpp"

#include "test_harness.hpp"

namespace {

constexpr const char* kCause = "test failure";
constexpr int kBudget = 3;

// Every Recoverable::cause must be a literal, so the view it holds outlives the
// error. If that ever regresses, the view dangles and this comparison is the
// cheapest place to notice.
void cause_is_stable(test::Checks& checks) {
    RetryMonitor monitor{Status::Stage::Source, kBudget};
    const auto error = monitor.record_failure(kCause, "operation");
    const auto& recoverable = std::get<Status::Recoverable>(error);
    checks.check(recoverable.cause == kCause, "Recoverable::cause points at the literal it was given");
}

void escalates_after_budget(test::Checks& checks) {
    RetryMonitor monitor{Status::Stage::Source, kBudget};

    // Failures 1..budget stay Recoverable and count up.
    for (int attempt = 1; attempt <= kBudget; ++attempt) {
        const auto error = monitor.record_failure(kCause, "operation");
        checks.check(test::is_recoverable(error),
                     "failure " + std::to_string(attempt) + " of " + std::to_string(kBudget) + " is Recoverable");
        if (test::is_recoverable(error)) {
            checks.check_eq(std::get<Status::Recoverable>(error).attempt_count, attempt,
                            "attempt_count tracks the streak");
        }
        checks.check(!monitor.exhausted(), "not exhausted while inside the budget");
    }

    // The first failure past the budget escalates.
    const auto error = monitor.record_failure(kCause, "operation");
    checks.check(test::is_fatal(error), "the failure after the budget is Fatal");
    checks.check(monitor.exhausted(), "exhausted() is true once escalated");
}

void success_resets_the_streak(test::Checks& checks) {
    RetryMonitor monitor{Status::Stage::Source, kBudget};

    monitor.record_failure(kCause, "operation");
    monitor.record_failure(kCause, "operation");
    checks.check_eq(monitor.consecutive_failures(), 2, "two failures counted");

    monitor.record_success();
    checks.check_eq(monitor.consecutive_failures(), 0, "record_success() clears the count");

    // The point of "consecutive": an intermittent fault must not accumulate
    // across recoveries into a spurious Fatal.
    const auto error = monitor.record_failure(kCause, "operation");
    checks.check(test::is_recoverable(error), "a failure after recovery is Recoverable again");
    if (test::is_recoverable(error)) {
        checks.check_eq(std::get<Status::Recoverable>(error).attempt_count, 1,
                        "the streak restarts at 1, it does not resume");
    }
}

void origin_comes_from_the_monitor(test::Checks& checks) {
    RetryMonitor monitor{Status::Stage::Tracking, kBudget};

    const auto recoverable = monitor.record_failure(kCause, "operation");
    checks.check(std::get<Status::Recoverable>(recoverable).origin == Status::Stage::Tracking,
                 "Recoverable carries the stage it was constructed with");

    for (int i = 0; i < kBudget; ++i) monitor.record_failure(kCause, "operation");
    const auto fatal = monitor.record_failure(kCause, "operation");
    checks.check(std::get<Status::Fatal>(fatal).origin == Status::Stage::Tracking,
                 "Fatal carries the same stage");
}

// A budget of 0 means "no retries": the first failure is already fatal. Worth
// pinning because it is the boundary a caller is most likely to configure by
// accident, and >= vs > in the monitor decides it.
void zero_budget_is_immediately_fatal(test::Checks& checks) {
    RetryMonitor monitor{Status::Stage::Source, 0};
    checks.check(test::is_fatal(monitor.record_failure(kCause, "operation")),
                 "a budget of 0 makes the first failure Fatal");
}

}  // namespace

int main() {
    test::Checks checks{"test_retry_monitor"};

    cause_is_stable(checks);
    escalates_after_budget(checks);
    success_resets_the_streak(checks);
    origin_comes_from_the_monitor(checks);
    zero_budget_is_immediately_fatal(checks);

    return checks.report();
}
