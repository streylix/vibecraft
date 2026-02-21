# Milestone 6 — Chunk Meshing v1 (Face Culling)

> **Status: COMPLETE** — All 12 tests passing. Face culling mesher with neighbor support, proper vertex format, boundary handling.

## Description
Convert chunk block data into renderable triangle meshes. Use face culling to only generate faces between solid and non-solid blocks (or at chunk boundaries). This is the naive meshing approach before greedy meshing optimization.

## Goals
- Generate correct vertex data for visible block faces
- Cull faces between two adjacent solid blocks
- Handle transparent blocks (render faces between transparent and air, and between solid and transparent)
- Correct normals, UVs, and texture indices per face
- Neighbor chunk data passed as parameter for boundary face culling

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/mesh.h` | Vertex struct, MeshData container |
| `include/vibecraft/chunk_mesher.h` | ChunkMesher class declaration |
| `src/mesh.cpp` | Mesh data utilities |
| `src/chunk_mesher.cpp` | Meshing algorithm implementation |

## Dependencies
- **M3** — Block types (need solid/transparent info)
- **M4** — Chunk data (the data to mesh)

## Test Specifications

### File: `tests/test_chunk_mesher.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `ChunkMesher.EmptyChunkZeroVerts` | All-air chunk produces no mesh | `vertices.size() == 0` |
| `ChunkMesher.SingleBlockCenter` | One block in chunk center (not at edge) | Exactly 24 vertices (6 faces x 4 verts) or 36 if using triangles (6 faces x 2 tris x 3 verts) |
| `ChunkMesher.TwoAdjacentBlocksCullSharedFace` | Two blocks sharing a face | 2 blocks x 6 faces - 2 culled faces = 10 visible faces |
| `ChunkMesher.FullLayerCulling` | Full 16x16 layer at y=0 | Only top faces + edge side faces visible (no bottom since y=0 is floor) |
| `ChunkMesher.TransparentAdjacentToSolid` | Glass next to stone | Face between them IS generated (not culled) |
| `ChunkMesher.TransparentAdjacentToAir` | Glass next to air | Face between them IS generated |
| `ChunkMesher.TransparentAdjacentToTransparent` | Glass next to glass | Face between them IS NOT generated (same-type culling) |
| `ChunkMesher.CorrectNormals` | Face normals point outward | +X face normal is (1,0,0), -Y face normal is (0,-1,0), etc. |
| `ChunkMesher.CorrectUVs` | UV coordinates are [0,1] | All UV components in [0,1] range |
| `ChunkMesher.CorrectTexIndices` | Texture index matches block face | Grass top face uses grass-top tex, side uses grass-side tex |
| `ChunkMesher.BoundaryFacesWithoutNeighbor` | Block at chunk edge, no neighbor data | Face IS generated (assume air outside) |
| `ChunkMesher.BoundaryFacesWithNeighbor` | Block at chunk edge, neighbor has solid block | Face is culled |

## Vertex Format
```cpp
struct Vertex {
    glm::vec3 position;   // Block-local position
    glm::vec2 tex_coord;  // UV within atlas tile
    float tex_index;      // Atlas tile index
    glm::vec3 normal;     // Face normal
    float ao;             // Ambient occlusion (0.0-1.0, computed later)
};
```

## Design Decisions
- Mesh function signature: `MeshData BuildMesh(const Chunk& chunk, const NeighborData& neighbors, const BlockRegistry& registry)`
- NeighborData: struct with optional pointers to 4 horizontal neighbor chunks (+X, -X, +Z, -Z)
- No world reference in mesher — pure function of chunk + neighbors
- Generates indexed mesh data (vertices + indices) to reduce vertex duplication
- UVs are within [0,1] per face; texture atlas lookup happens in the shader using tex_index
- At chunk boundaries without neighbor data, assume air (generate face)
