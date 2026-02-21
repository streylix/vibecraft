#include <gtest/gtest.h>

#include <cmath>
#include <numeric>
#include <set>
#include <vector>

#include "vibecraft/biome.h"
#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/terrain_generator.h"

// M11: Terrain Generation (Basic) -- updated for M16 biome integration.

namespace {

constexpr uint32_t kTestSeed = 12345;

/// Helper: generate a chunk at given chunk coords with the test seed.
vibecraft::Chunk MakeChunk(const vibecraft::TerrainGenerator& gen, int cx, int cz) {
    vibecraft::Chunk chunk(cx, cz);
    gen.GenerateChunk(chunk);
    return chunk;
}

/// Set of block types that are vegetation / tree parts (not terrain surface).
bool IsVegetation(vibecraft::BlockId id) {
    return id == vibecraft::BlockRegistry::kOakLog ||
           id == vibecraft::BlockRegistry::kOakLeaves;
}

/// Helper: find the terrain surface Y (highest non-air, non-vegetation block).
int FindTerrainSurface(const vibecraft::Chunk& chunk, int lx, int lz) {
    for (int y = vibecraft::kChunkSizeY - 1; y >= 0; --y) {
        vibecraft::BlockId id = chunk.GetBlock(lx, y, lz);
        if (id != vibecraft::BlockRegistry::kAir && !IsVegetation(id)) {
            return y;
        }
    }
    return -1;
}

/// Helper: find the highest non-air Y in a column (local coords).
int FindSurface(const vibecraft::Chunk& chunk, int lx, int lz) {
    for (int y = vibecraft::kChunkSizeY - 1; y >= 0; --y) {
        if (chunk.GetBlock(lx, y, lz) != vibecraft::BlockRegistry::kAir) {
            return y;
        }
    }
    return -1;
}

/// Set of valid biome surface blocks.
bool IsSurfaceBlock(vibecraft::BlockId id) {
    return id == vibecraft::BlockRegistry::kGrass ||
           id == vibecraft::BlockRegistry::kSand ||
           id == vibecraft::BlockRegistry::kSnow ||
           id == vibecraft::BlockRegistry::kStone;
}

}  // namespace

TEST(TerrainGen, BedrockAtZero) {
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 0, 0);

    // Check every column in the chunk: y=0 must be bedrock.
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            EXPECT_EQ(chunk.GetBlock(x, 0, z), vibecraft::BlockRegistry::kBedrock)
                << "at local (" << x << ", 0, " << z << ")";
        }
    }

    // Also test a negative-coordinate chunk.
    auto neg_chunk = MakeChunk(gen, -3, -5);
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            EXPECT_EQ(neg_chunk.GetBlock(x, 0, z), vibecraft::BlockRegistry::kBedrock)
                << "at neg chunk local (" << x << ", 0, " << z << ")";
        }
    }
}

TEST(TerrainGen, AirAtTop) {
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 0, 0);

    // y=255 must always be air.
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            EXPECT_EQ(chunk.GetBlock(x, 255, z), vibecraft::BlockRegistry::kAir)
                << "at local (" << x << ", 255, " << z << ")";
        }
    }
}

TEST(TerrainGen, BiomeSurface) {
    // With biomes, the surface block depends on the biome type.
    // Verify that every column's surface block matches the biome's definition.
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 1, 2);

    int world_x0 = 1 * vibecraft::kChunkSizeX;
    int world_z0 = 2 * vibecraft::kChunkSizeZ;

    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            int surface = FindTerrainSurface(chunk, x, z);
            ASSERT_GE(surface, 0) << "Column (" << x << ", " << z << ") is all air";

            int bx = world_x0 + x;
            int bz = world_z0 + z;
            vibecraft::BiomeType biome = gen.GetBiome(bx, bz);
            const auto& props = vibecraft::GetBiomeProperties(biome);

            EXPECT_EQ(chunk.GetBlock(x, surface, z), props.surface_block)
                << "at local (" << x << ", " << surface << ", " << z << ")";
        }
    }
}

TEST(TerrainGen, FillerBelowSurface) {
    // With biomes, the filler block (below surface) depends on the biome type.
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 0, 0);

    int world_x0 = 0;
    int world_z0 = 0;

    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            int surface = FindTerrainSurface(chunk, x, z);
            ASSERT_GE(surface, 5)
                << "Surface too low for full filler layer at (" << x << ", " << z << ")";

            int bx = world_x0 + x;
            int bz = world_z0 + z;
            vibecraft::BiomeType biome = gen.GetBiome(bx, bz);
            const auto& props = vibecraft::GetBiomeProperties(biome);

            for (int dy = 1; dy <= 4; ++dy) {
                EXPECT_EQ(chunk.GetBlock(x, surface - dy, z), props.filler_block)
                    << "at local (" << x << ", " << (surface - dy) << ", " << z << ")";
            }
        }
    }
}

/// Check whether a block id is stone or an ore that replaces stone.
/// After M17 (caves & ores), the stone layer may contain ore blocks and
/// air pockets from cave carving.
bool IsStoneOrOre(vibecraft::BlockId id) {
    return id == vibecraft::BlockRegistry::kStone ||
           id == vibecraft::BlockRegistry::kCoalOre ||
           id == vibecraft::BlockRegistry::kIronOre ||
           id == vibecraft::BlockRegistry::kGoldOre ||
           id == vibecraft::BlockRegistry::kDiamondOre;
}

TEST(TerrainGen, StoneBelowFiller) {
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 0, 0);

    // Stone (or ore / cave air from M17) from y=1 up to surface - 5.
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            int surface = FindTerrainSurface(chunk, x, z);
            ASSERT_GE(surface, 6)
                << "Surface too low for stone layer at (" << x << ", " << z << ")";

            for (int y = 1; y <= surface - 5; ++y) {
                vibecraft::BlockId block = chunk.GetBlock(x, y, z);
                // After cave generation (M17), some blocks may be carved to air.
                // After ore generation (M17), some stone blocks become ores.
                EXPECT_TRUE(IsStoneOrOre(block) ||
                            block == vibecraft::BlockRegistry::kAir)
                    << "Unexpected block " << static_cast<int>(block)
                    << " at local (" << x << ", " << y << ", " << z << ")";
            }
        }
    }
}

TEST(TerrainGen, Deterministic) {
    vibecraft::TerrainGenerator gen1(42);
    vibecraft::TerrainGenerator gen2(42);

    auto chunk1 = MakeChunk(gen1, 3, -7);
    auto chunk2 = MakeChunk(gen2, 3, -7);

    // Every block in the chunk must be identical.
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            for (int y = 0; y < vibecraft::kChunkSizeY; ++y) {
                EXPECT_EQ(chunk1.GetBlock(x, y, z), chunk2.GetBlock(x, y, z))
                    << "Mismatch at (" << x << ", " << y << ", " << z << ")";
            }
        }
    }
}

TEST(TerrainGen, DifferentSeeds) {
    vibecraft::TerrainGenerator gen1(100);
    vibecraft::TerrainGenerator gen2(200);

    auto chunk1 = MakeChunk(gen1, 0, 0);
    auto chunk2 = MakeChunk(gen2, 0, 0);

    // At least some blocks must differ between different seeds.
    int differences = 0;
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            int s1 = FindTerrainSurface(chunk1, x, z);
            int s2 = FindTerrainSurface(chunk2, x, z);
            if (s1 != s2) {
                ++differences;
            }
        }
    }
    EXPECT_GT(differences, 0)
        << "Different seeds should produce different terrain heights";
}

TEST(TerrainGen, HeightRange) {
    vibecraft::TerrainGenerator gen(kTestSeed);

    // Check multiple chunks including negative coordinates.
    std::vector<std::pair<int, int>> coords = {
        {0, 0}, {1, 0}, {0, 1}, {-1, -1}, {5, 5}, {-10, 3}
    };

    for (auto [cx, cz] : coords) {
        auto chunk = MakeChunk(gen, cx, cz);
        for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
            for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
                int surface = FindTerrainSurface(chunk, x, z);
                EXPECT_GE(surface, vibecraft::TerrainGenerator::kMinHeight)
                    << "Height below min at chunk (" << cx << "," << cz
                    << ") local (" << x << "," << z << ")";
                EXPECT_LE(surface, vibecraft::TerrainGenerator::kMaxHeight)
                    << "Height above max at chunk (" << cx << "," << cz
                    << ") local (" << x << "," << z << ")";
            }
        }
    }
}

TEST(TerrainGen, HeightVariation) {
    vibecraft::TerrainGenerator gen(kTestSeed);

    // Collect heights from a large area to ensure variation.
    std::vector<double> heights;
    for (int cx = -2; cx <= 2; ++cx) {
        for (int cz = -2; cz <= 2; ++cz) {
            auto chunk = MakeChunk(gen, cx, cz);
            for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
                for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
                    heights.push_back(
                        static_cast<double>(FindTerrainSurface(chunk, x, z)));
                }
            }
        }
    }

    // Calculate standard deviation.
    double sum = std::accumulate(heights.begin(), heights.end(), 0.0);
    double mean = sum / static_cast<double>(heights.size());
    double sq_sum = 0.0;
    for (double h : heights) {
        sq_sum += (h - mean) * (h - mean);
    }
    double stddev = std::sqrt(sq_sum / static_cast<double>(heights.size()));

    EXPECT_GT(stddev, 5.0)
        << "Terrain should have significant height variation (stddev=" << stddev << ")";
}

TEST(TerrainGen, ChunkBorderContinuity) {
    vibecraft::TerrainGenerator gen(kTestSeed);

    // Generate two adjacent chunks along the X axis: (0,0) and (1,0).
    auto chunk_a = MakeChunk(gen, 0, 0);
    auto chunk_b = MakeChunk(gen, 1, 0);

    // The right edge of chunk A (local x=15) should match the left edge
    // of chunk B (local x=0) in terms of terrain height. Adjacent columns
    // should be close, but biome boundaries can cause modest jumps.
    for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
        int height_a = FindTerrainSurface(chunk_a, 15, z);
        int height_b = FindTerrainSurface(chunk_b, 0, z);

        // With biome blending, adjacent columns should be within a reasonable delta.
        // Biome boundaries with blending can cause up to ~10-block jumps.
        EXPECT_NEAR(height_a, height_b, 10)
            << "Height discontinuity at chunk border z=" << z
            << " (height_a=" << height_a << ", height_b=" << height_b << ")";
    }

    // Also check the Z axis border: (0,0) and (0,1).
    auto chunk_c = MakeChunk(gen, 0, 1);
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        int height_a = FindTerrainSurface(chunk_a, x, 15);
        int height_c = FindTerrainSurface(chunk_c, x, 0);
        EXPECT_NEAR(height_a, height_c, 10)
            << "Height discontinuity at Z chunk border x=" << x
            << " (height_a=" << height_a << ", height_c=" << height_c << ")";
    }

    // Verify exact height match via GetHeight.
    // GetHeight returns the terrain surface (no trees), so it should match
    // FindTerrainSurface from the chunk.
    for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
        int world_z = z;  // chunk (0,0)
        int height_direct = gen.GetHeight(15, world_z);
        int height_from_chunk = FindTerrainSurface(chunk_a, 15, z);
        EXPECT_EQ(height_direct, height_from_chunk)
            << "GetHeight and chunk generation disagree at world (15, " << world_z << ")";
    }
}
