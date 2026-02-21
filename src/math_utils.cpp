#include "vibecraft/math_utils.h"

#include <cmath>

namespace vibecraft {

float Lerp(float a, float b, float t) {
    return a + t * (b - a);
}

int IntFloor(float x) {
    return static_cast<int>(std::floor(x));
}

int Mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

}  // namespace vibecraft
