#pragma once
#include <cstddef>
#include <expected>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace Status {

// One entry per pipeline stage that can fail on its own.
enum class Stage { Source, Preprocess, Inference, Postprocess, Tracking, Visualization };

// Update the last enumerator here if you add a stage after Visualization.
inline constexpr std::size_t stage_count = static_cast<std::size_t>(Stage::Visualization) + 1;

inline constexpr std::string_view stage_name(Stage stage) {
    switch (stage) {
        case Stage::Source:        return "Source";
        case Stage::Preprocess:    return "Preprocess";
        case Stage::Inference:     return "Inference";
        case Stage::Postprocess:   return "Postprocess";
        case Stage::Tracking:      return "Tracking";
        case Stage::Visualization: return "Visualization";
    }
    return "Unknown";
}

struct Fatal {
    Stage origin;
    std::string cause;  // a Fatal happens once, so the allocation is fine
};

struct Recoverable {
    Stage origin;
    std::string_view cause;  // avoid re-allocation during retries
    int attempt_count;  // no. of attempts for turning a recoverable error to fatal
};

using Error = std::variant<Fatal, Recoverable>;  // std::variant for stack allocation

template <typename T>
using Result = std::expected<T, Error>;

// Carries a Fatal, just thrown instead of returned: constructors throw
// (nothing to return from), the loop returns.
class FatalException : public std::runtime_error {
public:
    explicit FatalException(Fatal fatal)
        : std::runtime_error{fatal.cause}, error_{std::move(fatal)} {}

    [[nodiscard]] const Fatal& error() const noexcept { return error_; }

   private:
    Fatal error_;
};

enum class SourceState { Streaming, EndOfStream, DisconnectedRetrying, Failed };

}
