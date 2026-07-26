#pragma once

// -----------------------------------------------------------------------------
// Vendored from: https://github.com/Vertical-Beach/ByteTrack-cpp
// Original file: include/ByteTrack/lapjv.h
// License: MIT (see upstream repository LICENSE)
// Note: ByteTrack-cpp itself vendored this from
//   https://github.com/gatagat/lap  (Jonker-Volgenant LAP solver).
// No local modifications.
// -----------------------------------------------------------------------------

#include <cstddef>

namespace byte_track
{
int lapjv_internal(const size_t n, double *cost[], int *x, int *y);
}