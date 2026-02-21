#include <gtest/gtest.h>

#include "vibecraft/particle.h"

// M23: Particle System

using vibecraft::BlockRegistry;
using vibecraft::Particle;
using vibecraft::ParticleEmitter;

TEST(Particles, Emit) {
    ParticleEmitter emitter(100);
    EXPECT_EQ(emitter.GetAliveCount(), 0u);

    emitter.Emit({0.0f, 0.0f, 0.0f}, {1.0f, 2.0f, 3.0f},
                 {1.0f, 1.0f, 1.0f}, 2.0f);

    EXPECT_EQ(emitter.GetAliveCount(), 1u);

    // Emit a few more.
    emitter.Emit({1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f}, 1.0f);
    emitter.Emit({2.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
                 {0.0f, 1.0f, 0.0f}, 3.0f);

    EXPECT_EQ(emitter.GetAliveCount(), 3u);
}

TEST(Particles, UpdatePosition) {
    ParticleEmitter emitter(100);
    glm::vec3 start_pos{5.0f, 10.0f, 5.0f};
    glm::vec3 velocity{1.0f, 0.0f, 0.0f};

    emitter.Emit(start_pos, velocity, {1.0f, 1.0f, 1.0f}, 5.0f);

    // Update with zero gravity so only velocity contributes.
    emitter.Update(1.0f, 0.0f);

    const auto& particles = emitter.GetParticles();
    // Find the alive particle.
    bool found = false;
    for (const auto& p : particles) {
        if (p.alive) {
            // After 1 second at velocity (1,0,0), position.x should be ~6.
            EXPECT_FLOAT_EQ(p.position.x, 6.0f);
            EXPECT_FLOAT_EQ(p.position.y, 10.0f);
            EXPECT_FLOAT_EQ(p.position.z, 5.0f);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected an alive particle after update";
}

TEST(Particles, Lifetime) {
    ParticleEmitter emitter(100);
    emitter.Emit({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
                 {1.0f, 1.0f, 1.0f}, 1.0f);

    EXPECT_EQ(emitter.GetAliveCount(), 1u);

    // Advance time by 0.5 seconds -- particle still alive.
    emitter.Update(0.5f, 0.0f);
    EXPECT_EQ(emitter.GetAliveCount(), 1u);

    // Advance past lifetime -- particle should be dead.
    emitter.Update(0.6f, 0.0f);
    EXPECT_EQ(emitter.GetAliveCount(), 0u);
}

TEST(Particles, Gravity) {
    ParticleEmitter emitter(100);
    glm::vec3 start_pos{0.0f, 100.0f, 0.0f};
    glm::vec3 velocity{0.0f, 0.0f, 0.0f};

    emitter.Emit(start_pos, velocity, {1.0f, 1.0f, 1.0f}, 10.0f);

    // Update with gravity for 1 second.
    float gravity = 9.81f;
    emitter.Update(1.0f, gravity);

    const auto& particles = emitter.GetParticles();
    bool found = false;
    for (const auto& p : particles) {
        if (p.alive) {
            // Y should have decreased due to gravity.
            // After 1s: velocity_y = -9.81, position_y = 100 + (-9.81*1) = ~90.19
            EXPECT_LT(p.position.y, start_pos.y);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected an alive particle after gravity update";
}

TEST(Particles, MaxLimit) {
    const size_t max_count = 10;
    ParticleEmitter emitter(max_count);

    EXPECT_EQ(emitter.GetMaxParticles(), max_count);

    // Emit more than max.
    for (size_t i = 0; i < max_count + 5; ++i) {
        emitter.Emit({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
                     {1.0f, 1.0f, 1.0f}, 10.0f);
    }

    // Alive count should be capped at max.
    EXPECT_EQ(emitter.GetAliveCount(), max_count);
}

TEST(Particles, BlockBreakColor) {
    // Stone should produce gray particles.
    glm::vec3 stone_color = ParticleEmitter::GetBlockColor(BlockRegistry::kStone);
    EXPECT_FLOAT_EQ(stone_color.r, 0.5f);
    EXPECT_FLOAT_EQ(stone_color.g, 0.5f);
    EXPECT_FLOAT_EQ(stone_color.b, 0.5f);

    // Grass should produce green particles.
    glm::vec3 grass_color = ParticleEmitter::GetBlockColor(BlockRegistry::kGrass);
    EXPECT_GT(grass_color.g, grass_color.r);
    EXPECT_GT(grass_color.g, grass_color.b);

    // EmitBlockBreak should create particles with the right color.
    ParticleEmitter emitter(100);
    emitter.EmitBlockBreak({5.0f, 10.0f, 5.0f}, BlockRegistry::kStone, 10);

    EXPECT_EQ(emitter.GetAliveCount(), 10u);

    // Check that all particles have stone color.
    for (const auto& p : emitter.GetParticles()) {
        if (p.alive) {
            EXPECT_FLOAT_EQ(p.color.r, stone_color.r);
            EXPECT_FLOAT_EQ(p.color.g, stone_color.g);
            EXPECT_FLOAT_EQ(p.color.b, stone_color.b);
        }
    }
}
