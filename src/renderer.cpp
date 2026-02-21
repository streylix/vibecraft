#include "vibecraft/renderer.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace vibecraft {

Renderer::Renderer()
    : clear_color_(0.529f, 0.808f, 0.922f, 1.0f),  // Light sky blue
      wireframe_(false) {
}

Renderer::~Renderer() = default;

void Renderer::Init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(clear_color_.r, clear_color_.g, clear_color_.b,
                 clear_color_.a);
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    clear_color_ = glm::vec4(r, g, b, a);
    glClearColor(r, g, b, a);
}

void Renderer::SetClearColor(const glm::vec4& color) {
    clear_color_ = color;
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::BeginFrame() {
    Clear();
}

void Renderer::EndFrame() {
    // Swap is handled by the Window.
}

void Renderer::SetWireframe(bool enabled) {
    wireframe_ = enabled;
    if (enabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

bool Renderer::IsWireframe() const {
    return wireframe_;
}

void Renderer::SetDepthTest(bool enabled) {
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void Renderer::SetFaceCulling(bool enabled) {
    if (enabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

}  // namespace vibecraft
