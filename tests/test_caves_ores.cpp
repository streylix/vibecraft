#include <gtest/gtest.h>

#include <set>
#include <tuple>

#include "vibecraft/block.h"
#include "vibecraft/cave_generator.h"
#include "vibecraft/chunk.h"
#include "vibecraft/ore_generator.h"
#include "vibecraft/terrain_generator.h"

// M17: Cave Generation & Ore Distribution

namespace vibecraft {
namespace {

/// Helper: generate a chunk with full terrain (terrain + caves + ores)
/// using the TerrainGenerator pipeline.
Chunk GenerateFullChunk(uint32_t seed, int cx, int cz) {
    Chunk chunk(cx, cz);
    TerrainGenerator gen(seed);
    gen.GenerateChunk(chunk);
    return chunk;
}

/// Helper: generate a chunk with terrain only (no caves, no ores).
Chunk GenerateTerrainOnly(uint32_t seed, int cx, int cz) {
    // We replicate terrain generation without caves/ores by using the
    // terrain generator's height + biome info to fill blocks manually.
    // Instead, we just generate terrain and compare pre/post cave state.
    Chunk chunk(cx, cz);
    TerrainGenerator gen(seed);

    int world_x0 = cx * kChunkSizeX;
    int world_z0 = cz * kChunkSizeZ;

    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            int bx = world_x0 + lx;
            int bz = world_z0 + lz;

            int surface = gen.GetHeight(bx, bz);
            BiomeType biome = gen.GetBiome(bx, bz);
            const BiomeProperties& props = GetBiomeProperties(biome);

            chunk.SetBlock(lx, 0, lz, BlockRegistry::kBedrock);

            int stone_top = surface - TerrainGenerator::kDirtLayers - 1;
            for (int y = 1; y <= stone_top; ++y) {
                chunk.SetBlock(lx, y, lz, BlockRegistry::kStone);
            }

            int filler_bottom = std::max(surface - TerrainGenerator::kDirtLayers, 1);
            for (int y = filler_bottom; y <= surface - 1; ++y) {
                chunk.SetBlock(lx, y, lz, props.filler_block);
            }

            if (surface >= 1) {
                chunk.SetBlock(lx, surface, lz, props.surface_block);
            }
        }
    }

    return chunk;
}

// ============================================================================
// Cave Tests
// ============================================================================

TEST(Caves, CavesExist) {
    // Generate a full chunk (with caves) and a terrain-only chunk.
    // Verify that some underground blocks that were solid in terrain-only
    // are now air in the full chunk (carved by caves).
    constexpr uint32_t kSeed = 42;
    constexpr int kCx = 0;
    constexpr int kCz = 0;

    Chunk terrain_only = GenerateTerrainOnly(kSeed, kCx, kCz);
    Chunk with_caves = GenerateTerrainOnly(kSeed, kCx, kCz);
    CaveGenerator cave_gen(kSeed);
    cave_gen.CarveCaves(with_caves);

    int carved_count = 0;
    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            for (int y = 1; y < 128; ++y) {
                BlockId before = terrain_only.GetBlock(lx, y, lz);
                BlockId after = with_caves.GetBlock(lx, y, lz);
                if (before != BlockRegistry::kAir && after == BlockRegistry::kAir) {
                    ++carved_count;
                }
            }
        }
    }

    // There should be a meaningful number of carved blocks.
    // A 16x16x128 chunk has ~32768 potential blocks; we expect at least
    // a few dozen carved blocks for caves to be considered present.
    EXPECT_GT(carved_count, 10)
        << "Expected caves to carve at least some blocks underground";
}

TEST(Caves, Deterministic) {
    constexpr uint32_t kSeed = 12345;

    // Generate the same chunk twice with the same seed.
    Chunk chunk_a = GenerateTerrainOnly(kSeed, 3, 7);
    CaveGenerator cave_gen(kSeed);
    cave_gen.CarveCaves(chunk_a);

    Chunk chunk_b = GenerateTerrainOnly(kSeed, 3, 7);
    cave_gen.CarveCaves(chunk_b);

    // Every block should be identical.
    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            for (int y = 0; y < kChunkSizeY; ++y) {
                ASSERT_EQ(chunk_a.GetBlock(lx, y, lz),
                          chunk_b.GetBlock(lx, y, lz))
                    << "Mismatch at (" << lx << ", " << y << ", " << lz << ")";
            }
        }
    }
}

TEST(Caves, DontReplaceBedrock) {
    constexpr uint32_t kSeed = 99;

    // Generate several chunks and verify y=0 is always bedrock.
    for (int cx = -2; cx <= 2; ++cx) {
        for (int cz = -2; cz <= 2; ++cz) {
            Chunk chunk = GenerateFullChunk(kSeed, cx, cz);
            for (int lx = 0; lx < kChunkSizeX; ++lx) {
                for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                    ASSERT_EQ(chunk.GetBlock(lx, 0, lz), BlockRegistry::kBedrock)
                        << "Bedrock missing at chunk (" << cx << ", " << cz
                        << ") local (" << lx << ", 0, " << lz << ")";
                }
            }
        }
    }
}

TEST(Caves, CaveSize) {
    // Verify that caves are multi-block regions, not just single scattered
    // air blocks. We look for air blocks below surface that have at least one
    // adjacent air block also created by cave carving.
    // Scan multiple chunks to accumulate enough cave data.
    constexpr uint32_t kSeed = 42;

    CaveGenerator cave_gen(kSeed);

    std::set<std::tuple<int, int, int>> carved;

    for (int cx = -2; cx <= 2; ++cx) {
        for (int cz = -2; cz <= 2; ++cz) {
            Chunk terrain_only = GenerateTerrainOnly(kSeed, cx, cz);
            Chunk with_caves = GenerateTerrainOnly(kSeed, cx, cz);
            cave_gen.CarveCaves(with_caves);

            // Use global coordinates so adjacency works across chunks.
            int world_x0 = cx * kChunkSizeX;
            int world_z0 = cz * kChunkSizeZ;

            for (int lx = 0; lx < kChunkSizeX; ++lx) {
                for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                    for (int y = 1; y < 128; ++y) {
                        if (terrain_only.GetBlock(lx, y, lz) != BlockRegistry::kAir &&
                            with_caves.GetBlock(lx, y, lz) == BlockRegistry::kAir) {
                            carved.insert({world_x0 + lx, y, world_z0 + lz});
                        }
                    }
                }
            }
        }
    }

    // At least some carved blocks should be present.
    ASSERT_GT(carved.size(), 0u) << "No caves found to test size";

    // Check that at least some carved blocks have adjacent carved blocks
    // (multi-block cave regions).
    int multi_block_count = 0;
    static const int dx[] = {1, -1, 0, 0, 0, 0};
    static const int dy[] = {0, 0, 1, -1, 0, 0};
    static const int dz[] = {0, 0, 0, 0, 1, -1};

    for (const auto& [x, y, z] : carved) {
        for (int i = 0; i < 6; ++i) {
            if (carved.count({x + dx[i], y + dy[i], z + dz[i]})) {
                ++multi_block_count;
                break;
            }
        }
    }

    // A significant portion of carved blocks should be part of multi-block caves.
    EXPECT_GT(multi_block_count, 0)
        << "Caves should be multi-block regions, not single scattered holes";
    // At least 50% of carved blocks should have an adjacent carved block.
    float ratio = static_cast<float>(multi_block_count) / static_cast<float>(carved.size());
    EXPECT_GT(ratio, 0.5f)
        << "Expected most cave blocks to be part of larger caves, got ratio " << ratio;
}

// ============================================================================
// Ore Tests
// ============================================================================

/// Helper: generate a chunk with terrain + ores (no caves, to isolate ore testing).
Chunk GenerateWithOres(uint32_t seed, int cx, int cz) {
    Chunk chunk = GenerateTerrainOnly(seed, cx, cz);
    OreGenerator ore_gen(seed);
    ore_gen.GenerateOres(chunk);
    return chunk;
}

/// Helper: scan a chunk and check that a specific ore never appears above max_y.
void CheckOreDepthLimit(const Chunk& chunk, BlockId ore, int max_y, const char* name) {
    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            for (int y = max_y + 1; y < kChunkSizeY; ++y) {
                ASSERT_NE(chunk.GetBlock(lx, y, lz), ore)
                    << name << " found above y=" << max_y
                    << " at (" << lx << ", " << y << ", " << lz << ")";
            }
        }
    }
}

TEST(Ores, DiamondBelowY16) {
    constexpr uint32_t kSeed = 42;
    // Check across multiple chunks to be thorough.
    for (int cx = -2; cx <= 2; ++cx) {
        for (int cz = -2; cz <= 2; ++cz) {
            Chunk chunk = GenerateWithOres(kSeed, cx, cz);
            CheckOreDepthLimit(chunk, BlockRegistry::kDiamondOre, 16, "Diamond ore");
        }
    }
}

TEST(Ores, GoldBelowY32) {
    constexpr uint32_t kSeed = 42;
    for (int cx = -2; cx <= 2; ++cx) {
        for (int cz = -2; cz <= 2; ++cz) {
            Chunk chunk = GenerateWithOres(kSeed, cx, cz);
            CheckOreDepthLimit(chunk, BlockRegistry::kGoldOre, 32, "Gold ore");
        }
    }
}

TEST(Ores, IronBelowY64) {
    constexpr uint32_t kSeed = 42;
    for (int cx = -2; cx <= 2; ++cx) {
        for (int cz = -2; cz <= 2; ++cz) {
            Chunk chunk = GenerateWithOres(kSeed, cx, cz);
            CheckOreDepthLimit(chunk, BlockRegistry::kIronOre, 64, "Iron ore");
        }
    }
}

TEST(Ores, CoalAnywhere) {
    // Coal ore should be able to appear at various heights below the surface.
    // Check that coal appears across a range of y-levels.
    constexpr uint32_t kSeed = 42;

    bool found_low = false;   // y < 32
    bool found_mid = false;   // 32 <= y < 64
    bool found_high = false;  // y >= 64

    // Scan multiple chunks to find coal at different heights.
    for (int cx = -5; cx <= 5 && !(found_low && found_mid && found_high); ++cx) {
        for (int cz = -5; cz <= 5 && !(found_low && found_mid && found_high); ++cz) {
            Chunk chunk = GenerateWithOres(kSeed, cx, cz);
            for (int lx = 0; lx < kChunkSizeX; ++lx) {
                for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                    for (int y = 1; y < kChunkSizeY; ++y) {
                        if (chunk.GetBlock(lx, y, lz) == BlockRegistry::kCoalOre) {
                            if (y < 32) found_low = true;
                            else if (y < 64) found_mid = true;
                            else found_high = true;
                        }
                    }
                }
            }
        }
    }

    EXPECT_TRUE(found_low) << "Expected coal ore at low depths (y < 32)";
    EXPECT_TRUE(found_mid) << "Expected coal ore at mid depths (32 <= y < 64)";
    EXPECT_TRUE(found_high) << "Expected coal ore at high depths (y >= 64)";
}

TEST(Ores, OreVeins) {
    // Verify that ores appear in clusters (at least some ore blocks have
    // adjacent ore of the same type).
    constexpr uint32_t kSeed = 42;
    constexpr int kCx = 0;
    constexpr int kCz = 0;

    Chunk chunk = GenerateWithOres(kSeed, kCx, kCz);

    // Check each ore type for adjacency.
    BlockId ore_types[] = {
        BlockRegistry::kCoalOre,
        BlockRegistry::kIronOre,
        BlockRegistry::kGoldOre,
        BlockRegistry::kDiamondOre,
    };

    static const int dx[] = {1, -1, 0, 0, 0, 0};
    static const int dy[] = {0, 0, 1, -1, 0, 0};
    static const int dz[] = {0, 0, 0, 0, 1, -1};

    int total_adjacent = 0;
    int total_ore_blocks = 0;

    for (BlockId ore : ore_types) {
        for (int lx = 0; lx < kChunkSizeX; ++lx) {
            for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                for (int y = 1; y < kChunkSizeY; ++y) {
                    if (chunk.GetBlock(lx, y, lz) == ore) {
                        ++total_ore_blocks;
                        // Check if any neighbor is the same ore type.
                        for (int i = 0; i < 6; ++i) {
                            int nx = lx + dx[i];
                            int ny = y + dy[i];
                            int nz = lz + dz[i];
                            if (nx >= 0 && nx < kChunkSizeX &&
                                ny >= 0 && ny < kChunkSizeY &&
                                nz >= 0 && nz < kChunkSizeZ) {
                                if (chunk.GetBlock(nx, ny, nz) == ore) {
                                    ++total_adjacent;
                                    goto next_block;  // Count each block once.
                                }
                            }
                        }
                        next_block:;
                    }
                }
            }
        }
    }

    EXPECT_GT(total_ore_blocks, 0) << "Expected at least some ore blocks";
    EXPECT_GT(total_adjacent, 0) << "Expected at least some ore blocks with adjacent same-type ore (veins)";
    // A meaningful fraction should be clustered.
    if (total_ore_blocks > 0) {
        float cluster_ratio = static_cast<float>(total_adjacent) / static_cast<float>(total_ore_blocks);
        EXPECT_GT(cluster_ratio, 0.3f)
            << "Expected at least 30% of ore blocks to be part of clusters, got "
            << cluster_ratio;
    }
}

TEST(Ores, OresOnlyInStone) {
    // Ores should only replace stone. Verify no ore block exists where the
    // terrain-only chunk had non-stone (air, dirt, grass, bedrock, etc.).
    constexpr uint32_t kSeed = 42;

    BlockId ore_types[] = {
        BlockRegistry::kCoalOre,
        BlockRegistry::kIronOre,
        BlockRegistry::kGoldOre,
        BlockRegistry::kDiamondOre,
    };

    for (int cx = -2; cx <= 2; ++cx) {
        for (int cz = -2; cz <= 2; ++cz) {
            Chunk terrain = GenerateTerrainOnly(kSeed, cx, cz);
            Chunk with_ores = GenerateTerrainOnly(kSeed, cx, cz);
            OreGenerator ore_gen(kSeed);
            ore_gen.GenerateOres(with_ores);

            for (int lx = 0; lx < kChunkSizeX; ++lx) {
                for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                    for (int y = 0; y < kChunkSizeY; ++y) {
                        BlockId after = with_ores.GetBlock(lx, y, lz);
                        bool is_ore = false;
                        for (BlockId ore : ore_types) {
                            if (after == ore) {
                                is_ore = true;
                                break;
                            }
                        }
                        if (is_ore) {
                            BlockId before = terrain.GetBlock(lx, y, lz);
                            ASSERT_EQ(before, BlockRegistry::kStone)
                                << "Ore placed in non-stone block (was "
                                << static_cast<int>(before)
                                << ") at chunk (" << cx << ", " << cz
                                << ") local (" << lx << ", " << y << ", " << lz << ")";
                        }
                    }
                }
            }
        }
    }
}

TEST(Ores, Deterministic) {
    constexpr uint32_t kSeed = 54321;
    constexpr int kCx = 5;
    constexpr int kCz = -3;

    Chunk chunk_a = GenerateWithOres(kSeed, kCx, kCz);
    Chunk chunk_b = GenerateWithOres(kSeed, kCx, kCz);

    for (int lx = 0; lx < kChunkSizeX; ++lx) {
        for (int lz = 0; lz < kChunkSizeZ; ++lz) {
            for (int y = 0; y < kChunkSizeY; ++y) {
                ASSERT_EQ(chunk_a.GetBlock(lx, y, lz),
                          chunk_b.GetBlock(lx, y, lz))
                    << "Ore mismatch at (" << lx << ", " << y << ", " << lz << ")";
            }
        }
    }
}

}  // namespace
}  // namespace vibecraft
