#ifndef VIBECRAFT_RENDERER_H
#define VIBECRAFT_RENDERER_H

#include <glm/glm.hpp>

namespace vibecraft {

class Window;
class Shader;
class Camera;

/// Basic OpenGL renderer.
///
/// Manages GL state such as depth testing, face culling, clear color, and
/// wireframe mode. Provides helpers for beginning/ending a frame.
class Renderer {
public:
    /// Construct a renderer. Requires an active OpenGL context.
    Renderer();

    ~Renderer();

    // Non-copyable.
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /// Initialize OpenGL state (depth test, face culling, clear color, etc.).
    /// Call once after the GL context is ready.
    void Init();

    /// Set the viewport dimensions.
    void SetViewport(int x, int y, int width, int height);

    /// Set the clear color (RGBA, each in [0, 1]).
    void SetClearColor(float r, float g, float b, float a = 1.0f);

    /// Set the clear color from a vec4.
    void SetClearColor(const glm::vec4& color);

    /// Clear the color and depth buffers.
    void Clear();

    /// Begin a new frame: clear buffers.
    void BeginFrame();

    /// End the frame (currently a no-op; swap is done by the Window).
    void EndFrame();

    /// Enable or disable wireframe rendering.
    void SetWireframe(bool enabled);

    /// Returns true if wireframe mode is active.
    bool IsWireframe() const;

    /// Enable or disable depth testing.
    void SetDepthTest(bool enabled);

    /// Enable or disable back-face culling.
    void SetFaceCulling(bool enabled);

private:
    glm::vec4 clear_color_;
    bool wireframe_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_RENDERER_H
