#include <gtest/gtest.h>

#include <cmath>
#include <numeric>
#include <vector>

#include "vibecraft/noise.h"

// M5: Noise Generation

TEST(Noise, RangeCheck2D) {
    vibecraft::PerlinNoise noise(42);

    for (int i = 0; i < 1000; ++i) {
        float x = static_cast<float>(i) * 0.137f - 50.0f;
        float y = static_cast<float>(i) * 0.251f - 50.0f;
        float val = noise.Noise2D(x, y);
        EXPECT_GE(val, -1.0f) << "2D noise below -1 at (" << x << ", " << y << ")";
        EXPECT_LE(val, 1.0f) << "2D noise above 1 at (" << x << ", " << y << ")";
    }
}

TEST(Noise, RangeCheck3D) {
    vibecraft::PerlinNoise noise(42);

    for (int i = 0; i < 1000; ++i) {
        float x = static_cast<float>(i) * 0.137f - 50.0f;
        float y = static_cast<float>(i) * 0.251f - 50.0f;
        float z = static_cast<float>(i) * 0.319f - 50.0f;
        float val = noise.Noise3D(x, y, z);
        EXPECT_GE(val, -1.0f) << "3D noise below -1 at (" << x << ", " << y << ", " << z << ")";
        EXPECT_LE(val, 1.0f) << "3D noise above 1 at (" << x << ", " << y << ", " << z << ")";
    }
}

TEST(Noise, Deterministic) {
    vibecraft::PerlinNoise noise1(12345);
    vibecraft::PerlinNoise noise2(12345);

    // Same seed + same coords must produce identical output.
    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(i) * 1.23f;
        float y = static_cast<float>(i) * 4.56f;
        float z = static_cast<float>(i) * 7.89f;

        EXPECT_FLOAT_EQ(noise1.Noise2D(x, y), noise2.Noise2D(x, y));
        EXPECT_FLOAT_EQ(noise1.Noise3D(x, y, z), noise2.Noise3D(x, y, z));
        EXPECT_FLOAT_EQ(noise1.OctaveNoise2D(x, y), noise2.OctaveNoise2D(x, y));
        EXPECT_FLOAT_EQ(noise1.OctaveNoise3D(x, y, z), noise2.OctaveNoise3D(x, y, z));
    }
}

TEST(Noise, DifferentSeedsDiffer) {
    vibecraft::PerlinNoise noise_a(100);
    vibecraft::PerlinNoise noise_b(200);

    int differences = 0;
    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(i) * 0.7f;
        float y = static_cast<float>(i) * 1.3f;
        if (noise_a.Noise2D(x, y) != noise_b.Noise2D(x, y)) {
            ++differences;
        }
    }

    // At least some samples must differ between different seeds.
    EXPECT_GT(differences, 0)
        << "Two different seeds produced identical noise for all 100 samples";
    // In practice, almost all should differ.
    EXPECT_GT(differences, 50)
        << "Expected majority of samples to differ between seeds";
}

TEST(Noise, SmoothContinuity) {
    vibecraft::PerlinNoise noise(42);

    const float step = 0.01f;
    const float threshold = 0.1f;  // Small step => small change.

    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(i) * 0.5f;
        float y = static_cast<float>(i) * 0.3f;

        float val_a = noise.Noise2D(x, y);
        float val_b = noise.Noise2D(x + step, y);
        float val_c = noise.Noise2D(x, y + step);

        EXPECT_LT(std::abs(val_a - val_b), threshold)
            << "2D noise not smooth along x at (" << x << ", " << y << ")";
        EXPECT_LT(std::abs(val_a - val_c), threshold)
            << "2D noise not smooth along y at (" << x << ", " << y << ")";
    }

    // Also check 3D smoothness.
    for (int i = 0; i < 100; ++i) {
        float x = static_cast<float>(i) * 0.5f;
        float y = static_cast<float>(i) * 0.3f;
        float z = static_cast<float>(i) * 0.7f;

        float val_a = noise.Noise3D(x, y, z);
        float val_b = noise.Noise3D(x + step, y, z);

        EXPECT_LT(std::abs(val_a - val_b), threshold)
            << "3D noise not smooth along x at (" << x << ", " << y << ", " << z << ")";
    }
}

TEST(Noise, OctaveAmplitude) {
    vibecraft::PerlinNoise noise(42);

    // Collect samples from 1-octave and 4-octave noise.
    std::vector<float> one_octave;
    std::vector<float> four_octave;

    for (int i = 0; i < 500; ++i) {
        float x = static_cast<float>(i) * 0.1f;
        float y = static_cast<float>(i) * 0.13f;
        one_octave.push_back(noise.OctaveNoise2D(x, y, 1, 0.5f, 2.0f));
        four_octave.push_back(noise.OctaveNoise2D(x, y, 4, 0.5f, 2.0f));
    }

    // They should produce different values (more octaves adds detail).
    int differences = 0;
    for (size_t i = 0; i < one_octave.size(); ++i) {
        if (std::abs(one_octave[i] - four_octave[i]) > 1e-6f) {
            ++differences;
        }
    }

    EXPECT_GT(differences, static_cast<int>(one_octave.size()) / 2)
        << "1-octave and 4-octave noise should differ for most samples";
}

TEST(Noise, OctaveCount) {
    vibecraft::PerlinNoise noise(42);

    // Compute standard deviation for 1-octave and 8-octave noise.
    // More octaves should produce different detail characteristics.
    auto compute_stddev = [](const std::vector<float>& samples) -> float {
        float sum = std::accumulate(samples.begin(), samples.end(), 0.0f);
        float mean = sum / static_cast<float>(samples.size());
        float sq_sum = 0.0f;
        for (float s : samples) {
            sq_sum += (s - mean) * (s - mean);
        }
        return std::sqrt(sq_sum / static_cast<float>(samples.size()));
    };

    std::vector<float> samples_1;
    std::vector<float> samples_8;

    for (int i = 0; i < 1000; ++i) {
        float x = static_cast<float>(i) * 0.05f;
        float y = static_cast<float>(i) * 0.07f;
        samples_1.push_back(noise.OctaveNoise2D(x, y, 1, 0.5f, 2.0f));
        samples_8.push_back(noise.OctaveNoise2D(x, y, 8, 0.5f, 2.0f));
    }

    float stddev_1 = compute_stddev(samples_1);
    float stddev_8 = compute_stddev(samples_8);

    // Both should have non-trivial variation.
    EXPECT_GT(stddev_1, 0.01f) << "1-octave noise has no variation";
    EXPECT_GT(stddev_8, 0.01f) << "8-octave noise has no variation";

    // The standard deviations should differ (more octaves changes the signal).
    EXPECT_NE(stddev_1, stddev_8)
        << "1-octave and 8-octave noise have identical stddev";
}

TEST(Noise, FrequencyScaling) {
    vibecraft::PerlinNoise noise(42);

    // Sample at two different frequencies along a line and measure the sum of
    // absolute differences between consecutive samples.  Use a step that is
    // small relative to the noise period so higher frequency means more
    // zero-crossings / larger accumulated deltas.
    auto compute_variation = [&](float frequency) -> float {
        float total_delta = 0.0f;
        const float step = 0.017f;  // Non-integer fraction avoids grid alignment.
        for (int i = 0; i < 500; ++i) {
            float x = static_cast<float>(i) * step;
            float y = 5.3f;
            float val_a = noise.OctaveNoise2D(x, y, 1, 0.5f, 2.0f, frequency);
            float val_b = noise.OctaveNoise2D(x + step, y, 1, 0.5f, 2.0f, frequency);
            total_delta += std::abs(val_a - val_b);
        }
        return total_delta;
    };

    float var_low = compute_variation(1.0f);
    float var_high = compute_variation(10.0f);

    // Higher frequency should produce more total variation (more changes per unit).
    EXPECT_GT(var_high, var_low)
        << "Higher frequency noise should have more variation than lower frequency";
}

TEST(Noise, LargeCoordinates) {
    vibecraft::PerlinNoise noise(42);

    // Test at large coordinates — should produce valid floats, no NaN/Inf.
    float val_2d = noise.Noise2D(10000.0f, 10000.0f);
    EXPECT_FALSE(std::isnan(val_2d)) << "2D noise is NaN at large coords";
    EXPECT_FALSE(std::isinf(val_2d)) << "2D noise is Inf at large coords";
    EXPECT_GE(val_2d, -1.0f);
    EXPECT_LE(val_2d, 1.0f);

    float val_3d = noise.Noise3D(10000.0f, 10000.0f, 10000.0f);
    EXPECT_FALSE(std::isnan(val_3d)) << "3D noise is NaN at large coords";
    EXPECT_FALSE(std::isinf(val_3d)) << "3D noise is Inf at large coords";
    EXPECT_GE(val_3d, -1.0f);
    EXPECT_LE(val_3d, 1.0f);

    // Also test octave noise at large coords.
    float val_oct = noise.OctaveNoise2D(50000.0f, 50000.0f);
    EXPECT_FALSE(std::isnan(val_oct)) << "Octave 2D noise is NaN at large coords";
    EXPECT_FALSE(std::isinf(val_oct)) << "Octave 2D noise is Inf at large coords";
    EXPECT_GE(val_oct, -1.0f);
    EXPECT_LE(val_oct, 1.0f);
}

TEST(Noise, NegativeCoordinates) {
    vibecraft::PerlinNoise noise(42);

    // Test at negative coordinates — should produce valid floats.
    float val_2d = noise.Noise2D(-100.0f, -100.0f);
    EXPECT_FALSE(std::isnan(val_2d)) << "2D noise is NaN at negative coords";
    EXPECT_FALSE(std::isinf(val_2d)) << "2D noise is Inf at negative coords";
    EXPECT_GE(val_2d, -1.0f);
    EXPECT_LE(val_2d, 1.0f);

    float val_3d = noise.Noise3D(-100.0f, -100.0f, -100.0f);
    EXPECT_FALSE(std::isnan(val_3d)) << "3D noise is NaN at negative coords";
    EXPECT_FALSE(std::isinf(val_3d)) << "3D noise is Inf at negative coords";
    EXPECT_GE(val_3d, -1.0f);
    EXPECT_LE(val_3d, 1.0f);

    // Check a range of negative coordinates for consistency.
    for (int i = 0; i < 100; ++i) {
        float x = -static_cast<float>(i) * 1.7f;
        float y = -static_cast<float>(i) * 2.3f;
        float val = noise.Noise2D(x, y);
        EXPECT_GE(val, -1.0f);
        EXPECT_LE(val, 1.0f);
    }
}
