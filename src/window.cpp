#include "vibecraft/window.h"

#include <stdexcept>

#include <GLFW/glfw3.h>

namespace vibecraft {

Window::Window(int width, int height, const std::string& title)
    : window_(nullptr), width_(width), height_(height) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Request OpenGL 3.3 Core Profile.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);

    // Store pointer to this Window so callbacks can access it.
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);

    // Get actual framebuffer size (may differ from window size on Retina).
    glfwGetFramebufferSize(window_, &width_, &height_);

    // Enable VSync by default.
    glfwSwapInterval(1);
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

bool Window::IsOpen() const {
    return !glfwWindowShouldClose(window_);
}

void Window::PollEvents() const {
    glfwPollEvents();
}

void Window::SwapBuffers() const {
    glfwSwapBuffers(window_);
}

int Window::GetWidth() const {
    return width_;
}

int Window::GetHeight() const {
    return height_;
}

float Window::GetAspectRatio() const {
    if (height_ == 0) return 1.0f;
    return static_cast<float>(width_) / static_cast<float>(height_);
}

GLFWwindow* Window::GetHandle() const {
    return window_;
}

void Window::SetCursorEnabled(bool enabled) {
    glfwSetInputMode(window_, GLFW_CURSOR,
                     enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Window::SetVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width,
                                     int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->width_ = width;
        self->height_ = height;
    }
}

}  // namespace vibecraft
