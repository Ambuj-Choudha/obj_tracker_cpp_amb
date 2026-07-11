#pragma once

#include <string>
#include <vector>

#include <opencv2/dnn.hpp>

#include "common/types.hpp"
#include "transforms.hpp"

// TODO: Seperate out the runtime constants from the fixed params
// clear distinction in what can be changed and what cannot

struct DetectorDefaults{
    static constexpr double ConfThreshold = 0.5;
    static constexpr int num_classes = 80;
};

// TODO: understand when to use inline const and when constexpr
namespace PreprocessorConfig {
    constexpr double rescale_factor = 1.0/255.0;
    inline const cv::Scalar mean{0, 0, 0};
    constexpr bool swapRB = true;
    constexpr int target_size = 640;
    inline const cv::Scalar LetterboxPaddingColour{114, 114, 114};
    constexpr int interp = cv::INTER_LINEAR;
}

class DetectorBase{
    public:
        virtual ~DetectorBase() = default;
        virtual std::vector<Data::Detection> detect(const Data::Frame& frame) = 0;
};


class YOLOv10DetectorONNX : public DetectorBase{
    public:
        YOLOv10DetectorONNX(const std::string& model_path, double confidence_threshold = DetectorDefaults::ConfThreshold);
        YOLOv10DetectorONNX(const YOLOv10DetectorONNX&) = delete;
        YOLOv10DetectorONNX& operator=(const YOLOv10DetectorONNX&) = delete;
        YOLOv10DetectorONNX(YOLOv10DetectorONNX&&) = delete;
        ~YOLOv10DetectorONNX() override = default;

        std::vector<Data::Detection> detect(const Data::Frame& frame) override;

    private:
        std::string model_path_;
        double confidence_threshold_;
        cv::dnn::Net net_;
        std::tuple<cv::Mat, double, int, int> preprocess_frames_(const Data::Frame& frame);
        cv::Mat predict_(const cv::Mat& preprocessed_frame);
        std::vector<Data::Detection> postprocess_frames_(const cv::Mat& raw_outputs, double scale, int dw, int dh);
};

