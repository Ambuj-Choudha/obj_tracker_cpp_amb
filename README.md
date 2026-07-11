# object-tracking-cpp

## 1.0 Goal

Real-time object detection and tracking pipeline using C++.
Combines YOLOv10m.

## 2.0 Setup

### Prerequisites

- A connected camera (update `DEFAULT_DEVICE_ID` in `src/main.cpp` to match your `/dev/videoX`)
- YOLOv10 Model (ONNX format) from [Huggingface](https://huggingface.co/onnx-community/yolov10m/tree/main)

### Compiling the code

Requires: CMake ≥ 3.22, Ninja, OpenCV (built with gcc), a C++23-capable compiler.

```bash
# Configure
cmake -B build-ninja -S . -G Ninja -DENABLE_CLANG_TIDY=ON -DCMAKE_BUILD_TYPE=Debug
```
You do not need to mention the compiler explicitly as it will pick it up from Path, if you use the -G flag for Ninja

```bash
# Build
cmake --build build-ninja
```

### To run the code 

```bash
# Run
.\build-ninja\obj_tracker_cpp.exe
```

### Controls

| Key | Action |
|-----|--------|
| `q` / `ESC` | Quit |
| `s` | Save current frame to disk |

## TODOs

- Load model configs from the model itself
- Add possibility to run inference on image & batch of images
-  

## 3.0 References
-


