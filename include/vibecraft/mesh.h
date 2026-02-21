#ifndef VIBECRAFT_MESH_H
#define VIBECRAFT_MESH_H

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace vibecraft {

/// Vertex format for chunk meshes.
/// Total: 10 floats = 40 bytes per vertex.
struct Vertex {
    glm::vec3 position;    // Block-local position within the chunk
    glm::vec2 tex_coord;   // UV within atlas tile [0,1]
    float tex_index;       // Atlas tile index
    glm::vec3 normal;      // Face normal
    float ao;              // Ambient occlusion (0.0-1.0)
};

/// Container for mesh data: vertices and indices forming a triangle mesh.
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    /// Return true if the mesh has no geometry.
    bool Empty() const { return vertices.empty(); }

    /// Clear all vertex and index data.
    void Clear() {
        vertices.clear();
        indices.clear();
    }
};

}  // namespace vibecraft

#endif  // VIBECRAFT_MESH_H
