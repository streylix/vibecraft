#include <gtest/gtest.h>

#include <cstdio>
#include <memory>
#include <string>

#include "vibecraft/screen.h"
#include "vibecraft/settings.h"

// M24: UI Screens & Settings

// --- Helper screen subclass for testing ---
class TestScreen : public vibecraft::Screen {
public:
    explicit TestScreen(const std::string& name) : name_(name) {}
    std::string GetName() const override { return name_; }

private:
    std::string name_;
};

// --- Screen tests ---

TEST(Screen, PushPop) {
    vibecraft::ScreenManager mgr;

    mgr.Push(std::make_unique<TestScreen>("GameScreen"));
    mgr.Push(std::make_unique<TestScreen>("PauseMenu"));

    // Top should be the second screen pushed.
    ASSERT_NE(mgr.Top(), nullptr);
    EXPECT_EQ(mgr.Top()->GetName(), "PauseMenu");
    EXPECT_EQ(mgr.Size(), 2);

    // Pop the top; now the first screen should be on top.
    EXPECT_TRUE(mgr.Pop());
    ASSERT_NE(mgr.Top(), nullptr);
    EXPECT_EQ(mgr.Top()->GetName(), "GameScreen");
    EXPECT_EQ(mgr.Size(), 1);
}

TEST(Screen, EmptyStack) {
    vibecraft::ScreenManager mgr;

    // Pop from empty stack should not crash and return false.
    EXPECT_FALSE(mgr.Pop());
    EXPECT_EQ(mgr.Top(), nullptr);
    EXPECT_TRUE(mgr.IsEmpty());
    EXPECT_EQ(mgr.Size(), 0);
}

// --- Settings tests ---

TEST(Settings, Defaults) {
    vibecraft::Settings settings;
    EXPECT_EQ(settings.GetRenderDistance(), 8);
    EXPECT_EQ(settings.GetFOV(), 70);
    EXPECT_FLOAT_EQ(settings.GetVolume(), 1.0f);
}

TEST(Settings, ClampRenderDistance) {
    vibecraft::Settings settings;

    settings.SetRenderDistance(100);
    EXPECT_EQ(settings.GetRenderDistance(), 32);

    settings.SetRenderDistance(0);
    EXPECT_EQ(settings.GetRenderDistance(), 2);
}

TEST(Settings, ClampFOV) {
    vibecraft::Settings settings;

    settings.SetFOV(200);
    EXPECT_EQ(settings.GetFOV(), 120);

    settings.SetFOV(10);
    EXPECT_EQ(settings.GetFOV(), 30);
}

TEST(Settings, ClampVolume) {
    vibecraft::Settings settings;

    settings.SetVolume(1.5f);
    EXPECT_FLOAT_EQ(settings.GetVolume(), 1.0f);

    settings.SetVolume(-0.5f);
    EXPECT_FLOAT_EQ(settings.GetVolume(), 0.0f);
}

TEST(Settings, Persist) {
    const std::string path = "test_settings_persist.cfg";

    // Save settings with non-default values.
    {
        vibecraft::Settings settings;
        settings.SetRenderDistance(16);
        settings.SetFOV(90);
        settings.SetVolume(0.5f);
        EXPECT_TRUE(settings.Save(path));
    }

    // Load into a fresh Settings object and verify.
    {
        vibecraft::Settings settings;
        EXPECT_TRUE(settings.Load(path));
        EXPECT_EQ(settings.GetRenderDistance(), 16);
        EXPECT_EQ(settings.GetFOV(), 90);
        EXPECT_FLOAT_EQ(settings.GetVolume(), 0.5f);
    }

    // Clean up the test file.
    std::remove(path.c_str());
}
