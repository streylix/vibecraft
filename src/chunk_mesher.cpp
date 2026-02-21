#include "vibecraft/chunk_mesher.h"

#include <vector>

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

    // Two triangles: (0,2,1) and (0,3,2) — corrected winding so the
    // outward normal matches the CCW front-face convention.
    mesh.indices.push_back(base);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base);
    mesh.indices.push_back(base + 3);
    mesh.indices.push_back(base + 2);
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

/// Emit a greedy-merged quad.
/// The quad lies on a plane perpendicular to the face direction.
/// (u_start, v_start) is the starting corner in the 2D slice.
/// (u_span, v_span) is the width/height of the merged rectangle.
/// slice_coord is the position along the face's normal axis.
///
/// The mapping of (slice_coord, u, v) to (x, y, z) depends on the face.
void EmitGreedyQuad(MeshData& mesh, Face face, int slice_coord,
                    int u_start, int v_start, int u_span, int v_span,
                    float tex_index, float ao) {
    glm::vec3 normal = FaceNormal(face);
    uint32_t base = static_cast<uint32_t>(mesh.vertices.size());

    float fu = static_cast<float>(u_start);
    float fv = static_cast<float>(v_start);
    float fw = static_cast<float>(u_span);
    float fh = static_cast<float>(v_span);
    float fs = static_cast<float>(slice_coord);

    // Tiled UVs: range [0, u_span] x [0, v_span].
    glm::vec2 uv00{0.0f, 0.0f};
    glm::vec2 uv10{fw, 0.0f};
    glm::vec2 uv11{fw, fh};
    glm::vec2 uv01{0.0f, fh};

    glm::vec3 v0, v1, v2, v3;

    // For each face, map (slice, u, v) back to (x, y, z).
    // The face sits at slice_coord (or slice_coord+1 for positive faces).
    switch (face) {
        case Face::kPosX:  // axis = X, u = Z, v = Y; face at x = slice_coord + 1
            v0 = {fs + 1, fv,      fu};
            v1 = {fs + 1, fv,      fu + fw};
            v2 = {fs + 1, fv + fh, fu + fw};
            v3 = {fs + 1, fv + fh, fu};
            break;
        case Face::kNegX:  // axis = X, u = Z, v = Y; face at x = slice_coord
            v0 = {fs, fv,      fu + fw};
            v1 = {fs, fv,      fu};
            v2 = {fs, fv + fh, fu};
            v3 = {fs, fv + fh, fu + fw};
            break;
        case Face::kPosY:  // axis = Y, u = X, v = Z; face at y = slice_coord + 1
            v0 = {fu,      fs + 1, fv};
            v1 = {fu + fw, fs + 1, fv};
            v2 = {fu + fw, fs + 1, fv + fh};
            v3 = {fu,      fs + 1, fv + fh};
            break;
        case Face::kNegY:  // axis = Y, u = X, v = Z; face at y = slice_coord
            v0 = {fu,      fs, fv + fh};
            v1 = {fu + fw, fs, fv + fh};
            v2 = {fu + fw, fs, fv};
            v3 = {fu,      fs, fv};
            break;
        case Face::kPosZ:  // axis = Z, u = X, v = Y; face at z = slice_coord + 1
            v0 = {fu + fw, fv,      fs + 1};
            v1 = {fu,      fv,      fs + 1};
            v2 = {fu,      fv + fh, fs + 1};
            v3 = {fu + fw, fv + fh, fs + 1};
            break;
        case Face::kNegZ:  // axis = Z, u = X, v = Y; face at z = slice_coord
            v0 = {fu,      fv,      fs};
            v1 = {fu + fw, fv,      fs};
            v2 = {fu + fw, fv + fh, fs};
            v3 = {fu,      fv + fh, fs};
            break;
    }

    mesh.vertices.push_back({v0, uv00, tex_index, normal, ao});
    mesh.vertices.push_back({v1, uv10, tex_index, normal, ao});
    mesh.vertices.push_back({v2, uv11, tex_index, normal, ao});
    mesh.vertices.push_back({v3, uv01, tex_index, normal, ao});

    // Corrected winding: (0,2,1) and (0,3,2) so outward normal is CCW.
    mesh.indices.push_back(base);
    mesh.indices.push_back(base + 2);
    mesh.indices.push_back(base + 1);
    mesh.indices.push_back(base);
    mesh.indices.push_back(base + 3);
    mesh.indices.push_back(base + 2);
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

MeshData BuildGreedyMesh(const Chunk& chunk, const NeighborData& neighbors,
                         const BlockRegistry& registry) {
    MeshData mesh;
    mesh.vertices.reserve(4096);
    mesh.indices.reserve(6144);

    // Mask entry: block id for the face to emit (0 = no face).
    // We also track AO per mask cell so different AO values break merges.
    struct MaskEntry {
        BlockId block_id = 0;
        float tex_index = -1.0f;
        float ao = 1.0f;

        bool operator==(const MaskEntry& other) const {
            return block_id == other.block_id &&
                   tex_index == other.tex_index &&
                   ao == other.ao;
        }
        bool Empty() const { return block_id == 0; }
    };

    static constexpr Face kAllFaces[] = {
        Face::kPosX, Face::kNegX,
        Face::kPosY, Face::kNegY,
        Face::kPosZ, Face::kNegZ,
    };

    // For each face direction, sweep slices perpendicular to that axis.
    for (Face face : kAllFaces) {
        // Determine axis dimensions.
        // axis_size = extent along the face normal direction.
        // u_size, v_size = extent of the 2D slice.
        int axis_size, u_size, v_size;

        switch (face) {
            case Face::kPosX:
            case Face::kNegX:
                axis_size = kChunkSizeX;
                u_size = kChunkSizeZ;
                v_size = kChunkSizeY;
                break;
            case Face::kPosY:
            case Face::kNegY:
                axis_size = kChunkSizeY;
                u_size = kChunkSizeX;
                v_size = kChunkSizeZ;
                break;
            case Face::kPosZ:
            case Face::kNegZ:
                axis_size = kChunkSizeZ;
                u_size = kChunkSizeX;
                v_size = kChunkSizeY;
                break;
        }

        // Allocate mask for a single slice.
        std::vector<MaskEntry> mask(u_size * v_size);

        for (int slice = 0; slice < axis_size; ++slice) {
            // Build the mask: for each (u, v) position in the slice,
            // determine if a face should be emitted.
            for (int v = 0; v < v_size; ++v) {
                for (int u = 0; u < u_size; ++u) {
                    // Map (slice, u, v) back to (x, y, z).
                    int x, y, z;
                    switch (face) {
                        case Face::kPosX:
                        case Face::kNegX:
                            x = slice; z = u; y = v;
                            break;
                        case Face::kPosY:
                        case Face::kNegY:
                            y = slice; x = u; z = v;
                            break;
                        case Face::kPosZ:
                        case Face::kNegZ:
                            z = slice; x = u; y = v;
                            break;
                    }

                    MaskEntry& entry = mask[v * u_size + u];
                    entry = MaskEntry{};  // Reset.

                    BlockId block_id = chunk.GetBlock(x, y, z);
                    if (block_id == BlockRegistry::kAir) {
                        continue;
                    }

                    BlockId neighbor_id = GetNeighborBlock(
                        chunk, neighbors, x, y, z, face);

                    if (ShouldEmitFace(block_id, neighbor_id, registry)) {
                        const BlockType& block_type = registry.GetBlock(block_id);
                        float tex_idx = FaceTexIndex(block_type.faces, face);
                        entry.block_id = block_id;
                        entry.tex_index = tex_idx;
                        entry.ao = 1.0f;  // Stubbed; proper AO in M24.
                    }
                }
            }

            // Greedy merge: scan the mask and find maximal rectangles.
            for (int v = 0; v < v_size; ++v) {
                for (int u = 0; u < u_size; ) {
                    MaskEntry& current = mask[v * u_size + u];
                    if (current.Empty()) {
                        ++u;
                        continue;
                    }

                    // Find the width: extend u while the mask matches.
                    int width = 1;
                    while (u + width < u_size &&
                           mask[v * u_size + u + width] == current) {
                        ++width;
                    }

                    // Find the height: extend v while all rows match.
                    int height = 1;
                    bool done = false;
                    while (v + height < v_size && !done) {
                        for (int du = 0; du < width; ++du) {
                            if (!(mask[(v + height) * u_size + u + du] == current)) {
                                done = true;
                                break;
                            }
                        }
                        if (!done) {
                            ++height;
                        }
                    }

                    // Emit the merged quad.
                    EmitGreedyQuad(mesh, face, slice,
                                   u, v, width, height,
                                   current.tex_index, current.ao);

                    // Clear the mask for the merged region.
                    for (int dv = 0; dv < height; ++dv) {
                        for (int du = 0; du < width; ++du) {
                            mask[(v + dv) * u_size + u + du] = MaskEntry{};
                        }
                    }

                    u += width;
                }
            }
        }
    }

    return mesh;
}

}  // namespace vibecraft
