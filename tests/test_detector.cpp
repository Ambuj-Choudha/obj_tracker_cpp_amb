// Smoke test for YOLOv10DetectorONNX on a known image (horse.jpg).
// Verifies: model loads, detect() returns at least one detection, and at
// least one detection is class 17 (horse in COCO) with sane bbox coords.
//
// Run from repo root: ./build-ninja/test_detector

#include <iostream>
#include <string>

#include <opencv2/imgcodecs.hpp>

#include "common/types.hpp"
#include "detector.hpp"
#include "test_harness.hpp"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

namespace {

// COCO class ids used by yolov10n
constexpr int COCO_HORSE = 17;

int fail(const std::string& msg) {
    std::cerr << "FAIL: " << msg << "\n";
    return 1;
}

}  // namespace

int main() {
    const std::string model_path = std::string{PROJECT_ROOT} + "/assets/model/yolov10n/yolov10n.onnx";
    const std::string image_path = std::string{PROJECT_ROOT} + "/data/input/horse.jpg";

    cv::Mat img = cv::imread(image_path);
    if (img.empty()) return fail("could not load image: " + image_path);

    YOLOv10DetectorONNX detector{model_path};

    Data::Frame frame{img};
    auto result = detector.detect(frame);

    if (!result) return fail("detect() failed - " + test::describe(result.error()));

    const auto& detections = *result;

    std::cout << "detections=" << detections.size() << "\n";
    if (detections.empty()) return fail("expected at least one detection, got 0");

    bool horse_seen = false;
    for (const auto& det : detections) {
        std::cout << "  class=" << det.class_id
                  << " conf=" << det.confidence_score
                  << " bbox=(" << det.bbox.x1 << "," << det.bbox.y1
                  << "," << det.bbox.x2 << "," << det.bbox.y2 << ")\n";

        if (det.bbox.x1 >= det.bbox.x2 || det.bbox.y1 >= det.bbox.y2) {
            return fail("degenerate bbox emitted");
        }
        if (det.confidence_score < 0.0 || det.confidence_score > 1.0) {
            return fail("confidence outside [0,1]");
        }
        if (det.class_id == COCO_HORSE) horse_seen = true;
    }

    if (!horse_seen) return fail("expected a horse (class 17) in horse.jpg");

    std::cout << "PASS\n";
    return 0;
}
