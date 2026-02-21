#include <gtest/gtest.h>

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vibecraft/camera.h"

// M9: Camera

using vibecraft::Camera;

/// Helper: check if two vec3 are approximately equal.
static void ExpectVec3Near(const glm::vec3& a, const glm::vec3& b,
                           float epsilon = 1e-4f) {
    EXPECT_NEAR(a.x, b.x, epsilon) << "x mismatch";
    EXPECT_NEAR(a.y, b.y, epsilon) << "y mismatch";
    EXPECT_NEAR(a.z, b.z, epsilon) << "z mismatch";
}

TEST(Camera, DefaultPosition) {
    Camera cam;
    glm::vec3 pos = cam.GetPosition();
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
    EXPECT_FLOAT_EQ(pos.z, 0.0f);
}

TEST(Camera, ViewMatrixIdentityLike) {
    // Default camera: position (0,0,0), yaw=0, pitch=0.
    // Forward should be (0, 0, -1).
    Camera cam;
    glm::vec3 forward = cam.GetForward();
    ExpectVec3Near(forward, glm::vec3(0.0f, 0.0f, -1.0f));

    // The view matrix should be similar to lookAt(origin, (0,0,-1), (0,1,0)).
    glm::mat4 view = cam.GetViewMatrix();
    glm::mat4 expected = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            EXPECT_NEAR(view[col][row], expected[col][row], 1e-4f)
                << "view[" << col << "][" << row << "]";
        }
    }
}

TEST(Camera, YawRotation) {
    Camera cam;
    // Yaw 90 degrees: forward should be approximately (1, 0, 0).
    cam.SetYaw(90.0f);
    glm::vec3 forward = cam.GetForward();
    ExpectVec3Near(forward, glm::vec3(1.0f, 0.0f, 0.0f));

    // Yaw 180 degrees: forward should be approximately (0, 0, 1).
    cam.SetYaw(180.0f);
    forward = cam.GetForward();
    ExpectVec3Near(forward, glm::vec3(0.0f, 0.0f, 1.0f));

    // Yaw -90 degrees: forward should be approximately (-1, 0, 0).
    cam.SetYaw(-90.0f);
    forward = cam.GetForward();
    ExpectVec3Near(forward, glm::vec3(-1.0f, 0.0f, 0.0f));
}

TEST(Camera, PitchRotation) {
    Camera cam;
    // Pitch 89 degrees (nearly straight up): forward should be approximately (0, 1, 0).
    cam.SetPitch(89.0f);
    glm::vec3 forward = cam.GetForward();
    // At pitch=89, forward.y should be very close to 1, forward.xz near 0.
    EXPECT_GT(forward.y, 0.99f);
    EXPECT_NEAR(forward.x, 0.0f, 0.02f);
    EXPECT_NEAR(forward.z, 0.0f, 0.02f);

    // Pitch -89 degrees (nearly straight down): forward.y should be near -1.
    cam.SetPitch(-89.0f);
    forward = cam.GetForward();
    EXPECT_LT(forward.y, -0.99f);
}

TEST(Camera, PitchClamp) {
    Camera cam;

    // Setting pitch beyond 89 should clamp to 89.
    cam.SetPitch(100.0f);
    EXPECT_FLOAT_EQ(cam.GetPitch(), 89.0f);

    // Setting pitch below -89 should clamp to -89.
    cam.SetPitch(-100.0f);
    EXPECT_FLOAT_EQ(cam.GetPitch(), -89.0f);

    // Normal values should pass through unchanged.
    cam.SetPitch(45.0f);
    EXPECT_FLOAT_EQ(cam.GetPitch(), 45.0f);

    cam.SetPitch(-30.0f);
    EXPECT_FLOAT_EQ(cam.GetPitch(), -30.0f);

    cam.SetPitch(0.0f);
    EXPECT_FLOAT_EQ(cam.GetPitch(), 0.0f);
}

TEST(Camera, ProjectionMatrix) {
    Camera cam;
    glm::mat4 proj = cam.GetProjectionMatrix();

    // The projection matrix should not be all zeros.
    bool all_zero = true;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            if (proj[col][row] != 0.0f) {
                all_zero = false;
                break;
            }
        }
    }
    EXPECT_FALSE(all_zero) << "Projection matrix is all zeros";

    // Compare with a manually-constructed perspective matrix.
    float fov = glm::radians(Camera::kDefaultFov);
    float aspect = Camera::kDefaultAspect;
    glm::mat4 expected = glm::perspective(fov, aspect,
                                          Camera::kDefaultNear,
                                          Camera::kDefaultFar);
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            EXPECT_NEAR(proj[col][row], expected[col][row], 1e-5f)
                << "proj[" << col << "][" << row << "]";
        }
    }
}

TEST(Camera, ViewProjectionProduct) {
    Camera cam(glm::vec3(5.0f, 10.0f, -3.0f), 30.0f, -15.0f);
    glm::mat4 vp = cam.GetViewProjectionMatrix();

    // The VP matrix should equal P * V.
    glm::mat4 expected = cam.GetProjectionMatrix() * cam.GetViewMatrix();
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            EXPECT_NEAR(vp[col][row], expected[col][row], 1e-4f)
                << "vp[" << col << "][" << row << "]";
        }
    }

    // The VP matrix determinant should be non-zero (it's invertible).
    float det = glm::determinant(vp);
    EXPECT_NE(det, 0.0f) << "VP matrix determinant is zero";
}

TEST(Camera, MoveForward) {
    Camera cam;
    // Default forward is (0, 0, -1). Moving forward by 5 units.
    glm::vec3 initial_pos = cam.GetPosition();
    cam.MoveForward(5.0f);
    glm::vec3 new_pos = cam.GetPosition();

    // Position should have changed.
    EXPECT_NE(new_pos, initial_pos);

    // Should have moved in the forward direction.
    glm::vec3 displacement = new_pos - initial_pos;
    ExpectVec3Near(displacement, glm::vec3(0.0f, 0.0f, -5.0f));

    // Test with a rotated camera.
    Camera cam2(glm::vec3(0.0f), 90.0f, 0.0f);
    // Forward is (1, 0, 0) when yaw=90.
    cam2.MoveForward(3.0f);
    ExpectVec3Near(cam2.GetPosition(), glm::vec3(3.0f, 0.0f, 0.0f));
}
