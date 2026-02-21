#include "vibecraft/terrain_generator.h"

#include <algorithm>
#include <cmath>

#include "vibecraft/block.h"

namespace vibecraft {

TerrainGenerator::TerrainGenerator(uint32_t seed)
    : seed_(seed), noise_(seed) {}

int TerrainGenerator::GetHeight(int bx, int bz) const {
    // Use world-space block coordinates directly so that terrain is seamless
    // regardless of chunk generation order.
    float noise_val = noise_.OctaveNoise2D(
        static_cast<float>(bx) * kFrequency,
        static_cast<float>(bz) * kFrequency,
        4,      // octaves
        0.5f,   // persistence
        2.0f,   // lacunarity
        1.0f    // base frequency (already applied via kFrequency)
    );

    // noise_val is in [-1, 1]. Map to [kMinHeight, kMaxHeight].
    // base_height + noise_val * amplitude, then clamp.
    int height = kBaseHeight + static_cast<int>(std::round(noise_val * kAmplitude));
    height = std::clamp(height, kMinHeight, kMaxHeight);
    return height;
}

void TerrainGenerator::GenerateChunk(Chunk& chunk) const {
    int cx = chunk.GetChunkX();
    int cz = chunk.GetChunkZ();

    // World-space origin of this chunk.
    int world_x0 = cx * kChunkSizeX;
    int world_z0 = cz * kChunkSizeZ;

    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            int bx = world_x0 + lx;
            int bz = world_z0 + lz;

            int surface = GetHeight(bx, bz);

            // Bedrock at y=0.
            chunk.SetBlock(lx, 0, lz, BlockRegistry::kBedrock);

            // Stone from y=1 up to surface - kDirtLayers - 1
            // (i.e., surface - 5 when kDirtLayers == 4).
            int stone_top = surface - kDirtLayers - 1;
            for (int y = 1; y <= stone_top; ++y) {
                chunk.SetBlock(lx, y, lz, BlockRegistry::kStone);
            }

            // Dirt for kDirtLayers blocks below the surface.
            // surface - kDirtLayers to surface - 1.
            int dirt_bottom = surface - kDirtLayers;
            // Dirt bottom must be at least 1 (don't overwrite bedrock).
            dirt_bottom = std::max(dirt_bottom, 1);
            for (int y = dirt_bottom; y <= surface - 1; ++y) {
                chunk.SetBlock(lx, y, lz, BlockRegistry::kDirt);
            }

            // Grass at surface.
            if (surface >= 1) {
                chunk.SetBlock(lx, surface, lz, BlockRegistry::kGrass);
            }

            // Everything above surface is already Air (chunk default).
        }
    }
}

}  // namespace vibecraft
