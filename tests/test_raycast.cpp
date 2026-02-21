#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "vibecraft/block.h"
#include "vibecraft/raycast.h"

// M13: Block Interaction - Raycast

using namespace vibecraft;

// Helper: create a block query that returns kAir for everything.
static BlockQueryFunc EmptyWorldQuery() {
    return [](int, int, int) -> BlockId {
        return BlockRegistry::kAir;
    };
}

// Helper: create a block query with a single solid block at a given position.
static BlockQueryFunc SingleBlockQuery(int bx, int by, int bz, BlockId id) {
    return [=](int x, int y, int z) -> BlockId {
        if (x == bx && y == by && z == bz) return id;
        return BlockRegistry::kAir;
    };
}

// Helper: create a block query with multiple solid blocks.
static BlockQueryFunc MultiBlockQuery(
    const std::vector<std::pair<glm::ivec3, BlockId>>& blocks) {
    return [blocks](int x, int y, int z) -> BlockId {
        for (const auto& [pos, id] : blocks) {
            if (pos.x == x && pos.y == y && pos.z == z) return id;
        }
        return BlockRegistry::kAir;
    };
}

TEST(Raycast, HitBlock) {
    BlockRegistry registry;

    // Place a stone block at (3, 0, 0).
    auto query = SingleBlockQuery(3, 0, 0, BlockRegistry::kStone);

    // Ray from origin looking along +X (y=0.5 to be inside the block's y-range).
    glm::vec3 origin(0.5f, 0.5f, 0.5f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);

    RaycastResult result = CastRay(origin, direction, 5.0f, query, registry);

    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.block_position, glm::ivec3(3, 0, 0));
    EXPECT_GT(result.distance, 0.0f);
}

TEST(Raycast, MissInEmptyWorld) {
    BlockRegistry registry;
    auto query = EmptyWorldQuery();

    glm::vec3 origin(0.0f, 0.0f, 0.0f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);

    RaycastResult result = CastRay(origin, direction, 5.0f, query, registry);

    EXPECT_FALSE(result.hit);
}

TEST(Raycast, MaxDistance) {
    BlockRegistry registry;

    // Place a stone block at (10, 0, 0) -- beyond 5 block reach.
    auto query = SingleBlockQuery(10, 0, 0, BlockRegistry::kStone);

    glm::vec3 origin(0.5f, 0.5f, 0.5f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);

    RaycastResult result = CastRay(origin, direction, 5.0f, query, registry);

    EXPECT_FALSE(result.hit);
}

TEST(Raycast, NearestBlock) {
    BlockRegistry registry;

    // Place two stone blocks along the +X axis.
    auto query = MultiBlockQuery({
        {glm::ivec3(2, 0, 0), BlockRegistry::kStone},
        {glm::ivec3(4, 0, 0), BlockRegistry::kStone},
    });

    glm::vec3 origin(0.5f, 0.5f, 0.5f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);

    RaycastResult result = CastRay(origin, direction, 5.0f, query, registry);

    EXPECT_TRUE(result.hit);
    // Should hit the nearer block at (2, 0, 0), not the one at (4, 0, 0).
    EXPECT_EQ(result.block_position, glm::ivec3(2, 0, 0));
}

TEST(Raycast, FaceNormalPosX) {
    BlockRegistry registry;

    // Place a stone block at (3, 0, 0). Ray from left hits the -X face,
    // but the face normal should point toward the ray, i.e., (-1, 0, 0).
    // To get +X face normal, approach from the +X side.
    auto query = SingleBlockQuery(3, 0, 0, BlockRegistry::kStone);

    // Approach from +X side (origin at x=5, looking -X).
    glm::vec3 origin(5.5f, 0.5f, 0.5f);
    glm::vec3 direction(-1.0f, 0.0f, 0.0f);

    RaycastResult result = CastRay(origin, direction, 5.0f, query, registry);

    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.block_position, glm::ivec3(3, 0, 0));
    // Normal points outward from the +X face.
    EXPECT_EQ(result.face_normal, glm::ivec3(1, 0, 0));
}

TEST(Raycast, FaceNormalNegY) {
    BlockRegistry registry;

    // Place a stone block at (0, 3, 0). Approach from below (negative Y).
    auto query = SingleBlockQuery(0, 3, 0, BlockRegistry::kStone);

    glm::vec3 origin(0.5f, 0.5f, 0.5f);
    glm::vec3 direction(0.0f, 1.0f, 0.0f);

    RaycastResult result = CastRay(origin, direction, 5.0f, query, registry);

    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.block_position, glm::ivec3(0, 3, 0));
    // Approached from below, so the face normal points down (-Y).
    EXPECT_EQ(result.face_normal, glm::ivec3(0, -1, 0));
}

TEST(Raycast, FaceNormalPosZ) {
    BlockRegistry registry;

    // Place a stone block at (0, 0, 3). Approach from +Z side.
    auto query = SingleBlockQuery(0, 0, 3, BlockRegistry::kStone);

    glm::vec3 origin(0.5f, 0.5f, 5.5f);
    glm::vec3 direction(0.0f, 0.0f, -1.0f);

    RaycastResult result = CastRay(origin, direction, 5.0f, query, registry);

    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.block_position, glm::ivec3(0, 0, 3));
    // Normal points outward from the +Z face.
    EXPECT_EQ(result.face_normal, glm::ivec3(0, 0, 1));
}

TEST(Raycast, DiagonalRay) {
    BlockRegistry registry;

    // Place a stone block at (3, 3, 3). Shoot a 45-degree diagonal ray.
    auto query = SingleBlockQuery(3, 3, 3, BlockRegistry::kStone);

    glm::vec3 origin(0.5f, 0.5f, 0.5f);
    glm::vec3 direction(1.0f, 1.0f, 1.0f);  // 45 degrees on all axes

    // Distance to (3,3,3) is sqrt(2.5^2 * 3) ~ 4.33, within 5.0 range.
    RaycastResult result = CastRay(origin, direction, 10.0f, query, registry);

    EXPECT_TRUE(result.hit);
    EXPECT_EQ(result.block_position, glm::ivec3(3, 3, 3));
}
