#include <cstdio>
#include <format>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

// include the interface file
#include "camera.hpp"
#include "common/status.hpp"

namespace {
    constexpr const char* kEmptyFrameCause = "capture returned an empty frame; device may be disconnected";
    constexpr const char* kDecodeFailedCause = "frame decode failed mid-stream";
    constexpr const char* kEmptyDecodeCause = "decoder reported success but produced no frame";
    constexpr const char* kOutOfMemoryCause = "out of memory allocating frame buffer";

    // allocation failure surfaces as cv::Exception with code StsNoMem, not std::bad_alloc
    bool is_out_of_memory(const cv::Exception& capture_error) noexcept {
        return capture_error.code == cv::Error::StsNoMem;
    }
}

WebcamCamera::WebcamCamera(int deviceID, int apiID, int retry_budget)
    : VideoCaptureBase{retry_budget}, deviceID{deviceID}, apiID{apiID} {

    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        throw Status::FatalException(Status::Fatal{
            Status::Stage::Source,
            std::format("Error: Could not open camera with deviceID: {}", deviceID)});
    }
    std::cout << "Camera initialized successfully!\n";
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CameraDefaults::FrameDefaultWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CameraDefaults::FrameDefaultHeight);

    std::cout << "Frame Width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << '\n';
    std::cout << "Frame Height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
}

auto WebcamCamera::getNextFrame() -> Status::Result<Data::Frame> {
    cv::Mat frame;
    bool read_img_ok = false;

    try {
        read_img_ok = cap.read(frame);
    } catch (const cv::Exception& capture_error) {
        if (is_out_of_memory(capture_error)) {
            return std::unexpected(record_failure(kOutOfMemoryCause, "camera frame allocation"));
        }
        throw;  // any other cv::Exception is not a modelled in this stage
    }

    if (!read_img_ok || frame.empty()) {
        return std::unexpected(record_failure(kEmptyFrameCause, "camera"));
    }

    record_success();
    updateFps();

    return Data::Frame{frame};
}

VideoFile::VideoFile(const std::string& source_file, int apiID, int retry_budget)
    : VideoCaptureBase{retry_budget}, source_file{source_file}, apiID{apiID} {

    cap.open(source_file, apiID);
    if (!cap.isOpened()) {
        throw Status::FatalException(Status::Fatal{
            Status::Stage::Source,
            std::format("ERROR: Unable to open source file '{}'. Please check the file path and format.", source_file)});
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CameraDefaults::FrameDefaultWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CameraDefaults::FrameDefaultHeight);

    expected_frame_count_ = static_cast<long long>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    std::cout << "Frame Width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << '\n';
    std::cout << "Frame Height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
}

auto VideoFile::getNextFrame() -> Status::Result<Data::Frame> {
    cv::Mat frame;
    bool read_file_ok = false;

    try {
        read_file_ok = cap.read(frame);
    } catch (const cv::Exception& capture_error) {
        if (is_out_of_memory(capture_error)) {
            return std::unexpected(record_failure(kOutOfMemoryCause, "video frame allocation"));
        }
        throw;
    }

    if (!read_file_ok) {
        if (expected_frame_count_ > 0 && frames_read_ < expected_frame_count_) {
            return std::unexpected(record_failure(kDecodeFailedCause, "video decode"));
        }
        source_state_ = Status::SourceState::EndOfStream;
        return Data::Frame{};
    }

    if (frame.empty()) {
        return std::unexpected(record_failure(kEmptyDecodeCause, "video decode"));
    }

    ++frames_read_;
    record_success();
    updateFps();

    return Data::Frame{frame};
}
