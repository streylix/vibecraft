#include <gtest/gtest.h>
#include "vibecraft/chunk.h"

using vibecraft::BlockRegistry;
using vibecraft::Chunk;

// M4: Chunk Data Storage

TEST(Chunk, DefaultAllAir) {
    Chunk chunk;
    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            for (int y = 0; y < 256; ++y) {
                ASSERT_EQ(chunk.GetBlock(x, y, z), BlockRegistry::kAir)
                    << "Block at (" << x << "," << y << "," << z << ") was not air";
            }
        }
    }
}

TEST(Chunk, SetGetBlock) {
    Chunk chunk;
    chunk.SetBlock(5, 64, 5, BlockRegistry::kStone);
    EXPECT_EQ(chunk.GetBlock(5, 64, 5), BlockRegistry::kStone);
}

TEST(Chunk, BoundsCheckX) {
    Chunk chunk;
    // Out-of-bounds x should return Air.
    EXPECT_EQ(chunk.GetBlock(-1, 0, 0), BlockRegistry::kAir);
    EXPECT_EQ(chunk.GetBlock(16, 0, 0), BlockRegistry::kAir);
    // Out-of-bounds set should be silently ignored.
    chunk.SetBlock(-1, 0, 0, BlockRegistry::kStone);
    chunk.SetBlock(16, 0, 0, BlockRegistry::kStone);
}

TEST(Chunk, BoundsCheckY) {
    Chunk chunk;
    EXPECT_EQ(chunk.GetBlock(0, -1, 0), BlockRegistry::kAir);
    EXPECT_EQ(chunk.GetBlock(0, 256, 0), BlockRegistry::kAir);
    chunk.SetBlock(0, -1, 0, BlockRegistry::kStone);
    chunk.SetBlock(0, 256, 0, BlockRegistry::kStone);
}

TEST(Chunk, BoundsCheckZ) {
    Chunk chunk;
    EXPECT_EQ(chunk.GetBlock(0, 0, -1), BlockRegistry::kAir);
    EXPECT_EQ(chunk.GetBlock(0, 0, 16), BlockRegistry::kAir);
    chunk.SetBlock(0, 0, -1, BlockRegistry::kStone);
    chunk.SetBlock(0, 0, 16, BlockRegistry::kStone);
}

TEST(Chunk, DirtyOnCreate) {
    Chunk chunk;
    EXPECT_TRUE(chunk.IsDirty());
}

TEST(Chunk, DirtyAfterSet) {
    Chunk chunk;
    chunk.ClearDirty();
    ASSERT_FALSE(chunk.IsDirty());
    chunk.SetBlock(0, 0, 0, BlockRegistry::kStone);
    EXPECT_TRUE(chunk.IsDirty());
}

TEST(Chunk, ClearDirty) {
    Chunk chunk;
    ASSERT_TRUE(chunk.IsDirty());
    chunk.ClearDirty();
    EXPECT_FALSE(chunk.IsDirty());
}

TEST(Chunk, HeightmapBasic) {
    Chunk chunk;
    chunk.SetBlock(3, 64, 7, BlockRegistry::kStone);
    EXPECT_EQ(chunk.GetHeightmapValue(3, 7), 64);
}

TEST(Chunk, HeightmapUpdate) {
    Chunk chunk;
    // Place two blocks in the same column at different heights.
    chunk.SetBlock(0, 10, 0, BlockRegistry::kStone);
    chunk.SetBlock(0, 20, 0, BlockRegistry::kStone);
    ASSERT_EQ(chunk.GetHeightmapValue(0, 0), 20);

    // Remove the highest block — heightmap should drop to 10.
    chunk.SetBlock(0, 20, 0, BlockRegistry::kAir);
    EXPECT_EQ(chunk.GetHeightmapValue(0, 0), 10);

    // Remove the remaining block — heightmap should be -1 (all air).
    chunk.SetBlock(0, 10, 0, BlockRegistry::kAir);
    EXPECT_EQ(chunk.GetHeightmapValue(0, 0), -1);
}

TEST(Chunk, HeightmapMultiColumn) {
    Chunk chunk;
    chunk.SetBlock(0, 50, 0, BlockRegistry::kDirt);
    chunk.SetBlock(5, 100, 5, BlockRegistry::kGrass);
    chunk.SetBlock(15, 200, 15, BlockRegistry::kStone);

    EXPECT_EQ(chunk.GetHeightmapValue(0, 0), 50);
    EXPECT_EQ(chunk.GetHeightmapValue(5, 5), 100);
    EXPECT_EQ(chunk.GetHeightmapValue(15, 15), 200);

    // Unmodified columns should still be -1.
    EXPECT_EQ(chunk.GetHeightmapValue(1, 1), -1);
}

TEST(Chunk, IndexConsistency) {
    Chunk chunk;

    // Write a unique value to a spread of coordinates and read them back.
    // We test a representative set to keep the test fast.
    struct TestCase {
        int x, y, z;
        vibecraft::BlockId id;
    };

    TestCase cases[] = {
        {0, 0, 0, 1},
        {15, 0, 0, 2},
        {0, 255, 0, 3},
        {0, 0, 15, 4},
        {15, 255, 15, 5},
        {7, 128, 7, 6},
        {1, 1, 1, 7},
        {14, 200, 13, 8},
    };

    for (const auto& tc : cases) {
        chunk.SetBlock(tc.x, tc.y, tc.z, tc.id);
    }

    for (const auto& tc : cases) {
        EXPECT_EQ(chunk.GetBlock(tc.x, tc.y, tc.z), tc.id)
            << "Mismatch at (" << tc.x << "," << tc.y << "," << tc.z << ")";
    }
}

TEST(Chunk, SetSameBlockNotDirty) {
    Chunk chunk;
    chunk.SetBlock(3, 3, 3, BlockRegistry::kStone);
    chunk.ClearDirty();
    ASSERT_FALSE(chunk.IsDirty());

    // Setting the same block to the same value should NOT mark dirty.
    chunk.SetBlock(3, 3, 3, BlockRegistry::kStone);
    EXPECT_FALSE(chunk.IsDirty());
}

TEST(Chunk, ChunkPosition) {
    Chunk chunk(5, -3);
    EXPECT_EQ(chunk.GetChunkX(), 5);
    EXPECT_EQ(chunk.GetChunkZ(), -3);
}

TEST(Chunk, FullColumnWrite) {
    Chunk chunk;
    // Write all 256 Y levels in one column.
    for (int y = 0; y < 256; ++y) {
        vibecraft::BlockId id = static_cast<vibecraft::BlockId>(y % 255 + 1);
        chunk.SetBlock(8, y, 8, id);
    }
    // Read them all back.
    for (int y = 0; y < 256; ++y) {
        vibecraft::BlockId expected = static_cast<vibecraft::BlockId>(y % 255 + 1);
        EXPECT_EQ(chunk.GetBlock(8, y, 8), expected)
            << "Mismatch at y=" << y;
    }
    // Heightmap should be 255 (the highest y written).
    EXPECT_EQ(chunk.GetHeightmapValue(8, 8), 255);
}
