#include <cstdio>
#include <format>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

// include the interface file
#include "camera.hpp"


WebcamCamera::WebcamCamera(int deviceID, int apiID) : deviceID{deviceID}, apiID{apiID} {
    
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        throw std::runtime_error(std::format("Error: Could not open camera with deviceID: {} \n", deviceID));
    }
    std::cout << "Camera initialized successfully!\n";
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CameraDefaults::FrameDefaultWidth);    
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CameraDefaults::FrameDefaultWidth);

    std::cout << "Frame Width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << '\n';
    std::cout << "Frame Height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
}

auto WebcamCamera::getNextFrame() -> std::optional<cv::Mat> { // std::optional for matching signature of base class
    cv::Mat frame;
    bool read_img_ok = cap.read(frame);
    if (!read_img_ok) {
        throw std::runtime_error("cap.read() failed, device maybe disconnected");
    }
    if (frame.empty()) {
        throw std::runtime_error("cap.read() succeeded but returned an empty frame. The camera may not be delivering images.");
    }
    
    updateFps();

    return frame;
}

VideoFile::VideoFile(const std::string& source_file, int apiID) : source_file{source_file}, apiID{apiID} {
    
    cap.open(source_file, apiID);
    if (!cap.isOpened()) {
        throw std::runtime_error(std::format("ERROR: Unable to open source file '{}'. Please check the file path and format.", source_file));
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CameraDefaults::FrameDefaultWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CameraDefaults::FrameDefaultHeight);

    std::cout << "Frame Width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << '\n';
    std::cout << "Frame Height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
}

auto VideoFile::getNextFrame() -> std::optional<cv::Mat> {
    cv::Mat frame;
    bool read_file_ok = cap.read(frame);

    if(!read_file_ok){
        return std::nullopt;
    }
    if(frame.empty()){
        throw std::runtime_error("Frame read was successful but the frame is empty. The video file may be corrupted or at EOF.");
    }

    updateFps();

    return frame;
}
