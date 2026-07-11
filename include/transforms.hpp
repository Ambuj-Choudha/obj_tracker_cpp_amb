#pragma once

#include "common/types.hpp"
#include <algorithm>  // std::min
#include <cmath>      // std::round
#include <opencv2/imgproc.hpp>

namespace preprocess{
    std::tuple<cv::Mat, double, int, int> apply_letterbox_transform(const Data::Frame& src);
}

namespace postprocess{
    Data::BBox undo_letter_box_transform(float x1, float y1, float x2, float y2,
                                         double scale, int dw, int dh);
}
