#include <gtest/gtest.h>

#include <cmath>
#include <numeric>
#include <vector>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/terrain_generator.h"

// M11: Terrain Generation (Basic)

namespace {

constexpr uint32_t kTestSeed = 12345;

/// Helper: generate a chunk at given chunk coords with the test seed.
vibecraft::Chunk MakeChunk(const vibecraft::TerrainGenerator& gen, int cx, int cz) {
    vibecraft::Chunk chunk(cx, cz);
    gen.GenerateChunk(chunk);
    return chunk;
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

TEST(TerrainGen, GrassSurface) {
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 1, 2);

    // The highest non-air block in every column must be grass.
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            int surface = FindSurface(chunk, x, z);
            ASSERT_GE(surface, 0) << "Column (" << x << ", " << z << ") is all air";
            EXPECT_EQ(chunk.GetBlock(x, surface, z), vibecraft::BlockRegistry::kGrass)
                << "at local (" << x << ", " << surface << ", " << z << ")";
        }
    }
}

TEST(TerrainGen, DirtBelowGrass) {
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 0, 0);

    // 4 blocks of dirt below the grass surface.
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            int surface = FindSurface(chunk, x, z);
            ASSERT_GE(surface, 5)
                << "Surface too low for full dirt layer at (" << x << ", " << z << ")";

            for (int dy = 1; dy <= 4; ++dy) {
                EXPECT_EQ(chunk.GetBlock(x, surface - dy, z),
                          vibecraft::BlockRegistry::kDirt)
                    << "at local (" << x << ", " << (surface - dy) << ", " << z << ")";
            }
        }
    }
}

TEST(TerrainGen, StoneBelowDirt) {
    vibecraft::TerrainGenerator gen(kTestSeed);
    auto chunk = MakeChunk(gen, 0, 0);

    // Stone from y=1 up to surface - 5.
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            int surface = FindSurface(chunk, x, z);
            ASSERT_GE(surface, 6)
                << "Surface too low for stone layer at (" << x << ", " << z << ")";

            for (int y = 1; y <= surface - 5; ++y) {
                EXPECT_EQ(chunk.GetBlock(x, y, z), vibecraft::BlockRegistry::kStone)
                    << "at local (" << x << ", " << y << ", " << z << ")";
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
            int s1 = FindSurface(chunk1, x, z);
            int s2 = FindSurface(chunk2, x, z);
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
                int surface = FindSurface(chunk, x, z);
                EXPECT_GE(surface, 40)
                    << "Height below 40 at chunk (" << cx << "," << cz
                    << ") local (" << x << "," << z << ")";
                EXPECT_LE(surface, 120)
                    << "Height above 120 at chunk (" << cx << "," << cz
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
                    heights.push_back(static_cast<double>(FindSurface(chunk, x, z)));
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
    // of chunk B (local x=0) in terms of terrain height. They share the
    // same world z coordinates but adjacent world x coordinates.
    // We verify by checking heights computed from world coordinates.
    for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
        // World coords at the border: chunk A x=15 is world x=15,
        // chunk B x=0 is world x=16. They should be close in height
        // (noise is continuous).
        int height_a = FindSurface(chunk_a, 15, z);
        int height_b = FindSurface(chunk_b, 0, z);

        // Adjacent columns should be within a small delta
        // (Perlin noise is smooth, so neighboring world coords differ by at most
        // a few blocks in height at this frequency).
        EXPECT_NEAR(height_a, height_b, 5)
            << "Height discontinuity at chunk border z=" << z
            << " (height_a=" << height_a << ", height_b=" << height_b << ")";
    }

    // Also check the Z axis border: (0,0) and (0,1).
    auto chunk_c = MakeChunk(gen, 0, 1);
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        int height_a = FindSurface(chunk_a, x, 15);
        int height_c = FindSurface(chunk_c, x, 0);
        EXPECT_NEAR(height_a, height_c, 5)
            << "Height discontinuity at Z chunk border x=" << x
            << " (height_a=" << height_a << ", height_c=" << height_c << ")";
    }

    // Verify exact height match via GetHeight (uses same world-space coords).
    // The height at a world position must be the same regardless of which
    // chunk generation computed it.
    for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
        int world_z = z;  // chunk (0,0)
        int height_direct = gen.GetHeight(15, world_z);
        int height_from_chunk = FindSurface(chunk_a, 15, z);
        EXPECT_EQ(height_direct, height_from_chunk)
            << "GetHeight and chunk generation disagree at world (15, " << world_z << ")";
    }
}
