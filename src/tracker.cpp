#include "tracker.hpp"

#include <algorithm>
#include <cmath>

#include "ByteTrack/BYTETracker.h"
#include "ByteTrack/Object.h"
#include "ByteTrack/Rect.h"
#include "ByteTrack/STrack.h"

namespace {
    constexpr const char* kSolverFailedCause = "tracker assignment solve failed for this frame";

    // IoU between a Data::BBox (tlbr int) and a byte_track::Rect<float> (tlwh).
    float iou_bbox_rect(const Data::BBox& b, const byte_track::Rect<float>& r) {
        const float ax1 = static_cast<float>(b.x1);
        const float ay1 = static_cast<float>(b.y1);
        const float ax2 = static_cast<float>(b.x2);
        const float ay2 = static_cast<float>(b.y2);
        const float bx1 = r.x();
        const float by1 = r.y();
        const float bx2 = r.x() + r.width();
        const float by2 = r.y() + r.height();

        const float ix1 = std::max(ax1, bx1);
        const float iy1 = std::max(ay1, by1);
        const float ix2 = std::min(ax2, bx2);
        const float iy2 = std::min(ay2, by2);
        const float iw = std::max(0.0f, ix2 - ix1);
        const float ih = std::max(0.0f, iy2 - iy1);
        const float inter = iw * ih;

        const float area_a = std::max(0.0f, ax2 - ax1) * std::max(0.0f, ay2 - ay1);
        const float area_b = std::max(0.0f, r.width()) * std::max(0.0f, r.height());
        const float uni = area_a + area_b - inter;
        return uni > 0.0f ? inter / uni : 0.0f;
    }
}

ByteTrackerAdapter::ByteTrackerAdapter(int frame_rate,
                                       int track_buffer,
                                       float track_thresh,
                                       float high_thresh,
                                       float match_thresh)
    : impl_{std::make_unique<byte_track::BYTETracker>(frame_rate,
                                                     track_buffer,
                                                     track_thresh,
                                                     high_thresh,
                                                     match_thresh)},
      retry_monitor_{Status::Stage::Tracking, ByteTrackerAdapterConfig::RetryBudget} {}

ByteTrackerAdapter::~ByteTrackerAdapter() = default;

Status::Result<std::vector<Data::TrackedDetection>>
ByteTrackerAdapter::update(const std::vector<Data::Detection>& detections) {
    // 1) Detection -> byte_track::Object (tlwh + label + prob)
    std::vector<byte_track::Object> objects;
    objects.reserve(detections.size());
    for (const auto& d : detections) {
        const float w = static_cast<float>(d.bbox.x2 - d.bbox.x1);
        const float h = static_cast<float>(d.bbox.y2 - d.bbox.y1);
        byte_track::Rect<float> rect{static_cast<float>(d.bbox.x1),
                                     static_cast<float>(d.bbox.y1),
                                     w, h};
        objects.emplace_back(rect, d.class_id, static_cast<float>(d.confidence_score));
    }

    // 2) Run tracker
    std::vector<byte_track::BYTETracker::STrackPtr> tracks;
    try {
        tracks = impl_->update(objects);
    } catch (const std::runtime_error&) {
        return std::unexpected(retry_monitor_.record_failure(kSolverFailedCause, "tracker assignment solve"));
    }
    retry_monitor_.record_success();

    // 3) STrack -> TrackedDetection; recover class_id by best-IoU match
    //    against this frame's input detections (ByteTrack is class-agnostic).
    std::vector<Data::TrackedDetection> out;
    out.reserve(tracks.size());
    for (const auto& t : tracks) {
        const auto& r = t->getRect();

        Data::BBox bbox{
            static_cast<int>(std::round(r.x())),
            static_cast<int>(std::round(r.y())),
            static_cast<int>(std::round(r.x() + r.width())),
            static_cast<int>(std::round(r.y() + r.height()))
        };

        int recovered_class = -1;
        float best_iou = ByteTrackerAdapterConfig::class_recovery_min_iou;  // require reasonable overlap to trust the recovery
        for (const auto& d : detections) {
            const float iou = iou_bbox_rect(d.bbox, r);
            if (iou > best_iou) {
                best_iou = iou;
                recovered_class = d.class_id;
            }
        }

        out.push_back(Data::TrackedDetection{
            Data::Detection{bbox, recovered_class, static_cast<double>(t->getScore())},
            t->getTrackId()
        });
    }
    return out;
}
