#pragma once
#include <opencv2/core.hpp>

namespace Data{
    struct Frame {
        cv::Mat mat;
    };

    struct BBox{
        // struct for the object's BBox Top-left (x1, y1) and bottom-right(x2, y2)
        int x1;
        int y1;
        int x2;
        int y2;
    };

    struct Detection{
        // detection object for each detection to encapsulate all the information
        BBox bbox;
        int class_id;
        double confidence_score;

    };
}
