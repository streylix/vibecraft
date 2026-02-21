#include "vibecraft/input.h"

#include <GLFW/glfw3.h>

#include <algorithm>

namespace vibecraft {

Input::Input()
    : mouse_x_(0.0),
      mouse_y_(0.0),
      first_mouse_(true),
      delta_x_(0.0),
      delta_y_(0.0),
      frame_delta_x_(0.0),
      frame_delta_y_(0.0),
      scroll_x_(0.0),
      scroll_y_(0.0),
      frame_scroll_x_(0.0),
      frame_scroll_y_(0.0),
      sensitivity_(kDefaultSensitivity) {
    current_keys_.fill(false);
    previous_keys_.fill(false);
}

// ---------------------------------------------------------------------------
// State feeding
// ---------------------------------------------------------------------------

void Input::SetKeyState(int key, bool down) {
    if (!IsValidKey(key)) return;
    current_keys_[static_cast<size_t>(key)] = down;
}

void Input::SetMousePosition(double x, double y) {
    if (first_mouse_) {
        mouse_x_ = x;
        mouse_y_ = y;
        first_mouse_ = false;
        return;
    }
    delta_x_ += (x - mouse_x_);
    delta_y_ += (y - mouse_y_);
    mouse_x_ = x;
    mouse_y_ = y;
}

void Input::SetScrollOffset(double x_offset, double y_offset) {
    scroll_x_ += x_offset;
    scroll_y_ += y_offset;
}

// ---------------------------------------------------------------------------
// Frame update
// ---------------------------------------------------------------------------

void Input::Update() {
    // Snapshot mouse delta (scaled by sensitivity) and reset accumulator.
    frame_delta_x_ = delta_x_ * static_cast<double>(sensitivity_);
    frame_delta_y_ = delta_y_ * static_cast<double>(sensitivity_);
    delta_x_ = 0.0;
    delta_y_ = 0.0;

    // Snapshot scroll and reset accumulator.
    frame_scroll_x_ = scroll_x_;
    frame_scroll_y_ = scroll_y_;
    scroll_x_ = 0.0;
    scroll_y_ = 0.0;

    // Advance key state: previous = what was current before this frame's
    // SetKeyState calls. We copy current into previous *after* the frame so
    // that queries during the frame can compare the two.
    // NOTE: previous_keys_ is set from the *last* frame's current, and
    // current_keys_ already reflects the new state fed since last Update().
    // The copy happens here so that IsKeyPressed/Released can compare.
    // Actually, the pattern is:
    //   - Between Update() calls, GLFW callbacks (or tests) modify current_.
    //   - Update() copies current_ into previous_ for *next* frame comparison.
    //   Wait -- that's wrong. We need previous_ to represent the state
    //   *before* the current frame so we can detect edges.
    //
    // Correct approach:
    //   previous_ = snapshot of keys at end of last frame
    //   current_  = modified by callbacks since last Update()
    //   At Update() time, queries for this frame compare current_ vs previous_.
    //   Then at the END of Update(), previous_ = current_ for next frame.
    //
    // But we are calling Update() at the *start* of the frame. So the flow is:
    //   1. Callbacks modify current_ between frames.
    //   2. Update() is called -- freeze state. Copy previous_ = current_.
    //      But wait, we need the *old* previous_ for comparisons this frame.
    //
    // Simplest correct pattern:
    //   Update() should be called AFTER polling events but BEFORE game logic.
    //   The previous keys from last frame are already in previous_.
    //   During this frame's game logic, queries compare current_ vs previous_.
    //   At the end of Update(), we copy current_ into previous_ so that
    //   *next* frame's Update() will have the correct previous_.

    // Copy current state into previous for next frame.
    previous_keys_ = current_keys_;
}

// ---------------------------------------------------------------------------
// Key queries
// ---------------------------------------------------------------------------

bool Input::IsKeyPressed(int key) const {
    if (!IsValidKey(key)) return false;
    auto k = static_cast<size_t>(key);
    return current_keys_[k] && !previous_keys_[k];
}

bool Input::IsKeyHeld(int key) const {
    if (!IsValidKey(key)) return false;
    return current_keys_[static_cast<size_t>(key)];
}

bool Input::IsKeyReleased(int key) const {
    if (!IsValidKey(key)) return false;
    auto k = static_cast<size_t>(key);
    return !current_keys_[k] && previous_keys_[k];
}

// ---------------------------------------------------------------------------
// Mouse queries
// ---------------------------------------------------------------------------

void Input::GetMouseDelta(double& dx, double& dy) const {
    dx = frame_delta_x_;
    dy = frame_delta_y_;
}

void Input::GetScrollDelta(double& dx, double& dy) const {
    dx = frame_scroll_x_;
    dy = frame_scroll_y_;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

void Input::SetSensitivity(float sensitivity) {
    sensitivity_ = sensitivity;
}

float Input::GetSensitivity() const {
    return sensitivity_;
}

// ---------------------------------------------------------------------------
// GLFW callback installation
// ---------------------------------------------------------------------------

void Input::InstallCallbacks(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(window,
        [](GLFWwindow* w, int key, int /*scancode*/, int action, int /*mods*/) {
            auto* input = static_cast<Input*>(glfwGetWindowUserPointer(w));
            if (!input) return;
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                input->SetKeyState(key, true);
            } else if (action == GLFW_RELEASE) {
                input->SetKeyState(key, false);
            }
        });

    glfwSetCursorPosCallback(window,
        [](GLFWwindow* w, double x, double y) {
            auto* input = static_cast<Input*>(glfwGetWindowUserPointer(w));
            if (!input) return;
            input->SetMousePosition(x, y);
        });

    glfwSetScrollCallback(window,
        [](GLFWwindow* w, double x_offset, double y_offset) {
            auto* input = static_cast<Input*>(glfwGetWindowUserPointer(w));
            if (!input) return;
            input->SetScrollOffset(x_offset, y_offset);
        });
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

bool Input::IsValidKey(int key) {
    return key >= 0 && key < kMaxKeys;
}

}  // namespace vibecraft
