# Milestone 4 — Chunk Data Storage

> **Status: COMPLETE** — All 15 tests passing. 16x16x256 chunk with O(1) access, heightmap, dirty flags.

## Description
Implement the core chunk data structure that stores a 16x16x256 column of blocks. This is the fundamental spatial data container for the voxel world.

## Goals
- Efficient storage of 65,536 blocks per chunk
- O(1) get/set by local coordinates
- Bounds checking to prevent out-of-range access
- Dirty flag for tracking when remeshing is needed
- Heightmap tracking (highest non-air block per column)

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/chunk.h` | Chunk class declaration |
| `src/chunk.cpp` | Chunk implementation |

## Dependencies
- **M1** — Build system
- **M3** — BlockId type definition

## Test Specifications

### File: `tests/test_chunk.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Chunk.DefaultAllAir` | Newly created chunk is all air | `GetBlock(x,y,z) == 0` for all coords |
| `Chunk.SetGetBlock` | Set then get a block | `SetBlock(5,64,5, 1); GetBlock(5,64,5) == 1` |
| `Chunk.BoundsCheckX` | x outside [0,15] | Returns Air (0) or throws |
| `Chunk.BoundsCheckY` | y outside [0,255] | Returns Air (0) or throws |
| `Chunk.BoundsCheckZ` | z outside [0,15] | Returns Air (0) or throws |
| `Chunk.DirtyOnCreate` | New chunk starts dirty | `IsDirty() == true` |
| `Chunk.DirtyAfterSet` | SetBlock marks dirty | After `ClearDirty()`, `SetBlock()` → `IsDirty() == true` |
| `Chunk.ClearDirty` | ClearDirty works | `ClearDirty()` → `IsDirty() == false` |
| `Chunk.HeightmapBasic` | Heightmap tracks highest block | Place block at y=64, heightmap at that column == 64 |
| `Chunk.HeightmapUpdate` | Heightmap updates on removal | Remove highest block, heightmap decreases |
| `Chunk.HeightmapMultiColumn` | Independent columns | Different heights in different columns |
| `Chunk.IndexConsistency` | All valid coords round-trip | Set unique values across chunk, all read back correctly |
| `Chunk.SetSameBlockNotDirty` | Setting same block doesn't dirty | Set block to value it already has, dirty flag unchanged |
| `Chunk.ChunkPosition` | Chunk knows its world position | `GetChunkX()`, `GetChunkZ()` return correct values |
| `Chunk.FullColumnWrite` | Write all 256 Y levels | No crash, all values correct |

## Design Decisions
- Storage: flat `std::array<uint8_t, 65536>` indexed as `y * 256 + z * 16 + x`
- Wait, that's wrong. Index should be: `y * 16 * 16 + z * 16 + x` = `y * 256 + z * 16 + x`
- Y-major layout for vertical locality (column operations are common)
- Heightmap: `std::array<int16_t, 256>` (16x16 columns), -1 if column is all air
- Chunk stores its chunk coordinates (cx, cz) for world position reference
- Out-of-bounds get returns Air (0) silently — simpler than exceptions for boundary queries
