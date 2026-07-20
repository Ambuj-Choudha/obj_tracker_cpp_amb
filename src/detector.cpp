#include "detector.hpp"
#include "common/types.hpp"

#include <opencv2/dnn.hpp>

YOLOv10DetectorONNX::YOLOv10DetectorONNX(
    const std::string& model_path, double confidence_threshold)
    : confidence_threshold_{confidence_threshold}, engine_{model_path} {}

std::vector<Data::Detection> YOLOv10DetectorONNX::detect(const Data::Frame& frame) {
    auto [preprocessed_frame, scale, dw, dh] = this->preprocess_frames_(frame);
    auto raw_outputs = engine_.infer(preprocessed_frame.ptr<float>(), preprocessed_frame.total());
    return this->postprocess_frames_(raw_outputs, scale, dw, dh);
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
        preConfig::LetterboxPaddingColour // unused when mode is NULL
    );

    auto blob = cv::dnn::blobFromImageWithParams(letterboxed_frame, imgParams);
    return std::tuple(blob, scale, dw, dh);
}

std::vector<Data::Detection> YOLOv10DetectorONNX::postprocess_frames_(
    InferenceEngine::Output raw_outputs, double scale, int dw, int dh) {
    std::vector<Data::Detection> detections;
    detections.reserve(raw_outputs.rows);

    // YOLOv10 output rows are sorted by score descending; break early below threshold.
    for (int64_t i = 0; i < raw_outputs.rows; ++i) {
        const float* row = raw_outputs.data + i * raw_outputs.cols;
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
