#include "vibecraft/camera.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace vibecraft {

Camera::Camera(const glm::vec3& position, float yaw, float pitch)
    : position_(position),
      yaw_(yaw),
      pitch_(0.0f),
      forward_(0.0f, 0.0f, -1.0f),
      right_(1.0f, 0.0f, 0.0f),
      up_(0.0f, 1.0f, 0.0f),
      fov_(kDefaultFov),
      aspect_(kDefaultAspect),
      near_(kDefaultNear),
      far_(kDefaultFar) {
    // Use SetPitch to apply clamping, then update vectors.
    SetPitch(pitch);
    UpdateVectors();
}

glm::vec3 Camera::GetPosition() const {
    return position_;
}

void Camera::SetPosition(const glm::vec3& position) {
    position_ = position;
}

float Camera::GetYaw() const {
    return yaw_;
}

void Camera::SetYaw(float yaw) {
    yaw_ = yaw;
    UpdateVectors();
}

float Camera::GetPitch() const {
    return pitch_;
}

void Camera::SetPitch(float pitch) {
    pitch_ = std::clamp(pitch, -kMaxPitch, kMaxPitch);
    UpdateVectors();
}

glm::vec3 Camera::GetForward() const {
    return forward_;
}

glm::vec3 Camera::GetRight() const {
    return right_;
}

glm::vec3 Camera::GetUp() const {
    return up_;
}

void Camera::MoveForward(float distance) {
    position_ += forward_ * distance;
}

void Camera::MoveRight(float distance) {
    position_ += right_ * distance;
}

void Camera::MoveUp(float distance) {
    position_ += kWorldUp * distance;
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position_, position_ + forward_, kWorldUp);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(fov_), aspect_, near_, far_);
}

glm::mat4 Camera::GetViewProjectionMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

void Camera::SetFov(float fov) {
    fov_ = fov;
}

float Camera::GetFov() const {
    return fov_;
}

void Camera::SetAspectRatio(float aspect) {
    aspect_ = aspect;
}

float Camera::GetAspectRatio() const {
    return aspect_;
}

void Camera::SetNearPlane(float near_plane) {
    near_ = near_plane;
}

void Camera::SetFarPlane(float far_plane) {
    far_ = far_plane;
}

void Camera::UpdateVectors() {
    // Convert yaw/pitch from degrees to radians.
    float yaw_rad = glm::radians(yaw_);
    float pitch_rad = glm::radians(pitch_);

    // Compute forward vector.
    // At yaw=0, pitch=0: forward = (0, 0, -1) (looking down -Z).
    // Yaw rotates around Y axis counter-clockwise.
    // sin(0) = 0, cos(0) = 1 => forward.z = -cos(0)*cos(0) = -1. Correct.
    // At yaw=90: sin(90)=1, cos(90)=0 => forward.x = sin(90)*cos(0) = 1,
    //   forward.z = -cos(90)*cos(0) = 0. So forward = (1, 0, 0). Correct.
    forward_.x = std::sin(yaw_rad) * std::cos(pitch_rad);
    forward_.y = std::sin(pitch_rad);
    forward_.z = -std::cos(yaw_rad) * std::cos(pitch_rad);
    forward_ = glm::normalize(forward_);

    // Right = forward x world_up (then normalize).
    right_ = glm::normalize(glm::cross(forward_, kWorldUp));

    // Camera up = right x forward.
    up_ = glm::normalize(glm::cross(right_, forward_));
}

}  // namespace vibecraft
