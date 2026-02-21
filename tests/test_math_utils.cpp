#include <gtest/gtest.h>

#include "vibecraft/math_utils.h"

// M2: Math Utilities

TEST(MathUtils, LerpZero) {
    EXPECT_FLOAT_EQ(vibecraft::Lerp(0.0f, 10.0f, 0.0f), 0.0f);
}

TEST(MathUtils, LerpHalf) {
    EXPECT_FLOAT_EQ(vibecraft::Lerp(0.0f, 10.0f, 0.5f), 5.0f);
}

TEST(MathUtils, LerpOne) {
    EXPECT_FLOAT_EQ(vibecraft::Lerp(0.0f, 10.0f, 1.0f), 10.0f);
}

TEST(MathUtils, ClampBelow) {
    EXPECT_EQ(vibecraft::Clamp(-5, 0, 10), 0);
}

TEST(MathUtils, ClampAbove) {
    EXPECT_EQ(vibecraft::Clamp(15, 0, 10), 10);
}

TEST(MathUtils, ClampInRange) {
    EXPECT_EQ(vibecraft::Clamp(5, 0, 10), 5);
}

TEST(MathUtils, IntFloorPositive) {
    EXPECT_EQ(vibecraft::IntFloor(3.7f), 3);
}

TEST(MathUtils, IntFloorNegative) {
    EXPECT_EQ(vibecraft::IntFloor(-0.1f), -1);
}

TEST(MathUtils, IntFloorExact) {
    EXPECT_EQ(vibecraft::IntFloor(4.0f), 4);
}

TEST(MathUtils, ModPositive) {
    EXPECT_EQ(vibecraft::Mod(17, 16), 1);
}

TEST(MathUtils, ModNegative) {
    EXPECT_EQ(vibecraft::Mod(-1, 16), 15);
}

TEST(MathUtils, ModZero) {
    EXPECT_EQ(vibecraft::Mod(0, 16), 0);
}
