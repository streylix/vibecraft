#include "vibecraft/raycast.h"

#include <cmath>
#include <limits>

#include "vibecraft/math_utils.h"

namespace vibecraft {

RaycastResult CastRay(const glm::vec3& origin,
                      const glm::vec3& direction,
                      float max_distance,
                      const BlockQueryFunc& block_query,
                      const BlockRegistry& registry) {
    RaycastResult result;

    // Normalize direction.
    float dir_length = glm::length(direction);
    if (dir_length < 1e-9f) {
        return result;  // Zero-length direction, no hit.
    }
    glm::vec3 dir = direction / dir_length;

    // Current voxel coordinates (the block the ray origin is in).
    int x = IntFloor(origin.x);
    int y = IntFloor(origin.y);
    int z = IntFloor(origin.z);

    // Step direction for each axis: +1 or -1.
    int step_x = (dir.x >= 0.0f) ? 1 : -1;
    int step_y = (dir.y >= 0.0f) ? 1 : -1;
    int step_z = (dir.z >= 0.0f) ? 1 : -1;

    // tMax: distance along the ray to the next voxel boundary on each axis.
    // tDelta: distance along the ray to traverse one full voxel on each axis.
    constexpr float kInfinity = std::numeric_limits<float>::infinity();

    float t_delta_x = (dir.x != 0.0f) ? std::abs(1.0f / dir.x) : kInfinity;
    float t_delta_y = (dir.y != 0.0f) ? std::abs(1.0f / dir.y) : kInfinity;
    float t_delta_z = (dir.z != 0.0f) ? std::abs(1.0f / dir.z) : kInfinity;

    // Distance from origin to the first voxel boundary on each axis.
    float t_max_x, t_max_y, t_max_z;

    if (dir.x != 0.0f) {
        float boundary_x = (step_x > 0) ? (x + 1.0f) : static_cast<float>(x);
        t_max_x = (boundary_x - origin.x) / dir.x;
    } else {
        t_max_x = kInfinity;
    }

    if (dir.y != 0.0f) {
        float boundary_y = (step_y > 0) ? (y + 1.0f) : static_cast<float>(y);
        t_max_y = (boundary_y - origin.y) / dir.y;
    } else {
        t_max_y = kInfinity;
    }

    if (dir.z != 0.0f) {
        float boundary_z = (step_z > 0) ? (z + 1.0f) : static_cast<float>(z);
        t_max_z = (boundary_z - origin.z) / dir.z;
    } else {
        t_max_z = kInfinity;
    }

    // Track the face normal: which axis boundary we last crossed.
    // Initialize to zero (no face crossed yet for the starting block).
    glm::ivec3 face_normal(0);

    // Traverse the grid. The maximum number of steps is bounded by distance.
    // We use a generous limit to avoid infinite loops.
    float distance_traversed = 0.0f;

    while (distance_traversed <= max_distance) {
        // Check the current voxel.
        BlockId block_id = block_query(x, y, z);
        if (block_id != BlockRegistry::kAir && registry.GetBlock(block_id).solid) {
            result.hit = true;
            result.block_position = glm::ivec3(x, y, z);
            result.face_normal = face_normal;
            result.distance = distance_traversed;
            return result;
        }

        // Advance to the next voxel boundary (whichever axis is closest).
        if (t_max_x < t_max_y) {
            if (t_max_x < t_max_z) {
                // Step on X axis.
                distance_traversed = t_max_x;
                if (distance_traversed > max_distance) break;
                x += step_x;
                t_max_x += t_delta_x;
                face_normal = glm::ivec3(-step_x, 0, 0);
            } else {
                // Step on Z axis.
                distance_traversed = t_max_z;
                if (distance_traversed > max_distance) break;
                z += step_z;
                t_max_z += t_delta_z;
                face_normal = glm::ivec3(0, 0, -step_z);
            }
        } else {
            if (t_max_y < t_max_z) {
                // Step on Y axis.
                distance_traversed = t_max_y;
                if (distance_traversed > max_distance) break;
                y += step_y;
                t_max_y += t_delta_y;
                face_normal = glm::ivec3(0, -step_y, 0);
            } else {
                // Step on Z axis.
                distance_traversed = t_max_z;
                if (distance_traversed > max_distance) break;
                z += step_z;
                t_max_z += t_delta_z;
                face_normal = glm::ivec3(0, 0, -step_z);
            }
        }
    }

    return result;  // No hit found within max_distance.
}

}  // namespace vibecraft
