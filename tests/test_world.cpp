#include <gtest/gtest.h>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/world.h"

// M8: World & Chunk Manager

using namespace vibecraft;

TEST(World, WorldToChunkCoord) {
    // Block (16,0,0) -> chunk (1, 0)
    EXPECT_EQ(World::WorldToChunkCoord(16), 1);
    EXPECT_EQ(World::WorldToChunkCoord(0), 0);

    // Block (-1,0,0) -> chunk (-1, 0)
    EXPECT_EQ(World::WorldToChunkCoord(-1), -1);

    // More boundary cases
    EXPECT_EQ(World::WorldToChunkCoord(15), 0);
    EXPECT_EQ(World::WorldToChunkCoord(-16), -1);
    EXPECT_EQ(World::WorldToChunkCoord(-17), -2);
    EXPECT_EQ(World::WorldToChunkCoord(32), 2);
}

TEST(World, WorldToLocalCoord) {
    // Block (17,0,0) -> local (1, 0)
    EXPECT_EQ(World::WorldToLocalCoord(17), 1);
    EXPECT_EQ(World::WorldToLocalCoord(0), 0);

    // Block (-1,0,0) -> local (15, 0)
    EXPECT_EQ(World::WorldToLocalCoord(-1), 15);

    // More cases
    EXPECT_EQ(World::WorldToLocalCoord(16), 0);
    EXPECT_EQ(World::WorldToLocalCoord(15), 15);
    EXPECT_EQ(World::WorldToLocalCoord(-16), 0);
    EXPECT_EQ(World::WorldToLocalCoord(-17), 15);
}

TEST(World, SetGetBlockSameChunk) {
    World world;
    world.SetBlock(5, 64, 5, BlockRegistry::kStone);
    EXPECT_EQ(world.GetBlock(5, 64, 5), BlockRegistry::kStone);
}

TEST(World, SetGetBlockCrossChunk) {
    World world;
    // Block at (15, 64, 0) is in chunk (0, 0), local (15, 0)
    world.SetBlock(15, 64, 0, BlockRegistry::kStone);
    // Block at (16, 64, 0) is in chunk (1, 0), local (0, 0)
    world.SetBlock(16, 64, 0, BlockRegistry::kDirt);

    EXPECT_EQ(world.GetBlock(15, 64, 0), BlockRegistry::kStone);
    EXPECT_EQ(world.GetBlock(16, 64, 0), BlockRegistry::kDirt);
}

TEST(World, GetBlockUnloadedChunk) {
    World world;
    // No chunks loaded — should return Air.
    EXPECT_EQ(world.GetBlock(100, 64, 100), BlockRegistry::kAir);
}

TEST(World, LoadChunk) {
    World world;
    EXPECT_FALSE(world.HasChunk(0, 0));
    world.LoadChunk(0, 0);
    EXPECT_TRUE(world.HasChunk(0, 0));

    // Chunk should be accessible and empty (all air).
    EXPECT_EQ(world.GetBlock(5, 64, 5), BlockRegistry::kAir);

    // Setting a block should work after loading.
    world.SetBlock(5, 64, 5, BlockRegistry::kGrass);
    EXPECT_EQ(world.GetBlock(5, 64, 5), BlockRegistry::kGrass);
}

TEST(World, UnloadChunk) {
    World world;
    world.LoadChunk(0, 0);
    world.SetBlock(5, 64, 5, BlockRegistry::kStone);
    EXPECT_EQ(world.GetBlock(5, 64, 5), BlockRegistry::kStone);

    world.UnloadChunk(0, 0);
    EXPECT_FALSE(world.HasChunk(0, 0));
    // After unloading, GetBlock should return Air.
    EXPECT_EQ(world.GetBlock(5, 64, 5), BlockRegistry::kAir);
}

TEST(World, NegativeCoords) {
    World world;
    world.SetBlock(-5, 64, -10, BlockRegistry::kDirt);
    EXPECT_EQ(world.GetBlock(-5, 64, -10), BlockRegistry::kDirt);

    // Verify chunk was created at the correct negative chunk coordinates.
    int cx = World::WorldToChunkCoord(-5);   // -1
    int cz = World::WorldToChunkCoord(-10);  // -1
    EXPECT_TRUE(world.HasChunk(cx, cz));
}

TEST(World, DirtyPropagation) {
    World world;
    // Load chunk (0,0) and its -X neighbor (-1, 0).
    world.LoadChunk(0, 0);
    world.LoadChunk(-1, 0);

    // Clear dirty flags on both chunks.
    Chunk* chunk_0_0 = world.GetChunk(0, 0);
    Chunk* chunk_neg1_0 = world.GetChunk(-1, 0);
    ASSERT_NE(chunk_0_0, nullptr);
    ASSERT_NE(chunk_neg1_0, nullptr);

    chunk_0_0->ClearDirty();
    chunk_neg1_0->ClearDirty();
    EXPECT_FALSE(chunk_0_0->IsDirty());
    EXPECT_FALSE(chunk_neg1_0->IsDirty());

    // Set block at world (0, 64, 0), which is chunk (0,0) local x=0.
    // This is a boundary edit — should dirty the -X neighbor chunk (-1, 0).
    world.SetBlock(0, 64, 0, BlockRegistry::kStone);

    EXPECT_TRUE(chunk_0_0->IsDirty());
    EXPECT_TRUE(chunk_neg1_0->IsDirty());
}

TEST(World, ChunkCount) {
    World world;
    EXPECT_EQ(world.ChunkCount(), 0u);

    world.LoadChunk(0, 0);
    world.LoadChunk(1, 0);
    world.LoadChunk(0, 1);
    EXPECT_EQ(world.ChunkCount(), 3u);

    world.UnloadChunk(1, 0);
    EXPECT_EQ(world.ChunkCount(), 2u);
}

TEST(World, MultipleChunksIndependent) {
    World world;
    // Set different blocks in different chunks.
    world.SetBlock(5, 64, 5, BlockRegistry::kStone);      // chunk (0, 0)
    world.SetBlock(20, 64, 5, BlockRegistry::kDirt);       // chunk (1, 0)
    world.SetBlock(5, 64, 20, BlockRegistry::kGrass);      // chunk (0, 1)

    // Each block should be independent.
    EXPECT_EQ(world.GetBlock(5, 64, 5), BlockRegistry::kStone);
    EXPECT_EQ(world.GetBlock(20, 64, 5), BlockRegistry::kDirt);
    EXPECT_EQ(world.GetBlock(5, 64, 20), BlockRegistry::kGrass);

    // Modifying one chunk should not affect others.
    world.SetBlock(5, 64, 5, BlockRegistry::kAir);
    EXPECT_EQ(world.GetBlock(5, 64, 5), BlockRegistry::kAir);
    EXPECT_EQ(world.GetBlock(20, 64, 5), BlockRegistry::kDirt);
    EXPECT_EQ(world.GetBlock(5, 64, 20), BlockRegistry::kGrass);
}

TEST(World, LargeCoordinates) {
    World world;
    world.SetBlock(10000, 64, 10000, BlockRegistry::kStone);
    EXPECT_EQ(world.GetBlock(10000, 64, 10000), BlockRegistry::kStone);

    // Verify the chunk coordinate is correct.
    int cx = World::WorldToChunkCoord(10000);  // 625
    int cz = World::WorldToChunkCoord(10000);  // 625
    EXPECT_EQ(cx, 625);
    EXPECT_EQ(cz, 625);
    EXPECT_TRUE(world.HasChunk(cx, cz));
}
