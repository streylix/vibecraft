#include "vibecraft/aabb.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace vibecraft {

AABB::AABB(const glm::vec3& min, const glm::vec3& max)
    : min(min), max(max) {}

bool AABB::Overlaps(const AABB& other) const {
    // Open intervals: touching (equal edges) does NOT count as overlap.
    return (min.x < other.max.x && max.x > other.min.x) &&
           (min.y < other.max.y && max.y > other.min.y) &&
           (min.z < other.max.z && max.z > other.min.z);
}

bool AABB::Contains(const glm::vec3& point) const {
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

bool AABB::RayIntersect(const glm::vec3& origin, const glm::vec3& dir,
                        float& t_out) const {
    // If the ray origin is inside the box, intersection at t=0.
    if (Contains(origin)) {
        t_out = 0.0f;
        return true;
    }

    float t_min = 0.0f;
    float t_max = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < 1e-8f) {
            // Ray is parallel to slab. No hit if origin outside slab.
            if (origin[i] < min[i] || origin[i] > max[i]) {
                return false;
            }
        } else {
            float inv_d = 1.0f / dir[i];
            float t1 = (min[i] - origin[i]) * inv_d;
            float t2 = (max[i] - origin[i]) * inv_d;

            if (t1 > t2) std::swap(t1, t2);

            t_min = std::max(t_min, t1);
            t_max = std::min(t_max, t2);

            if (t_min > t_max) {
                return false;
            }
        }
    }

    if (t_min < 0.0f) {
        return false;
    }

    t_out = t_min;
    return true;
}

SweptResult AABB::Sweep(const glm::vec3& velocity, const AABB& other) const {
    SweptResult result;
    result.t = 1.0f;
    result.normal = glm::vec3(0.0f);

    // If already overlapping, return immediate collision.
    if (Overlaps(other)) {
        result.t = 0.0f;
        return result;
    }

    float entry_time = -std::numeric_limits<float>::max();
    float exit_time = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i) {
        if (std::abs(velocity[i]) < 1e-8f) {
            // Not moving on this axis — check if already overlapping on this axis.
            if (max[i] <= other.min[i] || min[i] >= other.max[i]) {
                // No overlap on this axis and not moving toward it: no collision.
                return result;  // t = 1.0, no collision
            }
            // Overlapping on this axis for all time; doesn't constrain entry/exit.
        } else {
            float inv_v = 1.0f / velocity[i];
            float t_enter = (other.min[i] - max[i]) * inv_v;
            float t_exit = (other.max[i] - min[i]) * inv_v;

            if (t_enter > t_exit) {
                std::swap(t_enter, t_exit);
            }

            if (t_enter > entry_time) {
                entry_time = t_enter;
                // Determine which face we entered from.
                result.normal = glm::vec3(0.0f);
                result.normal[i] = velocity[i] > 0.0f ? -1.0f : 1.0f;
            }

            exit_time = std::min(exit_time, t_exit);
        }
    }

    // Must enter before exit, and entry must be in [0, 1].
    if (entry_time < exit_time && entry_time >= 0.0f && entry_time < 1.0f) {
        result.t = entry_time;
    } else {
        result.t = 1.0f;
        result.normal = glm::vec3(0.0f);
    }

    return result;
}

}  // namespace vibecraft
