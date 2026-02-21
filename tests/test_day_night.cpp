#include <gtest/gtest.h>

#include <cmath>

#include "vibecraft/sky.h"

// M19: Day/Night Cycle & Sky

TEST(DayNight, TimeAdvances) {
    vibecraft::Sky sky;
    EXPECT_EQ(sky.GetTime(), 0);

    sky.Tick();
    EXPECT_EQ(sky.GetTime(), 1);

    sky.Tick(99);
    EXPECT_EQ(sky.GetTime(), 100);

    sky.Tick(400);
    EXPECT_EQ(sky.GetTime(), 500);
}

TEST(DayNight, TimeWraps) {
    vibecraft::Sky sky;

    // Set time to just before the wrap point.
    sky.SetTime(23999);
    EXPECT_EQ(sky.GetTime(), 23999);

    sky.Tick();
    EXPECT_EQ(sky.GetTime(), 0);

    sky.Tick();
    EXPECT_EQ(sky.GetTime(), 1);

    // Also verify SetTime wraps.
    sky.SetTime(24001);
    EXPECT_EQ(sky.GetTime(), 1);

    sky.SetTime(48000);
    EXPECT_EQ(sky.GetTime(), 0);

    // Tick past the boundary in one call.
    sky.SetTime(23990);
    sky.Tick(20);
    EXPECT_EQ(sky.GetTime(), 10);
}

TEST(DayNight, SunAngleNoon) {
    vibecraft::Sky sky;
    sky.SetTime(6000);  // Noon

    float angle = sky.GetSunAngle();
    // At noon (6000 ticks), sun angle should be ~90 degrees (directly overhead).
    EXPECT_NEAR(angle, 90.0f, 0.1f);
}

TEST(DayNight, SunAngleMidnight) {
    vibecraft::Sky sky;
    sky.SetTime(18000);  // Midnight

    float angle = sky.GetSunAngle();
    // At midnight (18000 ticks), sun angle should be ~270 degrees (directly below).
    EXPECT_NEAR(angle, 270.0f, 0.1f);
}

TEST(DayNight, AmbientLightDay) {
    vibecraft::Sky sky;
    sky.SetTime(6000);  // Noon

    int ambient = sky.GetAmbientLight();
    // At noon, ambient light should be maximum (15).
    EXPECT_EQ(ambient, 15);
}

TEST(DayNight, AmbientLightNight) {
    vibecraft::Sky sky;
    sky.SetTime(18000);  // Midnight

    int ambient = sky.GetAmbientLight();
    // At midnight, ambient light should be minimum (4).
    EXPECT_EQ(ambient, 4);
}

TEST(DayNight, SkyColorDay) {
    vibecraft::Sky sky;
    sky.SetTime(6000);  // Noon

    glm::vec3 color = sky.GetSkyColor();
    // At noon, sky should be blue: approximately (0.5, 0.7, 1.0).
    EXPECT_NEAR(color.r, 0.5f, 0.05f);
    EXPECT_NEAR(color.g, 0.7f, 0.05f);
    EXPECT_NEAR(color.b, 1.0f, 0.05f);
}

TEST(DayNight, SkyColorNight) {
    vibecraft::Sky sky;
    sky.SetTime(18000);  // Midnight

    glm::vec3 color = sky.GetSkyColor();
    // At midnight, sky should be near-black: approximately (0.01, 0.01, 0.05).
    EXPECT_NEAR(color.r, 0.01f, 0.05f);
    EXPECT_NEAR(color.g, 0.01f, 0.05f);
    EXPECT_NEAR(color.b, 0.05f, 0.05f);
}

TEST(DayNight, SkyColorTransition) {
    vibecraft::Sky sky;
    sky.SetTime(12000);  // Dusk / sunset

    glm::vec3 color = sky.GetSkyColor();
    glm::vec3 day = vibecraft::Sky::kDaySkyColor;
    glm::vec3 night = vibecraft::Sky::kNightSkyColor;

    // At dusk, sky color should be between day and night values (component-wise).
    EXPECT_GT(color.r, night.r);
    EXPECT_LT(color.r, day.r);
    EXPECT_GT(color.g, night.g);
    EXPECT_LT(color.g, day.g);
    EXPECT_GT(color.b, night.b);
    EXPECT_LT(color.b, day.b);
}

TEST(DayNight, FogMatchesSky) {
    vibecraft::Sky sky;

    // Test at several different times that fog always matches sky.
    for (int time : {0, 3000, 6000, 9000, 12000, 15000, 18000, 21000}) {
        sky.SetTime(time);
        glm::vec3 sky_color = sky.GetSkyColor();
        glm::vec3 fog_color = sky.GetFogColor();

        EXPECT_FLOAT_EQ(fog_color.r, sky_color.r) << "at time " << time;
        EXPECT_FLOAT_EQ(fog_color.g, sky_color.g) << "at time " << time;
        EXPECT_FLOAT_EQ(fog_color.b, sky_color.b) << "at time " << time;
    }
}
