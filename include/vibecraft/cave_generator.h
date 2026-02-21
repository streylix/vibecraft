#ifndef VIBECRAFT_CAVE_GENERATOR_H
#define VIBECRAFT_CAVE_GENERATOR_H

#include <cstdint>

#include "vibecraft/chunk.h"
#include "vibecraft/noise.h"

namespace vibecraft {

/// Generates caves by carving air pockets into terrain using 3D Perlin noise.
///
/// Two noise layers at different frequencies are combined to create varied,
/// worm-like tunnel systems. A block is carved to air if the combined noise
/// value exceeds a threshold. Bedrock at y=0 is never carved.
///
/// Cave generation is deterministic: the same seed always produces the same
/// cave pattern.
class CaveGenerator {
public:
    /// Construct a cave generator with the given world seed.
    explicit CaveGenerator(uint32_t seed);

    /// Carve caves into an already-populated chunk.
    /// The chunk must have terrain blocks placed before calling this.
    /// Bedrock at y=0 is preserved. Only solid blocks are carved.
    void CarveCaves(Chunk& chunk) const;

    /// Get the seed used by this generator.
    uint32_t GetSeed() const { return seed_; }

    /// Combined noise threshold. A block is carved when the weighted
    /// combination of primary and secondary noise exceeds this value.
    static constexpr float kCaveThreshold = 0.45f;

    /// Primary cave noise frequency (lower = larger, worm-like tunnels).
    static constexpr float kCaveFrequency = 0.05f;

    /// Secondary cave noise frequency (higher = finer detail/variety).
    static constexpr float kCaveFrequency2 = 0.08f;

    /// Weight of primary noise in the combination (secondary = 1 - this).
    static constexpr float kPrimaryWeight = 0.7f;

    /// Minimum y-level for cave carving (y=0 is bedrock, never carved).
    static constexpr int kMinCaveY = 1;

    /// Maximum y-level for cave carving.
    static constexpr int kMaxCaveY = 128;

private:
    uint32_t seed_;
    PerlinNoise noise_primary_;
    PerlinNoise noise_secondary_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_CAVE_GENERATOR_H
