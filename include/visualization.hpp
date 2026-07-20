#pragma once

#include<optional>
#include<string>
#include<tuple>
#include<unordered_map>
#include<vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "common/types.hpp"

// Almost all of the params can be modified during runtime that is why defined in the struct
struct VisualizerDefaults {
    static inline std::tuple<int,int,int> text_colour{0, 0, 0};
    static inline int font = cv::FONT_HERSHEY_SIMPLEX;
    static inline double font_scale = 0.5;
    static constexpr int colour_seed = 42;
    static inline std::string class_labels_file_path = "assets/labels/coco.names";
};


class Visualizer{
    public:
        Visualizer(int border_thickness, std::optional<std::tuple<int, int, int>> text_colour = std::nullopt);

        void draw_detections(Data::Frame& frame, const std::vector<Data::Detection>& detections);
        void set_font_scale(double new_font_scale);
    private:
        int border_thickness_;
        std::tuple<int, int, int> text_colour_;
        double font_scale_;

        // define from the asset/coco.names
        const std::unordered_map<int, std::string> class_labels_dict_;
        
        const std::unordered_map<int, cv::Scalar> colour_map_;
        void draw_bbox_w_labels_(cv::Mat& img, const Data::Detection& detected_obj);
};
