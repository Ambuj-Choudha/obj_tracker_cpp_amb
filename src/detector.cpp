#include "detector.hpp"
#include "common/types.hpp"
#include <opencv2/dnn.hpp>

YOLOv10DetectorONNX::YOLOv10DetectorONNX(
    const std::string& model_path, double confidence_threshold)
    : model_path_{model_path},
      confidence_threshold_{confidence_threshold} {
    net_ = cv::dnn::readNetFromONNX(model_path_);
}

std::vector<Data::Detection> YOLOv10DetectorONNX::detect(const Data::Frame& frame) {
    auto [preprocessed_frame, scale, dw, dh] = this->preprocess_frames_(frame);
    auto raw_outputs = this->predict_(preprocessed_frame);
    auto detections = this->postprocess_frames_(raw_outputs, scale, dw, dh);
    return detections;
}

std::tuple<cv::Mat, double, int, int> YOLOv10DetectorONNX::preprocess_frames_(const Data::Frame& frame) {
    namespace preConfig = PreprocessorConfig;
    auto [letterboxed_frame, scale, dw, dh] = preprocess::apply_letterbox_transform(frame);

    cv::dnn::Image2BlobParams imgParams(
        preConfig::rescale_factor,
        cv::Size(preConfig::target_size, preConfig::target_size),
        preConfig::mean,
        preConfig::swapRB,
        CV_32F,
        cv::dnn::DNN_LAYOUT_NCHW,
        cv::dnn::DNN_PMODE_NULL,          // letterbox already padded to target size
        preConfig::LetterboxPaddingColour        // unused when mode is NULL
    );

    auto blob = cv::dnn::blobFromImageWithParams(letterboxed_frame, imgParams);
    return std::tuple(blob, scale, dw, dh);
}

cv::Mat YOLOv10DetectorONNX::predict_(const cv::Mat& preprocessed_frame) {
    this->net_.setInput(preprocessed_frame);
    cv::Mat raw_outputs = this->net_.forward();
    return raw_outputs;
}


std::vector<Data::Detection> YOLOv10DetectorONNX::postprocess_frames_(
    const cv::Mat& raw_outputs, double scale, int dw, int dh) {
    std::vector<Data::Detection> detections;

    // YOLOv10 output shape: [1, N, 6]. Each row = [x1, y1, x2, y2, score, class_id]
    // in 640x640 letterboxed input space, sorted by score descending.
    
    int num_det = raw_outputs.size[1];

    for (int i = 0; i < num_det; ++i) {
        const float* row = raw_outputs.ptr<float>(0, i);
        const double conf = row[4];

        // rows are score-sorted, so once we drop below threshold we're done
        if (conf < this->confidence_threshold_) break;

        Data::BBox bbox_orig = postprocess::undo_letter_box_transform(row[0], row[1], row[2], row[3], 
                                                                    scale, dw, dh);
        const int class_id = static_cast<int>(row[5]);

        detections.push_back(Data::Detection{bbox_orig, class_id, conf});
    }

    return detections;
}
