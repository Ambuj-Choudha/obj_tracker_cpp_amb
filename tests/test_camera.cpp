// Tests the source stage's contract with the loop in main():
//   - a clean end of stream is not an error, and is visible via getSourceState()
//   - a failed read is Recoverable until the budget runs out, then Fatal
//   - getSourceState() tracks that escalation, because main() reads it to
//     decide between "continue" and "return -1"
//
// The state machine lives in VideoCaptureBase, so a fake subclass exercises it
// without a webcam. VideoFile is then run against a generated clip for the EOF
// path, which is the one thing the fake cannot tell us about.
//
// Run from repo root: ./build-ninja/test_camera

#include <filesystem>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "camera.hpp"
#include "test_harness.hpp"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

namespace {

constexpr const char* kFakeCause = "fake source failure";
constexpr int kFakeBudget = 2;

// Drives VideoCaptureBase's failure bookkeeping directly. It never opens a
// cv::VideoCapture, so it isolates the state machine from OpenCV entirely.
class FakeSource : public VideoCaptureBase {
    public:
        FakeSource() : VideoCaptureBase{kFakeBudget} {}

        Status::Result<Data::Frame> getNextFrame() override {
            if (fail_next_) {
                return std::unexpected(record_failure(kFakeCause, "fake source"));
            }
            record_success();
            return Data::Frame{cv::Mat::zeros(2, 2, CV_8UC3)};
        }

        void set_failing(bool failing) { fail_next_ = failing; }

    private:
        bool fail_next_{true};
};

void starts_streaming(test::Checks& checks) {
    FakeSource source;
    checks.check(source.getSourceState() == Status::SourceState::Streaming,
                 "a fresh source starts in Streaming");
}

void failures_escalate_and_move_the_state(test::Checks& checks) {
    FakeSource source;

    for (int attempt = 1; attempt <= kFakeBudget; ++attempt) {
        auto frame = source.getNextFrame();
        checks.check(!frame.has_value(), "failing read " + std::to_string(attempt) + " returns an error");
        checks.check(test::is_recoverable(frame.error()),
                     "read " + std::to_string(attempt) + " is Recoverable");
        checks.check(source.getSourceState() == Status::SourceState::DisconnectedRetrying,
                     "state is DisconnectedRetrying while retrying");
    }

    auto frame = source.getNextFrame();
    checks.check(!frame.has_value(), "the read past the budget still returns an error");
    checks.check(test::is_fatal(frame.error()), "the read past the budget is Fatal");

    // main() branches on this to decide whether to return -1, so it has to move
    // in step with the error itself.
    checks.check(source.getSourceState() == Status::SourceState::Failed,
                 "state is Failed once the budget is spent");
}

void recovery_returns_to_streaming(test::Checks& checks) {
    FakeSource source;

    source.getNextFrame();  // one failure, inside the budget
    source.set_failing(false);

    auto frame = source.getNextFrame();
    checks.check(frame.has_value(), "a good read after a failure succeeds");
    checks.check(source.getSourceState() == Status::SourceState::Streaming,
                 "state returns to Streaming after a good read");

    // The streak restarted, so the budget is available again in full.
    source.set_failing(true);
    auto after = source.getNextFrame();
    checks.check(test::is_recoverable(after.error()),
                 "the budget is whole again after recovery");
}

// Writes a short clip so the EOF path is exercised against a real decoder
// rather than a stub. Returns an empty path if no encoder is available.
std::string make_clip(const std::filesystem::path& path, int frames) {
    cv::VideoWriter writer{path.string(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10.0, cv::Size{64, 64}};
    if (!writer.isOpened()) return {};

    for (int i = 0; i < frames; ++i) {
        writer.write(cv::Mat::zeros(64, 64, CV_8UC3));
    }
    writer.release();
    return path.string();
}

void end_of_stream_is_not_an_error(test::Checks& checks) {
    const auto dir = std::filesystem::temp_directory_path();
    const std::string clip = make_clip(dir / "obj_tracker_eof_test.avi", 5);
    if (clip.empty()) {
        std::cout << "  skip no MJPG encoder available; EOF path not covered\n";
        return;
    }

    VideoFile source{clip};

    int decoded = 0;
    // Bounded so a decoder that never reports EOF fails the test instead of
    // hanging it.
    for (int i = 0; i < 50; ++i) {
        auto frame = source.getNextFrame();
        if (source.getSourceState() == Status::SourceState::EndOfStream) break;
        if (!frame.has_value()) {
            checks.check(false, "unexpected error mid-clip: " + test::describe(frame.error()));
            break;
        }
        ++decoded;
    }

    checks.check_eq(decoded, 5, "every written frame is decoded before EOF");
    checks.check(source.getSourceState() == Status::SourceState::EndOfStream,
                 "running out of frames sets EndOfStream");

    // main() checks the state before unwrapping, so EOF must not also raise an
    // error - that would report a normal shutdown as a failure.
    auto past_eof = source.getNextFrame();
    checks.check(past_eof.has_value(), "reading past EOF is not an error");
    checks.check(source.getSourceState() == Status::SourceState::EndOfStream,
                 "EndOfStream is sticky");

    std::filesystem::remove(clip);
}

void missing_file_is_fatal_at_construction(test::Checks& checks) {
    // Taxonomy: a source that cannot be opened at all has no retry story, so it
    // throws from the constructor rather than returning an error per frame.
    try {
        VideoFile source{std::string{PROJECT_ROOT} + "/data/input/does_not_exist.mp4"};
        checks.check(false, "expected a FatalException for a missing video file");
    } catch (const Status::FatalException& e) {
        checks.check(e.error().origin == Status::Stage::Source,
                     "a missing video file is Fatal[Source]");
    }
}

}  // namespace

int main() {
    test::Checks checks{"test_camera"};

    starts_streaming(checks);
    failures_escalate_and_move_the_state(checks);
    recovery_returns_to_streaming(checks);
    end_of_stream_is_not_an_error(checks);
    missing_file_is_fatal_at_construction(checks);

    return checks.report();
}
