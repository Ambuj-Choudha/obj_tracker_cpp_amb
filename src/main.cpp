#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <string>

#include "common/types.hpp"
#include "camera.hpp"
#include "detector.hpp"
#include "tracker.hpp"
#include "visualization.hpp"

int main(int argc, char* argv[]) {
    // TODO: add command line parser for changing runtime configs
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
        using namespace std::string_literals;
        std::string model_path = "assets/model/yolov10m/yolov10m.onnx"s;

        auto detector = YOLOv10DetectorONNX(model_path);
        auto tracker = ByteTrackerAdapter{};
        auto visualizer_obj = Visualizer(2);

        while (true) {
            auto frame = source->getNextFrame();  // calling method from base pointer
            if (!frame) break;                    // nullopt = VideoFile EOF

            Data::Frame input_frame{*frame};  // * as frame is of type std::optional
            auto detections_in_current_frame = detector.detect(input_frame);
            auto tracked_in_current_frame = tracker.update(detections_in_current_frame);
            visualizer_obj.draw_tracked_detections(input_frame, tracked_in_current_frame);
            cv::imshow("Detected Objects", input_frame.mat);

            int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q' || key == 27) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Fatal exception: " << e.what() << "\n";
        return -1;
    }
    return 0;
}
