#include <opencv2/core.hpp>
#include "common/types.hpp"
#include "transforms.hpp"
#include "detector.hpp"

std::tuple<cv::Mat, double, int, int> preprocess::apply_letterbox_transform(const Data::Frame& src) {
    
    int target_size_width = PreprocessorConfig::target_size;
    int target_size_height = PreprocessorConfig::target_size;
    auto pad_colour = PreprocessorConfig::LetterboxPaddingColour;
    
    const cv::Mat& img = src.mat;
    int img_w = img.cols;
    int img_h = img.rows;

    // 1. Uniform scale that fits the image inside the square, preserving aspect ratio.
    double scale = std::min(static_cast<double>(target_size_width) / img_w,
                            static_cast<double>(target_size_height) / img_h);

    // 2. Resized (un-padded) dimensions.
    int new_w = static_cast<int>(std::round(img_w * scale));
    int new_h = static_cast<int>(std::round(img_h * scale));

    cv::Mat resized;
    cv::resize(img, resized, cv::Size(new_w, new_h));

    // 3. Padding needed to reach the square, split evenly (centered).
    int pad_w = target_size_width - new_w;
    int pad_h = target_size_height - new_h;
    int pad_left = pad_w / 2;
    int pad_right = pad_w - pad_left;
    int pad_top = pad_h / 2;
    int pad_bottom = pad_h - pad_top;

    cv::Mat output;
    cv::copyMakeBorder(resized, output, pad_top, pad_bottom, pad_left, pad_right,
                       cv::BORDER_CONSTANT, pad_colour);

    return std::tuple(output, scale, pad_left, pad_top);
}

Data::BBox postprocess::undo_letter_box_transform(float x1, float y1, float x2, float y2,
                                                  double scale, int dw, int dh) {
    // Inverse of apply_letterbox: strip the pad, then undo the uniform scale.
    Data::BBox bbox;
    bbox.x1 = static_cast<int>(std::round((x1 - dw) / scale));
    bbox.y1 = static_cast<int>(std::round((y1 - dh) / scale));
    bbox.x2 = static_cast<int>(std::round((x2 - dw) / scale));
    bbox.y2 = static_cast<int>(std::round((y2 - dh) / scale));

    // TODO: maybe add logic that clips these coordinates to valid image dimensions
    return bbox;
}
