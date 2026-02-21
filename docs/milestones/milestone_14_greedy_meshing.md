# Milestone 14 — Greedy Meshing Upgrade

> **Status: COMPLETE** — All 8 tests passing. Greedy quad merging, 96x vertex reduction on uniform layers, tiled UVs, AO-aware merging.

## Description
Upgrade the chunk mesher from naive per-face meshing to greedy meshing. Greedy meshing merges adjacent coplanar faces of the same block type into larger quads, dramatically reducing vertex count.

## Goals
- Merge adjacent faces of the same block type into larger quads
- Different block types must not be merged
- Vertex count significantly lower than naive meshing
- UV coordinates tile correctly across merged faces
- Ambient occlusion differences prevent merging

## Deliverables
| File | Purpose |
|------|---------|
| `src/chunk_mesher.cpp` | Updated with greedy meshing algorithm |

## Dependencies
- **M6** — Original chunk meshing (to upgrade)

## Test Specifications

### File: `tests/test_greedy_meshing.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `GreedyMeshing.MergedRow` | Row of same blocks merges faces | N blocks in a row → 1 quad instead of N quads for that face |
| `GreedyMeshing.MergedPlane` | Plane of same blocks merges | 4x4 flat layer → 1 quad per direction instead of 16 |
| `GreedyMeshing.DifferentBlocksNotMerged` | Adjacent different blocks stay separate | Stone next to dirt = 2 separate quads |
| `GreedyMeshing.FewerVertsThanNaive` | Total vertex count reduced | Greedy verts < naive verts for a filled chunk layer |
| `GreedyMeshing.CorrectTiledUVs` | UVs tile across merged face | A 4-block-wide merged face has UV range [0, 4] |
| `GreedyMeshing.AOBreaksMerge` | Different AO values prevent merge | Two adjacent blocks with different AO → not merged |
| `GreedyMeshing.CorrectNormalsPreserved` | Normals still correct after merge | Merged quad has same normal as individual faces |
| `GreedyMeshing.SingleBlockUnchanged` | Isolated block = same as naive | One block → 6 faces, same vertex count |

## Design Decisions
- Greedy meshing sweeps each layer (y-level) for each face direction
- Uses a 2D mask array for each slice, marks which faces are visible
- Iterates mask to find maximal rectangles of same block type + same AO
- UV coordinates scale with rectangle size (tiling: U range = width, V range = height)
- AO value per vertex is checked — different AO breaks the merge to preserve lighting
- Algorithm: for each axis direction, for each slice perpendicular to that axis, build mask, greedily merge rectangles
