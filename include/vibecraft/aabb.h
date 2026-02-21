#ifndef VIBECRAFT_AABB_H
#define VIBECRAFT_AABB_H

#include <glm/glm.hpp>

namespace vibecraft {

/// Result of a swept AABB collision test.
struct SweptResult {
    float t;            ///< Time of collision in [0, 1]. 1.0 means no collision.
    glm::vec3 normal;   ///< Surface normal at collision point (zero if no collision).
};

/// Axis-Aligned Bounding Box stored as min/max corners.
class AABB {
public:
    glm::vec3 min;
    glm::vec3 max;

    /// Construct an AABB from min and max corners.
    AABB(const glm::vec3& min, const glm::vec3& max);

    /// Test whether this AABB overlaps another (open intervals — touching faces
    /// do NOT count as overlapping).
    bool Overlaps(const AABB& other) const;

    /// Test whether a point is strictly inside this AABB (closed interval).
    bool Contains(const glm::vec3& point) const;

    /// Ray-AABB intersection using the slab method.
    /// @param origin   Ray origin.
    /// @param dir      Ray direction (does not need to be normalized).
    /// @param t_out    Output: distance along ray to the nearest intersection.
    /// @return true if the ray intersects this AABB (t >= 0).
    bool RayIntersect(const glm::vec3& origin, const glm::vec3& dir,
                      float& t_out) const;

    /// Swept AABB collision: move this box by `velocity` and test against a
    /// stationary `other` box. Returns the fraction of velocity at which
    /// collision occurs and the collision normal.
    SweptResult Sweep(const glm::vec3& velocity, const AABB& other) const;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_AABB_H
