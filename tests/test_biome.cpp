#include <gtest/gtest.h>

#include <set>
#include <vector>

#include "vibecraft/biome.h"
#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/terrain_generator.h"

// M16: Biome System

namespace {

constexpr uint32_t kTestSeed = 12345;

/// Helper: generate a chunk at given chunk coords.
vibecraft::Chunk MakeChunk(const vibecraft::TerrainGenerator& gen, int cx, int cz) {
    vibecraft::Chunk chunk(cx, cz);
    gen.GenerateChunk(chunk);
    return chunk;
}

/// Helper: find the terrain surface Y (highest non-air, non-tree block).
bool IsVegetation(vibecraft::BlockId id) {
    return id == vibecraft::BlockRegistry::kOakLog ||
           id == vibecraft::BlockRegistry::kOakLeaves;
}

int FindTerrainSurface(const vibecraft::Chunk& chunk, int lx, int lz) {
    for (int y = vibecraft::kChunkSizeY - 1; y >= 0; --y) {
        vibecraft::BlockId id = chunk.GetBlock(lx, y, lz);
        if (id != vibecraft::BlockRegistry::kAir && !IsVegetation(id)) {
            return y;
        }
    }
    return -1;
}

/// Helper: find a chunk position dominated by a specific biome.
/// Scans a grid of chunks to find one where the majority is the desired biome.
/// Returns {chunk_x, chunk_z} or {INT_MIN, INT_MIN} if not found.
std::pair<int, int> FindBiomeChunk(const vibecraft::TerrainGenerator& gen,
                                   vibecraft::BiomeType target) {
    // Search over a wide area.
    for (int cx = -20; cx <= 20; ++cx) {
        for (int cz = -20; cz <= 20; ++cz) {
            int count = 0;
            int world_x0 = cx * vibecraft::kChunkSizeX;
            int world_z0 = cz * vibecraft::kChunkSizeZ;
            for (int lx = 0; lx < vibecraft::kChunkSizeX; ++lx) {
                for (int lz = 0; lz < vibecraft::kChunkSizeZ; ++lz) {
                    if (gen.GetBiome(world_x0 + lx, world_z0 + lz) == target) {
                        ++count;
                    }
                }
            }
            // At least 75% of columns are this biome.
            if (count >= vibecraft::kChunkColumns * 3 / 4) {
                return {cx, cz};
            }
        }
    }
    return {INT_MIN, INT_MIN};
}

}  // namespace

TEST(Biome, AtLeastFiveBiomes) {
    // The biome system must define at least 5 distinct biome types.
    EXPECT_GE(vibecraft::GetBiomeCount(), 5);
}

TEST(Biome, PlainsHasGrass) {
    const auto& props = vibecraft::GetBiomeProperties(vibecraft::BiomeType::kPlains);
    EXPECT_EQ(props.surface_block, vibecraft::BlockRegistry::kGrass);
}

TEST(Biome, DesertHasSand) {
    const auto& props = vibecraft::GetBiomeProperties(vibecraft::BiomeType::kDesert);
    EXPECT_EQ(props.surface_block, vibecraft::BlockRegistry::kSand);
}

TEST(Biome, TundraHasSnow) {
    const auto& props = vibecraft::GetBiomeProperties(vibecraft::BiomeType::kTundra);
    EXPECT_EQ(props.surface_block, vibecraft::BlockRegistry::kSnow);
}

TEST(Biome, HeightAffectedByBiome) {
    vibecraft::TerrainGenerator gen(kTestSeed);

    // Find a mountain-dominated chunk and a plains-dominated chunk.
    auto [mcx, mcz] = FindBiomeChunk(gen, vibecraft::BiomeType::kMountains);
    auto [pcx, pcz] = FindBiomeChunk(gen, vibecraft::BiomeType::kPlains);
    ASSERT_NE(mcx, INT_MIN) << "Could not find a mountains-dominated chunk";
    ASSERT_NE(pcx, INT_MIN) << "Could not find a plains-dominated chunk";

    // Compute average terrain height for each.
    auto mountain_chunk = MakeChunk(gen, mcx, mcz);
    auto plains_chunk = MakeChunk(gen, pcx, pcz);

    double mountain_avg = 0.0;
    double plains_avg = 0.0;
    int mountain_count = 0;
    int plains_count = 0;

    int mx0 = mcx * vibecraft::kChunkSizeX;
    int mz0 = mcz * vibecraft::kChunkSizeZ;
    for (int lx = 0; lx < vibecraft::kChunkSizeX; ++lx) {
        for (int lz = 0; lz < vibecraft::kChunkSizeZ; ++lz) {
            if (gen.GetBiome(mx0 + lx, mz0 + lz) == vibecraft::BiomeType::kMountains) {
                mountain_avg += FindTerrainSurface(mountain_chunk, lx, lz);
                ++mountain_count;
            }
        }
    }

    int px0 = pcx * vibecraft::kChunkSizeX;
    int pz0 = pcz * vibecraft::kChunkSizeZ;
    for (int lx = 0; lx < vibecraft::kChunkSizeX; ++lx) {
        for (int lz = 0; lz < vibecraft::kChunkSizeZ; ++lz) {
            if (gen.GetBiome(px0 + lx, pz0 + lz) == vibecraft::BiomeType::kPlains) {
                plains_avg += FindTerrainSurface(plains_chunk, lx, lz);
                ++plains_count;
            }
        }
    }

    ASSERT_GT(mountain_count, 0);
    ASSERT_GT(plains_count, 0);

    mountain_avg /= mountain_count;
    plains_avg /= plains_count;

    EXPECT_GT(mountain_avg, plains_avg)
        << "Mountain biome average height (" << mountain_avg
        << ") should be greater than plains average height (" << plains_avg << ")";
}

TEST(Biome, SmoothTransition) {
    // Verify that biome boundaries are smooth: no isolated single-block biome
    // patches in a large smooth region. We check that for any column, at least
    // one of its 4 neighbors shares the same biome (no 1-block biome islands).
    vibecraft::TerrainGenerator gen(kTestSeed);
    const auto& biome_map = gen.GetBiomeMap();

    int violations = 0;
    int total = 0;
    // Scan a large area.
    for (int bx = -100; bx < 100; ++bx) {
        for (int bz = -100; bz < 100; ++bz) {
            vibecraft::BiomeType center = biome_map.GetBiome(bx, bz);
            // Check 4 cardinal neighbors.
            bool has_same_neighbor = false;
            if (biome_map.GetBiome(bx + 1, bz) == center) has_same_neighbor = true;
            if (biome_map.GetBiome(bx - 1, bz) == center) has_same_neighbor = true;
            if (biome_map.GetBiome(bx, bz + 1) == center) has_same_neighbor = true;
            if (biome_map.GetBiome(bx, bz - 1) == center) has_same_neighbor = true;

            if (!has_same_neighbor) {
                ++violations;
            }
            ++total;
        }
    }

    // Allow a small percentage of isolated pixels (biome noise can create
    // some boundary artifacts), but it should be very rare.
    double violation_rate = static_cast<double>(violations) / total;
    EXPECT_LT(violation_rate, 0.01)
        << "Too many isolated single-block biome patches: " << violations
        << " out of " << total << " (" << (violation_rate * 100.0) << "%)";
}

TEST(Biome, TreesInForest) {
    vibecraft::TerrainGenerator gen(kTestSeed);

    auto [fcx, fcz] = FindBiomeChunk(gen, vibecraft::BiomeType::kForest);
    ASSERT_NE(fcx, INT_MIN) << "Could not find a forest-dominated chunk";

    auto chunk = MakeChunk(gen, fcx, fcz);

    // Look for OakLog and OakLeaves in the chunk.
    bool found_log = false;
    bool found_leaves = false;
    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            for (int y = 0; y < vibecraft::kChunkSizeY; ++y) {
                vibecraft::BlockId id = chunk.GetBlock(x, y, z);
                if (id == vibecraft::BlockRegistry::kOakLog) found_log = true;
                if (id == vibecraft::BlockRegistry::kOakLeaves) found_leaves = true;
                if (found_log && found_leaves) break;
            }
            if (found_log && found_leaves) break;
        }
        if (found_log && found_leaves) break;
    }

    EXPECT_TRUE(found_log) << "Forest chunk should contain OakLog blocks";
    EXPECT_TRUE(found_leaves) << "Forest chunk should contain OakLeaves blocks";
}

TEST(Biome, NoTreesInDesert) {
    vibecraft::TerrainGenerator gen(kTestSeed);

    auto [dcx, dcz] = FindBiomeChunk(gen, vibecraft::BiomeType::kDesert);
    ASSERT_NE(dcx, INT_MIN) << "Could not find a desert-dominated chunk";

    auto chunk = MakeChunk(gen, dcx, dcz);

    // Desert chunks should not contain any tree blocks.
    // Note: edge columns near biome boundaries might get trees from adjacent
    // forest, but a 75%-desert chunk shouldn't have significant trees.
    // We check that no Log/Leaves are placed in columns that are actually desert.
    int desert_tree_count = 0;
    int world_x0 = dcx * vibecraft::kChunkSizeX;
    int world_z0 = dcz * vibecraft::kChunkSizeZ;

    for (int x = 0; x < vibecraft::kChunkSizeX; ++x) {
        for (int z = 0; z < vibecraft::kChunkSizeZ; ++z) {
            if (gen.GetBiome(world_x0 + x, world_z0 + z) != vibecraft::BiomeType::kDesert) {
                continue;  // Skip non-desert columns.
            }
            for (int y = 0; y < vibecraft::kChunkSizeY; ++y) {
                vibecraft::BlockId id = chunk.GetBlock(x, y, z);
                if (id == vibecraft::BlockRegistry::kOakLog ||
                    id == vibecraft::BlockRegistry::kOakLeaves) {
                    ++desert_tree_count;
                }
            }
        }
    }

    EXPECT_EQ(desert_tree_count, 0)
        << "Desert columns should not contain tree blocks";
}

TEST(Biome, Deterministic) {
    vibecraft::BiomeMap map1(42);
    vibecraft::BiomeMap map2(42);

    // Check many coordinates: same seed must produce same biomes.
    for (int bx = -50; bx < 50; ++bx) {
        for (int bz = -50; bz < 50; ++bz) {
            EXPECT_EQ(map1.GetBiome(bx, bz), map2.GetBiome(bx, bz))
                << "Biome mismatch at (" << bx << ", " << bz << ")";
        }
    }
}

TEST(Biome, BiomeAtCoord) {
    vibecraft::BiomeMap map(kTestSeed);

    // GetBiome should return a valid biome type for any coordinate.
    std::vector<std::pair<int, int>> coords = {
        {0, 0}, {100, 200}, {-500, 300}, {1000, -1000}, {-50, -50}
    };

    for (auto [bx, bz] : coords) {
        vibecraft::BiomeType biome = map.GetBiome(bx, bz);
        int biome_int = static_cast<int>(biome);
        EXPECT_GE(biome_int, 0)
            << "Biome at (" << bx << ", " << bz << ") is negative";
        EXPECT_LT(biome_int, vibecraft::GetBiomeCount())
            << "Biome at (" << bx << ", " << bz << ") exceeds count";
    }
}
