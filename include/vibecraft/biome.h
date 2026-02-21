#ifndef VIBECRAFT_BIOME_H
#define VIBECRAFT_BIOME_H

#include <array>
#include <cstdint>

#include "vibecraft/block.h"
#include "vibecraft/noise.h"

namespace vibecraft {

/// Enumeration of all biome types.
enum class BiomeType : uint8_t {
    kPlains = 0,
    kForest,
    kDesert,
    kMountains,
    kTundra,
    kCount  // Must be last; gives the total number of biome types.
};

/// Properties that define how a biome affects terrain generation.
struct BiomeProperties {
    BiomeType type;
    const char* name;
    BlockId surface_block;     // Topmost block (e.g., Grass, Sand, Snow)
    BlockId filler_block;      // Sub-surface block (e.g., Dirt, Sand)
    int height_modifier;       // Added to base terrain height
    float tree_density;        // Probability of a tree per column (0.0 = none)
};

/// Returns the properties for a given biome type.
/// The returned reference is valid for the lifetime of the program.
const BiomeProperties& GetBiomeProperties(BiomeType type);

/// Returns the total number of defined biome types (not counting kCount).
constexpr int GetBiomeCount() {
    return static_cast<int>(BiomeType::kCount);
}

/// 2D biome map that determines which biome exists at any world (x, z).
///
/// Uses two independent Perlin noise channels -- temperature and moisture --
/// to classify each column into a biome type. The noise parameters are:
///   - Frequency: 0.005
///   - Octaves: 1
///   - Temperature seed offset: 0
///   - Moisture seed offset: 1000
///
/// A simple 2D lookup table maps (temperature range, moisture range) to a
/// BiomeType.
class BiomeMap {
public:
    /// Construct a biome map with the given world seed.
    explicit BiomeMap(uint32_t seed);

    /// Get the biome type at world block coordinates (bx, bz).
    BiomeType GetBiome(int bx, int bz) const;

    /// Get the blended height modifier at world block coordinates (bx, bz).
    /// This smooths transitions between biomes by averaging height modifiers
    /// from nearby columns within a blending radius.
    float GetBlendedHeightModifier(int bx, int bz) const;

    /// Get the raw temperature noise value at (bx, bz), mapped to [0, 1].
    float GetTemperature(int bx, int bz) const;

    /// Get the raw moisture noise value at (bx, bz), mapped to [0, 1].
    float GetMoisture(int bx, int bz) const;

    /// Get the seed used by this biome map.
    uint32_t GetSeed() const { return seed_; }

    /// Blending radius in blocks for height transitions.
    static constexpr int kBlendRadius = 4;

    /// Noise frequency for temperature and moisture channels.
    static constexpr float kBiomeFrequency = 0.005f;

private:
    /// Classify a biome from temperature and moisture values in [0, 1].
    static BiomeType ClassifyBiome(float temperature, float moisture);

    uint32_t seed_;
    PerlinNoise temperature_noise_;
    PerlinNoise moisture_noise_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_BIOME_H
