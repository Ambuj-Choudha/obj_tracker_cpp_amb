#include "engine.hpp"

InferenceEngine::InferenceEngine(const std::string& model_path)
    : env_{ORT_LOGGING_LEVEL_WARNING, "InferenceEngine"},
      session_options_{} {

    session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    // ORT session ctor takes ORTCHAR_T*, which is wchar_t on Windows and char elsewhere.
#ifdef _WIN32
    std::wstring wpath(model_path.begin(), model_path.end());
    session_ = Ort::Session{env_, wpath.c_str(), session_options_};
#else
    session_ = Ort::Session{env_, model_path.c_str(), session_options_};
#endif

    Ort::AllocatorWithDefaultOptions allocator;
    input_name_  = std::string{session_.GetInputNameAllocated(0, allocator).get()};
    output_name_ = std::string{session_.GetOutputNameAllocated(0, allocator).get()};

    // Read the input shape the model declares (e.g. {1, 3, 640, 640}).
    input_shape_ = session_.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    // Some models declare batch dim as -1 (dynamic). Pin it to 1 for our single-frame use.
    if (!input_shape_.empty() && input_shape_[0] < 0) {
        input_shape_[0] = 1;
    }
}

InferenceEngine::Output InferenceEngine::infer(const float* input, size_t input_len) {
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        const_cast<float*>(input),  // ORT C API is non-const; we don't mutate.
        input_len,
        input_shape_.data(),
        input_shape_.size()
    );

    const char* input_names[]  = {input_name_.c_str()};
    const char* output_names[] = {output_name_.c_str()};

    auto output_tensors = session_.Run(Ort::RunOptions{nullptr},
                                       input_names, &input_tensor, 1,
                                       output_names, 1);
    last_output_ = std::move(output_tensors[0]);

    auto shape = last_output_.GetTensorTypeAndShapeInfo().GetShape();
    // Expected: [1, N, K] — squeeze the leading batch dim.
    if (shape.size() != 3 || shape[0] != 1) {
        throw std::runtime_error("Unexpected model output shape (expected [1,N,K])");
    }
    return Output{last_output_.GetTensorData<float>(), shape[1], shape[2]};
    };
}
