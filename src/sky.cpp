#include "vibecraft/sky.h"

#include <algorithm>
#include <cmath>

#include "vibecraft/math_utils.h"

namespace vibecraft {

Sky::Sky() : time_(0) {}

void Sky::Tick() {
    time_ = (time_ + 1) % kMaxTime;
}

void Sky::Tick(int ticks) {
    // Handle negative ticks and large values correctly.
    time_ = Mod(time_ + ticks, kMaxTime);
}

int Sky::GetTime() const {
    return time_;
}

void Sky::SetTime(int time) {
    time_ = Mod(time, kMaxTime);
}

float Sky::GetSunAngle() const {
    // At time 0 (sunrise), angle = 0.
    // At time 6000 (noon), angle = 90 (directly overhead).
    // At time 12000 (sunset), angle = 180.
    // At time 18000 (midnight), angle = 270 (directly below).
    float angle = (static_cast<float>(time_) / static_cast<float>(kMaxTime)) * 360.0f;
    // Wrap to [0, 360).
    angle = std::fmod(angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle;
}

int Sky::GetAmbientLight() const {
    float day_factor = ComputeDayFactor();
    float ambient = Lerp(static_cast<float>(kNightAmbient),
                         static_cast<float>(kDayAmbient),
                         day_factor);
    return static_cast<int>(std::round(ambient));
}

glm::vec3 Sky::GetSkyColor() const {
    float day_factor = ComputeDayFactor();

    // Lerp between night and day sky colors based on the day factor.
    glm::vec3 color;
    color.r = Lerp(kNightSkyColor.r, kDaySkyColor.r, day_factor);
    color.g = Lerp(kNightSkyColor.g, kDaySkyColor.g, day_factor);
    color.b = Lerp(kNightSkyColor.b, kDaySkyColor.b, day_factor);

    return color;
}

glm::vec3 Sky::GetFogColor() const {
    // Fog color always matches sky color for seamless horizon blending.
    return GetSkyColor();
}

bool Sky::IsDay() const {
    return time_ < 12000;
}

bool Sky::IsNight() const {
    return time_ >= 12000;
}

float Sky::ComputeDayFactor() const {
    // Use the sine of the sun angle to determine the day factor.
    // sin(90) = 1 at noon, sin(270) = -1 at midnight.
    float angle_rad = GetSunAngle() * (static_cast<float>(M_PI) / 180.0f);
    float sun_sin = std::sin(angle_rad);

    // Map from [-1, 1] to [0, 1].
    float factor = (sun_sin + 1.0f) / 2.0f;

    return Clamp(factor, 0.0f, 1.0f);
}

}  // namespace vibecraft
