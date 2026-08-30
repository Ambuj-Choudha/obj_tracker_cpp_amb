#!/usr/bin/env bash
set -euo pipefail

MODEL_DIR="assets/model/yolov10n"
MODEL_PATH="${MODEL_DIR}/yolov10n.onnx"
MODEL_URL="https://huggingface.co/onnx-community/yolov10n/resolve/main/onnx/model.onnx"

if [[ -f "${MODEL_PATH}" ]]; then
    echo "YOLOv10n model already present at ${MODEL_PATH}"
else
    echo "Fetching YOLOv10n ONNX model..."
    mkdir -p "${MODEL_DIR}"
    curl -fL --retry 3 -o "${MODEL_PATH}" "${MODEL_URL}"
    echo "Model saved to ${MODEL_PATH}"
fi
