#include "vibecraft/ore_generator.h"

#include <algorithm>
#include <queue>
#include <tuple>

namespace vibecraft {

OreGenerator::OreGenerator(uint32_t seed) : seed_(seed) {
    // Configure ore distribution table.
    // Order: coal (most common) -> iron -> gold -> diamond (rarest).
    ore_configs_ = {
        {BlockRegistry::kCoalOre,    128, 3, 8, 20},  // Coal: y=0-128, high freq
        {BlockRegistry::kIronOre,     64, 3, 6, 15},  // Iron: y=0-64, medium freq
        {BlockRegistry::kGoldOre,     32, 3, 5,  6},  // Gold: y=0-32, low freq
        {BlockRegistry::kDiamondOre,  16, 2, 4,  3},  // Diamond: y=0-16, very low freq
    };
}

uint32_t OreGenerator::HashState(uint32_t state) {
    state ^= state >> 16;
    state *= 0x45d9f3bu;
    state ^= state >> 16;
    state *= 0x45d9f3bu;
    state ^= state >> 16;
    return state;
}

float OreGenerator::RandFloat(uint32_t& state) {
    state = HashState(state + 1);
    return static_cast<float>(state & 0xFFFFu) / 65536.0f;
}

int OreGenerator::RandInt(uint32_t& state, int min_val, int max_val) {
    if (min_val >= max_val) return min_val;
    state = HashState(state + 1);
    int range = max_val - min_val + 1;
    return min_val + static_cast<int>(state % static_cast<uint32_t>(range));
}

void OreGenerator::PlaceVein(Chunk& chunk, const OreConfig& config,
                              int start_lx, int start_y, int start_lz,
                              uint32_t& rng_state) const {
    // Determine vein size.
    int target_size = RandInt(rng_state, config.min_vein_size, config.max_vein_size);

    // BFS outward from seed block.
    struct Pos {
        int x, y, z;
    };

    std::queue<Pos> queue;
    int placed = 0;

    // Try to place the seed block.
    if (start_lx >= 0 && start_lx < kChunkSizeX &&
        start_y >= 1 && start_y < kChunkSizeY &&
        start_lz >= 0 && start_lz < kChunkSizeZ &&
        start_y <= config.max_y) {
        if (chunk.GetBlock(start_lx, start_y, start_lz) == BlockRegistry::kStone) {
            chunk.SetBlock(start_lx, start_y, start_lz, config.ore_block);
            ++placed;
            queue.push({start_lx, start_y, start_lz});
        }
    }

    // BFS expansion with decreasing probability.
    // 6-connected neighbors (cardinal directions).
    static const int dx[] = {1, -1, 0, 0, 0, 0};
    static const int dy[] = {0, 0, 1, -1, 0, 0};
    static const int dz[] = {0, 0, 0, 0, 1, -1};

    while (!queue.empty() && placed < target_size) {
        Pos current = queue.front();
        queue.pop();

        for (int i = 0; i < 6; ++i) {
            if (placed >= target_size) break;

            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            int nz = current.z + dz[i];

            // Bounds check.
            if (nx < 0 || nx >= kChunkSizeX ||
                ny < 1 || ny >= kChunkSizeY ||
                nz < 0 || nz >= kChunkSizeZ) {
                continue;
            }

            // Depth check.
            if (ny > config.max_y) continue;

            // Only replace stone.
            if (chunk.GetBlock(nx, ny, nz) != BlockRegistry::kStone) continue;

            // Decreasing probability as vein grows.
            float prob = 0.6f;
            if (RandFloat(rng_state) < prob) {
                chunk.SetBlock(nx, ny, nz, config.ore_block);
                ++placed;
                queue.push({nx, ny, nz});
            }
        }
    }
}

void OreGenerator::GenerateOres(Chunk& chunk) const {
    int cx = chunk.GetChunkX();
    int cz = chunk.GetChunkZ();

    // Build a deterministic RNG state from the chunk coordinates and seed.
    uint32_t base_rng = seed_ ^ 0xABCDEF01u;
    base_rng ^= static_cast<uint32_t>(cx) * 374761393u;
    base_rng ^= static_cast<uint32_t>(cz) * 668265263u;
    base_rng = HashState(base_rng);

    for (const auto& config : ore_configs_) {
        // Separate RNG stream per ore type for determinism.
        uint32_t ore_rng = base_rng ^ (static_cast<uint32_t>(config.ore_block) * 1103515245u);
        ore_rng = HashState(ore_rng);

        for (int v = 0; v < config.veins_per_chunk; ++v) {
            // Pick a random position within the chunk for the vein seed.
            int lx = RandInt(ore_rng, 0, kChunkSizeX - 1);
            int y = RandInt(ore_rng, 1, config.max_y);
            int lz = RandInt(ore_rng, 0, kChunkSizeZ - 1);

            PlaceVein(chunk, config, lx, y, lz, ore_rng);
        }
    }
}

}  // namespace vibecraft
