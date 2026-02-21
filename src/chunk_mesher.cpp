#include "vibecraft/chunk_mesher.h"

#include <glm/glm.hpp>

namespace vibecraft {

namespace {

/// The six face directions.
enum class Face { kPosX, kNegX, kPosY, kNegY, kPosZ, kNegZ };

/// Get the outward normal for a face.
glm::vec3 FaceNormal(Face face) {
    switch (face) {
        case Face::kPosX: return { 1.0f,  0.0f,  0.0f};
        case Face::kNegX: return {-1.0f,  0.0f,  0.0f};
        case Face::kPosY: return { 0.0f,  1.0f,  0.0f};
        case Face::kNegY: return { 0.0f, -1.0f,  0.0f};
        case Face::kPosZ: return { 0.0f,  0.0f,  1.0f};
        case Face::kNegZ: return { 0.0f,  0.0f, -1.0f};
    }
    return {0.0f, 0.0f, 0.0f};
}

/// Get the texture index for a specific face of a block type.
float FaceTexIndex(const BlockFaces& faces, Face face) {
    switch (face) {
        case Face::kPosX: return static_cast<float>(faces.pos_x);
        case Face::kNegX: return static_cast<float>(faces.neg_x);
        case Face::kPosY: return static_cast<float>(faces.pos_y);
        case Face::kNegY: return static_cast<float>(faces.neg_y);
        case Face::kPosZ: return static_cast<float>(faces.pos_z);
        case Face::kNegZ: return static_cast<float>(faces.neg_z);
    }
    return -1.0f;
}

/// Generate the 4 vertices for a face of a block at position (bx, by, bz).
/// Vertices are in chunk-local coordinates.
/// Each face is a unit quad on the appropriate side of the unit cube.
///
/// The vertex winding is counter-clockwise when viewed from outside the block
/// (standard front-face winding for OpenGL).
void EmitFace(MeshData& mesh, int bx, int by, int bz,
              Face face, float tex_index) {
    const float x = static_cast<float>(bx);
    const float y = static_cast<float>(by);
    const float z = static_cast<float>(bz);

    glm::vec3 normal = FaceNormal(face);
    const float ao = 1.0f;  // Stubbed for M6; proper AO in M24.

    // Base index for the 4 new vertices.
    uint32_t base = static_cast<uint32_t>(mesh.vertices.size());

    // UV corners: (0,0), (1,0), (1,1), (0,1) — bottom-left origin.
    glm::vec2 uv00{0.0f, 0.0f};
    glm::vec2 uv10{1.0f, 0.0f};
    glm::vec2 uv11{1.0f, 1.0f};
    glm::vec2 uv01{0.0f, 1.0f};

    // Define 4 corner positions and their UVs per face.
    // Convention: vertices listed counter-clockwise when viewed from outside.
    glm::vec3 v0, v1, v2, v3;

    switch (face) {
        case Face::kPosX:  // +X face (x+1 plane)
            v0 = {x + 1, y,     z};
            v1 = {x + 1, y,     z + 1};
            v2 = {x + 1, y + 1, z + 1};
            v3 = {x + 1, y + 1, z};
            break;
        case Face::kNegX:  // -X face (x plane)
            v0 = {x, y,     z + 1};
            v1 = {x, y,     z};
            v2 = {x, y + 1, z};
            v3 = {x, y + 1, z + 1};
            break;
        case Face::kPosY:  // +Y face (y+1 plane, top)
            v0 = {x,     y + 1, z};
            v1 = {x + 1, y + 1, z};
            v2 = {x + 1, y + 1, z + 1};
            v3 = {x,     y + 1, z + 1};
            break;
        case Face::kNegY:  // -Y face (y plane, bottom)
            v0 = {x,     y, z + 1};
            v1 = {x + 1, y, z + 1};
            v2 = {x + 1, y, z};
            v3 = {x,     y, z};
            break;
        case Face::kPosZ:  // +Z face (z+1 plane)
            v0 = {x + 1, y,     z + 1};
            v1 = {x,     y,     z + 1};
            v2 = {x,     y + 1, z + 1};
            v3 = {x + 1, y + 1, z + 1};
            break;
        case Face::kNegZ:  // -Z face (z plane)
            v0 = {x,     y,     z};
            v1 = {x + 1, y,     z};
            v2 = {x + 1, y + 1, z};
            v3 = {x,     y + 1, z};
            break;
    }

    mesh.vertices.push_back({v0, uv00, tex_index, normal, ao});
    mesh.vertices.push_back({v1, uv10, tex_index, normal, ao});
    mesh.vertices.push_back({v2, uv11, tex_index, normal, ao});
    mesh.vertices.push_back({v3, uv01, tex_index, normal, ao});

    // Two triangles: (0,1,2) and (0,2,3) — counter-clockwise.
    mesh.indices.push_back(base);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 3);
}

/// Get the block id of the neighbor in the given direction.
/// Handles chunk boundaries by consulting neighbor chunks.
/// If no neighbor chunk is available, returns kAir.
BlockId GetNeighborBlock(const Chunk& chunk, const NeighborData& neighbors,
                         int x, int y, int z, Face face) {
    int nx = x, ny = y, nz = z;
    switch (face) {
        case Face::kPosX: nx = x + 1; break;
        case Face::kNegX: nx = x - 1; break;
        case Face::kPosY: ny = y + 1; break;
        case Face::kNegY: ny = y - 1; break;
        case Face::kPosZ: nz = z + 1; break;
        case Face::kNegZ: nz = z - 1; break;
    }

    // Check Y bounds first — above or below chunk is always air.
    if (ny < 0 || ny >= kChunkSizeY) {
        return BlockRegistry::kAir;
    }

    // Check X bounds — consult neighbor chunks.
    if (nx < 0) {
        if (neighbors.neg_x) {
            return neighbors.neg_x->GetBlock(kChunkSizeX - 1, ny, nz);
        }
        return BlockRegistry::kAir;
    }
    if (nx >= kChunkSizeX) {
        if (neighbors.pos_x) {
            return neighbors.pos_x->GetBlock(0, ny, nz);
        }
        return BlockRegistry::kAir;
    }

    // Check Z bounds — consult neighbor chunks.
    if (nz < 0) {
        if (neighbors.neg_z) {
            return neighbors.neg_z->GetBlock(nx, ny, kChunkSizeZ - 1);
        }
        return BlockRegistry::kAir;
    }
    if (nz >= kChunkSizeZ) {
        if (neighbors.pos_z) {
            return neighbors.pos_z->GetBlock(nx, ny, 0);
        }
        return BlockRegistry::kAir;
    }

    // Within this chunk.
    return chunk.GetBlock(nx, ny, nz);
}

/// Determine whether a face should be emitted given the current block and
/// its neighbor.
///
/// Rules:
///   - Air blocks never emit faces.
///   - If the neighbor is air, always emit.
///   - Solid vs solid (both non-transparent): cull.
///   - Solid vs transparent neighbor: emit.
///   - Transparent vs non-transparent solid: emit.
///   - Transparent vs same-type transparent: cull.
///   - Transparent vs different-type transparent: emit.
///   - Non-solid non-transparent (shouldn't exist) vs anything: emit if neighbor is air.
bool ShouldEmitFace(BlockId current_id, BlockId neighbor_id,
                    const BlockRegistry& registry) {
    // Air blocks never produce geometry.
    if (current_id == BlockRegistry::kAir) {
        return false;
    }

    const BlockType& current = registry.GetBlock(current_id);
    const BlockType& neighbor = registry.GetBlock(neighbor_id);

    // If the current block is not solid and not transparent, skip
    // (e.g. torch — we don't mesh them in this pass).
    // Actually, we should still mesh non-solid transparent blocks like water.
    // Let's check: if current has no valid face textures, skip.
    // For simplicity, we emit faces for any non-air block.

    // Neighbor is air: always emit.
    if (neighbor_id == BlockRegistry::kAir) {
        return true;
    }

    // Current is transparent.
    if (current.transparent) {
        // Same block type: cull.
        if (current_id == neighbor_id) {
            return false;
        }
        // Neighbor is also transparent but different type: emit.
        if (neighbor.transparent) {
            return true;
        }
        // Neighbor is solid non-transparent: emit (we can see the face from
        // the transparent side).
        return true;
    }

    // Current is opaque (solid, non-transparent).
    if (neighbor.transparent) {
        // Neighbor is transparent: emit (we can see through).
        return true;
    }

    // Both opaque: cull the shared face.
    return false;
}

}  // namespace

MeshData BuildMesh(const Chunk& chunk, const NeighborData& neighbors,
                   const BlockRegistry& registry) {
    MeshData mesh;

    // Reserve a reasonable amount of memory to avoid frequent reallocations.
    // A chunk with moderate geometry might have ~10k faces.
    mesh.vertices.reserve(4096);
    mesh.indices.reserve(6144);

    static constexpr Face kAllFaces[] = {
        Face::kPosX, Face::kNegX,
        Face::kPosY, Face::kNegY,
        Face::kPosZ, Face::kNegZ,
    };

    for (int y = 0; y < kChunkSizeY; ++y) {
        for (int z = 0; z < kChunkSizeZ; ++z) {
            for (int x = 0; x < kChunkSizeX; ++x) {
                BlockId block_id = chunk.GetBlock(x, y, z);
                if (block_id == BlockRegistry::kAir) {
                    continue;
                }

                const BlockType& block_type = registry.GetBlock(block_id);

                for (Face face : kAllFaces) {
                    BlockId neighbor_id = GetNeighborBlock(
                        chunk, neighbors, x, y, z, face);

                    if (ShouldEmitFace(block_id, neighbor_id, registry)) {
                        float tex_idx = FaceTexIndex(block_type.faces, face);
                        EmitFace(mesh, x, y, z, face, tex_idx);
                    }
                }
            }
        }
    }

    return mesh;
}

}  // namespace vibecraft
