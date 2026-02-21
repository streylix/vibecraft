#include <gtest/gtest.h>

#include <cmath>

#include "vibecraft/game_loop.h"

// M24: Game Loop & Polish

TEST(GameLoop, AOCornerValues) {
    // 0 neighbors = brightest (1.0)
    EXPECT_FLOAT_EQ(vibecraft::ao::ComputeAO(0), 1.0f);

    // 1 neighbor
    EXPECT_FLOAT_EQ(vibecraft::ao::ComputeAO(1), 0.7f);

    // 2 neighbors
    EXPECT_FLOAT_EQ(vibecraft::ao::ComputeAO(2), 0.4f);

    // 3 neighbors = darkest (0.2)
    EXPECT_FLOAT_EQ(vibecraft::ao::ComputeAO(3), 0.2f);
}

TEST(GameLoop, FogLinearInterp) {
    float start = 10.0f;
    float end = 30.0f;

    // At start distance: fully visible (1.0).
    EXPECT_FLOAT_EQ(vibecraft::fog::ComputeLinearFog(10.0f, start, end), 1.0f);

    // At end distance: fully fogged (0.0).
    EXPECT_FLOAT_EQ(vibecraft::fog::ComputeLinearFog(30.0f, start, end), 0.0f);

    // At 50% between start and end: factor = 0.5.
    EXPECT_FLOAT_EQ(vibecraft::fog::ComputeLinearFog(20.0f, start, end), 0.5f);

    // Before start: clamped to 1.0.
    EXPECT_FLOAT_EQ(vibecraft::fog::ComputeLinearFog(5.0f, start, end), 1.0f);

    // Beyond end: clamped to 0.0.
    EXPECT_FLOAT_EQ(vibecraft::fog::ComputeLinearFog(50.0f, start, end), 0.0f);
}

TEST(GameLoop, FixedTimestepAccumulator) {
    vibecraft::GameLoop loop(0.05f);  // 50ms per tick

    // Feed 75ms: should process 1 tick with 25ms remaining.
    int ticks = loop.Accumulate(0.075f);
    EXPECT_EQ(ticks, 1);
    EXPECT_NEAR(loop.GetAccumulator(), 0.025f, 1e-6f);

    // Interpolation factor: 25ms / 50ms = 0.5
    EXPECT_NEAR(loop.GetInterpolation(), 0.5f, 1e-6f);
}

TEST(GameLoop, FixedTimestepMultiple) {
    vibecraft::GameLoop loop(0.05f);  // 50ms per tick

    // Feed 150ms: should process 3 ticks with 0ms remaining.
    int ticks = loop.Accumulate(0.15f);
    EXPECT_EQ(ticks, 3);
    EXPECT_NEAR(loop.GetAccumulator(), 0.0f, 1e-6f);
    EXPECT_EQ(loop.GetTotalTicks(), 3u);
}
