#ifndef VIBECRAFT_SKY_H
#define VIBECRAFT_SKY_H

#include <glm/glm.hpp>

namespace vibecraft {

/// Manages the day/night cycle, computing sun/moon angle, ambient light,
/// sky color, and fog color based on in-game time.
///
/// Time ranges from 0 to 24000 ticks (wraps around).
///   - 0     = sunrise
///   - 6000  = noon (sun directly overhead)
///   - 12000 = sunset / dusk
///   - 18000 = midnight (sun directly below)
///
/// A full day cycle is 24000 ticks = 20 minutes at 20 ticks/second.
class Sky {
public:
    /// Maximum time value before wrapping.
    static constexpr int kMaxTime = 24000;

    /// Ambient light at peak daytime (noon).
    static constexpr int kDayAmbient = 15;

    /// Ambient light at peak nighttime (midnight).
    static constexpr int kNightAmbient = 4;

    /// Daytime sky color (blue).
    static constexpr glm::vec3 kDaySkyColor{0.5f, 0.7f, 1.0f};

    /// Nighttime sky color (near-black).
    static constexpr glm::vec3 kNightSkyColor{0.01f, 0.01f, 0.05f};

    /// Sunset/sunrise sky color (orange).
    static constexpr glm::vec3 kSunsetSkyColor{0.9f, 0.5f, 0.2f};

    /// Construct a Sky with time starting at 0.
    Sky();

    /// Advance time by one tick. Time wraps at kMaxTime.
    void Tick();

    /// Advance time by the given number of ticks. Time wraps at kMaxTime.
    void Tick(int ticks);

    /// Get the current time in ticks [0, kMaxTime).
    int GetTime() const;

    /// Set the time directly (will be wrapped to [0, kMaxTime)).
    void SetTime(int time);

    /// Get the sun angle in degrees.
    /// At noon (6000), angle = 90 (overhead).
    /// At midnight (18000), angle = 270 (directly below).
    /// Angle = (time / 24000) * 360 + 90, so at time 0 angle = 90
    /// offset so noon = 90 degrees.
    float GetSunAngle() const;

    /// Get the current ambient light level (integer, 4..15).
    /// 15 at noon, 4 at midnight, smoothly interpolated.
    int GetAmbientLight() const;

    /// Get the current sky color as an RGB vec3.
    /// Blue during day, dark at night, orange at dawn/dusk transitions.
    glm::vec3 GetSkyColor() const;

    /// Get the fog color. Always matches the sky color for seamless horizon.
    glm::vec3 GetFogColor() const;

    /// Check if it is currently daytime (roughly ticks 0-12000).
    bool IsDay() const;

    /// Check if it is currently nighttime (roughly ticks 12000-24000).
    bool IsNight() const;

private:
    /// Compute a normalized day factor in [0, 1] where 1 = full day (noon)
    /// and 0 = full night (midnight). Used for ambient and color lerping.
    float ComputeDayFactor() const;

    /// Current time in ticks [0, kMaxTime).
    int time_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_SKY_H
