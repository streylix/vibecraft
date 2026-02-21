#ifndef VIBECRAFT_RAYCAST_H
#define VIBECRAFT_RAYCAST_H

#include <functional>

#include <glm/glm.hpp>

#include "vibecraft/block.h"
#include "vibecraft/player.h"

namespace vibecraft {

/// Result of a ray-voxel traversal.
struct RaycastResult {
    bool hit = false;                              ///< True if a solid block was hit.
    glm::ivec3 block_position = glm::ivec3(0);     ///< Block coordinates of the hit block.
    glm::ivec3 face_normal = glm::ivec3(0);        ///< Normal of the face that was hit.
    float distance = 0.0f;                          ///< Distance from origin to the hit.
};

/// Default maximum raycast distance (Minecraft-standard reach).
constexpr float kMaxRaycastDistance = 5.0f;

/// Cast a ray through the voxel grid using the DDA (Digital Differential
/// Analyzer) algorithm. Steps one voxel at a time along the ray.
///
/// @param origin        Ray origin in world coordinates.
/// @param direction     Ray direction (does not need to be normalized).
/// @param max_distance  Maximum distance to traverse.
/// @param block_query   Callback returning the BlockId at (x, y, z).
/// @param registry      Block registry for checking solid property.
/// @return RaycastResult with hit info and face normal.
RaycastResult CastRay(const glm::vec3& origin,
                      const glm::vec3& direction,
                      float max_distance,
                      const BlockQueryFunc& block_query,
                      const BlockRegistry& registry);

}  // namespace vibecraft

#endif  // VIBECRAFT_RAYCAST_H
