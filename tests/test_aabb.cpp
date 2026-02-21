#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "vibecraft/aabb.h"

using vibecraft::AABB;

// M2: AABB Primitives

TEST(AABB, Overlap) {
    AABB a(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    AABB b(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
    EXPECT_TRUE(a.Overlaps(b));
}

TEST(AABB, NoOverlap) {
    AABB a(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    AABB b(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(6.0f, 6.0f, 6.0f));
    EXPECT_FALSE(a.Overlaps(b));
}

TEST(AABB, TouchingNotOverlapping) {
    // Boxes share a face at x=1.
    AABB a(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    AABB b(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(2.0f, 1.0f, 1.0f));
    EXPECT_FALSE(a.Overlaps(b));
}

TEST(AABB, ContainsPoint) {
    AABB box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(box.Contains(glm::vec3(0.5f, 0.5f, 0.5f)));
}

TEST(AABB, DoesNotContainPoint) {
    AABB box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    EXPECT_FALSE(box.Contains(glm::vec3(2.0f, 0.5f, 0.5f)));
}

TEST(AABB, RayHit) {
    AABB box(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(4.0f, 2.0f, 2.0f));
    glm::vec3 origin(0.0f, 1.0f, 1.0f);
    glm::vec3 dir(1.0f, 0.0f, 0.0f);
    float t = -1.0f;
    EXPECT_TRUE(box.RayIntersect(origin, dir, t));
    EXPECT_GT(t, 0.0f);
    EXPECT_FLOAT_EQ(t, 2.0f);
}

TEST(AABB, RayMiss) {
    AABB box(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(4.0f, 2.0f, 2.0f));
    glm::vec3 origin(0.0f, 1.0f, 1.0f);
    glm::vec3 dir(-1.0f, 0.0f, 0.0f);  // Pointing away from box
    float t = -1.0f;
    EXPECT_FALSE(box.RayIntersect(origin, dir, t));
}

TEST(AABB, RayFromInside) {
    AABB box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    glm::vec3 origin(1.0f, 1.0f, 1.0f);  // Inside the box
    glm::vec3 dir(1.0f, 0.0f, 0.0f);
    float t = -1.0f;
    EXPECT_TRUE(box.RayIntersect(origin, dir, t));
    EXPECT_FLOAT_EQ(t, 0.0f);
}

TEST(AABB, SweptCollision) {
    // Moving box approaches stationary box from the left.
    AABB moving(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    AABB stationary(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(4.0f, 1.0f, 1.0f));
    glm::vec3 velocity(4.0f, 0.0f, 0.0f);

    auto result = moving.Sweep(velocity, stationary);
    EXPECT_GE(result.t, 0.0f);
    EXPECT_LE(result.t, 1.0f);
    // Moving box's max.x (1) + t * 4 = stationary min.x (3), so t = 0.5
    EXPECT_FLOAT_EQ(result.t, 0.5f);
    // Normal should point in -x (opposing the movement direction).
    EXPECT_FLOAT_EQ(result.normal.x, -1.0f);
    EXPECT_FLOAT_EQ(result.normal.y, 0.0f);
    EXPECT_FLOAT_EQ(result.normal.z, 0.0f);
}

TEST(AABB, SweptNoCollision) {
    // Moving box moves away from stationary box.
    AABB moving(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    AABB stationary(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(4.0f, 1.0f, 1.0f));
    glm::vec3 velocity(-2.0f, 0.0f, 0.0f);  // Moving away

    auto result = moving.Sweep(velocity, stationary);
    EXPECT_FLOAT_EQ(result.t, 1.0f);
}

TEST(AABB, SweptEdgeSlide) {
    // Moving box slides along the Y axis, approaching stationary from below.
    AABB moving(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    AABB stationary(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1.0f, 3.0f, 1.0f));
    glm::vec3 velocity(0.0f, 2.0f, 0.0f);

    auto result = moving.Sweep(velocity, stationary);
    EXPECT_GE(result.t, 0.0f);
    EXPECT_LE(result.t, 1.0f);
    // moving max.y (1) + t * 2 = stationary min.y (2), so t = 0.5
    EXPECT_FLOAT_EQ(result.t, 0.5f);
    // Normal should point in -y (opposing the upward movement).
    EXPECT_FLOAT_EQ(result.normal.x, 0.0f);
    EXPECT_FLOAT_EQ(result.normal.y, -1.0f);
    EXPECT_FLOAT_EQ(result.normal.z, 0.0f);
}
