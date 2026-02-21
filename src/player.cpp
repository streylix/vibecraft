#include "vibecraft/player.h"

#include <algorithm>
#include <cmath>

#include "vibecraft/math_utils.h"

namespace vibecraft {

Player::Player(const glm::vec3& position)
    : position_(position), velocity_(0.0f), grounded_(false) {}

void Player::Update(const BlockQueryFunc& block_query,
                    const BlockRegistry& registry) {
    // Apply gravity.
    velocity_.y -= kGravity * kTickDt;

    // Clamp to terminal velocity.
    if (velocity_.y < -kTerminalVelocity) {
        velocity_.y = -kTerminalVelocity;
    }

    // Compute displacement for this tick.
    glm::vec3 displacement = velocity_ * kTickDt;

    // Resolve collisions axis by axis: Y first (gravity/jump), then X, then Z.
    // Resolving Y first ensures ground detection works properly.
    ResolveAxis(1, displacement, block_query, registry);  // Y
    ResolveAxis(0, displacement, block_query, registry);  // X
    ResolveAxis(2, displacement, block_query, registry);  // Z

    // Update grounded state.
    UpdateGrounded(block_query, registry);
}

void Player::Jump() {
    if (grounded_) {
        velocity_.y = kJumpVelocity;
        grounded_ = false;
    }
}

glm::vec3 Player::GetPosition() const {
    return position_;
}

void Player::SetPosition(const glm::vec3& position) {
    position_ = position;
}

glm::vec3 Player::GetEyePosition() const {
    return position_ + glm::vec3(0.0f, kEyeHeight, 0.0f);
}

glm::vec3 Player::GetVelocity() const {
    return velocity_;
}

void Player::SetVelocity(const glm::vec3& velocity) {
    velocity_ = velocity;
}

bool Player::IsGrounded() const {
    return grounded_;
}

AABB Player::GetAABB() const {
    return MakeAABB(position_);
}

AABB Player::MakeAABB(const glm::vec3& feet_position) {
    float half_w = kWidth * 0.5f;
    glm::vec3 min_corner(feet_position.x - half_w,
                         feet_position.y,
                         feet_position.z - half_w);
    glm::vec3 max_corner(feet_position.x + half_w,
                         feet_position.y + kHeight,
                         feet_position.z + half_w);
    return AABB(min_corner, max_corner);
}

void Player::ResolveAxis(int axis, const glm::vec3& displacement,
                          const BlockQueryFunc& block_query,
                          const BlockRegistry& registry) {
    if (std::abs(displacement[axis]) < 1e-8f) {
        return;
    }

    // Build the player AABB at the current position.
    AABB player_box = GetAABB();

    // Create a velocity vector that only moves along the target axis.
    glm::vec3 axis_vel(0.0f);
    axis_vel[axis] = displacement[axis];

    // Determine the range of blocks we need to check.
    // Expand the AABB by the displacement to find all potentially colliding blocks.
    AABB expanded = player_box;
    if (axis_vel[axis] > 0.0f) {
        expanded.max[axis] += axis_vel[axis];
    } else {
        expanded.min[axis] += axis_vel[axis];
    }

    // Convert to block coordinates (inclusive range).
    int bx_min = IntFloor(expanded.min.x);
    int by_min = IntFloor(expanded.min.y);
    int bz_min = IntFloor(expanded.min.z);
    int bx_max = IntFloor(expanded.max.x - 1e-6f);
    int by_max = IntFloor(expanded.max.y - 1e-6f);
    int bz_max = IntFloor(expanded.max.z - 1e-6f);

    // Ensure we check at least a 3x4x3 region around the player.
    // (The milestone says check 3x4x3 but we expand based on displacement.)

    float min_t = 1.0f;
    glm::vec3 hit_normal(0.0f);

    for (int bx = bx_min; bx <= bx_max; ++bx) {
        for (int by = by_min; by <= by_max; ++by) {
            for (int bz = bz_min; bz <= bz_max; ++bz) {
                BlockId block_id = block_query(bx, by, bz);
                const BlockType& block_type = registry.GetBlock(block_id);

                if (!block_type.solid) {
                    continue;
                }

                // Block AABB: unit cube at (bx, by, bz).
                AABB block_box(glm::vec3(bx, by, bz),
                               glm::vec3(bx + 1, by + 1, bz + 1));

                SweptResult result = player_box.Sweep(axis_vel, block_box);

                if (result.t < min_t) {
                    min_t = result.t;
                    hit_normal = result.normal;
                }
            }
        }
    }

    // Move the player along this axis by the resolved amount.
    // Add a small epsilon to prevent floating-point drift into blocks.
    float move = displacement[axis] * min_t;

    // Apply a tiny nudge away from the collision surface to prevent sticking.
    if (min_t < 1.0f) {
        constexpr float kEpsilon = 1e-4f;
        move += hit_normal[axis] * kEpsilon;
    }

    position_[axis] += move;

    // If we hit something, zero velocity on that axis.
    if (min_t < 1.0f) {
        velocity_[axis] = 0.0f;
    }
}

void Player::UpdateGrounded(const BlockQueryFunc& block_query,
                            const BlockRegistry& registry) {
    // Check if there's a solid block just below the player's feet.
    // We probe slightly below the player's AABB bottom.
    constexpr float kGroundProbe = 0.01f;

    AABB player_box = GetAABB();

    // Check all blocks that the player's horizontal footprint overlaps,
    // one block below.
    int bx_min = IntFloor(player_box.min.x);
    int bx_max = IntFloor(player_box.max.x - 1e-6f);
    int bz_min = IntFloor(player_box.min.z);
    int bz_max = IntFloor(player_box.max.z - 1e-6f);
    int by_check = IntFloor(player_box.min.y - kGroundProbe);

    grounded_ = false;

    for (int bx = bx_min; bx <= bx_max; ++bx) {
        for (int bz = bz_min; bz <= bz_max; ++bz) {
            BlockId block_id = block_query(bx, by_check, bz);
            const BlockType& block_type = registry.GetBlock(block_id);

            if (block_type.solid) {
                // Verify the player is actually resting on top of this block.
                float block_top = static_cast<float>(by_check + 1);
                if (std::abs(player_box.min.y - block_top) < kGroundProbe + 1e-4f) {
                    grounded_ = true;
                    return;
                }
            }
        }
    }
}

}  // namespace vibecraft
