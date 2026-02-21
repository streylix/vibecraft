#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/region.h"
#include "vibecraft/world.h"
#include "vibecraft/world_save.h"

// M22: World Persistence (Region Files)

namespace vibecraft {
namespace {

/// RAII helper that creates a temporary directory and removes it on destruction.
class TempDir {
public:
    TempDir() {
        // Use std::filesystem::temp_directory_path + unique name.
        std::string base = std::filesystem::temp_directory_path().string();
        path_ = base + "/vibecraft_test_" + std::to_string(std::rand()) + "_" +
                std::to_string(reinterpret_cast<uintptr_t>(this));
        std::filesystem::create_directories(path_);
    }

    ~TempDir() {
        std::filesystem::remove_all(path_);
    }

    const std::string& Path() const { return path_; }

    // Non-copyable.
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

private:
    std::string path_;
};

// --- Tests ---

TEST(WorldSave, SaveLoadRoundTrip) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    // Create a chunk with some interesting block data.
    Chunk chunk(3, 7);
    chunk.SetBlock(0, 0, 0, BlockRegistry::kStone);
    chunk.SetBlock(5, 64, 5, BlockRegistry::kGrass);
    chunk.SetBlock(15, 255, 15, BlockRegistry::kDirt);
    chunk.SetBlock(8, 128, 8, BlockRegistry::kDiamondOre);

    // Save the chunk.
    ASSERT_TRUE(saver.SaveChunk(chunk));

    // Load it back.
    auto loaded = saver.LoadChunk(3, 7);
    ASSERT_NE(loaded, nullptr);

    // Verify all blocks are identical.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                ASSERT_EQ(loaded->GetBlock(x, y, z), chunk.GetBlock(x, y, z))
                    << "Mismatch at (" << x << ", " << y << ", " << z << ")";
            }
        }
    }

    // Verify chunk coordinates.
    EXPECT_EQ(loaded->GetChunkX(), 3);
    EXPECT_EQ(loaded->GetChunkZ(), 7);
}

TEST(WorldSave, OverwriteSave) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    // Save a chunk with stone at (0,0,0).
    Chunk chunk1(0, 0);
    chunk1.SetBlock(0, 0, 0, BlockRegistry::kStone);
    ASSERT_TRUE(saver.SaveChunk(chunk1));

    // Load and verify.
    auto loaded1 = saver.LoadChunk(0, 0);
    ASSERT_NE(loaded1, nullptr);
    EXPECT_EQ(loaded1->GetBlock(0, 0, 0), BlockRegistry::kStone);

    // Modify and save again — overwrite.
    Chunk chunk2(0, 0);
    chunk2.SetBlock(0, 0, 0, BlockRegistry::kDirt);
    chunk2.SetBlock(1, 1, 1, BlockRegistry::kGrass);
    ASSERT_TRUE(saver.SaveChunk(chunk2));

    // Load and verify the second save overwrote the first.
    auto loaded2 = saver.LoadChunk(0, 0);
    ASSERT_NE(loaded2, nullptr);
    EXPECT_EQ(loaded2->GetBlock(0, 0, 0), BlockRegistry::kDirt);
    EXPECT_EQ(loaded2->GetBlock(1, 1, 1), BlockRegistry::kGrass);
}

TEST(WorldSave, CoordToRegion) {
    // Chunk (33, 0) should map to region (1, 0).
    EXPECT_EQ(ChunkToRegionCoord(33), 1);
    EXPECT_EQ(ChunkToRegionCoord(0), 0);

    // Chunk (31, 31) should still be in region (0, 0).
    EXPECT_EQ(ChunkToRegionCoord(31), 0);

    // Chunk (32, 32) should be in region (1, 1).
    EXPECT_EQ(ChunkToRegionCoord(32), 1);

    // Negative coords: chunk (-1, 0) -> region (-1, 0).
    EXPECT_EQ(ChunkToRegionCoord(-1), -1);

    // Chunk (-32, 0) -> region (-1, 0).
    EXPECT_EQ(ChunkToRegionCoord(-32), -1);

    // Chunk (-33, 0) -> region (-2, 0).
    EXPECT_EQ(ChunkToRegionCoord(-33), -2);
}

TEST(WorldSave, CoordToOffset) {
    // Chunk (1, 2) -> offset (1, 2) within region.
    EXPECT_EQ(ChunkToRegionOffset(1), 1);
    EXPECT_EQ(ChunkToRegionOffset(2), 2);

    // Chunk (33, 5) -> offset (1, 5).
    EXPECT_EQ(ChunkToRegionOffset(33), 1);
    EXPECT_EQ(ChunkToRegionOffset(5), 5);

    // Chunk (0, 0) -> offset (0, 0).
    EXPECT_EQ(ChunkToRegionOffset(0), 0);

    // Chunk (31, 31) -> offset (31, 31).
    EXPECT_EQ(ChunkToRegionOffset(31), 31);

    // Negative: chunk (-1, -1) -> offset (31, 31).
    EXPECT_EQ(ChunkToRegionOffset(-1), 31);

    // Chunk (-32, 0) -> offset (0, 0).
    EXPECT_EQ(ChunkToRegionOffset(-32), 0);
}

TEST(WorldSave, Compression) {
    TempDir tmp;
    std::string region_path = tmp.Path() + "/test_region.dat";

    // Create a mostly-air chunk (compresses very well).
    Chunk chunk(0, 0);
    chunk.SetBlock(0, 0, 0, BlockRegistry::kStone);

    RegionFile region(region_path);
    ASSERT_TRUE(region.SaveChunk(chunk));

    // Raw chunk data: 65536 * 3 = 196608 bytes.
    constexpr std::size_t kRawChunkSize =
        static_cast<std::size_t>(kChunkVolume) * 3;

    // The file should be significantly smaller than raw data + header.
    std::size_t file_size = region.GetFileSize();
    EXPECT_GT(file_size, 0u);
    EXPECT_LT(file_size, kRawChunkSize);
}

TEST(WorldSave, FluidPreserved) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    Chunk chunk(0, 0);
    chunk.SetBlock(4, 10, 4, BlockRegistry::kWater);
    chunk.SetFluidLevel(4, 10, 4, 5);

    ASSERT_TRUE(saver.SaveChunk(chunk));

    auto loaded = saver.LoadChunk(0, 0);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->GetBlock(4, 10, 4), BlockRegistry::kWater);
    EXPECT_EQ(loaded->GetFluidLevel(4, 10, 4), 5);
}

TEST(WorldSave, LightPreserved) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    Chunk chunk(0, 0);
    // Set block light to 14 at a specific location.
    chunk.SetBlockLight(8, 64, 8, 14);
    // Set sunlight to 10 at another location.
    chunk.SetSunLight(2, 100, 2, 10);

    ASSERT_TRUE(saver.SaveChunk(chunk));

    auto loaded = saver.LoadChunk(0, 0);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->GetBlockLight(8, 64, 8), 14);
    EXPECT_EQ(loaded->GetSunLight(2, 100, 2), 10);
}

TEST(WorldSave, PlayerPositionSaved) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    WorldMetadata meta;
    meta.player_position = glm::vec3(10.0f, 64.0f, 20.0f);
    meta.world_seed = 0;
    meta.game_time = 0.0f;

    ASSERT_TRUE(saver.SaveMetadata(meta));

    WorldMetadata loaded_meta;
    ASSERT_TRUE(saver.LoadMetadata(loaded_meta));

    EXPECT_FLOAT_EQ(loaded_meta.player_position.x, 10.0f);
    EXPECT_FLOAT_EQ(loaded_meta.player_position.y, 64.0f);
    EXPECT_FLOAT_EQ(loaded_meta.player_position.z, 20.0f);
}

TEST(WorldSave, WorldSeedSaved) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    WorldMetadata meta;
    meta.world_seed = 12345;

    ASSERT_TRUE(saver.SaveMetadata(meta));

    WorldMetadata loaded_meta;
    ASSERT_TRUE(saver.LoadMetadata(loaded_meta));

    EXPECT_EQ(loaded_meta.world_seed, 12345u);
}

TEST(WorldSave, GameTimeSaved) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    WorldMetadata meta;
    meta.game_time = 6000.0f;

    ASSERT_TRUE(saver.SaveMetadata(meta));

    WorldMetadata loaded_meta;
    ASSERT_TRUE(saver.LoadMetadata(loaded_meta));

    EXPECT_FLOAT_EQ(loaded_meta.game_time, 6000.0f);
}

TEST(WorldSave, NegativeCoords) {
    TempDir tmp;
    WorldSave saver(tmp.Path() + "/save");

    // Create a chunk at negative coordinates.
    Chunk chunk(-5, -3);
    chunk.SetBlock(0, 0, 0, BlockRegistry::kStone);
    chunk.SetBlock(7, 32, 7, BlockRegistry::kGrass);
    chunk.SetBlock(15, 255, 15, BlockRegistry::kBedrock);
    chunk.SetFluidLevel(3, 10, 3, 4);
    chunk.SetBlockLight(10, 50, 10, 12);

    ASSERT_TRUE(saver.SaveChunk(chunk));

    auto loaded = saver.LoadChunk(-5, -3);
    ASSERT_NE(loaded, nullptr);

    // Verify chunk coordinates.
    EXPECT_EQ(loaded->GetChunkX(), -5);
    EXPECT_EQ(loaded->GetChunkZ(), -3);

    // Verify all blocks identical.
    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                ASSERT_EQ(loaded->GetBlock(x, y, z), chunk.GetBlock(x, y, z))
                    << "Block mismatch at (" << x << ", " << y << ", " << z << ")";
            }
        }
    }

    // Verify fluid and light.
    EXPECT_EQ(loaded->GetFluidLevel(3, 10, 3), 4);
    EXPECT_EQ(loaded->GetBlockLight(10, 50, 10), 12);
}

}  // namespace
}  // namespace vibecraft
