#include <gtest/gtest.h>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/fluid.h"
#include "vibecraft/world.h"

// M21: Fluid Simulation

using namespace vibecraft;

/// Helper: create a world with a single loaded chunk at (0,0) and a stone
/// floor at y=0 (so fluids have a surface to flow on).
static World MakeTestWorld() {
    World world;
    world.LoadChunk(0, 0);

    // Place a stone floor at y=0 across the entire chunk.
    for (int x = 0; x < kChunkSizeX; ++x) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            world.SetBlock(x, 0, z, BlockRegistry::kStone);
        }
    }
    return world;
}

TEST(Fluid, WaterFlowsDown) {
    // Place a water source with air below; after one tick water should appear below.
    World world;
    world.LoadChunk(0, 0);

    // Stone floor at y=0.
    world.SetBlock(5, 0, 5, BlockRegistry::kStone);

    // Place water source at y=5 (air below it down to y=1).
    FluidSimulator sim(&world);
    sim.PlaceWaterSource(5, 5, 5);

    // After one tick, water should flow down to y=4.
    sim.Tick();

    EXPECT_EQ(world.GetBlock(5, 4, 5), BlockRegistry::kWater);
    EXPECT_GT(world.GetFluidLevel(5, 4, 5), 0);
}

TEST(Fluid, WaterSpreadsHorizontal) {
    // Water source on a flat surface should spread horizontally.
    World world = MakeTestWorld();
    FluidSimulator sim(&world);

    // Place water source at y=1 (on top of stone floor).
    sim.PlaceWaterSource(8, 1, 8);

    // After enough ticks, water should spread to neighbors.
    for (int i = 0; i < 10; ++i) {
        sim.Tick();
    }

    // Check that horizontal neighbors have water.
    EXPECT_EQ(world.GetBlock(9, 1, 8), BlockRegistry::kWater);
    EXPECT_EQ(world.GetBlock(7, 1, 8), BlockRegistry::kWater);
    EXPECT_EQ(world.GetBlock(8, 1, 9), BlockRegistry::kWater);
    EXPECT_EQ(world.GetBlock(8, 1, 7), BlockRegistry::kWater);

    // Neighbors should have lower fluid levels than the source.
    uint8_t source_level = world.GetFluidLevel(8, 1, 8);
    uint8_t neighbor_level = world.GetFluidLevel(9, 1, 8);
    EXPECT_EQ(source_level, FluidSimulator::kWaterSourceLevel);
    EXPECT_LT(neighbor_level, source_level);
}

TEST(Fluid, WaterLevels) {
    // Level decreases with distance from source: source=7, adj=6, next=5, etc.
    World world = MakeTestWorld();
    FluidSimulator sim(&world);

    sim.PlaceWaterSource(8, 1, 8);

    // Run enough ticks for full horizontal spread.
    for (int i = 0; i < 20; ++i) {
        sim.Tick();
    }

    EXPECT_EQ(world.GetFluidLevel(8, 1, 8), 7);  // source
    EXPECT_EQ(world.GetFluidLevel(9, 1, 8), 6);  // 1 block away
    EXPECT_EQ(world.GetFluidLevel(10, 1, 8), 5); // 2 blocks away
    EXPECT_EQ(world.GetFluidLevel(11, 1, 8), 4); // 3 blocks away
    EXPECT_EQ(world.GetFluidLevel(12, 1, 8), 3); // 4 blocks away
    EXPECT_EQ(world.GetFluidLevel(13, 1, 8), 2); // 5 blocks away
    EXPECT_EQ(world.GetFluidLevel(14, 1, 8), 1); // 6 blocks away

    // 7 blocks away should be air (level 0, no water).
    EXPECT_EQ(world.GetBlock(15, 1, 8), BlockRegistry::kAir);
}

TEST(Fluid, WaterRemoval) {
    // Remove source block; downstream water should drain away.
    World world = MakeTestWorld();
    FluidSimulator sim(&world);

    sim.PlaceWaterSource(8, 1, 8);

    // Let water spread.
    for (int i = 0; i < 20; ++i) {
        sim.Tick();
    }

    // Verify water is flowing.
    EXPECT_EQ(world.GetBlock(9, 1, 8), BlockRegistry::kWater);

    // Remove the source.
    sim.RemoveFluid(8, 1, 8);

    // Run ticks to let water drain.
    for (int i = 0; i < 30; ++i) {
        sim.Tick();
    }

    // All water should be gone.
    EXPECT_EQ(world.GetBlock(8, 1, 8), BlockRegistry::kAir);
    EXPECT_EQ(world.GetBlock(9, 1, 8), BlockRegistry::kAir);
    EXPECT_EQ(world.GetBlock(10, 1, 8), BlockRegistry::kAir);
    EXPECT_EQ(world.GetBlock(7, 1, 8), BlockRegistry::kAir);
}

TEST(Fluid, BasinFill) {
    // Water fills a depression: create a walled pit and add water at the top.
    World world;
    world.LoadChunk(0, 0);

    // Create a 3x3 pit: stone floor at y=0, walls at y=1 and y=2 around the
    // edges, with a 1x1 interior at (5,1,5).
    // Floor:
    for (int x = 4; x <= 6; ++x) {
        for (int z = 4; z <= 6; ++z) {
            world.SetBlock(x, 0, z, BlockRegistry::kStone);
        }
    }
    // Walls (y=1 and y=2): surround (5,_,5) with stone.
    for (int y = 1; y <= 2; ++y) {
        world.SetBlock(4, y, 4, BlockRegistry::kStone);
        world.SetBlock(4, y, 5, BlockRegistry::kStone);
        world.SetBlock(4, y, 6, BlockRegistry::kStone);
        world.SetBlock(5, y, 4, BlockRegistry::kStone);
        world.SetBlock(5, y, 6, BlockRegistry::kStone);
        world.SetBlock(6, y, 4, BlockRegistry::kStone);
        world.SetBlock(6, y, 5, BlockRegistry::kStone);
        world.SetBlock(6, y, 6, BlockRegistry::kStone);
    }

    // The interior is air at (5,1,5) and (5,2,5).
    FluidSimulator sim(&world);

    // Place water above the pit at (5,3,5) — it should flow down and fill.
    sim.PlaceWaterSource(5, 3, 5);

    // Run enough ticks for water to flow down and fill the pit.
    for (int i = 0; i < 20; ++i) {
        sim.Tick();
    }

    // Water should have flowed down into the basin.
    EXPECT_EQ(world.GetBlock(5, 1, 5), BlockRegistry::kWater);
    EXPECT_EQ(world.GetBlock(5, 2, 5), BlockRegistry::kWater);
}

TEST(Fluid, InfiniteSource) {
    // Two water source blocks adjacent with air between → new source.
    World world = MakeTestWorld();
    FluidSimulator sim(&world);

    // Place two water sources with one air gap between them.
    // Sources at (6,1,8) and (8,1,8), air at (7,1,8).
    sim.PlaceWaterSource(6, 1, 8);
    sim.PlaceWaterSource(8, 1, 8);

    // Run ticks so flow blocks appear in the gap.
    for (int i = 0; i < 20; ++i) {
        sim.Tick();
    }

    // The block at (7,1,8) should now be a water source (level 7)
    // because it has two adjacent source blocks.
    EXPECT_EQ(world.GetBlock(7, 1, 8), BlockRegistry::kWater);
    EXPECT_EQ(world.GetFluidLevel(7, 1, 8), FluidSimulator::kWaterSourceLevel);
}

TEST(Fluid, LavaMaxLevel) {
    // Lava source level should be 3.
    World world = MakeTestWorld();
    FluidSimulator sim(&world);

    sim.PlaceLavaSource(8, 1, 8);

    EXPECT_EQ(world.GetBlock(8, 1, 8), BlockRegistry::kLava);
    EXPECT_EQ(world.GetFluidLevel(8, 1, 8), FluidSimulator::kLavaSourceLevel);
    EXPECT_EQ(FluidSimulator::kLavaSourceLevel, 3);
}

TEST(Fluid, LavaSlower) {
    // Lava should spread more slowly than water (takes more ticks).
    World world_water = MakeTestWorld();
    World world_lava = MakeTestWorld();

    FluidSimulator sim_water(&world_water);
    FluidSimulator sim_lava(&world_lava);

    sim_water.PlaceWaterSource(8, 1, 8);
    sim_lava.PlaceLavaSource(8, 1, 8);

    // After 1 tick, water should spread but lava should not (lava updates every
    // 3 ticks, so first lava update is at tick 3).
    sim_water.Tick();
    sim_lava.Tick();

    bool water_spread = (world_water.GetBlock(9, 1, 8) == BlockRegistry::kWater);
    bool lava_spread = (world_lava.GetBlock(9, 1, 8) == BlockRegistry::kLava);

    EXPECT_TRUE(water_spread) << "Water should spread after 1 tick";
    EXPECT_FALSE(lava_spread) << "Lava should NOT spread after 1 tick";

    // After 3 ticks total, lava should have spread.
    sim_lava.Tick();  // tick 2
    sim_lava.Tick();  // tick 3

    lava_spread = (world_lava.GetBlock(9, 1, 8) == BlockRegistry::kLava);
    EXPECT_TRUE(lava_spread) << "Lava should spread after 3 ticks";
}

TEST(Fluid, WaterLavaCobblestone) {
    // Flowing water meets lava source → cobblestone.
    World world = MakeTestWorld();
    FluidSimulator sim(&world);

    // Place lava source at (10,1,8).
    sim.PlaceLavaSource(10, 1, 8);

    // Place water source at (8,1,8) — it will flow toward the lava.
    sim.PlaceWaterSource(8, 1, 8);

    // Run ticks so water reaches the lava.
    for (int i = 0; i < 20; ++i) {
        sim.Tick();
    }

    // The lava at (10,1,8) should have been converted to cobblestone
    // by flowing water (water at (9,1,8) is flowing, not source).
    EXPECT_EQ(world.GetBlock(10, 1, 8), BlockRegistry::kCobblestone);
}

TEST(Fluid, WaterLavaObsidian) {
    // Water source meets lava → obsidian.
    World world = MakeTestWorld();
    FluidSimulator sim(&world);

    // Place water source directly adjacent to lava.
    sim.PlaceWaterSource(8, 1, 8);
    sim.PlaceLavaSource(9, 1, 8);

    // One tick should cause the interaction.
    sim.Tick();

    // The lava at (9,1,8) should be converted to obsidian since a water
    // source is directly adjacent.
    EXPECT_EQ(world.GetBlock(9, 1, 8), BlockRegistry::kObsidian);
}

TEST(Fluid, WaterDoesntFlowUp) {
    // Water below a solid block doesn't flow upward through it.
    World world;
    world.LoadChunk(0, 0);

    // Stone floor at y=0.
    world.SetBlock(5, 0, 5, BlockRegistry::kStone);
    // Stone ceiling at y=2.
    world.SetBlock(5, 2, 5, BlockRegistry::kStone);

    FluidSimulator sim(&world);

    // Place water source at y=1 (below the stone ceiling).
    sim.PlaceWaterSource(5, 1, 5);

    // Run ticks.
    for (int i = 0; i < 20; ++i) {
        sim.Tick();
    }

    // Water should NOT appear above the stone ceiling.
    EXPECT_NE(world.GetBlock(5, 3, 5), BlockRegistry::kWater);
    // The stone at y=2 should still be stone.
    EXPECT_EQ(world.GetBlock(5, 2, 5), BlockRegistry::kStone);
}
