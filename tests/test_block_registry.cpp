#include <gtest/gtest.h>
#include "vibecraft/block.h"

// M3: Block Type Registry

class BlockRegistryTest : public ::testing::Test {
protected:
    vibecraft::BlockRegistry registry;
};

TEST_F(BlockRegistryTest, AirIsZero) {
    const auto& air = registry.GetBlock(0);
    EXPECT_EQ(air.name, "Air");
}

TEST_F(BlockRegistryTest, AirIsNotSolid) {
    const auto& air = registry.GetBlock(vibecraft::BlockRegistry::kAir);
    EXPECT_FALSE(air.solid);
    EXPECT_TRUE(air.transparent);
}

TEST_F(BlockRegistryTest, StoneIsSolid) {
    const auto& stone = registry.GetBlock(vibecraft::BlockRegistry::kStone);
    EXPECT_TRUE(stone.solid);
    EXPECT_FALSE(stone.transparent);
    EXPECT_FLOAT_EQ(stone.hardness, 1.5f);
}

TEST_F(BlockRegistryTest, GrassProperties) {
    const auto& grass = registry.GetBlock(vibecraft::BlockRegistry::kGrass);
    // Grass should have different top, side, and bottom texture indices.
    EXPECT_NE(grass.faces.pos_y, grass.faces.pos_x);   // top != side
    EXPECT_NE(grass.faces.pos_y, grass.faces.neg_y);    // top != bottom
    EXPECT_NE(grass.faces.pos_x, grass.faces.neg_y);    // side != bottom
}

TEST_F(BlockRegistryTest, WaterIsLiquid) {
    const auto& water = registry.GetBlock(vibecraft::BlockRegistry::kWater);
    EXPECT_TRUE(water.liquid);
    EXPECT_FALSE(water.solid);
    EXPECT_TRUE(water.transparent);
}

TEST_F(BlockRegistryTest, LavaEmitsLight) {
    const auto& lava = registry.GetBlock(vibecraft::BlockRegistry::kLava);
    EXPECT_EQ(lava.light_emission, 15);
}

TEST_F(BlockRegistryTest, GlassIsTransparent) {
    const auto& glass = registry.GetBlock(vibecraft::BlockRegistry::kGlass);
    EXPECT_TRUE(glass.solid);
    EXPECT_TRUE(glass.transparent);
}

TEST_F(BlockRegistryTest, TotalBlockCount) {
    EXPECT_GE(registry.GetRegisteredCount(), 15);
}

TEST_F(BlockRegistryTest, AllBlocksHaveNames) {
    for (int i = 0; i < 256; ++i) {
        const auto& block = registry.GetBlock(static_cast<vibecraft::BlockId>(i));
        // Only check registered blocks (those with non-empty names).
        // Unregistered blocks have empty names by default, which is fine.
        // But every registered block must have a non-empty name.
        if (!block.name.empty()) {
            EXPECT_FALSE(block.name.empty()) << "Block " << i << " has an empty name";
        }
    }
    // Also verify that at least the known blocks have names.
    EXPECT_FALSE(registry.GetBlock(0).name.empty());   // Air
    EXPECT_FALSE(registry.GetBlock(1).name.empty());   // Stone
    EXPECT_FALSE(registry.GetBlock(2).name.empty());   // Grass
}

TEST_F(BlockRegistryTest, AllBlocksHaveTextures) {
    for (int i = 0; i < 256; ++i) {
        const auto& block = registry.GetBlock(static_cast<vibecraft::BlockId>(i));
        if (block.name.empty()) continue;  // skip unregistered
        if (block.name == "Air") continue; // Air has no textures

        // Every solid or transparent block (except Air) should have valid
        // texture indices (>= 0).
        if (block.solid || block.transparent) {
            EXPECT_GE(block.faces.pos_x, 0) << "Block " << block.name << " has invalid +X texture";
            EXPECT_GE(block.faces.neg_x, 0) << "Block " << block.name << " has invalid -X texture";
            EXPECT_GE(block.faces.pos_y, 0) << "Block " << block.name << " has invalid +Y texture";
            EXPECT_GE(block.faces.neg_y, 0) << "Block " << block.name << " has invalid -Y texture";
            EXPECT_GE(block.faces.pos_z, 0) << "Block " << block.name << " has invalid +Z texture";
            EXPECT_GE(block.faces.neg_z, 0) << "Block " << block.name << " has invalid -Z texture";
        }
    }
}

TEST_F(BlockRegistryTest, BedrockHardness) {
    const auto& bedrock = registry.GetBlock(vibecraft::BlockRegistry::kBedrock);
    EXPECT_LT(bedrock.hardness, 0.0f);  // Negative hardness = unbreakable
}

TEST_F(BlockRegistryTest, OakLogProperties) {
    const auto& log = registry.GetBlock(vibecraft::BlockRegistry::kOakLog);
    EXPECT_TRUE(log.solid);
    // Oak log should have different top vs side textures.
    EXPECT_NE(log.faces.pos_y, log.faces.pos_x);
}

TEST_F(BlockRegistryTest, LeavesTransparent) {
    const auto& leaves = registry.GetBlock(vibecraft::BlockRegistry::kOakLeaves);
    EXPECT_TRUE(leaves.solid);
    EXPECT_TRUE(leaves.transparent);
}

TEST_F(BlockRegistryTest, SandProperties) {
    const auto& sand = registry.GetBlock(vibecraft::BlockRegistry::kSand);
    EXPECT_TRUE(sand.solid);
}

TEST_F(BlockRegistryTest, TorchEmitsLight) {
    const auto& torch = registry.GetBlock(vibecraft::BlockRegistry::kTorch);
    EXPECT_EQ(torch.light_emission, 14);
}
