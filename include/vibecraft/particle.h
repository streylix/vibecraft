#ifndef VIBECRAFT_PARTICLE_H
#define VIBECRAFT_PARTICLE_H

#include <cstddef>
#include <vector>

#include <glm/vec3.hpp>

#include "vibecraft/block.h"

namespace vibecraft {

/// A single particle with physics and visual properties.
struct Particle {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec3 color{1.0f};      // RGB color
    float lifetime = 1.0f;       // Total lifetime in seconds
    float age = 0.0f;            // Current age in seconds
    bool alive = false;          // Whether this particle slot is active
};

/// Manages a pool of particles with configurable emission and physics.
///
/// Particles are pre-allocated in a fixed-size pool. Emission reuses
/// expired (dead) slots. Update applies gravity, advances age, and
/// kills particles that exceed their lifetime.
class ParticleEmitter {
public:
    /// Construct an emitter with the given maximum particle count.
    /// @param max_particles Maximum number of simultaneous particles.
    explicit ParticleEmitter(size_t max_particles = 1000);

    /// Emit a single particle with the given properties.
    /// If the pool is full, the particle is silently dropped.
    void Emit(const glm::vec3& position, const glm::vec3& velocity,
              const glm::vec3& color, float lifetime);

    /// Emit particles for a block break effect at the given position.
    /// Spawns multiple particles colored to match the block type.
    /// @param position Center of the broken block.
    /// @param block_id The type of block that was broken.
    /// @param count Number of particles to spawn (default 15).
    void EmitBlockBreak(const glm::vec3& position, BlockId block_id,
                        int count = 15);

    /// Update all alive particles by the given time step.
    /// Applies gravity, advances age, and kills expired particles.
    /// @param dt Time step in seconds.
    /// @param gravity Gravitational acceleration (positive = downward).
    void Update(float dt, float gravity = 9.81f);

    /// Get a const reference to the particle pool.
    const std::vector<Particle>& GetParticles() const;

    /// Return the number of currently alive particles.
    size_t GetAliveCount() const;

    /// Return the maximum particle capacity.
    size_t GetMaxParticles() const;

    /// Get the representative color for a block type (used for break particles).
    /// @param block_id The block type to look up.
    /// @return RGB color as a vec3.
    static glm::vec3 GetBlockColor(BlockId block_id);

private:
    /// Find the index of the first dead particle slot, or -1 if full.
    int FindDeadSlot() const;

    std::vector<Particle> particles_;
    size_t max_particles_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_PARTICLE_H
