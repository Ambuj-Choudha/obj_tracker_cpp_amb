#pragma once

#include <memory>
#include <vector>

#include "common/types.hpp"

// Forward declaration so users of tracker.hpp don't have to pull in
// ByteTrack's Eigen-heavy headers.
namespace byte_track { class BYTETracker; }

struct TrackerDefaults {
    // Mirrors byte_track::BYTETracker defaults; kept here so tuning happens
    // in one place next to the rest of the project's runtime config.
    static constexpr int frame_rate    = 30;
    static constexpr int track_buffer  = 30;
    static constexpr float track_thresh = 0.5f;
    static constexpr float high_thresh  = 0.6f;
    static constexpr float match_thresh = 0.8f;
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

        // Runs one frame of tracking. Detections go in as-is (pixel coords);
        // returned TrackedDetection carries a track_id stable across frames.
        // Class id is recovered by IoU-matching each output track back to
        // the input detections (ByteTrack itself is class-agnostic and drops
        // the label internally).
        std::vector<Data::TrackedDetection> update(const std::vector<Data::Detection>& detections);

    private:
        std::unique_ptr<byte_track::BYTETracker> impl_;
};
