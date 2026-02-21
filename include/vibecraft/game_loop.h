#ifndef VIBECRAFT_GAME_LOOP_H
#define VIBECRAFT_GAME_LOOP_H

#include <cstdint>

namespace vibecraft {

/// Ambient occlusion computation for block vertices.
///
/// AO is computed per-vertex based on the number of adjacent solid blocks
/// at each corner. Possible values:
///   - 3 neighbors (darkest) = 0.2
///   - 2 neighbors           = 0.4
///   - 1 neighbor            = 0.7
///   - 0 neighbors (brightest) = 1.0
namespace ao {

/// AO brightness values indexed by neighbor count.
/// Index 0 = 0 neighbors (brightest), index 3 = 3 neighbors (darkest).
constexpr float kAOValues[4] = {1.0f, 0.7f, 0.4f, 0.2f};

/// Compute the AO factor for a vertex given the number of adjacent
/// solid neighbors (0-3). Clamps to valid range.
float ComputeAO(int neighbor_count);

}  // namespace ao

/// Linear fog computation.
///
/// Fog factor is computed as: (end - dist) / (end - start), clamped to [0, 1].
/// A factor of 1.0 means no fog (fully visible), 0.0 means fully fogged.
namespace fog {

/// Compute the linear fog factor for a given distance.
/// @param dist   Distance from the camera.
/// @param start  Distance at which fog begins (factor = 1.0).
/// @param end    Distance at which fog is fully opaque (factor = 0.0).
/// @return Fog factor clamped to [0, 1].
float ComputeLinearFog(float dist, float start, float end);

}  // namespace fog

/// Fixed-timestep game loop with accumulator pattern.
///
/// The loop accumulates real elapsed time and simulates discrete ticks
/// at a fixed rate (default 20 ticks/second = 50ms per tick). The
/// interpolation factor allows smooth rendering between ticks.
class GameLoop {
public:
    /// Default tick rate: 20 ticks per second.
    static constexpr float kDefaultTickRate = 20.0f;
    static constexpr float kDefaultTickDuration = 1.0f / kDefaultTickRate;  // 0.05s = 50ms

    /// Construct a game loop with the given tick duration in seconds.
    explicit GameLoop(float tick_duration = kDefaultTickDuration);

    /// Feed elapsed real time (in seconds) into the accumulator.
    /// Returns the number of ticks that should be processed.
    int Accumulate(float elapsed_seconds);

    /// Get the interpolation factor for rendering (accumulator / tick_duration).
    /// Range: [0, 1).
    float GetInterpolation() const;

    /// Get the current accumulator value in seconds.
    float GetAccumulator() const;

    /// Get the tick duration in seconds.
    float GetTickDuration() const;

    /// Get the total number of ticks processed since creation.
    uint64_t GetTotalTicks() const;

    /// Reset the accumulator to zero.
    void Reset();

private:
    float tick_duration_;
    float accumulator_;
    uint64_t total_ticks_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_GAME_LOOP_H
