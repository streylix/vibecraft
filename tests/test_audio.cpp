#include <gtest/gtest.h>

#include <cmath>
#include <fstream>
#include <string>

#include "vibecraft/audio.h"

// M20: Audio System

#ifndef VIBECRAFT_ASSET_DIR
#define VIBECRAFT_ASSET_DIR "assets"
#endif

class AudioTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create audio system in headless mode for testing.
        audio_ = std::make_unique<vibecraft::AudioSystem>(
            /*headless=*/true, std::string(VIBECRAFT_ASSET_DIR));
        audio_->Init();
        audio_->RegisterDefaultSounds();
    }

    void TearDown() override { audio_.reset(); }

    std::unique_ptr<vibecraft::AudioSystem> audio_;
};

TEST_F(AudioTest, InitShutdown) {
    // System should be initialized after Init().
    EXPECT_TRUE(audio_->IsInitialized());

    // Shutdown should not crash or throw.
    audio_->Shutdown();
    EXPECT_FALSE(audio_->IsInitialized());

    // Re-init should work.
    EXPECT_TRUE(audio_->Init());
    EXPECT_TRUE(audio_->IsInitialized());
}

TEST_F(AudioTest, AllSoundsRegistered) {
    // All four expected sounds must be registered.
    EXPECT_TRUE(audio_->HasSound("block_break"));
    EXPECT_TRUE(audio_->HasSound("block_place"));
    EXPECT_TRUE(audio_->HasSound("footstep"));
    EXPECT_TRUE(audio_->HasSound("splash"));

    // Verify we have at least these 4 sounds.
    EXPECT_GE(audio_->GetSounds().size(), 4u);
}

TEST_F(AudioTest, SoundFilesExist) {
    // Every registered sound must point to a file that exists on disk.
    for (const auto& [name, entry] : audio_->GetSounds()) {
        std::ifstream file(entry.file_path, std::ios::binary);
        EXPECT_TRUE(file.good())
            << "WAV file missing for sound '" << name << "': "
            << entry.file_path;
    }
}

TEST_F(AudioTest, PlayNoThrow) {
    // Playing a registered sound should not throw.
    EXPECT_NO_THROW(audio_->Play("block_break"));

    // Playing an unregistered sound should also not throw.
    EXPECT_NO_THROW(audio_->Play("nonexistent_sound"));
}

TEST_F(AudioTest, VolumeClampLow) {
    audio_->SetVolume(-0.5f);
    EXPECT_FLOAT_EQ(audio_->GetVolume(), 0.0f);
}

TEST_F(AudioTest, VolumeClampHigh) {
    audio_->SetVolume(1.5f);
    EXPECT_FLOAT_EQ(audio_->GetVolume(), 1.0f);
}

TEST_F(AudioTest, DistanceAttenuation) {
    // Volume at distance 10 should be less than at distance 1.
    float atten_near = vibecraft::AudioSystem::ComputeAttenuation(1.0f);
    float atten_far = vibecraft::AudioSystem::ComputeAttenuation(10.0f);
    EXPECT_GT(atten_near, atten_far);

    // Both should be in [0, 1].
    EXPECT_GE(atten_near, 0.0f);
    EXPECT_LE(atten_near, 1.0f);
    EXPECT_GE(atten_far, 0.0f);
    EXPECT_LE(atten_far, 1.0f);

    // Verify formula: max(0, 1 - distance / 16)
    EXPECT_FLOAT_EQ(atten_near, 1.0f - 1.0f / 16.0f);   // 0.9375
    EXPECT_FLOAT_EQ(atten_far, 1.0f - 10.0f / 16.0f);    // 0.375
}

TEST_F(AudioTest, ZeroDistanceFullVolume) {
    float atten = vibecraft::AudioSystem::ComputeAttenuation(0.0f);
    EXPECT_FLOAT_EQ(atten, 1.0f);
}

TEST_F(AudioTest, MaxDistanceSilent) {
    // At exactly max distance, attenuation should be 0.
    float atten_at_max =
        vibecraft::AudioSystem::ComputeAttenuation(vibecraft::kMaxAudioDistance);
    EXPECT_FLOAT_EQ(atten_at_max, 0.0f);

    // Beyond max distance, attenuation should still be 0.
    float atten_beyond =
        vibecraft::AudioSystem::ComputeAttenuation(vibecraft::kMaxAudioDistance + 5.0f);
    EXPECT_FLOAT_EQ(atten_beyond, 0.0f);
}
