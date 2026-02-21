#ifndef VIBECRAFT_MATH_UTILS_H
#define VIBECRAFT_MATH_UTILS_H

#include <cmath>

namespace vibecraft {

/// Linear interpolation between a and b by factor t.
/// Returns a when t=0, b when t=1.
float Lerp(float a, float b, float t);

/// Clamp value to the range [lo, hi].
template <typename T>
T Clamp(T value, T lo, T hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

/// Floor a float to int, correctly handling negative numbers.
/// e.g., IntFloor(-0.1f) == -1, IntFloor(3.7f) == 3
int IntFloor(float x);

/// Euclidean modulo — always returns a non-negative result.
/// e.g., Mod(-1, 16) == 15
int Mod(int a, int b);

}  // namespace vibecraft

#endif  // VIBECRAFT_MATH_UTILS_H
