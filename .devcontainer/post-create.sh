#!/usr/bin/env bash
set -euo pipefail

MODEL_DIR="assets/model/yolov10m"
MODEL_PATH="${MODEL_DIR}/yolov10m.onnx"
MODEL_URL="https://huggingface.co/onnx-community/yolov10m/resolve/main/onnx/model.onnx"

if [[ -f "${MODEL_PATH}" ]]; then
    echo "YOLOv10m model already present at ${MODEL_PATH}"
else
    echo "Fetching YOLOv10m ONNX model..."
    mkdir -p "${MODEL_DIR}"
    curl -fL --retry 3 -o "${MODEL_PATH}" "${MODEL_URL}"
    echo "Model saved to ${MODEL_PATH}"
fi
