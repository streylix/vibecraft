#include <gtest/gtest.h>

#include "vibecraft/input.h"

// M12: Input Handling & Camera Control
//
// All tests drive the Input class directly via SetKeyState / SetMousePosition /
// SetScrollOffset + Update(), with no GLFW window required.

// Use arbitrary key codes for testing (within valid range).
static constexpr int kKeyW = 87;   // ASCII 'W', same as GLFW_KEY_W
static constexpr int kKeyA = 65;   // ASCII 'A', same as GLFW_KEY_A
static constexpr int kKeyS = 83;
static constexpr int kKeyD = 68;

TEST(Input, KeyPressDetection) {
    vibecraft::Input input;

    // Frame 0: no keys pressed yet.
    input.Update();

    // Simulate pressing W between frames.
    input.SetKeyState(kKeyW, true);

    // W should now register as "just pressed".
    EXPECT_TRUE(input.IsKeyPressed(kKeyW));
}

TEST(Input, KeyReleaseDetection) {
    vibecraft::Input input;

    // Frame 0: establish W as held.
    input.Update();
    input.SetKeyState(kKeyW, true);

    // Frame 1: W is held (previous=false, current=true -> pressed this frame).
    input.Update();
    // Now previous=true, current=true. Release the key.
    input.SetKeyState(kKeyW, false);

    // W should register as "just released".
    EXPECT_TRUE(input.IsKeyReleased(kKeyW));
}

TEST(Input, KeyHeld) {
    vibecraft::Input input;

    // Frame 0: press W.
    input.Update();
    input.SetKeyState(kKeyW, true);

    // IsKeyHeld checks current state directly.
    EXPECT_TRUE(input.IsKeyHeld(kKeyW));

    // Frame 1: W is still down (no release).
    input.Update();
    // After Update, previous=true, current=true (key still down).
    // SetKeyState not called again -- current remains true.
    EXPECT_TRUE(input.IsKeyHeld(kKeyW));

    // Frame 2: still held.
    input.Update();
    EXPECT_TRUE(input.IsKeyHeld(kKeyW));
}

TEST(Input, EdgeDetection) {
    vibecraft::Input input;

    // Frame 0: press W.
    input.Update();
    input.SetKeyState(kKeyW, true);

    // First query: pressed should be true.
    EXPECT_TRUE(input.IsKeyPressed(kKeyW));

    // Advance to next frame without changing key state.
    input.Update();

    // W is still held, but IsKeyPressed should be false (no new edge).
    EXPECT_FALSE(input.IsKeyPressed(kKeyW));

    // Another frame: still no new press.
    input.Update();
    EXPECT_FALSE(input.IsKeyPressed(kKeyW));
}

TEST(Input, MouseDelta) {
    vibecraft::Input input;

    // First call initializes position, no delta generated.
    input.SetMousePosition(0.0, 0.0);

    // Move mouse.
    input.SetMousePosition(100.0, 50.0);

    // Update to snapshot the delta.
    input.Update();

    double dx = 0.0, dy = 0.0;
    input.GetMouseDelta(dx, dy);

    EXPECT_DOUBLE_EQ(dx, 100.0);
    EXPECT_DOUBLE_EQ(dy, 50.0);
}

TEST(Input, MouseDeltaReset) {
    vibecraft::Input input;

    // Initialize and move.
    input.SetMousePosition(0.0, 0.0);
    input.SetMousePosition(100.0, 50.0);
    input.Update();

    double dx = 0.0, dy = 0.0;
    input.GetMouseDelta(dx, dy);
    EXPECT_DOUBLE_EQ(dx, 100.0);
    EXPECT_DOUBLE_EQ(dy, 50.0);

    // Next frame with no new mouse movement.
    input.Update();
    input.GetMouseDelta(dx, dy);

    EXPECT_DOUBLE_EQ(dx, 0.0);
    EXPECT_DOUBLE_EQ(dy, 0.0);
}

TEST(Input, ScrollInput) {
    vibecraft::Input input;

    // Simulate scroll events between frames.
    input.SetScrollOffset(0.0, 3.0);

    input.Update();

    double sx = 0.0, sy = 0.0;
    input.GetScrollDelta(sx, sy);

    EXPECT_DOUBLE_EQ(sy, 3.0);
    // Scroll value is non-zero as required by the spec.
    EXPECT_NE(sy, 0.0);
}

TEST(Input, SimultaneousKeys) {
    vibecraft::Input input;

    input.Update();

    // Press both W and A.
    input.SetKeyState(kKeyW, true);
    input.SetKeyState(kKeyA, true);

    // Both should report as held simultaneously.
    EXPECT_TRUE(input.IsKeyHeld(kKeyW));
    EXPECT_TRUE(input.IsKeyHeld(kKeyA));

    // And both as pressed (first frame).
    EXPECT_TRUE(input.IsKeyPressed(kKeyW));
    EXPECT_TRUE(input.IsKeyPressed(kKeyA));
}

TEST(Input, Sensitivity) {
    vibecraft::Input input;
    input.SetSensitivity(2.0f);

    input.SetMousePosition(0.0, 0.0);
    input.SetMousePosition(100.0, 50.0);
    input.Update();

    double dx = 0.0, dy = 0.0;
    input.GetMouseDelta(dx, dy);

    // Sensitivity of 2.0 should double the delta.
    EXPECT_DOUBLE_EQ(dx, 200.0);
    EXPECT_DOUBLE_EQ(dy, 100.0);
}

TEST(Input, DefaultSensitivity) {
    vibecraft::Input input;

    EXPECT_FLOAT_EQ(input.GetSensitivity(), 1.0f);
}
