#include "detector.hpp"
#include "common/types.hpp"

#include <opencv2/dnn.hpp>

namespace {
    constexpr const char* kBlobOutOfMemoryCause = "out of memory building preprocess blob";

    bool is_out_of_memory(const cv::Exception& preprocess_error) noexcept {
        return preprocess_error.code == cv::Error::StsNoMem;
    }
}

YOLOv10DetectorONNX::YOLOv10DetectorONNX(
    const std::string& model_path, double confidence_threshold, int retry_budget)
    : confidence_threshold_{confidence_threshold},
      engine_{model_path},
      target_size_{static_cast<int>(engine_.input_shape()[2])},
      retry_monitor_{Status::Stage::Preprocess, retry_budget} {}

Status::Result<std::vector<Data::Detection>>
YOLOv10DetectorONNX::detect(const Data::Frame& frame) {
    auto preprocessed_frame = this->preprocess_frames_(frame);
    if (!preprocessed_frame) {
        return std::unexpected(preprocessed_frame.error());  // drop this frame, keep the loop alive
    }
    retry_monitor_.record_success();  // one good frame resets the monitor

    auto raw_outputs = engine_.infer(preprocessed_frame->blob.ptr<float>(), preprocessed_frame->blob.total());
    return this->postprocess_frames_(raw_outputs, preprocessed_frame->scale, preprocessed_frame->dw, preprocessed_frame->dh);
}

Status::Result<YOLOv10DetectorONNX::LetterboxedBlob> YOLOv10DetectorONNX::preprocess_frames_(const Data::Frame& frame) {
    namespace preConfig = PreprocessorConfig;

    try {
    auto [letterboxed_frame, scale, dw, dh] = preprocess::apply_letterbox_transform(frame, target_size_);

    cv::dnn::Image2BlobParams imgParams(
        preConfig::rescale_factor,
        cv::Size(target_size_, target_size_),
        preConfig::mean,
        preConfig::swapRB,
        CV_32F,
        cv::dnn::DNN_LAYOUT_NCHW,
        cv::dnn::DNN_PMODE_NULL,          // letterbox already padded to target size
        preConfig::LetterboxPaddingColour // unused when mode is NULL
    );

    auto blob = cv::dnn::blobFromImageWithParams(letterboxed_frame, imgParams);
    return LetterboxedBlob{blob, scale, dw, dh};
    } catch (const cv::Exception& preprocess_error) {
        if (is_out_of_memory(preprocess_error)) {
            return std::unexpected(retry_monitor_.record_failure(kBlobOutOfMemoryCause, "preprocess"));
        }
        throw;  // any other cv::Exception is not a modelled in this stage
    }
}

std::vector<Data::Detection> YOLOv10DetectorONNX::postprocess_frames_(
    InferenceEngine::Output raw_outputs, double scale, int dw, int dh) {
    std::vector<Data::Detection> detections;
    detections.reserve(raw_outputs.rows);

    // YOLOv10 output rows are sorted by score descending; break early below threshold.
    for (int64_t i = 0; i < raw_outputs.rows; ++i) {
        const float* row = raw_outputs.row(i);  // i-th detection
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
