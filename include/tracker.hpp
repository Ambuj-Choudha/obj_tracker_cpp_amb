#pragma once

#include <memory>
#include <vector>

#include "common/retry_monitor.hpp"
#include "common/status.hpp"
#include "common/types.hpp"

// Forward declaration so users of tracker.hpp don't have to pull in
// ByteTrack's Eigen-heavy headers.
namespace byte_track { class BYTETracker; }

struct TrackerDefaults {
    static constexpr int frame_rate    = 30;
    static constexpr int track_buffer  = 30;
    static constexpr float track_thresh = 0.5f;
    static constexpr float high_thresh  = 0.6f;
    static constexpr float match_thresh = 0.8f;
};

struct ByteTrackerAdapterConfig {
    // Min IoU before a track's class_id is recovered from a detection
    static constexpr float class_recovery_min_iou = 0.5f;

    static constexpr int RetryBudget = 5;
};

class ByteTrackerAdapter {
    public:
        ByteTrackerAdapter(int frame_rate    = TrackerDefaults::frame_rate,
                           int track_buffer  = TrackerDefaults::track_buffer,
                           float track_thresh = TrackerDefaults::track_thresh,
                           float high_thresh  = TrackerDefaults::high_thresh,
                           float match_thresh = TrackerDefaults::match_thresh);
        ByteTrackerAdapter(const ByteTrackerAdapter&) = delete;
        ByteTrackerAdapter& operator=(const ByteTrackerAdapter&) = delete;
        ByteTrackerAdapter(ByteTrackerAdapter&&) = delete;
        ~ByteTrackerAdapter();

        // Per-frame, so a failed solve drops the frame and continues
        Status::Result<std::vector<Data::TrackedDetection>> update(const std::vector<Data::Detection>& detections);

    private:
        std::unique_ptr<byte_track::BYTETracker> impl_;
        RetryMonitor retry_monitor_;
};
