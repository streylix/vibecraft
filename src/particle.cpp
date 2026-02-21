#include "vibecraft/particle.h"

#include <cmath>

namespace vibecraft {

ParticleEmitter::ParticleEmitter(size_t max_particles)
    : max_particles_(max_particles) {
    particles_.resize(max_particles_);
    // All particles start dead (alive = false by default).
}

void ParticleEmitter::Emit(const glm::vec3& position,
                           const glm::vec3& velocity,
                           const glm::vec3& color, float lifetime) {
    int slot = FindDeadSlot();
    if (slot < 0) {
        return;  // Pool is full; silently drop.
    }

    Particle& p = particles_[static_cast<size_t>(slot)];
    p.position = position;
    p.velocity = velocity;
    p.color = color;
    p.lifetime = lifetime;
    p.age = 0.0f;
    p.alive = true;
}

void ParticleEmitter::EmitBlockBreak(const glm::vec3& position,
                                     BlockId block_id, int count) {
    glm::vec3 color = GetBlockColor(block_id);

    for (int i = 0; i < count; ++i) {
        // Generate deterministic spread based on index.
        // Particles fan out in a hemisphere above the break point.
        float angle = static_cast<float>(i) * 2.0f * 3.14159265f /
                      static_cast<float>(count);
        float speed = 2.0f + static_cast<float>(i % 3) * 0.5f;
        glm::vec3 vel{
            std::cos(angle) * speed,
            3.0f + static_cast<float>(i % 4) * 0.5f,  // Upward burst
            std::sin(angle) * speed};

        Emit(position, vel, color, 1.0f + static_cast<float>(i % 3) * 0.3f);
    }
}

void ParticleEmitter::Update(float dt, float gravity) {
    for (auto& p : particles_) {
        if (!p.alive) {
            continue;
        }

        // Apply gravity (Y-down acceleration).
        p.velocity.y -= gravity * dt;

        // Integrate position.
        p.position += p.velocity * dt;

        // Age the particle.
        p.age += dt;

        // Kill if expired.
        if (p.age >= p.lifetime) {
            p.alive = false;
        }
    }
}

const std::vector<Particle>& ParticleEmitter::GetParticles() const {
    return particles_;
}

size_t ParticleEmitter::GetAliveCount() const {
    size_t count = 0;
    for (const auto& p : particles_) {
        if (p.alive) {
            ++count;
        }
    }
    return count;
}

size_t ParticleEmitter::GetMaxParticles() const {
    return max_particles_;
}

glm::vec3 ParticleEmitter::GetBlockColor(BlockId block_id) {
    // Map known block types to representative colors.
    switch (block_id) {
        case BlockRegistry::kStone:
            return {0.5f, 0.5f, 0.5f};  // Gray
        case BlockRegistry::kGrass:
            return {0.3f, 0.7f, 0.2f};  // Green
        case BlockRegistry::kDirt:
            return {0.6f, 0.4f, 0.2f};  // Brown
        case BlockRegistry::kCobblestone:
            return {0.4f, 0.4f, 0.4f};  // Dark gray
        case BlockRegistry::kOakPlanks:
            return {0.7f, 0.5f, 0.3f};  // Light brown
        case BlockRegistry::kBedrock:
            return {0.2f, 0.2f, 0.2f};  // Very dark gray
        case BlockRegistry::kSand:
            return {0.9f, 0.85f, 0.6f}; // Tan
        case BlockRegistry::kGravel:
            return {0.55f, 0.5f, 0.5f}; // Grayish
        case BlockRegistry::kGoldOre:
            return {0.8f, 0.7f, 0.2f};  // Gold
        case BlockRegistry::kIronOre:
            return {0.7f, 0.6f, 0.55f}; // Iron tan
        case BlockRegistry::kCoalOre:
            return {0.15f, 0.15f, 0.15f}; // Near black
        case BlockRegistry::kDiamondOre:
            return {0.4f, 0.9f, 0.9f};  // Cyan
        case BlockRegistry::kOakLog:
            return {0.5f, 0.35f, 0.15f}; // Dark brown
        case BlockRegistry::kOakLeaves:
            return {0.2f, 0.6f, 0.1f};  // Dark green
        case BlockRegistry::kGlass:
            return {0.8f, 0.9f, 1.0f};  // Light blue tint
        case BlockRegistry::kSnow:
            return {0.95f, 0.95f, 0.98f}; // White
        case BlockRegistry::kObsidian:
            return {0.1f, 0.05f, 0.15f}; // Dark purple
        default:
            return {0.7f, 0.7f, 0.7f};  // Default gray
    }
}

int ParticleEmitter::FindDeadSlot() const {
    for (size_t i = 0; i < particles_.size(); ++i) {
        if (!particles_[i].alive) {
            return static_cast<int>(i);
        }
    }
    return -1;  // Pool is full.
}

}  // namespace vibecraft
