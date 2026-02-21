#include <gtest/gtest.h>

#include <cmath>

#include "vibecraft/block.h"
#include "vibecraft/player.h"

// M7: Player Physics

using namespace vibecraft;

/// Helper: returns Air for all coordinates (empty world).
static BlockId EmptyWorld(int /*x*/, int /*y*/, int /*z*/) {
    return BlockRegistry::kAir;
}

/// Helper: returns Stone for y < 0, Air otherwise (flat ground at y=0).
static BlockId FlatGround(int /*x*/, int y, int /*z*/) {
    return (y < 0) ? BlockRegistry::kStone : BlockRegistry::kAir;
}

/// Helper: returns Stone for y <= ground_y for a configurable ground level.
class GroundAt {
public:
    explicit GroundAt(int ground_y) : ground_y_(ground_y) {}
    BlockId operator()(int /*x*/, int y, int /*z*/) const {
        return (y <= ground_y_) ? BlockRegistry::kStone : BlockRegistry::kAir;
    }
private:
    int ground_y_;
};

/// Helper: flat ground with a wall at a specific x position.
class FlatGroundWithWall {
public:
    FlatGroundWithWall(int wall_x) : wall_x_(wall_x) {}
    BlockId operator()(int x, int y, int /*z*/) const {
        if (y < 0) return BlockRegistry::kStone;  // ground
        if (x == wall_x_ && y >= 0 && y <= 2) return BlockRegistry::kStone;  // wall
        return BlockRegistry::kAir;
    }
private:
    int wall_x_;
};

/// Helper: flat ground with a ceiling at a specific y position.
class FlatGroundWithCeiling {
public:
    FlatGroundWithCeiling(int ceiling_y) : ceiling_y_(ceiling_y) {}
    BlockId operator()(int /*x*/, int y, int /*z*/) const {
        if (y < 0) return BlockRegistry::kStone;       // ground
        if (y == ceiling_y_) return BlockRegistry::kStone;  // ceiling
        return BlockRegistry::kAir;
    }
private:
    int ceiling_y_;
};

/// Helper: flat ground with water at y=0.
static BlockId FlatGroundWithWater(int /*x*/, int y, int /*z*/) {
    if (y < 0) return BlockRegistry::kStone;
    if (y == 0) return BlockRegistry::kWater;
    return BlockRegistry::kAir;
}

TEST(PlayerPhysics, GravityAccelerates) {
    BlockRegistry registry;
    Player player(glm::vec3(0.0f, 100.0f, 0.0f));  // High up, no ground

    player.Update(EmptyWorld, registry);

    // After 1 tick at 20 tps: dv = -32 * (1/20) = -1.6
    float expected_vy = -Player::kGravity * Player::kTickDt;
    EXPECT_NEAR(player.GetVelocity().y, expected_vy, 0.01f);

    // Position should have moved down.
    EXPECT_LT(player.GetPosition().y, 100.0f);
}

TEST(PlayerPhysics, LandingOnGround) {
    BlockRegistry registry;
    // Player starts slightly above ground (ground top is y=0, player at y=0.5).
    Player player(glm::vec3(0.5f, 0.5f, 0.5f));
    player.SetVelocity(glm::vec3(0.0f, -5.0f, 0.0f));

    // Run several ticks to let the player land.
    for (int i = 0; i < 20; ++i) {
        player.Update(FlatGround, registry);
    }

    // Player should be on the ground (y=0 is the top of the ground blocks at y=-1).
    EXPECT_NEAR(player.GetPosition().y, 0.0f, 0.02f);
    EXPECT_NEAR(player.GetVelocity().y, 0.0f, 0.1f);
    EXPECT_TRUE(player.IsGrounded());
}

TEST(PlayerPhysics, TerminalVelocity) {
    BlockRegistry registry;
    Player player(glm::vec3(0.0f, 10000.0f, 0.0f));

    // Run many ticks to let velocity build up.
    for (int i = 0; i < 1000; ++i) {
        player.Update(EmptyWorld, registry);
    }

    EXPECT_LE(std::abs(player.GetVelocity().y), Player::kTerminalVelocity + 0.01f);
}

TEST(PlayerPhysics, JumpFromGround) {
    BlockRegistry registry;
    // Place player on the ground.
    Player player(glm::vec3(0.5f, 0.0f, 0.5f));

    // Run a tick to detect ground.
    player.Update(FlatGround, registry);
    ASSERT_TRUE(player.IsGrounded());

    // Jump.
    player.Jump();

    EXPECT_NEAR(player.GetVelocity().y, Player::kJumpVelocity, 0.01f);
}

TEST(PlayerPhysics, NoJumpInAir) {
    BlockRegistry registry;
    Player player(glm::vec3(0.0f, 100.0f, 0.0f));

    // One tick in the air.
    player.Update(EmptyWorld, registry);
    ASSERT_FALSE(player.IsGrounded());

    float vy_before = player.GetVelocity().y;
    player.Jump();  // Should have no effect.
    EXPECT_FLOAT_EQ(player.GetVelocity().y, vy_before);
}

TEST(PlayerPhysics, WallCollision) {
    BlockRegistry registry;
    // Wall at x=3 (block occupies x=[3,4]).
    // Player starts at x=2.0 on ground, moving +x.
    FlatGroundWithWall world(3);
    Player player(glm::vec3(2.0f, 0.0f, 0.5f));

    // Give the player velocity toward the wall.
    player.SetVelocity(glm::vec3(10.0f, 0.0f, 0.0f));

    // Run a tick to establish grounding first.
    player.Update(world, registry);

    // Now set velocity toward the wall again and tick.
    player.SetVelocity(glm::vec3(10.0f, 0.0f, 0.0f));
    for (int i = 0; i < 20; ++i) {
        player.Update(world, registry);
    }

    // Player should not have passed through the wall at x=3.
    // Player's right edge (pos.x + 0.3) should be <= 3.0.
    float player_right = player.GetPosition().x + Player::kWidth * 0.5f;
    EXPECT_LE(player_right, 3.0f + 0.01f);

    // X velocity should be zeroed by the collision.
    EXPECT_NEAR(player.GetVelocity().x, 0.0f, 0.1f);
}

TEST(PlayerPhysics, CeilingCollision) {
    BlockRegistry registry;
    // Ceiling at y=3 (block occupies y=[3,4]).
    // Player on ground at y=0, height is 1.8, so player top at y=1.8.
    // Jump velocity = 9.2, should hit ceiling.
    FlatGroundWithCeiling world(3);
    Player player(glm::vec3(0.5f, 0.0f, 0.5f));

    // Establish grounding.
    player.Update(world, registry);
    ASSERT_TRUE(player.IsGrounded());

    // Jump.
    player.Jump();
    ASSERT_NEAR(player.GetVelocity().y, Player::kJumpVelocity, 0.01f);

    // Run ticks until the player hits the ceiling.
    for (int i = 0; i < 10; ++i) {
        player.Update(world, registry);
        // If ceiling hit, vy should go to 0.
        if (player.GetPosition().y + Player::kHeight >= 3.0f - 0.02f) {
            break;
        }
    }

    // Player top should not exceed the ceiling.
    float player_top = player.GetPosition().y + Player::kHeight;
    EXPECT_LE(player_top, 3.0f + 0.02f);

    // Velocity should be zeroed or negative after ceiling hit.
    EXPECT_LE(player.GetVelocity().y, 0.01f);
}

TEST(PlayerPhysics, WaterPassthrough) {
    BlockRegistry registry;
    // Ground at y < 0, water at y=0.
    Player player(glm::vec3(0.5f, 2.0f, 0.5f));
    player.SetVelocity(glm::vec3(0.0f, -5.0f, 0.0f));

    // Run ticks: the player should fall through the water and land on the
    // stone ground below (y=0, top of stone at y=0).
    for (int i = 0; i < 40; ++i) {
        player.Update(FlatGroundWithWater, registry);
    }

    // Player should be at the ground level (top of stone at y=-1 + 1 = 0).
    EXPECT_NEAR(player.GetPosition().y, 0.0f, 0.02f);
    EXPECT_TRUE(player.IsGrounded());
}

TEST(PlayerPhysics, PlayerAABBSize) {
    Player player(glm::vec3(5.0f, 10.0f, 5.0f));
    AABB box = player.GetAABB();

    float width_x = box.max.x - box.min.x;
    float height = box.max.y - box.min.y;
    float width_z = box.max.z - box.min.z;

    EXPECT_NEAR(width_x, Player::kWidth, 1e-5f);
    EXPECT_NEAR(height, Player::kHeight, 1e-5f);
    EXPECT_NEAR(width_z, Player::kWidth, 1e-5f);

    // Check centering: min.x = pos.x - width/2, max.x = pos.x + width/2.
    EXPECT_NEAR(box.min.x, 5.0f - 0.3f, 1e-5f);
    EXPECT_NEAR(box.max.x, 5.0f + 0.3f, 1e-5f);
    EXPECT_NEAR(box.min.y, 10.0f, 1e-5f);
    EXPECT_NEAR(box.max.y, 11.8f, 1e-5f);
}

TEST(PlayerPhysics, GroundedOnEdge) {
    BlockRegistry registry;
    // Player standing on the very edge of a block.
    // Block at (0, -1, 0) has top at y=0. Player at x=0.71 so their left edge
    // (0.71 - 0.3 = 0.41) is still over the block (block x range [0,1]).
    Player player(glm::vec3(0.71f, 0.0f, 0.5f));

    // Run a tick to establish grounding.
    player.Update(FlatGround, registry);

    EXPECT_TRUE(player.IsGrounded());
}

TEST(PlayerPhysics, FallingNotGrounded) {
    BlockRegistry registry;
    // Player starts on ground, then walks off the edge.
    // We simulate this by placing the player in the air after being grounded.
    Player player(glm::vec3(0.5f, 0.0f, 0.5f));

    // Establish grounding.
    player.Update(FlatGround, registry);
    ASSERT_TRUE(player.IsGrounded());

    // Move the player to a position with no ground below.
    player.SetPosition(glm::vec3(0.5f, 5.0f, 0.5f));
    player.SetVelocity(glm::vec3(0.0f, 0.0f, 0.0f));

    // Tick once in the air.
    player.Update(EmptyWorld, registry);

    EXPECT_FALSE(player.IsGrounded());
}
