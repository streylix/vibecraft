#ifndef VIBECRAFT_CHUNK_MESHER_H
#define VIBECRAFT_CHUNK_MESHER_H

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/mesh.h"

namespace vibecraft {

/// Optional pointers to the four horizontal neighbor chunks.
/// Used for face culling at chunk boundaries.
/// A null pointer means "assume air" for that neighbor.
struct NeighborData {
    const Chunk* pos_x = nullptr;  // +X neighbor (x = 15 border)
    const Chunk* neg_x = nullptr;  // -X neighbor (x = 0 border)
    const Chunk* pos_z = nullptr;  // +Z neighbor (z = 15 border)
    const Chunk* neg_z = nullptr;  // -Z neighbor (z = 0 border)
};

/// Build a renderable mesh from a chunk's block data (naive per-face).
///
/// For each non-air block, checks all 6 face directions. A face is emitted
/// only when the adjacent block does not occlude it:
///   - Solid vs air: face emitted
///   - Solid vs transparent: face emitted
///   - Transparent vs air: face emitted
///   - Transparent vs same-type transparent: face culled
///   - Transparent vs different-type transparent: face emitted
///   - Solid vs solid: face culled
///
/// At chunk boundaries, neighbor data is consulted. If no neighbor is
/// provided, air is assumed (face emitted).
///
/// Generates indexed mesh data (vertices + indices) with triangle topology.
/// Each face is 2 triangles = 4 vertices + 6 indices.
///
/// AO values are stubbed to 1.0 (proper AO is M24).
///
/// @param chunk     The chunk to mesh.
/// @param neighbors Neighbor chunk data for boundary face culling.
/// @param registry  Block type registry for property lookups.
/// @return MeshData containing vertices and indices.
MeshData BuildMesh(const Chunk& chunk, const NeighborData& neighbors,
                   const BlockRegistry& registry);

/// Build a renderable mesh using greedy meshing.
///
/// Same face-culling rules as BuildMesh, but merges coplanar adjacent faces
/// of the same block type (and same AO value) into larger quads, dramatically
/// reducing vertex count.
///
/// UV coordinates tile across merged faces: a quad spanning W blocks wide
/// and H blocks tall will have U range [0, W] and V range [0, H].
///
/// @param chunk     The chunk to mesh.
/// @param neighbors Neighbor chunk data for boundary face culling.
/// @param registry  Block type registry for property lookups.
/// @return MeshData containing vertices and indices.
MeshData BuildGreedyMesh(const Chunk& chunk, const NeighborData& neighbors,
                         const BlockRegistry& registry);

}  // namespace vibecraft

#endif  // VIBECRAFT_CHUNK_MESHER_H
