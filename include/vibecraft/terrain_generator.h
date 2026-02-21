#ifndef VIBECRAFT_TERRAIN_GENERATOR_H
#define VIBECRAFT_TERRAIN_GENERATOR_H

#include <cstdint>

#include "vibecraft/chunk.h"
#include "vibecraft/noise.h"

namespace vibecraft {

/// Generates terrain for chunks using Perlin noise.
///
/// The generator is deterministic: given the same seed, it always produces
/// the same terrain. It uses world-space block coordinates so that chunks
/// generated in any order produce seamless terrain at borders.
///
/// Terrain layers (from bottom to top):
///   - y=0: Bedrock
///   - y=1 to surface-5: Stone
///   - surface-4 to surface-1: Dirt
///   - surface: Grass
///   - above surface: Air
class TerrainGenerator {
public:
    /// Construct a terrain generator with the given world seed.
    explicit TerrainGenerator(uint32_t seed);

    /// Fill a chunk with terrain based on its chunk coordinates.
    /// The chunk must already have its coordinates set.
    void GenerateChunk(Chunk& chunk) const;

    /// Get the terrain surface height at a given world block coordinate (bx, bz).
    /// Returns the y-level of the topmost solid block (grass).
    int GetHeight(int bx, int bz) const;

    /// Get the seed used by this generator.
    uint32_t GetSeed() const { return seed_; }

    /// Default terrain parameters.
    static constexpr int kBaseHeight = 64;
    static constexpr float kAmplitude = 40.0f;
    static constexpr float kFrequency = 0.01f;
    static constexpr int kMinHeight = 40;
    static constexpr int kMaxHeight = 120;
    static constexpr int kDirtLayers = 4;

private:
    uint32_t seed_;
    PerlinNoise noise_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_TERRAIN_GENERATOR_H
