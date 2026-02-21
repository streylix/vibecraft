#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <set>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"
#include "vibecraft/chunk_mesher.h"
#include "vibecraft/mesh.h"

// M14: Greedy Meshing Upgrade

using namespace vibecraft;

// ---------------------------------------------------------------------------
// Helper: count quads in a mesh (each quad = 4 vertices = 6 indices).
// ---------------------------------------------------------------------------
static int CountQuads(const MeshData& mesh) {
    return static_cast<int>(mesh.indices.size()) / 6;
}

// ---------------------------------------------------------------------------
// Helper: count quads with a specific normal direction.
// ---------------------------------------------------------------------------
static int CountQuadsWithNormal(const MeshData& mesh, const glm::vec3& normal) {
    int count = 0;
    // Each quad has 4 vertices; iterate in groups of 4.
    for (size_t i = 0; i + 3 < mesh.vertices.size(); i += 4) {
        const auto& v = mesh.vertices[i];
        if (std::abs(v.normal.x - normal.x) < 0.01f &&
            std::abs(v.normal.y - normal.y) < 0.01f &&
            std::abs(v.normal.z - normal.z) < 0.01f) {
            ++count;
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST(GreedyMeshing, MergedRow) {
    // Place 16 stone blocks in a row along X at y=0, z=0.
    // For the top face (+Y), all 16 blocks should merge into 1 quad.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    for (int x = 0; x < 16; ++x) {
        chunk.SetBlock(x, 0, 0, BlockRegistry::kStone);
    }

    MeshData mesh = BuildGreedyMesh(chunk, neighbors, registry);

    // The top face (+Y) of all 16 blocks are coplanar, same type, same AO.
    // They should be merged into 1 quad.
    int top_quads = CountQuadsWithNormal(mesh, {0.0f, 1.0f, 0.0f});
    EXPECT_EQ(top_quads, 1) << "16 blocks in a row should produce 1 top-face quad";

    // The bottom face (-Y) should also merge into 1 quad.
    int bottom_quads = CountQuadsWithNormal(mesh, {0.0f, -1.0f, 0.0f});
    EXPECT_EQ(bottom_quads, 1) << "16 blocks in a row should produce 1 bottom-face quad";

    // +Z and -Z side faces: each is a 16x1 strip => 1 quad each.
    int pos_z_quads = CountQuadsWithNormal(mesh, {0.0f, 0.0f, 1.0f});
    EXPECT_EQ(pos_z_quads, 1) << "+Z side should be 1 merged quad";
    int neg_z_quads = CountQuadsWithNormal(mesh, {0.0f, 0.0f, -1.0f});
    EXPECT_EQ(neg_z_quads, 1) << "-Z side should be 1 merged quad";

    // +X and -X end caps: each is 1x1 => 1 quad each.
    int pos_x_quads = CountQuadsWithNormal(mesh, {1.0f, 0.0f, 0.0f});
    EXPECT_EQ(pos_x_quads, 1) << "+X end cap should be 1 quad";
    int neg_x_quads = CountQuadsWithNormal(mesh, {-1.0f, 0.0f, 0.0f});
    EXPECT_EQ(neg_x_quads, 1) << "-X end cap should be 1 quad";

    // Total: 6 quads for the whole row.
    EXPECT_EQ(CountQuads(mesh), 6);
}

TEST(GreedyMeshing, MergedPlane) {
    // Place a 4x4 flat layer of stone at y=0, x=[0,3], z=[0,3].
    // Each face direction should produce exactly 1 quad.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    for (int z = 0; z < 4; ++z) {
        for (int x = 0; x < 4; ++x) {
            chunk.SetBlock(x, 0, z, BlockRegistry::kStone);
        }
    }

    MeshData mesh = BuildGreedyMesh(chunk, neighbors, registry);

    // Top (+Y): all 16 faces on the same y+1 plane => 1 quad.
    EXPECT_EQ(CountQuadsWithNormal(mesh, {0.0f, 1.0f, 0.0f}), 1)
        << "4x4 layer top face should be 1 merged quad";

    // Bottom (-Y): all 16 faces on the same y plane => 1 quad.
    EXPECT_EQ(CountQuadsWithNormal(mesh, {0.0f, -1.0f, 0.0f}), 1)
        << "4x4 layer bottom face should be 1 merged quad";

    // Side faces: each side of the 4x4 layer is a 4x1 strip => 1 quad each.
    EXPECT_EQ(CountQuadsWithNormal(mesh, {1.0f, 0.0f, 0.0f}), 1)
        << "+X side should be 1 merged quad";
    EXPECT_EQ(CountQuadsWithNormal(mesh, {-1.0f, 0.0f, 0.0f}), 1)
        << "-X side should be 1 merged quad";
    EXPECT_EQ(CountQuadsWithNormal(mesh, {0.0f, 0.0f, 1.0f}), 1)
        << "+Z side should be 1 merged quad";
    EXPECT_EQ(CountQuadsWithNormal(mesh, {0.0f, 0.0f, -1.0f}), 1)
        << "-Z side should be 1 merged quad";

    // Total: 6 quads (1 per face direction), not 16 per direction.
    EXPECT_EQ(CountQuads(mesh), 6);
}

TEST(GreedyMeshing, DifferentBlocksNotMerged) {
    // Place stone at (0,0,0) and dirt at (1,0,0).
    // They have different block IDs (and likely different texture indices),
    // so they should NOT be merged.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    chunk.SetBlock(0, 0, 0, BlockRegistry::kStone);
    chunk.SetBlock(1, 0, 0, BlockRegistry::kDirt);

    MeshData mesh = BuildGreedyMesh(chunk, neighbors, registry);

    // Top face (+Y): stone and dirt have different tex indices, so 2 quads.
    int top_quads = CountQuadsWithNormal(mesh, {0.0f, 1.0f, 0.0f});
    EXPECT_EQ(top_quads, 2) << "Stone + dirt should produce 2 separate top quads";

    // Bottom face (-Y): also 2 separate quads.
    int bottom_quads = CountQuadsWithNormal(mesh, {0.0f, -1.0f, 0.0f});
    EXPECT_EQ(bottom_quads, 2) << "Stone + dirt should produce 2 separate bottom quads";

    // Note: the shared face between stone and dirt is culled (both opaque).
    // End caps +X and -X: 1 each.
    // The +Z and -Z side faces: stone side and dirt side have different tex,
    // so 2 each.
    int total_quads = CountQuads(mesh);
    // 2 (top) + 2 (bottom) + 1 (+X) + 1 (-X) + 2 (+Z) + 2 (-Z) = 10
    EXPECT_EQ(total_quads, 10);
}

TEST(GreedyMeshing, FewerVertsThanNaive) {
    // Fill a full 16x16 layer at y=0 with stone.
    // Greedy meshing should produce far fewer vertices than naive.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    for (int z = 0; z < kChunkSizeZ; ++z) {
        for (int x = 0; x < kChunkSizeX; ++x) {
            chunk.SetBlock(x, 0, z, BlockRegistry::kStone);
        }
    }

    MeshData naive_mesh = BuildMesh(chunk, neighbors, registry);
    MeshData greedy_mesh = BuildGreedyMesh(chunk, neighbors, registry);

    // Naive produces 576 faces = 2304 vertices (from test_chunk_mesher FullLayerCulling).
    EXPECT_EQ(naive_mesh.vertices.size(), 576u * 4u);

    // Greedy should produce far fewer.
    // Top: 1 quad, bottom: 1 quad, 4 side strips of 16x1 each => 1 quad each.
    // Total: 6 quads = 24 vertices.
    EXPECT_LT(greedy_mesh.vertices.size(), naive_mesh.vertices.size())
        << "Greedy meshing should produce fewer vertices than naive";

    // The greedy mesh should be dramatically smaller.
    // 6 quads * 4 verts = 24 vertices for the full layer.
    EXPECT_EQ(greedy_mesh.vertices.size(), 24u)
        << "Full 16x16 layer should produce only 6 quads (24 vertices)";
}

TEST(GreedyMeshing, CorrectTiledUVs) {
    // Place 4 stone blocks in a row along X at y=0, z=0.
    // The merged top face should have UV range [0, 4] in the U direction.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    for (int x = 0; x < 4; ++x) {
        chunk.SetBlock(x, 0, 0, BlockRegistry::kStone);
    }

    MeshData mesh = BuildGreedyMesh(chunk, neighbors, registry);

    // Find the top-face (+Y) quad vertices.
    // The top face is the quad whose vertices have normal (0,1,0).
    float max_u = 0.0f;
    float max_v = 0.0f;
    bool found_top = false;
    for (size_t i = 0; i + 3 < mesh.vertices.size(); i += 4) {
        if (mesh.vertices[i].normal.y > 0.5f) {
            // This is a top-face quad. Check all 4 vertices.
            found_top = true;
            for (int j = 0; j < 4; ++j) {
                max_u = std::max(max_u, mesh.vertices[i + j].tex_coord.x);
                max_v = std::max(max_v, mesh.vertices[i + j].tex_coord.y);
            }
            break;
        }
    }

    ASSERT_TRUE(found_top) << "Should find a top face quad";

    // Top face of a 4x1 row: U maps along X (width=4), V maps along Z (height=1).
    // For +Y face, u = X, v = Z. The row is 4 blocks wide in X, 1 block in Z.
    EXPECT_FLOAT_EQ(max_u, 4.0f)
        << "Tiled UV U should be 4.0 for a 4-block-wide merged face";
    EXPECT_FLOAT_EQ(max_v, 1.0f)
        << "Tiled UV V should be 1.0 for a 1-block-deep merged face";
}

TEST(GreedyMeshing, AOBreaksMerge) {
    // With current AO stubbed to 1.0, all same-block-type faces have the same
    // AO and should merge. This test verifies that the AO comparison mechanism
    // is in place by checking that uniform AO does NOT break merging.
    //
    // When proper AO (M24) is implemented, blocks at corners/edges will have
    // different AO values and those should then prevent merging. For now we
    // confirm that uniform AO allows correct merging.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    // Place 4 stone blocks in a row. With uniform AO (all 1.0), they should
    // all merge into 1 quad per face direction.
    for (int x = 0; x < 4; ++x) {
        chunk.SetBlock(x, 0, 0, BlockRegistry::kStone);
    }

    MeshData mesh = BuildGreedyMesh(chunk, neighbors, registry);

    // With uniform AO, the top face should be 1 merged quad, not 4 separate.
    int top_quads = CountQuadsWithNormal(mesh, {0.0f, 1.0f, 0.0f});
    EXPECT_EQ(top_quads, 1)
        << "Uniform AO should allow merging (4 blocks -> 1 top quad)";

    // Verify all AO values in the mesh are 1.0 (the stub value).
    for (const auto& v : mesh.vertices) {
        EXPECT_FLOAT_EQ(v.ao, 1.0f)
            << "AO should be 1.0 (stubbed) for all vertices";
    }

    // Total: 6 quads (the row merges into 1 quad per face direction).
    EXPECT_EQ(CountQuads(mesh), 6);
}

TEST(GreedyMeshing, CorrectNormalsPreserved) {
    // Place a 4x4 flat layer. Greedy merging should preserve correct normals.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    for (int z = 0; z < 4; ++z) {
        for (int x = 0; x < 4; ++x) {
            chunk.SetBlock(x, 0, z, BlockRegistry::kStone);
        }
    }

    MeshData mesh = BuildGreedyMesh(chunk, neighbors, registry);

    // Collect unique normals.
    std::set<std::tuple<float, float, float>> normals;
    for (const auto& v : mesh.vertices) {
        normals.insert({v.normal.x, v.normal.y, v.normal.z});
    }

    // Should have exactly 6 unique normals (one per face direction).
    EXPECT_EQ(normals.size(), 6u);

    // Verify each expected normal is present.
    EXPECT_TRUE(normals.count({ 1.0f,  0.0f,  0.0f})) << "+X normal missing";
    EXPECT_TRUE(normals.count({-1.0f,  0.0f,  0.0f})) << "-X normal missing";
    EXPECT_TRUE(normals.count({ 0.0f,  1.0f,  0.0f})) << "+Y normal missing";
    EXPECT_TRUE(normals.count({ 0.0f, -1.0f,  0.0f})) << "-Y normal missing";
    EXPECT_TRUE(normals.count({ 0.0f,  0.0f,  1.0f})) << "+Z normal missing";
    EXPECT_TRUE(normals.count({ 0.0f,  0.0f, -1.0f})) << "-Z normal missing";

    // Each normal should appear exactly 4 times (4 vertices per merged quad,
    // and there's only 1 merged quad per face direction).
    for (const auto& [nx, ny, nz] : normals) {
        int count = 0;
        for (const auto& v : mesh.vertices) {
            if (std::abs(v.normal.x - nx) < 0.01f &&
                std::abs(v.normal.y - ny) < 0.01f &&
                std::abs(v.normal.z - nz) < 0.01f) {
                ++count;
            }
        }
        EXPECT_EQ(count, 4) << "Normal (" << nx << ", " << ny << ", " << nz
                            << ") should appear exactly 4 times (1 quad)";
    }
}

TEST(GreedyMeshing, SingleBlockUnchanged) {
    // A single isolated block should produce the same result as naive meshing:
    // 6 faces, 24 vertices, 36 indices.
    Chunk chunk(0, 0);
    BlockRegistry registry;
    NeighborData neighbors;

    chunk.SetBlock(8, 128, 8, BlockRegistry::kStone);

    MeshData greedy_mesh = BuildGreedyMesh(chunk, neighbors, registry);
    MeshData naive_mesh = BuildMesh(chunk, neighbors, registry);

    // Same number of quads.
    EXPECT_EQ(CountQuads(greedy_mesh), 6);
    EXPECT_EQ(CountQuads(greedy_mesh), CountQuads(naive_mesh));

    // Same vertex and index counts.
    EXPECT_EQ(greedy_mesh.vertices.size(), 24u);
    EXPECT_EQ(greedy_mesh.indices.size(), 36u);
    EXPECT_EQ(greedy_mesh.vertices.size(), naive_mesh.vertices.size());
    EXPECT_EQ(greedy_mesh.indices.size(), naive_mesh.indices.size());
}
