#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "vibecraft/aabb.h"
#include "vibecraft/block.h"
#include "vibecraft/block_interaction.h"

// M13: Block Interaction - Break/Place

using namespace vibecraft;

// Helper: create a block query backed by a simple map.
class SimpleWorld {
public:
    SimpleWorld() = default;

    void SetBlock(int x, int y, int z, BlockId id) {
        blocks_[Key(x, y, z)] = id;
    }

    BlockId GetBlock(int x, int y, int z) const {
        auto it = blocks_.find(Key(x, y, z));
        if (it != blocks_.end()) return it->second;
        return BlockRegistry::kAir;
    }

    BlockQueryFunc QueryFunc() const {
        return [this](int x, int y, int z) -> BlockId {
            return GetBlock(x, y, z);
        };
    }

    BlockSetFunc SetFunc() {
        return [this](int x, int y, int z, BlockId id) {
            SetBlock(x, y, z, id);
        };
    }

private:
    static int64_t Key(int x, int y, int z) {
        // Pack coords into a single int64_t for map key.
        return (static_cast<int64_t>(x) & 0xFFFFF) |
               ((static_cast<int64_t>(y) & 0xFFFFF) << 20) |
               ((static_cast<int64_t>(z) & 0xFFFFF) << 40);
    }

    std::unordered_map<int64_t, BlockId> blocks_;
};

TEST(BlockInteraction, BreakTimedByHardness) {
    BlockRegistry registry;
    BlockInteraction interaction;
    SimpleWorld world;

    // Stone has hardness 1.5, dirt has hardness 0.5.
    // Break time = hardness * kBaseBreakTime (1.0).
    // So stone takes 1.5s, dirt takes 0.5s.

    world.SetBlock(0, 0, 0, BlockRegistry::kStone);
    world.SetBlock(1, 0, 0, BlockRegistry::kDirt);

    // Try breaking stone for 0.4 seconds — should NOT be broken.
    bool broken = interaction.UpdateBreaking(
        glm::ivec3(0, 0, 0), BlockRegistry::kStone, registry,
        0.4f, world.SetFunc());
    EXPECT_FALSE(broken);

    // Reset and try breaking dirt for 0.4 seconds — should NOT be broken yet.
    interaction.ResetBreaking();
    broken = interaction.UpdateBreaking(
        glm::ivec3(1, 0, 0), BlockRegistry::kDirt, registry,
        0.4f, world.SetFunc());
    EXPECT_FALSE(broken);

    // Continue breaking dirt for another 0.2 seconds (total 0.6 > 0.5).
    broken = interaction.UpdateBreaking(
        glm::ivec3(1, 0, 0), BlockRegistry::kDirt, registry,
        0.2f, world.SetFunc());
    EXPECT_TRUE(broken);  // Dirt should be broken now.

    // Stone should still be there.
    EXPECT_EQ(world.GetBlock(0, 0, 0), BlockRegistry::kStone);
}

TEST(BlockInteraction, BreakCompletesAtZero) {
    BlockRegistry registry;
    BlockInteraction interaction;
    SimpleWorld world;

    world.SetBlock(5, 5, 5, BlockRegistry::kDirt);

    // Dirt hardness = 0.5, break time = 0.5s. Apply exactly 0.5s.
    bool broken = interaction.UpdateBreaking(
        glm::ivec3(5, 5, 5), BlockRegistry::kDirt, registry,
        0.5f, world.SetFunc());
    EXPECT_TRUE(broken);

    // The block should now be air.
    EXPECT_EQ(world.GetBlock(5, 5, 5), BlockRegistry::kAir);
}

TEST(BlockInteraction, BedrockUnbreakable) {
    BlockRegistry registry;
    BlockInteraction interaction;
    SimpleWorld world;

    world.SetBlock(0, 0, 0, BlockRegistry::kBedrock);

    // Try to break bedrock — hardness is -1.0 (negative = unbreakable).
    bool broken = interaction.UpdateBreaking(
        glm::ivec3(0, 0, 0), BlockRegistry::kBedrock, registry,
        100.0f, world.SetFunc());
    EXPECT_FALSE(broken);

    // Bedrock should still be there.
    EXPECT_EQ(world.GetBlock(0, 0, 0), BlockRegistry::kBedrock);
}

TEST(BlockInteraction, PlaceOnFace) {
    BlockRegistry registry;
    SimpleWorld world;

    // Place a stone block at (5, 5, 5).
    world.SetBlock(5, 5, 5, BlockRegistry::kStone);

    // Player is far away so their AABB doesn't interfere.
    AABB player_aabb(glm::vec3(100.0f, 100.0f, 100.0f),
                     glm::vec3(100.6f, 101.8f, 100.6f));

    // Place dirt on the +X face of the stone block.
    bool placed = BlockInteraction::PlaceBlock(
        glm::ivec3(5, 5, 5),
        glm::ivec3(1, 0, 0),  // +X face normal
        BlockRegistry::kDirt,
        player_aabb,
        world.QueryFunc(),
        world.SetFunc(),
        registry);

    EXPECT_TRUE(placed);
    // Block should be placed at (6, 5, 5).
    EXPECT_EQ(world.GetBlock(6, 5, 5), BlockRegistry::kDirt);
}

TEST(BlockInteraction, PlaceNotInsidePlayer) {
    BlockRegistry registry;
    SimpleWorld world;

    // Place a stone block at (5, 5, 5).
    world.SetBlock(5, 5, 5, BlockRegistry::kStone);

    // Player AABB overlaps position (6, 5, 5) — the +X placement spot.
    // Player feet at (5.7, 5.0, 5.0), AABB centered at x with width 0.6.
    AABB player_aabb(glm::vec3(5.7f, 5.0f, 4.7f),
                     glm::vec3(6.3f, 6.8f, 5.3f));

    bool placed = BlockInteraction::PlaceBlock(
        glm::ivec3(5, 5, 5),
        glm::ivec3(1, 0, 0),  // +X face
        BlockRegistry::kDirt,
        player_aabb,
        world.QueryFunc(),
        world.SetFunc(),
        registry);

    EXPECT_FALSE(placed);
    // No block should have been placed.
    EXPECT_EQ(world.GetBlock(6, 5, 5), BlockRegistry::kAir);
}

TEST(BlockInteraction, BreakResetOnLookAway) {
    BlockRegistry registry;
    BlockInteraction interaction;
    SimpleWorld world;

    world.SetBlock(0, 0, 0, BlockRegistry::kStone);
    world.SetBlock(1, 0, 0, BlockRegistry::kStone);

    // Stone hardness = 1.5, break time = 1.5s.
    // Break block (0,0,0) for 1.0s (2/3 through).
    interaction.UpdateBreaking(
        glm::ivec3(0, 0, 0), BlockRegistry::kStone, registry,
        1.0f, world.SetFunc());

    float progress_before = interaction.GetBreakProgress();
    EXPECT_GT(progress_before, 0.5f);  // Should be ~0.667

    // Look at a different block — this should cause a reset.
    interaction.UpdateBreaking(
        glm::ivec3(1, 0, 0), BlockRegistry::kStone, registry,
        0.1f, world.SetFunc());

    float progress_after = interaction.GetBreakProgress();
    // Progress on the new block should be very small (just 0.1/1.5 ~ 0.067).
    EXPECT_LT(progress_after, 0.15f);

    // The first block should still be intact.
    EXPECT_EQ(world.GetBlock(0, 0, 0), BlockRegistry::kStone);
}

TEST(BlockInteraction, PlaceOnlyOnSolid) {
    BlockRegistry registry;
    SimpleWorld world;

    // No solid block at (5, 5, 5) — it's air.
    // Player far away.
    AABB player_aabb(glm::vec3(100.0f, 100.0f, 100.0f),
                     glm::vec3(100.6f, 101.8f, 100.6f));

    // Try to place dirt on the +X face of air — should fail.
    bool placed = BlockInteraction::PlaceBlock(
        glm::ivec3(5, 5, 5),
        glm::ivec3(1, 0, 0),
        BlockRegistry::kDirt,
        player_aabb,
        world.QueryFunc(),
        world.SetFunc(),
        registry);

    EXPECT_FALSE(placed);
    // No block should have been placed.
    EXPECT_EQ(world.GetBlock(6, 5, 5), BlockRegistry::kAir);
}
