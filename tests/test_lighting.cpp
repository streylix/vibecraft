#include <gtest/gtest.h>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/lighting.h"
#include "vibecraft/world.h"

// M15: Lighting System (Sun + Block)

using namespace vibecraft;

class LightingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Load a single chunk at (0,0) for most tests.
        world.LoadChunk(0, 0);
    }

    BlockRegistry registry;
    World world;
};

TEST_F(LightingTest, TorchEmission) {
    // Place a torch at (8, 64, 8) in the chunk.
    world.SetBlock(8, 64, 8, BlockRegistry::kTorch);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(8, 64, 8);

    // Block at torch position should have block_light == 14.
    EXPECT_EQ(engine.GetBlockLight(8, 64, 8), 14);
}

TEST_F(LightingTest, TorchSpread) {
    // Place a torch at (8, 64, 8).
    world.SetBlock(8, 64, 8, BlockRegistry::kTorch);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(8, 64, 8);

    // Light should decrease by 1 per block distance along +X.
    // 1 block away = 13, 2 = 12, ..., 13 away = 1, 14 away = 0
    for (int d = 1; d <= 14; ++d) {
        int expected = 14 - d;
        int bx = 8 + d;
        // Only check if within chunk bounds (x < 16).
        if (bx < 16) {
            EXPECT_EQ(engine.GetBlockLight(bx, 64, 8), expected)
                << "Distance " << d << " along +X";
        }
    }

    // Also check along +Y direction.
    for (int d = 1; d <= 14; ++d) {
        int expected = 14 - d;
        int by = 64 + d;
        if (by < 256) {
            EXPECT_EQ(engine.GetBlockLight(8, by, 8), expected)
                << "Distance " << d << " along +Y";
        }
    }
}

TEST_F(LightingTest, SolidBlocksLight) {
    // Place a torch at (8, 64, 8).
    world.SetBlock(8, 64, 8, BlockRegistry::kTorch);

    // Place a stone wall at x=9, blocking light in +X direction.
    world.SetBlock(9, 64, 8, BlockRegistry::kStone);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(8, 64, 8);

    // Torch should still be 14.
    EXPECT_EQ(engine.GetBlockLight(8, 64, 8), 14);

    // Stone block at x=9 should have 0 block light (solid, not transparent).
    EXPECT_EQ(engine.GetBlockLight(9, 64, 8), 0);

    // Block at x=10 should not receive direct +X light from the torch
    // because stone is in the way. It may receive light from other
    // directions though (going around the stone). Check it's less than
    // what it would be without the stone (which would be 12).
    // Without stone: block at x=10 would be 14-2=12.
    // With stone: light must go around, so it will be less.
    EXPECT_LT(engine.GetBlockLight(10, 64, 8), 12);
}

TEST_F(LightingTest, TorchRemoval) {
    // Place a torch at (8, 64, 8).
    world.SetBlock(8, 64, 8, BlockRegistry::kTorch);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(8, 64, 8);

    // Verify torch light is present.
    EXPECT_EQ(engine.GetBlockLight(8, 64, 8), 14);
    EXPECT_EQ(engine.GetBlockLight(9, 64, 8), 13);

    // Remove the torch.
    engine.RemoveBlockLight(8, 64, 8);
    world.SetBlock(8, 64, 8, BlockRegistry::kAir);

    // Light should be removed.
    EXPECT_EQ(engine.GetBlockLight(8, 64, 8), 0);
    EXPECT_EQ(engine.GetBlockLight(9, 64, 8), 0);
    EXPECT_EQ(engine.GetBlockLight(10, 64, 8), 0);
}

TEST_F(LightingTest, SunlightFromSky) {
    // All air chunk — sunlight should propagate from top at level 15.
    LightingEngine engine(world, registry);
    engine.PropagateSunlightColumn(8, 8);

    // Every block in the column should have sunlight = 15
    // (all transparent, no solid blocks).
    for (int y = 0; y < 256; ++y) {
        EXPECT_EQ(engine.GetSunLight(8, y, 8), 15)
            << "Sunlight at y=" << y << " should be 15 in open air column";
    }
}

TEST_F(LightingTest, SunlightBlocked) {
    // Place a stone "roof" at y=100 spanning the column.
    world.SetBlock(8, 100, 8, BlockRegistry::kStone);

    LightingEngine engine(world, registry);
    engine.PropagateSunlightColumn(8, 8);

    // Above the roof: sunlight = 15.
    EXPECT_EQ(engine.GetSunLight(8, 101, 8), 15);
    EXPECT_EQ(engine.GetSunLight(8, 200, 8), 15);

    // At the roof (stone): sunlight = 0.
    EXPECT_EQ(engine.GetSunLight(8, 100, 8), 0);

    // Below the roof: sunlight = 0.
    EXPECT_EQ(engine.GetSunLight(8, 99, 8), 0);
    EXPECT_EQ(engine.GetSunLight(8, 50, 8), 0);
    EXPECT_EQ(engine.GetSunLight(8, 0, 8), 0);
}

TEST_F(LightingTest, CrossChunkLight) {
    // Load a neighbor chunk at (1, 0).
    world.LoadChunk(1, 0);

    // Place a torch at x=15 (right edge of chunk 0), y=64, z=8.
    world.SetBlock(15, 64, 8, BlockRegistry::kTorch);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(15, 64, 8);

    // Torch at x=15 emits 14.
    EXPECT_EQ(engine.GetBlockLight(15, 64, 8), 14);

    // x=16 is in chunk (1,0), local x=0. Should be 13.
    EXPECT_EQ(engine.GetBlockLight(16, 64, 8), 13);

    // x=17 should be 12.
    EXPECT_EQ(engine.GetBlockLight(17, 64, 8), 12);
}

TEST_F(LightingTest, GlassTransmitsLight) {
    // Place a torch at (8, 64, 8).
    world.SetBlock(8, 64, 8, BlockRegistry::kTorch);

    // Place glass at x=9 (between torch and x=10).
    world.SetBlock(9, 64, 8, BlockRegistry::kGlass);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(8, 64, 8);

    // Torch is 14.
    EXPECT_EQ(engine.GetBlockLight(8, 64, 8), 14);

    // Glass at x=9 should let light through: 14 - 1 = 13.
    EXPECT_EQ(engine.GetBlockLight(9, 64, 8), 13);

    // Block at x=10 should be 12 (light passed through glass with -1).
    EXPECT_EQ(engine.GetBlockLight(10, 64, 8), 12);
}

TEST_F(LightingTest, MultiSourceMax) {
    // Place two torches: one at (5, 64, 8) and one at (11, 64, 8).
    world.SetBlock(5, 64, 8, BlockRegistry::kTorch);
    world.SetBlock(11, 64, 8, BlockRegistry::kTorch);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(5, 64, 8);
    engine.AddBlockLight(11, 64, 8);

    // At the midpoint (8, 64, 8), distance from torch at 5 is 3 => 14-3=11.
    // Distance from torch at 11 is 3 => 14-3=11.
    // Should be max(11, 11) = 11, not sum.
    EXPECT_EQ(engine.GetBlockLight(8, 64, 8), 11);

    // At (6, 64, 8): dist from torch at 5 is 1 => 13.
    // dist from torch at 11 is 5 => 9. Max = 13.
    EXPECT_EQ(engine.GetBlockLight(6, 64, 8), 13);

    // At (10, 64, 8): dist from torch at 11 is 1 => 13.
    // dist from torch at 5 is 5 => 9. Max = 13.
    EXPECT_EQ(engine.GetBlockLight(10, 64, 8), 13);
}

TEST_F(LightingTest, LavaEmission) {
    // Place lava at (8, 64, 8).
    world.SetBlock(8, 64, 8, BlockRegistry::kLava);

    LightingEngine engine(world, registry);
    engine.AddBlockLight(8, 64, 8);

    // Lava emits light level 15.
    EXPECT_EQ(engine.GetBlockLight(8, 64, 8), 15);

    // 1 block away should be 14.
    EXPECT_EQ(engine.GetBlockLight(9, 64, 8), 14);

    // 2 blocks away should be 13.
    EXPECT_EQ(engine.GetBlockLight(10, 64, 8), 13);
}
