#include "vibecraft/weather.h"

#include <algorithm>

namespace vibecraft {

WeatherSystem::WeatherSystem()
    : current_state_(WeatherState::kClear),
      target_state_(WeatherState::kClear),
      intensity_(0.0f),
      transition_duration_(5.0f),
      transitioning_(false) {}

void WeatherSystem::SetWeather(WeatherState state) {
    if (state == current_state_ && !transitioning_) {
        return;
    }

    WeatherState old_state = current_state_;
    current_state_ = state;
    target_state_ = state;

    if (state == WeatherState::kClear && old_state != WeatherState::kClear) {
        // Transitioning to clear: fade out intensity.
        transitioning_ = true;
    } else if (state != WeatherState::kClear &&
               old_state == WeatherState::kClear) {
        // Transitioning from clear to precipitation: fade in.
        intensity_ = 0.0f;
        transitioning_ = true;
    } else {
        // Same category or same state: no transition needed.
        transitioning_ = false;
    }
}

WeatherState WeatherSystem::GetWeather() const {
    return current_state_;
}

WeatherState WeatherSystem::GetWeatherForBiome(BiomeType biome) const {
    // Desert never gets precipitation.
    if (biome == BiomeType::kDesert) {
        return WeatherState::kClear;
    }

    // Tundra converts rain to snow.
    if (biome == BiomeType::kTundra) {
        if (current_state_ == WeatherState::kRain) {
            return WeatherState::kSnow;
        }
        return current_state_;
    }

    return current_state_;
}

void WeatherSystem::Update(float dt) {
    if (!transitioning_) {
        return;
    }

    float rate = (transition_duration_ > 0.0f)
                     ? (1.0f / transition_duration_)
                     : 1000.0f;

    if (current_state_ == WeatherState::kClear) {
        // Fading out.
        intensity_ -= rate * dt;
        if (intensity_ <= 0.0f) {
            intensity_ = 0.0f;
            transitioning_ = false;
        }
    } else {
        // Fading in.
        intensity_ += rate * dt;
        if (intensity_ >= 1.0f) {
            intensity_ = 1.0f;
            transitioning_ = false;
        }
    }
}

float WeatherSystem::GetIntensity() const {
    return intensity_;
}

void WeatherSystem::SetTransitionDuration(float seconds) {
    transition_duration_ = std::max(0.0f, seconds);
}

float WeatherSystem::GetTransitionDuration() const {
    return transition_duration_;
}

}  // namespace vibecraft
