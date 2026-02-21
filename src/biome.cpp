#include "vibecraft/biome.h"

#include <cmath>

namespace vibecraft {

namespace {

/// Static table of biome properties, indexed by BiomeType ordinal.
const std::array<BiomeProperties, GetBiomeCount()> kBiomeTable = {{
    {BiomeType::kPlains,    "Plains",    BlockRegistry::kGrass, BlockRegistry::kDirt,  0,   0.02f},
    {BiomeType::kForest,    "Forest",    BlockRegistry::kGrass, BlockRegistry::kDirt,  5,   0.15f},
    {BiomeType::kDesert,    "Desert",    BlockRegistry::kSand,  BlockRegistry::kSand,  -10, 0.00f},
    {BiomeType::kMountains, "Mountains", BlockRegistry::kStone, BlockRegistry::kStone, 30,  0.00f},
    {BiomeType::kTundra,    "Tundra",    BlockRegistry::kSnow,  BlockRegistry::kDirt,  -5,  0.02f},
}};

}  // namespace

const BiomeProperties& GetBiomeProperties(BiomeType type) {
    int index = static_cast<int>(type);
    if (index < 0 || index >= GetBiomeCount()) {
        // Fallback to Plains for invalid types.
        return kBiomeTable[0];
    }
    return kBiomeTable[static_cast<size_t>(index)];
}

// ---------------------------------------------------------------------------
// BiomeMap
// ---------------------------------------------------------------------------

BiomeMap::BiomeMap(uint32_t seed)
    : seed_(seed),
      temperature_noise_(seed),
      moisture_noise_(seed + 1000) {}

float BiomeMap::GetTemperature(int bx, int bz) const {
    float raw = temperature_noise_.Noise2D(
        static_cast<float>(bx) * kBiomeFrequency,
        static_cast<float>(bz) * kBiomeFrequency);
    // Map from [-1, 1] to [0, 1].
    return (raw + 1.0f) * 0.5f;
}

float BiomeMap::GetMoisture(int bx, int bz) const {
    float raw = moisture_noise_.Noise2D(
        static_cast<float>(bx) * kBiomeFrequency,
        static_cast<float>(bz) * kBiomeFrequency);
    // Map from [-1, 1] to [0, 1].
    return (raw + 1.0f) * 0.5f;
}

BiomeType BiomeMap::ClassifyBiome(float temperature, float moisture) {
    // Temperature ranges:
    //   cold:   [0.0, 0.3)
    //   warm:   [0.3, 0.6)
    //   hot:    [0.6, 1.0]
    //
    // Moisture ranges:
    //   dry:    [0.0, 0.4)
    //   wet:    [0.4, 1.0]
    //
    // Lookup:
    //   hot  + dry  -> Desert
    //   hot  + wet  -> Plains
    //   warm + dry  -> Plains
    //   warm + wet  -> Forest
    //   cold + dry  -> Tundra
    //   cold + wet  -> Tundra
    //
    // Mountains are selected when temperature is in the mid-warm range
    // AND moisture is moderate, giving them distinct high terrain zones.

    if (temperature < 0.3f) {
        // Cold => Tundra
        return BiomeType::kTundra;
    } else if (temperature < 0.6f) {
        // Warm
        if (moisture < 0.4f) {
            // Warm + dry => Mountains (rocky, dry highlands)
            return BiomeType::kMountains;
        } else {
            // Warm + wet => Forest
            return BiomeType::kForest;
        }
    } else {
        // Hot
        if (moisture < 0.4f) {
            // Hot + dry => Desert
            return BiomeType::kDesert;
        } else {
            // Hot + wet => Plains
            return BiomeType::kPlains;
        }
    }
}

BiomeType BiomeMap::GetBiome(int bx, int bz) const {
    float temp = GetTemperature(bx, bz);
    float moist = GetMoisture(bx, bz);
    return ClassifyBiome(temp, moist);
}

float BiomeMap::GetBlendedHeightModifier(int bx, int bz) const {
    // Average the height modifier over a (2*kBlendRadius+1)^2 area.
    float total = 0.0f;
    int count = 0;
    for (int dx = -kBlendRadius; dx <= kBlendRadius; ++dx) {
        for (int dz = -kBlendRadius; dz <= kBlendRadius; ++dz) {
            BiomeType biome = GetBiome(bx + dx, bz + dz);
            total += static_cast<float>(GetBiomeProperties(biome).height_modifier);
            ++count;
        }
    }
    return total / static_cast<float>(count);
}

}  // namespace vibecraft
