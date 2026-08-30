#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <variant>

#include "camera.hpp"
#include "common/scoped_timer.hpp"
#include "common/status.hpp"
#include "common/types.hpp"
#include "detector.hpp"
#include "reporting.hpp"
#include "tracker.hpp"
#include "visualization.hpp"

namespace {

struct PerfSummary {
    using Clock = std::chrono::steady_clock;

    const YOLOv10DetectorONNX* detector{nullptr};
    Clock::duration capture{};
    Clock::duration track{};
    Clock::duration draw{};
    long long frame_count{0};

    ~PerfSummary() {
        if (frame_count == 0 || !detector) {
            std::cout << "No frames processed; nothing to report.\n";
            return;
        }

        auto ms_per_frame = [this](Clock::duration total) {
            return std::chrono::duration<double, std::milli>(total).count() /
                   static_cast<double>(frame_count);
        };

        std::cout << std::format("\n--- Stage timing (avg ms/frame over {} frames) ---\n", frame_count);
        std::cout << std::format("  {:<12} {:8.3f} ms\n", Status::stage_name(Status::Stage::Source), ms_per_frame(capture));
        std::cout << std::format("  {:<12} {:8.3f} ms\n", Status::stage_name(Status::Stage::Preprocess), ms_per_frame(detector->preprocess_time()));
        std::cout << std::format("  {:<12} {:8.3f} ms\n", Status::stage_name(Status::Stage::Inference), ms_per_frame(detector->inference_time()));
        std::cout << std::format("  {:<12} {:8.3f} ms\n", Status::stage_name(Status::Stage::Postprocess), ms_per_frame(detector->postprocess_time()));
        std::cout << std::format("  {:<12} {:8.3f} ms\n", Status::stage_name(Status::Stage::Tracking), ms_per_frame(track));
        std::cout << std::format("  {:<12} {:8.3f} ms\n", Status::stage_name(Status::Stage::Visualization), ms_per_frame(draw));

        const auto total = capture + detector->preprocess_time() + detector->inference_time()
                          + detector->postprocess_time() + track + draw;
        const double total_ms = ms_per_frame(total);
        std::cout << std::format("  {:<12} {:8.3f} ms\n", "Total", total_ms);
        std::cout << std::format("Approx throughput: {:.2f} FPS\n", total_ms > 0.0 ? 1000.0 / total_ms : 0.0);
    }
};

}  // namespace

int main(int argc, char* argv[]) {
    std::unique_ptr<IInputSource> source;  // use pointer from base class
    ConsoleReporter reporter;

    if (argc > 2) {
        std::cout << "Wrong number of arguments passed!" << "\n";
        std::cout << "Use <program name> <source_video_file> OR simply <program name> for webcam..." << "\n";
        return -1;
    }

    try {
        using namespace std::string_literals;
        std::string model_path = "assets/model/yolov10n/yolov10n.onnx"s;

        if (argc < 2) {
            source = std::make_unique<WebcamCamera>();
        } else {
            source = std::make_unique<VideoFile>(argv[1]);
        }

        auto detector = YOLOv10DetectorONNX(model_path);
        auto tracker = ByteTrackerAdapter{};
        auto visualizer_obj = Visualizer(2);

        PerfSummary perf;
        perf.detector = &detector;

        while (true) {
            int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q' || key == 27) {
                break;
            }

            Status::Result<Data::Frame> frame;
            {
                Timing::ScopedAccumulator timer{perf.capture};
                frame = source->getNextFrame();
            }

            // EOF: Check before unwrapping
            if (source->getSourceState() == Status::SourceState::EndOfStream) {
                std::cout << "End of stream reached.\n";
                break;
            }

            if (!frame) {
                reporter.report(frame.error());

                if (source->getSourceState() == Status::SourceState::Failed) {
                    return -1;
                }
                continue;  // Recoverable: drop this frame, keep looping
            }

            Data::Frame input_frame = *frame;

            auto detections_in_current_frame = detector.detect(input_frame);
            if (!detections_in_current_frame) {
                reporter.report(detections_in_current_frame.error());

                // The detector spent its retry budget and returned Fatal.
                if (std::holds_alternative<Status::Fatal>(detections_in_current_frame.error())) {
                    return -1;
                }
                continue;
            }

            Status::Result<std::vector<Data::TrackedDetection>> tracked_in_current_frame;
            {
                Timing::ScopedAccumulator timer{perf.track};
                tracked_in_current_frame = tracker.update(*detections_in_current_frame);
            }
            if (!tracked_in_current_frame) {
                reporter.report(tracked_in_current_frame.error());

                if (std::holds_alternative<Status::Fatal>(tracked_in_current_frame.error())) {
                    return -1;
                }
                continue;  // one bad solve: drop this frame's tracks, keep looping
            }

            ++perf.frame_count;
            {
                Timing::ScopedAccumulator timer{perf.draw};
                visualizer_obj.draw_tracked_detections(input_frame, *tracked_in_current_frame);
                cv::imshow("Detected Objects", input_frame.mat);
            }
        }
    } catch (const Status::FatalException& e) {
        reporter.report(e.error());
        return -1;
    } catch (const std::exception& e) {
        // catches third-party (ByteTrack) and stdlib exceptions
        std::cout << "Fatal exception: " << e.what() << "\n";
        return -1;
    }
    return 0;
}
