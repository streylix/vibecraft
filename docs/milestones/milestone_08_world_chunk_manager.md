# Milestone 8 — World & Chunk Manager

> **Status: COMPLETE** — All 12 tests passing. Transparent cross-chunk get/set, lazy creation, dirty propagation, negative coords.

## Description
Implement the World class that manages multiple chunks, translates between world and chunk coordinates, and handles chunk loading/unloading. This is the spatial backbone connecting individual chunks into a coherent world.

## Goals
- World-to-chunk coordinate translation
- Get/set blocks using world coordinates (cross-chunk transparent)
- Chunk load and unload by chunk coordinates
- Support negative world coordinates
- Dirty flag propagation to neighbor chunks on boundary edits

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/world.h` | World class declaration |
| `src/world.cpp` | World implementation |

## Dependencies
- **M3** — Block types
- **M4** — Chunk data structure

## Test Specifications

### File: `tests/test_world.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `World.WorldToChunkCoord` | Coordinate translation | Block (16,0,0) → chunk (1,0); block (-1,0,0) → chunk (-1,0) |
| `World.WorldToLocalCoord` | Local coordinate extraction | Block (17,0,0) → local (1,0); block (-1,0,0) → local (15,0) |
| `World.SetGetBlockSameChunk` | Set/get within one chunk | Set block at (5,64,5), get returns correct id |
| `World.SetGetBlockCrossChunk` | Set/get across chunk boundary | Set at (15,64,0) and (16,64,0) in different chunks, both correct |
| `World.GetBlockUnloadedChunk` | Query in non-existent chunk | Returns Air (0) |
| `World.LoadChunk` | Load a chunk | Chunk becomes accessible |
| `World.UnloadChunk` | Unload a chunk | Chunk no longer accessible, GetBlock returns Air |
| `World.NegativeCoords` | Negative world coordinates work | Set/get at (-5, 64, -10) succeeds |
| `World.DirtyPropagation` | Boundary edit dirties neighbor | Set block at local x=0, neighbor chunk (-X) also marked dirty |
| `World.ChunkCount` | Track loaded chunk count | Load 3 chunks, count == 3; unload 1, count == 2 |
| `World.MultipleChunksIndependent` | Chunks don't interfere | Set different blocks in different chunks, all correct |
| `World.LargeCoordinates` | Far coords work | Set/get at (10000, 64, 10000) succeeds |

## Design Decisions
- Chunks stored in `std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>>`
- `ChunkCoord` is a struct `{int cx, int cz}` with hash function
- World-to-chunk: `cx = IntFloor(bx / 16)`, `cz = IntFloor(bz / 16)` (uses M2 IntFloor)
- World-to-local: `lx = Mod(bx, 16)`, `lz = Mod(bz, 16)` (uses M2 Mod)
- Boundary edits (local x=0,15 or z=0,15) propagate dirty flag to adjacent chunk
- No automatic chunk generation — caller decides when to create/populate chunks
