#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <set>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/chunk_mesher.h"
#include "vibecraft/mesh.h"

// M6: Chunk Meshing v1 (Face Culling)

using namespace vibecraft;

// ---------------------------------------------------------------------------
// Helper: count faces in a mesh (each face = 4 vertices = 6 indices).
// ---------------------------------------------------------------------------
static int CountFaces(const MeshData& mesh) {
    // Each face contributes 4 vertices and 6 indices.
    return static_cast<int>(mesh.indices.size()) / 6;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST(ChunkMesher, EmptyChunkZeroVerts) {
    Chunk chunk(0, 0);  // All air by default.
    BlockRegistry registry;
    NeighborData neighbors;

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    EXPECT_EQ(mesh.vertices.size(), 0u);
    EXPECT_EQ(mesh.indices.size(), 0u);
}

TEST(ChunkMesher, SingleBlockCenter) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Place a single stone block in the center of the chunk (not at any edge).
    chunk.SetBlock(8, 128, 8, BlockRegistry::kStone);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // 6 faces, each with 4 vertices and 6 indices.
    EXPECT_EQ(CountFaces(mesh), 6);
    EXPECT_EQ(mesh.vertices.size(), 24u);   // 6 faces * 4 verts
    EXPECT_EQ(mesh.indices.size(), 36u);    // 6 faces * 6 indices
}

TEST(ChunkMesher, TwoAdjacentBlocksCullSharedFace) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Two stone blocks adjacent along the X axis.
    chunk.SetBlock(8, 128, 8, BlockRegistry::kStone);
    chunk.SetBlock(9, 128, 8, BlockRegistry::kStone);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // 2 blocks * 6 faces - 2 culled faces (the shared +X/-X face) = 10 faces.
    EXPECT_EQ(CountFaces(mesh), 10);
    EXPECT_EQ(mesh.vertices.size(), 40u);   // 10 * 4
    EXPECT_EQ(mesh.indices.size(), 60u);    // 10 * 6
}

TEST(ChunkMesher, FullLayerCulling) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;  // No neighbors — assume air outside.

    // Fill the entire y=0 layer with stone.
    for (int z = 0; z < kChunkSizeZ; ++z) {
        for (int x = 0; x < kChunkSizeX; ++x) {
            chunk.SetBlock(x, 0, z, BlockRegistry::kStone);
        }
    }

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // Top faces: 16*16 = 256  (all blocks have air above)
    // Bottom faces: 16*16 = 256  (y=0, below is out-of-bounds -> air)
    // Edge side faces:
    //   +X edge (x=15): 16 faces
    //   -X edge (x=0):  16 faces
    //   +Z edge (z=15): 16 faces
    //   -Z edge (z=0):  16 faces
    //   = 64 side faces
    // Total: 256 + 256 + 64 = 576 faces
    int expected_faces = 256 + 256 + 64;
    EXPECT_EQ(CountFaces(mesh), expected_faces);
}

TEST(ChunkMesher, TransparentAdjacentToSolid) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Place stone and glass adjacent along X.
    chunk.SetBlock(8, 128, 8, BlockRegistry::kStone);
    chunk.SetBlock(9, 128, 8, BlockRegistry::kGlass);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // Stone has 6 faces; normally +X would be culled if neighbor were solid+opaque.
    // But glass is transparent, so the stone +X face IS emitted.
    // Glass has 6 faces; its -X face is toward stone, and since glass is
    // transparent adjacent to a solid, it IS emitted.
    // So both blocks emit all 6 faces each = 12 faces.
    EXPECT_EQ(CountFaces(mesh), 12);
}

TEST(ChunkMesher, TransparentAdjacentToAir) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Single glass block in center — surrounded by air.
    chunk.SetBlock(8, 128, 8, BlockRegistry::kGlass);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // All 6 faces emitted (transparent next to air).
    EXPECT_EQ(CountFaces(mesh), 6);
}

TEST(ChunkMesher, TransparentAdjacentToTransparent) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Two glass blocks adjacent along X — same type.
    chunk.SetBlock(8, 128, 8, BlockRegistry::kGlass);
    chunk.SetBlock(9, 128, 8, BlockRegistry::kGlass);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // Same-type transparent: shared face is culled.
    // 2 * 6 - 2 = 10 faces.
    EXPECT_EQ(CountFaces(mesh), 10);
}

TEST(ChunkMesher, CorrectNormals) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Single block in center — all 6 faces visible.
    chunk.SetBlock(8, 128, 8, BlockRegistry::kStone);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // Collect the unique normals from the mesh. Each face has 4 vertices
    // with the same normal.
    std::set<std::tuple<float, float, float>> normals;
    for (const auto& v : mesh.vertices) {
        normals.insert({v.normal.x, v.normal.y, v.normal.z});
    }

    // Should have exactly 6 unique normals.
    EXPECT_EQ(normals.size(), 6u);

    // Verify each expected normal is present.
    EXPECT_TRUE(normals.count({ 1.0f,  0.0f,  0.0f}));  // +X
    EXPECT_TRUE(normals.count({-1.0f,  0.0f,  0.0f}));  // -X
    EXPECT_TRUE(normals.count({ 0.0f,  1.0f,  0.0f}));  // +Y
    EXPECT_TRUE(normals.count({ 0.0f, -1.0f,  0.0f}));  // -Y
    EXPECT_TRUE(normals.count({ 0.0f,  0.0f,  1.0f}));  // +Z
    EXPECT_TRUE(normals.count({ 0.0f,  0.0f, -1.0f}));  // -Z
}

TEST(ChunkMesher, CorrectUVs) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    chunk.SetBlock(8, 128, 8, BlockRegistry::kStone);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // All UV components must be in [0, 1].
    for (const auto& v : mesh.vertices) {
        EXPECT_GE(v.tex_coord.x, 0.0f);
        EXPECT_LE(v.tex_coord.x, 1.0f);
        EXPECT_GE(v.tex_coord.y, 0.0f);
        EXPECT_LE(v.tex_coord.y, 1.0f);
    }
}

TEST(ChunkMesher, CorrectTexIndices) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Place a grass block — it has different textures for top, bottom, side.
    chunk.SetBlock(8, 128, 8, BlockRegistry::kGrass);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // Grass block faces:
    //   top (+Y):    tex index 1 (grass_top)
    //   bottom (-Y): tex index 3 (dirt)
    //   sides:       tex index 2 (grass_side)

    // Group vertices by normal to identify faces.
    for (const auto& v : mesh.vertices) {
        if (v.normal.y > 0.5f) {
            // +Y (top) face.
            EXPECT_FLOAT_EQ(v.tex_index, 1.0f) << "Grass top should use tex index 1";
        } else if (v.normal.y < -0.5f) {
            // -Y (bottom) face.
            EXPECT_FLOAT_EQ(v.tex_index, 3.0f) << "Grass bottom should use tex index 3";
        } else {
            // Side faces.
            EXPECT_FLOAT_EQ(v.tex_index, 2.0f) << "Grass sides should use tex index 2";
        }
    }
}

TEST(ChunkMesher, BoundaryFacesWithoutNeighbor) {
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;  // All null — no neighbors.

    // Place a block at the +X edge of the chunk.
    chunk.SetBlock(15, 128, 8, BlockRegistry::kStone);

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // With no neighbor, assume air outside. All 6 faces should be emitted.
    EXPECT_EQ(CountFaces(mesh), 6);

    // Verify the +X face exists by checking normals.
    bool has_pos_x_face = false;
    for (const auto& v : mesh.vertices) {
        if (v.normal.x > 0.5f) {
            has_pos_x_face = true;
            break;
        }
    }
    EXPECT_TRUE(has_pos_x_face) << "+X boundary face should be generated when no neighbor";
}

TEST(ChunkMesher, BoundaryFacesWithNeighbor) {
    Chunk chunk(0, 0);
    BlockRegistry registry;

    // Place a block at the +X edge of the chunk.
    chunk.SetBlock(15, 128, 8, BlockRegistry::kStone);

    // Create a neighbor chunk with a solid block at x=0 (adjacent to our x=15).
    Chunk neighbor_chunk(1, 0);
    neighbor_chunk.SetBlock(0, 128, 8, BlockRegistry::kStone);

    NeighborData neighbors;
    neighbors.pos_x = &neighbor_chunk;

    MeshData mesh = BuildMesh(chunk, neighbors, registry);

    // The +X face should be culled because the neighbor has a solid block there.
    // So only 5 faces should be emitted.
    EXPECT_EQ(CountFaces(mesh), 5);

    // Verify no +X face normal exists.
    bool has_pos_x_face = false;
    for (const auto& v : mesh.vertices) {
        if (v.normal.x > 0.5f) {
            has_pos_x_face = true;
            break;
        }
    }
    EXPECT_FALSE(has_pos_x_face) << "+X boundary face should be culled when neighbor is solid";
}
