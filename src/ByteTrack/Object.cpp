// -----------------------------------------------------------------------------
// Vendored from: https://github.com/Vertical-Beach/ByteTrack-cpp
// Original file: src/Object.cpp
// License: MIT (see upstream repository LICENSE)
// No local modifications.
// -----------------------------------------------------------------------------

#include "ByteTrack/Object.h"

byte_track::Object::Object(const Rect<float> &_rect,
                           const int &_label,
                           const float &_prob) : rect(_rect), label(_label), prob(_prob)
{
}