#include <cstdio>
#include <format>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

// include the interface file
#include "camera.hpp"


WebcamCamera::WebcamCamera(int deviceID, int apiID) : deviceID{deviceID}, apiID{apiID} {
    
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        throw std::runtime_error(std::format("Error: Could not open camera with deviceID: {} \n", deviceID));
    }
    std::cout << "Camera initialized successfully!\n";
    cap.set(cv::CAP_PROP_FRAME_WIDTH, Defaults::FrameDefaultWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, Defaults::FrameDefaultWidth);
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
    return frame;
}

VideoFile::VideoFile(const std::string& source_file, int apiID) : source_file{source_file}, apiID{apiID} {
    
    cap.open(source_file, apiID);
    if (!cap.isOpened()) {
        throw std::runtime_error(std::format("ERROR: Unable to open source file '{}'. Please check the file path and format.", source_file));
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, Defaults::FrameDefaultWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, Defaults::FrameDefaultHeight);
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
    return frame;
}

//==================================================================
// ****************ONLY FOR TESTING*********************************
int main(int argc, char* argv[]) {
    std::unique_ptr<IInputSource> source;  // use pointer from base class

    if (argc < 2) {
        source = std::make_unique<WebcamCamera>();
    } else if (argc == 2) {
        source = std::make_unique<VideoFile>(argv[1]);
    } else {
        std::cout << "Wrong number of arguments passed!" << "\n";
        std::cout << "Use <program name> <source_video_file> OR simply <program name> for webcam..." << "\n";
        return -1;
    }

    try {
        cv::namedWindow("Input Feed", cv::WINDOW_NORMAL);
        std::cout << "Starting webcam feed. Press 'q' or 'ESC' to quit.\n";
        while (true) {
            auto frame = source->getNextFrame();  // calling method from base pointer
            if (!frame) {
              break;  // nullopt = VideoFile EOF
            }

            cv::imshow("Live", *frame);  //std::optional needs to be unpacked using '*' operator
            
            int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q' || key == 27) {
              break;
            }
        }
    } catch (const std::runtime_error& e) {
        std::cout << "Runtime error: " << e.what() << "\n";
    }
    return 0;
}

