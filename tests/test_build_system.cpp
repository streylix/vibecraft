#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

TEST(BuildSystem, CppStandardIs17) {
    EXPECT_GE(__cplusplus, 201703L);
}

TEST(BuildSystem, GlmAvailable) {
    glm::vec3 v(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

TEST(BuildSystem, GlfwLinked) {
    EXPECT_GE(GLFW_VERSION_MAJOR, 3);
}

TEST(BuildSystem, GtestWorks) {
    EXPECT_TRUE(true);
}
