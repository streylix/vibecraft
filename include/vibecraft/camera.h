#ifndef VIBECRAFT_CAMERA_H
#define VIBECRAFT_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vibecraft {

/// FPS camera using Euler angles (yaw/pitch).
///
/// Convention:
///   - Yaw: 0 degrees = looking down -Z, increases counter-clockwise (left).
///     Yaw 90 degrees = looking down +X.
///   - Pitch: 0 degrees = horizontal, +90 = up, -90 = down.
///   - Pitch is clamped to [-89, 89] degrees to avoid gimbal lock.
///
/// The camera produces a view matrix via glm::lookAt and a perspective
/// projection matrix via glm::perspective.
class Camera {
public:
    /// Default FOV, near, and far planes.
    static constexpr float kDefaultFov = 70.0f;       // degrees (vertical)
    static constexpr float kDefaultNear = 0.1f;
    static constexpr float kDefaultFar = 1000.0f;
    static constexpr float kDefaultAspect = 16.0f / 9.0f;
    static constexpr float kMaxPitch = 89.0f;

    /// Construct a camera at the given position with optional yaw/pitch.
    /// Yaw and pitch are in degrees.
    explicit Camera(const glm::vec3& position = glm::vec3(0.0f),
                    float yaw = 0.0f,
                    float pitch = 0.0f);

    /// Get the camera position in world space.
    glm::vec3 GetPosition() const;

    /// Set the camera position in world space.
    void SetPosition(const glm::vec3& position);

    /// Get the current yaw in degrees.
    float GetYaw() const;

    /// Set the yaw in degrees.
    void SetYaw(float yaw);

    /// Get the current pitch in degrees.
    float GetPitch() const;

    /// Set the pitch in degrees. Automatically clamped to [-89, 89].
    void SetPitch(float pitch);

    /// Get the forward direction vector (unit length).
    glm::vec3 GetForward() const;

    /// Get the right direction vector (unit length).
    glm::vec3 GetRight() const;

    /// Get the up direction vector (unit length, camera-local).
    glm::vec3 GetUp() const;

    /// Move the camera along its forward vector by the given distance.
    /// Positive distance = move forward, negative = move backward.
    void MoveForward(float distance);

    /// Move the camera along its right vector by the given distance.
    /// Positive distance = move right, negative = move left.
    void MoveRight(float distance);

    /// Move the camera along the world up axis by the given distance.
    void MoveUp(float distance);

    /// Compute the view matrix (world-to-camera transform).
    glm::mat4 GetViewMatrix() const;

    /// Compute the perspective projection matrix.
    glm::mat4 GetProjectionMatrix() const;

    /// Compute the combined view-projection matrix.
    glm::mat4 GetViewProjectionMatrix() const;

    /// Set the vertical field of view in degrees.
    void SetFov(float fov);

    /// Get the vertical field of view in degrees.
    float GetFov() const;

    /// Set the aspect ratio (width / height).
    void SetAspectRatio(float aspect);

    /// Get the aspect ratio.
    float GetAspectRatio() const;

    /// Set the near clipping plane distance.
    void SetNearPlane(float near_plane);

    /// Set the far clipping plane distance.
    void SetFarPlane(float far_plane);

private:
    /// Recalculate the forward, right, and up vectors from yaw and pitch.
    void UpdateVectors();

    glm::vec3 position_;
    float yaw_;        // degrees
    float pitch_;      // degrees

    glm::vec3 forward_;
    glm::vec3 right_;
    glm::vec3 up_;

    float fov_;        // degrees (vertical)
    float aspect_;
    float near_;
    float far_;

    /// World up vector (constant).
    static constexpr glm::vec3 kWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
};

}  // namespace vibecraft

#endif  // VIBECRAFT_CAMERA_H
