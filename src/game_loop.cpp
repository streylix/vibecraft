#include "vibecraft/game_loop.h"

#include <algorithm>

namespace vibecraft {

namespace ao {

float ComputeAO(int neighbor_count) {
    int clamped = std::clamp(neighbor_count, 0, 3);
    return kAOValues[clamped];
}

}  // namespace ao

namespace fog {

float ComputeLinearFog(float dist, float start, float end) {
    if (end <= start) {
        return 1.0f;  // Degenerate case: no fog.
    }
    float factor = (end - dist) / (end - start);
    return std::clamp(factor, 0.0f, 1.0f);
}

}  // namespace fog

GameLoop::GameLoop(float tick_duration)
    : tick_duration_(tick_duration),
      accumulator_(0.0f),
      total_ticks_(0) {}

int GameLoop::Accumulate(float elapsed_seconds) {
    accumulator_ += elapsed_seconds;
    int ticks = 0;
    while (accumulator_ >= tick_duration_) {
        accumulator_ -= tick_duration_;
        ++ticks;
        ++total_ticks_;
    }
    return ticks;
}

float GameLoop::GetInterpolation() const {
    if (tick_duration_ <= 0.0f) {
        return 0.0f;
    }
    return accumulator_ / tick_duration_;
}

float GameLoop::GetAccumulator() const {
    return accumulator_;
}

float GameLoop::GetTickDuration() const {
    return tick_duration_;
}

uint64_t GameLoop::GetTotalTicks() const {
    return total_ticks_;
}

void GameLoop::Reset() {
    accumulator_ = 0.0f;
}

}  // namespace vibecraft
