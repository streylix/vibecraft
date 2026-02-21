#include "vibecraft/cave_generator.h"

#include "vibecraft/block.h"

namespace vibecraft {

CaveGenerator::CaveGenerator(uint32_t seed)
    : seed_(seed),
      noise_primary_(seed ^ 0x12345678u),
      noise_secondary_(seed ^ 0x87654321u) {}

void CaveGenerator::CarveCaves(Chunk& chunk) const {
    int cx = chunk.GetChunkX();
    int cz = chunk.GetChunkZ();

    // World-space origin of this chunk.
    int world_x0 = cx * kChunkSizeX;
    int world_z0 = cz * kChunkSizeZ;

    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            int bx = world_x0 + lx;
            int bz = world_z0 + lz;

            for (int y = kMinCaveY; y <= kMaxCaveY; ++y) {
                BlockId current = chunk.GetBlock(lx, y, lz);

                // Only carve solid, non-bedrock blocks.
                if (current == BlockRegistry::kAir ||
                    current == BlockRegistry::kBedrock) {
                    continue;
                }

                // Don't carve liquids.
                if (current == BlockRegistry::kWater ||
                    current == BlockRegistry::kLava) {
                    continue;
                }

                // Sample primary 3D noise.
                float wx = static_cast<float>(bx) * kCaveFrequency;
                float wy = static_cast<float>(y) * kCaveFrequency;
                float wz = static_cast<float>(bz) * kCaveFrequency;

                float primary = noise_primary_.Noise3D(wx, wy, wz);

                // Sample secondary 3D noise for variety.
                float wx2 = static_cast<float>(bx) * kCaveFrequency2;
                float wy2 = static_cast<float>(y) * kCaveFrequency2;
                float wz2 = static_cast<float>(bz) * kCaveFrequency2;

                float secondary = noise_secondary_.Noise3D(wx2, wy2, wz2);

                // Combine the two noise layers with weighted average.
                float combined = kPrimaryWeight * primary +
                                 (1.0f - kPrimaryWeight) * secondary;

                // Carve if combined noise exceeds the threshold.
                if (combined > kCaveThreshold) {
                    chunk.SetBlock(lx, y, lz, BlockRegistry::kAir);
                }
            }
        }
    }
}

}  // namespace vibecraft
