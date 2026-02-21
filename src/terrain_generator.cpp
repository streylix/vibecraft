#include "vibecraft/terrain_generator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>

#include "vibecraft/block.h"

namespace vibecraft {

TerrainGenerator::TerrainGenerator(uint32_t seed)
    : seed_(seed), noise_(seed), biome_map_(seed) {}

int TerrainGenerator::GetBaseHeight(int bx, int bz) const {
    float noise_val = noise_.OctaveNoise2D(
        static_cast<float>(bx) * kFrequency,
        static_cast<float>(bz) * kFrequency,
        4,      // octaves
        0.5f,   // persistence
        2.0f,   // lacunarity
        1.0f    // base frequency (already applied via kFrequency)
    );

    // noise_val is in [-1, 1]. Map to base height + amplitude.
    int height = kBaseHeight + static_cast<int>(std::round(noise_val * kAmplitude));
    return height;
}

int TerrainGenerator::GetHeight(int bx, int bz) const {
    int base = GetBaseHeight(bx, bz);
    float biome_mod = biome_map_.GetBlendedHeightModifier(bx, bz);
    int height = base + static_cast<int>(std::round(biome_mod));
    height = std::clamp(height, kMinHeight, kMaxHeight);
    return height;
}

BiomeType TerrainGenerator::GetBiome(int bx, int bz) const {
    return biome_map_.GetBiome(bx, bz);
}

bool TerrainGenerator::ShouldPlaceTree(int bx, int bz, float density) const {
    if (density <= 0.0f) return false;

    // Deterministic hash based on world position and seed.
    // Uses a simple hash to produce a pseudo-random value in [0, 1).
    uint32_t h = seed_ ^ 0xDEADBEEF;
    h ^= static_cast<uint32_t>(bx) * 374761393u;
    h ^= static_cast<uint32_t>(bz) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h = h ^ (h >> 16);

    float value = static_cast<float>(h & 0xFFFF) / 65536.0f;
    return value < density;
}

void TerrainGenerator::PlaceTree(Chunk& chunk, int lx, int surface_y, int lz) const {
    // Simple tree: 1x1x4 log trunk + 3x3x2 leaf canopy on top.
    // Trunk: 4 blocks of OakLog above the surface.
    constexpr int kTrunkHeight = 4;
    constexpr int kCanopyRadius = 1;  // 3x3
    constexpr int kCanopyHeight = 2;

    // Don't place trees too close to chunk edges (canopy would extend outside).
    if (lx < kCanopyRadius || lx >= kChunkSizeX - kCanopyRadius) return;
    if (lz < kCanopyRadius || lz >= kChunkSizeZ - kCanopyRadius) return;

    // Check that tree fits vertically.
    int tree_top = surface_y + kTrunkHeight + kCanopyHeight;
    if (tree_top >= kChunkSizeY) return;

    // Place trunk.
    for (int dy = 1; dy <= kTrunkHeight; ++dy) {
        chunk.SetBlock(lx, surface_y + dy, lz, BlockRegistry::kOakLog);
    }

    // Place canopy (3x3x2 centered on trunk top).
    int canopy_base = surface_y + kTrunkHeight + 1;
    for (int dy = 0; dy < kCanopyHeight; ++dy) {
        for (int dx = -kCanopyRadius; dx <= kCanopyRadius; ++dx) {
            for (int dz = -kCanopyRadius; dz <= kCanopyRadius; ++dz) {
                int cx = lx + dx;
                int cy = canopy_base + dy;
                int cz = lz + dz;
                // Don't overwrite the trunk with leaves.
                if (dx == 0 && dz == 0 && dy == 0) continue;
                if (cx >= 0 && cx < kChunkSizeX &&
                    cz >= 0 && cz < kChunkSizeZ &&
                    cy < kChunkSizeY) {
                    // Only place leaves if the space is air.
                    if (chunk.GetBlock(cx, cy, cz) == BlockRegistry::kAir) {
                        chunk.SetBlock(cx, cy, cz, BlockRegistry::kOakLeaves);
                    }
                }
            }
        }
    }
}

void TerrainGenerator::GenerateChunk(Chunk& chunk) const {
    int cx = chunk.GetChunkX();
    int cz = chunk.GetChunkZ();

    // World-space origin of this chunk.
    int world_x0 = cx * kChunkSizeX;
    int world_z0 = cz * kChunkSizeZ;

    // First pass: terrain blocks.
    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            int bx = world_x0 + lx;
            int bz = world_z0 + lz;

            int surface = GetHeight(bx, bz);
            BiomeType biome = biome_map_.GetBiome(bx, bz);
            const BiomeProperties& props = GetBiomeProperties(biome);

            // Bedrock at y=0.
            chunk.SetBlock(lx, 0, lz, BlockRegistry::kBedrock);

            // Stone from y=1 up to surface - kDirtLayers - 1.
            int stone_top = surface - kDirtLayers - 1;
            for (int y = 1; y <= stone_top; ++y) {
                chunk.SetBlock(lx, y, lz, BlockRegistry::kStone);
            }

            // Filler (biome-dependent) for kDirtLayers blocks below the surface.
            int filler_bottom = std::max(surface - kDirtLayers, 1);
            for (int y = filler_bottom; y <= surface - 1; ++y) {
                chunk.SetBlock(lx, y, lz, props.filler_block);
            }

            // Surface block (biome-dependent) at the top.
            if (surface >= 1) {
                chunk.SetBlock(lx, surface, lz, props.surface_block);
            }

            // Everything above surface is already Air (chunk default).
        }
    }

    // Second pass: trees.
    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            int bx = world_x0 + lx;
            int bz = world_z0 + lz;

            BiomeType biome = biome_map_.GetBiome(bx, bz);
            const BiomeProperties& props = GetBiomeProperties(biome);

            if (props.tree_density > 0.0f && ShouldPlaceTree(bx, bz, props.tree_density)) {
                int surface = GetHeight(bx, bz);
                PlaceTree(chunk, lx, surface, lz);
            }
        }
    }
}

}  // namespace vibecraft
