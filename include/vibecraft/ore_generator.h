#ifndef VIBECRAFT_ORE_GENERATOR_H
#define VIBECRAFT_ORE_GENERATOR_H

#include <cstdint>
#include <vector>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"

namespace vibecraft {

/// Configuration for a single ore type's distribution.
struct OreConfig {
    BlockId ore_block;    ///< The block ID of this ore.
    int max_y;            ///< Maximum y-level where this ore can appear.
    int min_vein_size;    ///< Minimum blocks per vein.
    int max_vein_size;    ///< Maximum blocks per vein.
    int veins_per_chunk;  ///< Approximate number of vein attempts per chunk.
};

/// Generates ore veins within chunks, placing ore blocks into stone at
/// appropriate depth ranges.
///
/// Each ore type has a depth range, vein size range, and frequency. Veins
/// are grown from a seed block using BFS outward expansion with decreasing
/// probability. Only stone blocks are replaced.
///
/// Ore generation is deterministic: the same seed always produces the same
/// ore placement.
class OreGenerator {
public:
    /// Construct an ore generator with the given world seed.
    explicit OreGenerator(uint32_t seed);

    /// Place all ore types into an already-populated chunk.
    /// Ores only replace stone blocks. This should be called after
    /// terrain generation and cave carving.
    void GenerateOres(Chunk& chunk) const;

    /// Get the seed used by this generator.
    uint32_t GetSeed() const { return seed_; }

    /// Get the ore configurations (for testing/inspection).
    const std::vector<OreConfig>& GetOreConfigs() const { return ore_configs_; }

private:
    /// Attempt to place a single vein of the given ore type in the chunk.
    /// @param chunk     The chunk to place ore into.
    /// @param config    The ore type configuration.
    /// @param start_lx  Local x of the vein seed block.
    /// @param start_y   Y-level of the vein seed block.
    /// @param start_lz  Local z of the vein seed block.
    /// @param rng_state Mutable RNG state for deterministic vein growth.
    void PlaceVein(Chunk& chunk, const OreConfig& config,
                   int start_lx, int start_y, int start_lz,
                   uint32_t& rng_state) const;

    /// Simple deterministic hash for RNG.
    static uint32_t HashState(uint32_t state);

    /// Get a pseudo-random float in [0, 1) from the RNG state.
    static float RandFloat(uint32_t& state);

    /// Get a pseudo-random int in [min, max] from the RNG state.
    static int RandInt(uint32_t& state, int min_val, int max_val);

    uint32_t seed_;
    std::vector<OreConfig> ore_configs_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_ORE_GENERATOR_H
