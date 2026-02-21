#ifndef VIBECRAFT_PLAYER_H
#define VIBECRAFT_PLAYER_H

#include <functional>

#include <glm/glm.hpp>

#include "vibecraft/aabb.h"
#include "vibecraft/block.h"

namespace vibecraft {

/// Callback type for querying what block exists at a given block coordinate.
/// The physics system uses this instead of holding a direct world reference.
using BlockQueryFunc = std::function<BlockId(int x, int y, int z)>;

/// Player entity with AABB-based physics.
///
/// The player's position is at their feet (bottom-center of the AABB).
/// AABB dimensions: 0.6 wide (x), 1.8 tall (y), 0.6 deep (z), centered
/// horizontally on the position.
class Player {
public:
    /// Physics constants.
    static constexpr float kGravity = 32.0f;          // blocks/s^2
    static constexpr float kTerminalVelocity = 78.0f;  // blocks/s
    static constexpr float kJumpVelocity = 9.2f;       // blocks/s upward
    static constexpr float kWidth = 0.6f;
    static constexpr float kHeight = 1.8f;
    static constexpr float kEyeHeight = 1.62f;
    static constexpr float kTickRate = 20.0f;           // ticks per second
    static constexpr float kTickDt = 1.0f / kTickRate;  // seconds per tick

    /// Construct a player at the given position (feet position).
    explicit Player(const glm::vec3& position = glm::vec3(0.0f));

    /// Update physics for one tick (1/20 second).
    /// @param block_query  Callback to query the block type at world coordinates.
    /// @param registry     Block registry for checking block properties.
    void Update(const BlockQueryFunc& block_query, const BlockRegistry& registry);

    /// Attempt to jump. Only succeeds if the player is grounded.
    void Jump();

    /// Get the player's foot position (bottom-center of AABB).
    glm::vec3 GetPosition() const;

    /// Set the player's foot position.
    void SetPosition(const glm::vec3& position);

    /// Get the player's eye position (position + eye height offset).
    glm::vec3 GetEyePosition() const;

    /// Get the player's current velocity.
    glm::vec3 GetVelocity() const;

    /// Set the player's velocity directly.
    void SetVelocity(const glm::vec3& velocity);

    /// Check whether the player is currently standing on solid ground.
    bool IsGrounded() const;

    /// Get the player's AABB at their current position.
    AABB GetAABB() const;

    /// Build an AABB for the player at a given foot position.
    static AABB MakeAABB(const glm::vec3& feet_position);

private:
    /// Resolve collisions along one axis using swept AABB tests.
    /// Updates position and velocity component for the given axis.
    void ResolveAxis(int axis, const glm::vec3& displacement,
                     const BlockQueryFunc& block_query,
                     const BlockRegistry& registry);

    /// Check if the player is standing on solid ground.
    void UpdateGrounded(const BlockQueryFunc& block_query,
                        const BlockRegistry& registry);

    glm::vec3 position_;
    glm::vec3 velocity_;
    bool grounded_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_PLAYER_H
