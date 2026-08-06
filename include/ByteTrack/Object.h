#pragma once

// -----------------------------------------------------------------------------
// Vendored from: https://github.com/Vertical-Beach/ByteTrack-cpp
// Original file: include/ByteTrack/Object.h
// License: MIT (see upstream repository LICENSE)
// No local modifications.
// -----------------------------------------------------------------------------

#include "ByteTrack/Rect.h"

namespace byte_track
{
struct Object
{
    Rect<float> rect;
    int label;
    float prob;

    Object(const Rect<float> &_rect,
           const int &_label,
           const float &_prob);
};
}