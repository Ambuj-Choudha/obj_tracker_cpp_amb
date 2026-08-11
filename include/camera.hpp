#pragma once

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <format>
#include <iostream>
#include <stdio.h>
#include <string>
#include <string_view>

#include "common/status.hpp"
#include "common/types.hpp"

namespace CameraDefaults {
    inline constexpr int FrameDefaultWidth  = 1280;
    inline constexpr int FrameDefaultHeight = 720;

    // Consecutive failed reads allowed before it becomes a fatal error
    inline constexpr int WebcamRetryBudget    = 150;
    inline constexpr int VideoFileRetryBudget = 10;
}

// Pure interface
class IInputSource {
    public:
        virtual ~IInputSource() = default;
        virtual Status::Result<Data::Frame> getNextFrame() = 0;
        virtual Status::SourceState getSourceState() const noexcept = 0;

        virtual double getFps() const = 0;
};

// Abstract base class
class VideoCaptureBase : public IInputSource {
    public:
        double getFps() const override { return current_fps_; }
        Status::SourceState getSourceState() const noexcept override { return source_state_; }

    protected:
        cv::VideoCapture cap;

        int consecutive_failures_{0};
        Status::SourceState source_state_{Status::SourceState::Streaming};

        // recoverable_cause is const char* rather than string_view on purpose:
        // it has to be a literal (it ends up in a non-owning view)
        Status::Error record_failure(int retry_budget, const char* recoverable_cause, std::string_view subject) {
            ++consecutive_failures_;

            if (consecutive_failures_ > retry_budget) {
                source_state_ = Status::SourceState::Failed;
                return Status::Fatal{Status::Stage::Source, std::format("{} unrecoverable after {} consecutive failed reads",
                                subject, consecutive_failures_)};
            }

            source_state_ = Status::SourceState::DisconnectedRetrying;
            return Status::Recoverable{Status::Stage::Source, recoverable_cause, consecutive_failures_};
        }

        void record_success() {
            consecutive_failures_ = 0;
            source_state_ = Status::SourceState::Streaming;
        }

        void updateFps() {
            int64_t current_tick = cv::getTickCount();
            double time_delta = static_cast<double>(current_tick - last_frame_tick_) / cv::getTickFrequency();
            if (time_delta > 0.0) {
                current_fps_ = 1.0 / time_delta;
            }
            last_frame_tick_ = current_tick;
        }

private:
    double current_fps_{0.0};
    int64_t last_frame_tick_{cv::getTickCount()};
};

class WebcamCamera : public VideoCaptureBase {
    public:
        WebcamCamera(int deviceID = 0, int apiID = cv::CAP_ANY,
                     int retry_budget = CameraDefaults::WebcamRetryBudget);
        WebcamCamera(const WebcamCamera&) = delete;
        WebcamCamera& operator=(const WebcamCamera&) = delete;
        WebcamCamera(WebcamCamera&&) = default;
        ~WebcamCamera() override = default;

        Status::Result<Data::Frame> getNextFrame() override;

    private:
        int deviceID;
        int apiID;
        int retry_budget_;  // policy lives on the component, never on the error
};

class VideoFile : public VideoCaptureBase {
    public:
        VideoFile(const std::string& source_file, int apiID = cv::CAP_ANY,
                  int retry_budget = CameraDefaults::VideoFileRetryBudget);
        VideoFile(const VideoFile&) = delete;
        VideoFile& operator=(const VideoFile&) = delete;
        VideoFile(VideoFile&&) = default;
        ~VideoFile() override = default;

        Status::Result<Data::Frame> getNextFrame() override;

    private:
        std::string source_file;
        int apiID;
        int retry_budget_;
};
