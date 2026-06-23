#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

namespace Data{
    struct Frame {
        cv::Mat mat;
    };

    struct BBox{
        // struct for the frame extracted from the video feed
        int width;
        int height;
        int pos_x;
        int pos_y;
    };

    struct Detection{
        // detection object for each detection to encapsulate all the information
        BBox bbox;
        int class_id;
        int obj_id;
        double confidence;

    };

    class Track{
        // to maintain and correlate object in one frame to the other frame
    public:
        auto update_tracks(std::vector<Detection>& const detections, std::vector<Track>& tracks);
    private:
        int track_id;
        auto create_new_track(std::vector<Detection>& const detections, std::vector<Track>& tracks);
        auto extend_track(std::vector<Detection>& const detections, std::vector<Track>& tracks);
        void delete_track(std::vector<Detection>& const detections, std::vector<Track>& tracks);
        auto check_object(std::vector<Detection>& const detections, std::vector<Track>& const tracks);
    };
};


using namespace Data;
    auto Track::create_new_track(std::vector<Detection>& const detections, std::vector<Track>& tracks) -> Track& { // !! Dangling reference possible
        // 1. new object enters --> create a new track
    }

    auto Track::extend_track(std::vector<Detection>& const detections, std::vector<Track>& tracks) -> Track& {
        // 2. existing object from previous frame --> correlate and extend the track
    }

    void Track::delete_track(std::vector<Detection>& const detections, std::vector<Track>& tracks) {
        // 3. object missing after some x frames --> delete the track
    }  

    auto Track::update_tracks(std::vector<Detection>& const detections, std::vector<Track>& tracks) -> Track& {
        // tracks {};
        // for each detection in detections create tracks
            auto status = check_object(detections, tracks);
            // based on status do one of the following:
                Track::create_new_track(detections, tracks); // 1. new object enters --> create a new track
                    // append new track to Tracks

                Track::extend_track(detections, tracks); // 2. existing object from previous frame --> correlate and extend the track
                    // update the relevant track

        // outside for loop
        Track::delete_track(detections, tracks);  // 3. object missing after some x frames --> delete the track
    }

    auto Track::check_object(Detection& const, std::vector<Track>& const) -> int{
        // check the status code / case for furthur processing
    }

    auto extract_frame(std::streambuf& stream) -> Frame& {
        // extract frames from the video stream
    }

    auto detect_objects(Frame& const frame) -> std::vector<Detection>& {  // !! Dangling reference possible
        // run object detection on each frame and get detections
    }

    auto draw_bbox(Frame& frame, std::vector<Detection>& const detections) -> Frame& {
        // draw bouding boxes in the frame
    }
    auto draw_tracks(Frame& frame, std::vector<Track>& const tracks) -> Frame& {
        // draw the tracks
    }

    auto annotate_frame(Frame& frame, std::vector<Detection>& const detections, std::vector<Track>& const tracks) -> Frame& {
        
        draw_bbox(frame, detections);  // 1. draw bounding box on each of the object
        draw_tracks(frame, tracks);  // 2. draw the tracks by consolidating the positional information
    }

    void save_frame(Frame& const frame) {
        // save the processed data on the disk
    }


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
        while (true) {
            auto frame = source->getNextFrame();  // calling method from base pointer
            if (!frame) break;                    // nullopt = VideoFile EOF

            cv::imshow("Live", *frame);  // JUST FOR TESTING

            int key = cv::waitKey(1);
            if (key == 'q' || key == 'Q' || key == 27) {
                break;

        }
    } catch (const std::runtime_error& e) {
        std::cout << "Runtime error: " << e.what() << "\n";
    }

/*
    // for each frame in frames
        // extract frames from the video stream
         Frame input_frame{*frame}; // create 'Frame' object by using the current frame

            auto detection_in_current_frame = detect_objects(input_frame); // run object detection on each frame and get detections

        Track::update_tracks(detection_in_current_frame, tracks);
            // create tracks from the detections
            // 1. new object enters --> create a new track
            // 2. existing object from previous frame --> correlate and extend the track
            // 3. object missing after some x frames --> delete the track


        auto annotated_frame = annotate_frame(frame, detection_in_current_frame, tracks); 
            // 1. draw bounding box on each of the object
            // 2. draw the tracks by consolidating the positional information

        save_frame(annotated_frame); // save the processed data on the disk
*/
    return 0;
}
