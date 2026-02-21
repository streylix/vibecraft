#ifndef VIBECRAFT_BLOCK_INTERACTION_H
#define VIBECRAFT_BLOCK_INTERACTION_H

#include <functional>

#include <glm/glm.hpp>

#include "vibecraft/aabb.h"
#include "vibecraft/block.h"
#include "vibecraft/raycast.h"

namespace vibecraft {

/// Callback type for setting a block at given world coordinates.
using BlockSetFunc = std::function<void(int x, int y, int z, BlockId id)>;

/// Manages block breaking and placement logic.
///
/// Breaking is timed: break_time = hardness * kBaseBreakTime.
/// Negative hardness (bedrock) means unbreakable.
/// The break timer resets when the player looks at a different block.
/// Placement puts a block at hit_pos + face_normal, rejecting if the
/// new block would overlap the player's AABB or if there is no solid
/// face to place against.
class BlockInteraction {
public:
    /// Base break time multiplier (seconds per hardness unit).
    static constexpr float kBaseBreakTime = 1.0f;

    BlockInteraction() = default;

    /// Start or continue breaking the block at the given position.
    /// Returns true if the block was fully broken this call.
    ///
    /// @param target_pos   Block coordinates of the target block.
    /// @param block_id     BlockId of the target block.
    /// @param registry     Block registry for looking up hardness.
    /// @param dt           Time elapsed since last call (seconds).
    /// @param block_set    Callback to set a block in the world.
    /// @return true if the block was broken, false otherwise.
    bool UpdateBreaking(const glm::ivec3& target_pos,
                        BlockId block_id,
                        const BlockRegistry& registry,
                        float dt,
                        const BlockSetFunc& block_set);

    /// Reset the break timer (e.g., when the player looks away).
    void ResetBreaking();

    /// Get the current break progress as a fraction in [0, 1].
    /// Returns 0 if not currently breaking.
    float GetBreakProgress() const;

    /// Get the block position currently being broken.
    glm::ivec3 GetBreakTarget() const;

    /// Check whether a block is currently being broken.
    bool IsBreaking() const;

    /// Attempt to place a block adjacent to the hit face.
    ///
    /// @param hit_pos       Block coordinates of the hit block.
    /// @param face_normal   Normal of the hit face (placement direction).
    /// @param block_to_place BlockId to place.
    /// @param player_aabb   Player's current AABB (to prevent self-entombment).
    /// @param block_query   Callback to query the block at a position.
    /// @param block_set     Callback to set a block in the world.
    /// @param registry      Block registry for checking solid property.
    /// @return true if placement succeeded, false if rejected.
    static bool PlaceBlock(const glm::ivec3& hit_pos,
                           const glm::ivec3& face_normal,
                           BlockId block_to_place,
                           const AABB& player_aabb,
                           const BlockQueryFunc& block_query,
                           const BlockSetFunc& block_set,
                           const BlockRegistry& registry);

private:
    bool breaking_ = false;
    glm::ivec3 break_target_ = glm::ivec3(0);
    float break_timer_ = 0.0f;
    float break_total_time_ = 0.0f;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_BLOCK_INTERACTION_H
