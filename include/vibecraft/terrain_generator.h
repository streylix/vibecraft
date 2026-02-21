#ifndef VIBECRAFT_TERRAIN_GENERATOR_H
#define VIBECRAFT_TERRAIN_GENERATOR_H

#include <cstdint>

#include "vibecraft/biome.h"
#include "vibecraft/cave_generator.h"
#include "vibecraft/chunk.h"
#include "vibecraft/noise.h"
#include "vibecraft/ore_generator.h"

namespace vibecraft {

/// Generates terrain for chunks using Perlin noise and biome maps.
///
/// The generator is deterministic: given the same seed, it always produces
/// the same terrain. It uses world-space block coordinates so that chunks
/// generated in any order produce seamless terrain at borders.
///
/// Terrain layers (from bottom to top):
///   - y=0: Bedrock
///   - y=1 to surface-5: Stone
///   - surface-4 to surface-1: Filler block (biome-dependent)
///   - surface: Surface block (biome-dependent)
///   - above surface: Air (+ optional trees in forest biomes)
class TerrainGenerator {
public:
    /// Construct a terrain generator with the given world seed.
    explicit TerrainGenerator(uint32_t seed);

    /// Fill a chunk with terrain based on its chunk coordinates.
    /// The chunk must already have its coordinates set.
    void GenerateChunk(Chunk& chunk) const;

    /// Get the terrain surface height at a given world block coordinate (bx, bz).
    /// Returns the y-level of the topmost solid block.
    /// This accounts for biome height modifiers with smooth blending.
    int GetHeight(int bx, int bz) const;

    /// Get the biome at a given world block coordinate (bx, bz).
    BiomeType GetBiome(int bx, int bz) const;

    /// Get a const reference to the internal BiomeMap.
    const BiomeMap& GetBiomeMap() const { return biome_map_; }

    /// Get the seed used by this generator.
    uint32_t GetSeed() const { return seed_; }

    /// Default terrain parameters.
    static constexpr int kBaseHeight = 64;
    static constexpr float kAmplitude = 40.0f;
    static constexpr float kFrequency = 0.01f;
    static constexpr int kMinHeight = 40;
    static constexpr int kMaxHeight = 150;
    static constexpr int kDirtLayers = 4;

private:
    /// Get the base terrain height (noise only, no biome modifier).
    int GetBaseHeight(int bx, int bz) const;

    /// Try to place a tree at the given world position in the chunk.
    /// The tree is a simple 1x1x4 log trunk + 3x3x2 leaf canopy.
    void PlaceTree(Chunk& chunk, int lx, int surface_y, int lz) const;

    /// Deterministic hash to decide tree placement at a world position.
    bool ShouldPlaceTree(int bx, int bz, float density) const;

    uint32_t seed_;
    PerlinNoise noise_;
    BiomeMap biome_map_;
    CaveGenerator cave_generator_;
    OreGenerator ore_generator_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_TERRAIN_GENERATOR_H
