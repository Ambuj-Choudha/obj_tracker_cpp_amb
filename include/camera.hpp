#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

namespace Defaults {
    inline constexpr int FrameDefaultWidth  = 1280;
    inline constexpr int FrameDefaultHeight = 720;
}


class IInputSource {
    public:
        virtual ~IInputSource() = default;
        virtual std::optional<cv::Mat> getNextFrame() = 0;
};

class WebcamCamera : public IInputSource{
    public:
        WebcamCamera(int deviceID = 0, int apiID = cv::CAP_ANY);
        WebcamCamera(const WebcamCamera&) = delete;  // copy constructor
        WebcamCamera& operator=(const WebcamCamera&) = delete;  // copy assignment constructor
        WebcamCamera(WebcamCamera&&) = default;  // move constructor
        ~WebcamCamera() override = default;      // destructor

        std::optional<cv::Mat> getNextFrame() override;
        
    private:
        int deviceID;
        int apiID;
        cv::VideoCapture cap;
};

class VideoFile : public IInputSource{
    public:
        VideoFile(const std::string& source_file, int apiID = cv::CAP_ANY);
        VideoFile(const VideoFile&) = delete;
        VideoFile& operator=(const VideoFile&) = delete;
        VideoFile(VideoFile&&) = default;
        ~VideoFile() override = default;

        std::optional<cv::Mat> getNextFrame() override;
    private:
        std::string source_file;
        int apiID;
        cv::VideoCapture cap;
};
