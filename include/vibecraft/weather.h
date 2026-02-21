#ifndef VIBECRAFT_WEATHER_H
#define VIBECRAFT_WEATHER_H

#include "vibecraft/biome.h"

namespace vibecraft {

/// Possible weather states.
enum class WeatherState {
    kClear = 0,
    kRain,
    kSnow,
};

/// Manages world weather with biome-aware overrides.
///
/// The global weather state can be set explicitly. When queried for a
/// specific biome, the system applies overrides:
///   - Desert: always Clear (no rain or snow).
///   - Tundra: rain is converted to snow.
///
/// Weather transitions can optionally fade over a configurable duration.
class WeatherSystem {
public:
    /// Construct with default Clear weather.
    WeatherSystem();

    /// Set the global weather state.
    void SetWeather(WeatherState state);

    /// Get the current global weather state.
    WeatherState GetWeather() const;

    /// Get the effective weather for a specific biome.
    /// Applies biome-specific overrides (e.g., desert = always clear).
    WeatherState GetWeatherForBiome(BiomeType biome) const;

    /// Update weather system (for transition timing, etc.).
    /// @param dt Time step in seconds.
    void Update(float dt);

    /// Get the weather intensity (0.0 = none, 1.0 = full).
    /// Ramps up/down during transitions.
    float GetIntensity() const;

    /// Set the transition duration in seconds.
    void SetTransitionDuration(float seconds);

    /// Get the transition duration in seconds.
    float GetTransitionDuration() const;

private:
    WeatherState current_state_;
    WeatherState target_state_;
    float intensity_;
    float transition_duration_;
    bool transitioning_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_WEATHER_H
