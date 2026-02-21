#ifndef VIBECRAFT_INPUT_H
#define VIBECRAFT_INPUT_H

#include <array>

struct GLFWwindow;

namespace vibecraft {

/// Input manager for keyboard, mouse, and scroll wheel state.
///
/// Tracks per-key state across frames to provide edge detection
/// (pressed/released) as well as continuous held detection. Mouse movement
/// is accumulated between frames and reported as a delta scaled by
/// configurable sensitivity. Scroll wheel offset is similarly accumulated.
///
/// Usage:
///   1. Feed raw input via SetKeyState(), SetMousePosition(), SetScrollOffset()
///      (called from GLFW callbacks or directly in tests).
///   2. Call Update() once per frame to snapshot state.
///   3. Query IsKeyPressed(), IsKeyHeld(), IsKeyReleased(), GetMouseDelta(),
///      GetScrollDelta() during the frame.
///
/// The class is fully testable without a GLFW window -- tests simply call
/// the Set*() methods and Update() directly.
class Input {
public:
    /// Maximum number of key codes tracked (GLFW keys go up to ~348).
    static constexpr int kMaxKeys = 512;

    /// Default mouse sensitivity multiplier.
    static constexpr float kDefaultSensitivity = 1.0f;

    Input();

    // -----------------------------------------------------------------------
    // State feeding (called from GLFW callbacks or test code)
    // -----------------------------------------------------------------------

    /// Set the current state of a key.
    /// @param key   Key code (GLFW_KEY_* constant or arbitrary int for tests).
    /// @param down  true if the key is currently pressed, false if released.
    void SetKeyState(int key, bool down);

    /// Set the current mouse cursor position (absolute, in screen pixels).
    /// On the very first call the position is recorded without generating a
    /// delta so that there is no initial "jump".
    void SetMousePosition(double x, double y);

    /// Accumulate a scroll wheel offset (typically from GLFW scroll callback).
    void SetScrollOffset(double x_offset, double y_offset);

    // -----------------------------------------------------------------------
    // Frame update
    // -----------------------------------------------------------------------

    /// Advance one frame: snapshot key states and reset per-frame accumulators
    /// (mouse delta, scroll delta). Must be called once per frame before
    /// querying state.
    void Update();

    // -----------------------------------------------------------------------
    // Key queries (valid after Update())
    // -----------------------------------------------------------------------

    /// Returns true on the frame a key transitioned from up to down.
    bool IsKeyPressed(int key) const;

    /// Returns true while a key is held down (including the press frame).
    bool IsKeyHeld(int key) const;

    /// Returns true on the frame a key transitioned from down to up.
    bool IsKeyReleased(int key) const;

    // -----------------------------------------------------------------------
    // Mouse queries (valid after Update())
    // -----------------------------------------------------------------------

    /// Get the mouse movement delta since the last frame, scaled by
    /// sensitivity.
    /// @param dx  Output: horizontal delta (positive = right).
    /// @param dy  Output: vertical delta (positive = down in screen coords).
    void GetMouseDelta(double& dx, double& dy) const;

    /// Get the scroll wheel delta since the last frame.
    /// @param dx  Output: horizontal scroll.
    /// @param dy  Output: vertical scroll (positive = scroll up on most mice).
    void GetScrollDelta(double& dx, double& dy) const;

    // -----------------------------------------------------------------------
    // Configuration
    // -----------------------------------------------------------------------

    /// Set the mouse sensitivity multiplier.
    void SetSensitivity(float sensitivity);

    /// Get the current mouse sensitivity multiplier.
    float GetSensitivity() const;

    // -----------------------------------------------------------------------
    // GLFW callback installation (convenience, requires a live GLFW window)
    // -----------------------------------------------------------------------

    /// Install GLFW key, cursor, and scroll callbacks that feed into this
    /// Input instance. The Input pointer is stored as the GLFW window's user
    /// pointer.
    void InstallCallbacks(GLFWwindow* window);

private:
    /// Returns true if the key code is within the valid range.
    static bool IsValidKey(int key);

    // Per-key state: current frame and previous frame.
    std::array<bool, kMaxKeys> current_keys_;
    std::array<bool, kMaxKeys> previous_keys_;

    // Mouse position tracking.
    double mouse_x_;
    double mouse_y_;
    bool first_mouse_;  ///< True until the first SetMousePosition call.

    // Accumulated mouse delta between Update() calls.
    double delta_x_;
    double delta_y_;

    // Mouse delta snapshot (set by Update(), read by GetMouseDelta()).
    double frame_delta_x_;
    double frame_delta_y_;

    // Accumulated scroll offset between Update() calls.
    double scroll_x_;
    double scroll_y_;

    // Scroll snapshot (set by Update(), read by GetScrollDelta()).
    double frame_scroll_x_;
    double frame_scroll_y_;

    // Configuration.
    float sensitivity_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_INPUT_H
