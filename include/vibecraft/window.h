#ifndef VIBECRAFT_WINDOW_H
#define VIBECRAFT_WINDOW_H

#include <string>

struct GLFWwindow;

namespace vibecraft {

/// GLFW window wrapper with OpenGL 3.3 Core Profile context.
///
/// Manages the lifecycle of a GLFW window: initialization, event polling,
/// buffer swapping, and destruction. Only one Window should exist at a time.
class Window {
public:
    /// Default window dimensions.
    static constexpr int kDefaultWidth = 1280;
    static constexpr int kDefaultHeight = 720;

    /// Create and open a GLFW window with the specified dimensions and title.
    /// Initializes GLFW and creates an OpenGL 3.3 Core Profile context.
    /// Throws std::runtime_error if initialization fails.
    Window(int width = kDefaultWidth, int height = kDefaultHeight,
           const std::string& title = "VibeCraft");

    /// Destroy the window and terminate GLFW.
    ~Window();

    // Non-copyable, non-movable.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    /// Returns true if the window should remain open (close not requested).
    bool IsOpen() const;

    /// Poll for events (keyboard, mouse, resize, etc.).
    void PollEvents() const;

    /// Swap the front and back buffers (present the rendered frame).
    void SwapBuffers() const;

    /// Get the current framebuffer width in pixels.
    int GetWidth() const;

    /// Get the current framebuffer height in pixels.
    int GetHeight() const;

    /// Get the aspect ratio (width / height). Returns 1.0 if height is 0.
    float GetAspectRatio() const;

    /// Get the raw GLFW window pointer (for input handling, etc.).
    GLFWwindow* GetHandle() const;

    /// Enable or disable the mouse cursor.
    void SetCursorEnabled(bool enabled);

    /// Enable or disable VSync.
    void SetVSync(bool enabled);

private:
    /// GLFW framebuffer resize callback.
    static void FramebufferSizeCallback(GLFWwindow* window, int width,
                                        int height);

    GLFWwindow* window_;
    int width_;
    int height_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_WINDOW_H
