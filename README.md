# object-tracking-cpp

## 1.0 Goal

Real-time object detection and tracking pipeline using C++.
Combines YOLOv10m (detection) with ByteTrack (multi-object tracking).

## 2.0 Setup

### Prerequisites

- A connected camera (default device id 0; change the argument passed to `WebcamCamera` in `src/main.cpp` to select a different camera device index)
- YOLOv10 Model (ONNX format) from [Huggingface](https://huggingface.co/onnx-community/yolov10m/tree/main)

### Compiling the code

Requires: CMake ≥ 3.10, Ninja, OpenCV (built with gcc), ONNX Runtime, and a C++23-capable compiler.

On Windows/MSYS2 (UCRT64), install ONNX Runtime with:
```bash
pacman -S mingw-w64-ucrt-x86_64-onnxruntime
```

```bash
# Configure
cmake -B build-ninja -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
```
You do not need to mention the compiler explicitly as it will pick it up from Path, if you use the -G flag for Ninja

```bash
# Build
cmake --build build-ninja
```

### To run the code

On Windows, launch from the UCRT64 shell so `onnxruntime.dll` and the MinGW runtime DLLs resolve correctly (Windows ships its own ABI-incompatible `onnxruntime.dll` in `SYSTEM32` for Windows ML).

```bash
# Run
./build-ninja/obj_tracker_cpp.exe
```

### Running tests

A smoke test for the detector is built by default (disable with `-DBUILD_TESTS=OFF`).

```bash
./build-ninja/test_detector.exe
```

The test expects `data/input/horse.jpg` and the YOLOv10m ONNX model at the paths defined in the test file.

### Controls

| Key | Action |
|-----|--------|
| `q` / `ESC` | Quit |

## TODOs

- Add command line parser for changing runtime configs
- Add Docker for reproducible environment
- Wrap the detector into a ROS2 Node

## 3.0 References

- YOLOv10 — https://huggingface.co/onnx-community/yolov10m
- ByteTrack (C++ port) — https://github.com/Vertical-Beach/ByteTrack-cpp


