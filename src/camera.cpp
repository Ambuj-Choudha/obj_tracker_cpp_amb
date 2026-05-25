#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

// include the interface file
#include "camera.hpp"


WebcamCamera::WebcamCamera(int deviceID, int apiID) : deviceID{deviceID}, apiID{apiID} {
    
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        throw std::runtime_error("ERROR! Unable to open camera");
    }
}

auto WebcamCamera::getNextFrame() -> cv::Mat { 
    cv::Mat frame;
    bool read_img_ok = cap.read(frame);
    if (!read_img_ok) {
        throw std::runtime_error("cap.read() failed - stream ended or device disconnected");
    }
    if (frame.empty()) {
        throw std::runtime_error("cap.read() succeeded but returned an empty frame");
    }
    return frame;
}

VideoFile::VideoFile(const std::string& source_file, int apiID) : source_file{source_file}, apiID{apiID} {
    
    cap.open(source_file, apiID);
    if (!cap.isOpened()) {
        throw std::runtime_error("ERROR! Unable to open the source file");
    }
}

auto VideoFile::getNextFrame() -> cv::Mat {
    cv::Mat frame;
    bool read_file_ok = cap.read(frame);

    if(!read_file_ok){
        return std::nullopt;
    }
    if(frame.empty()){
        throw std::runtime_error("Read successful but empty frame!");
    }
    return frame;
}

int main(int argc, char* argv[]){
    if(argc < 2){
        WebcamCamera camera;
        try {
            while(true){
                auto frame = camera.getNextFrame();
                // cv::imshow("Live", frame);
                // if (cv::waitKey(0)){
                //     break;
                // }
            }
        } catch (const std::runtime_error& error){
            std::cout << "Runtime error: " << error.what() << "\n";
        }
    } else if(argc == 2){
        std::string source_file_name = argv[1];

        VideoFile video_file(source_file_name);
        try{
            while(true){
                auto frame = video_file.getNextFrame();
                // cv::imshow("Live", frame);
                // if (cv::waitKey(0)){
                //     break;
                // }
            }
        } catch(const std::runtime_error& error){
            std::cout << "Runtime error: " << error.what() << "\n";
        }
    } else {
        std::cout << "Wrong number of arguments passed!" << "\n";
        std::cout << "Use <program name> <source_video_file> OR simply <program name> for webcam..." << "\n";
        return -1;
    }
    return 0;
}

