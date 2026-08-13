#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>

#include "common/retry_monitor.hpp"
#include "common/status.hpp"
#include "common/types.hpp"
#include "engine.hpp"
#include "transforms.hpp"

// TODO: Seperate out the runtime constants from the fixed params
// clear distinction in what can be changed and what cannot

struct DetectorDefaults{
    static constexpr double ConfThreshold = 0.5;
    static constexpr int num_classes = 80;
    static constexpr int PreprocessRetryBudget = 10;
    static constexpr int InferenceRetryBudget  = 10;
    static constexpr int PostprocessRetryBudget = 10;

    // YOLOv10 emits one row per detection as [x1, y1, x2, y2, score, class_id].
    // postprocess_frames_ indexes row[0]..row[5], declare default for verifying o/p shape
    static constexpr int64_t OutputFieldsPerRow = 6;
};

// TODO: understand when to use inline const and when constexpr
namespace PreprocessorConfig {
    constexpr double rescale_factor = 1.0/255.0;
    inline const cv::Scalar mean{0, 0, 0};
    constexpr bool swapRB = true;
    inline const cv::Scalar LetterboxPaddingColour{114, 114, 114};
    constexpr int interp = cv::INTER_LINEAR;
}

class DetectorBase{
    public:
        virtual ~DetectorBase() = default;
        virtual Status::Result<std::vector<Data::Detection>> detect(const Data::Frame& frame) = 0;
};


class YOLOv10DetectorONNX : public DetectorBase{
    public:
        YOLOv10DetectorONNX(const std::string& model_path, double confidence_threshold = DetectorDefaults::ConfThreshold,
                            int preprocess_retry_budget = DetectorDefaults::PreprocessRetryBudget,
                            int inference_retry_budget = DetectorDefaults::InferenceRetryBudget,
                            int postprocess_retry_budget = DetectorDefaults::PostprocessRetryBudget);
        YOLOv10DetectorONNX(const YOLOv10DetectorONNX&) = delete;
        YOLOv10DetectorONNX& operator=(const YOLOv10DetectorONNX&) = delete;
        YOLOv10DetectorONNX(YOLOv10DetectorONNX&&) = delete;
        ~YOLOv10DetectorONNX() override = default;

        Status::Result<std::vector<Data::Detection>> detect(const Data::Frame& frame) override;

    private:
        double confidence_threshold_;
        InferenceEngine engine_;
        int target_size_;  // cached from engine_.input_shape() at construction

        RetryMonitor preprocess_monitor_;
        RetryMonitor inference_monitor_;
        RetryMonitor postprocess_monitor_;

        // letterboxed Blob amd the parameters postprocess needs for inversion
        struct LetterboxedBlob {
            cv::Mat blob;
            double scale;
            int dw;
            int dh;
        };

        Status::Result<LetterboxedBlob> preprocess_frames_(const Data::Frame& frame);
        Status::Result<std::vector<Data::Detection>> postprocess_frames_(InferenceEngine::Output raw_outputs, double scale, int dw, int dh);
};

