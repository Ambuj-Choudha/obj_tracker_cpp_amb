#!/usr/bin/env bash
# Build, then run clang-tidy --fix over obj_tracker_lib + main.cpp only —
# never src/ByteTrack (vendored, unmodified) or tests/ (not covered yet).
# Keep SOURCES/HEADER_FILTER in sync with CMakeLists.txt by hand.
#
# Usage: ./scripts/clang_tidy_fix.sh
# Run before every commit, then review the diff — not every fix is a keeper.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}"

echo "==> Building to refresh compile_commands.json"
cmake --preset dev
cmake --build --preset dev

SOURCES=(
  src/main.cpp
  src/camera.cpp
  src/detector.cpp
  src/engine.cpp
  src/reporting.cpp
  src/transforms.cpp
  src/tracker.cpp
  src/visualization.cpp
)

# Positive match on our own headers only (clang-tidy's regex has no negative lookahead)
HEADER_FILTER='include/(common/|camera\.hpp|detector\.hpp|engine\.hpp|reporting\.hpp|transforms\.hpp|tracker\.hpp|visualization\.hpp)'

echo "==> Running clang-tidy --fix"
clang-tidy -p build-ninja --fix --header-filter="${HEADER_FILTER}" "${SOURCES[@]}"

echo "==> Done. Review the diff before committing: git diff"
