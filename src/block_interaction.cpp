#include "vibecraft/block_interaction.h"

namespace vibecraft {

bool BlockInteraction::UpdateBreaking(const glm::ivec3& target_pos,
                                       BlockId block_id,
                                       const BlockRegistry& registry,
                                       float dt,
                                       const BlockSetFunc& block_set) {
    const BlockType& block_type = registry.GetBlock(block_id);

    // Negative hardness means unbreakable (e.g., bedrock).
    if (block_type.hardness < 0.0f) {
        return false;
    }

    // Air or zero-hardness non-solid blocks cannot be broken meaningfully.
    if (block_id == BlockRegistry::kAir) {
        return false;
    }

    // If we're targeting a new block, reset the timer.
    if (!breaking_ || break_target_ != target_pos) {
        breaking_ = true;
        break_target_ = target_pos;
        break_total_time_ = block_type.hardness * kBaseBreakTime;
        break_timer_ = break_total_time_;
    }

    // Decrement the timer.
    break_timer_ -= dt;

    // Check if breaking is complete.
    if (break_timer_ <= 0.0f) {
        // Replace the block with air.
        block_set(target_pos.x, target_pos.y, target_pos.z, BlockRegistry::kAir);
        ResetBreaking();
        return true;
    }

    return false;
}

void BlockInteraction::ResetBreaking() {
    breaking_ = false;
    break_target_ = glm::ivec3(0);
    break_timer_ = 0.0f;
    break_total_time_ = 0.0f;
}

float BlockInteraction::GetBreakProgress() const {
    if (!breaking_ || break_total_time_ <= 0.0f) {
        return 0.0f;
    }
    float progress = 1.0f - (break_timer_ / break_total_time_);
    if (progress < 0.0f) return 0.0f;
    if (progress > 1.0f) return 1.0f;
    return progress;
}

glm::ivec3 BlockInteraction::GetBreakTarget() const {
    return break_target_;
}

bool BlockInteraction::IsBreaking() const {
    return breaking_;
}

bool BlockInteraction::PlaceBlock(const glm::ivec3& hit_pos,
                                   const glm::ivec3& face_normal,
                                   BlockId block_to_place,
                                   const AABB& player_aabb,
                                   const BlockQueryFunc& block_query,
                                   const BlockSetFunc& block_set,
                                   const BlockRegistry& registry) {
    // The hit block must be solid (can't place against air).
    BlockId hit_block = block_query(hit_pos.x, hit_pos.y, hit_pos.z);
    if (!registry.GetBlock(hit_block).solid) {
        return false;
    }

    // Calculate placement position.
    glm::ivec3 place_pos = hit_pos + face_normal;

    // The target position must be air (or non-solid) to place into.
    BlockId existing = block_query(place_pos.x, place_pos.y, place_pos.z);
    if (existing != BlockRegistry::kAir &&
        registry.GetBlock(existing).solid) {
        return false;
    }

    // Build the AABB of the block that would be placed.
    glm::vec3 block_min(static_cast<float>(place_pos.x),
                        static_cast<float>(place_pos.y),
                        static_cast<float>(place_pos.z));
    glm::vec3 block_max = block_min + glm::vec3(1.0f);
    AABB block_aabb(block_min, block_max);

    // Reject if the placed block would overlap the player's AABB.
    if (block_aabb.Overlaps(player_aabb)) {
        return false;
    }

    // Place the block.
    block_set(place_pos.x, place_pos.y, place_pos.z, block_to_place);
    return true;
}

}  // namespace vibecraft
