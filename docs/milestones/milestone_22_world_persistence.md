# Milestone 22 — World Persistence (Region Files)

> **Status: COMPLETE** — All 11 tests passing. Region files (32x32 chunks, zlib compressed), block/fluid/light round-trip, player pos/seed/time metadata.

## Description
Save and load the world to disk using a region file format. Each region file stores 32x32 chunks. Includes compression for efficient storage and preserves all world state.

## Goals
- Save/load chunk data with full round-trip fidelity
- Region files group 32x32 chunks per file
- Overwrite existing save data
- Coordinate mapping: chunk coords → region file + offset
- Compression (zlib) for reduced file size
- Preserve fluid levels and light data
- Save/load player position, world seed, and game time

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/region.h` | Region file I/O class |
| `src/region.cpp` | Region file implementation |
| `include/vibecraft/world_save.h` | World save/load coordinator |
| `src/world_save.cpp` | World save/load implementation |

## Dependencies
- **M4** — Chunk data (what to save)
- **M8** — World manager (chunk access)
- **M19** — Day/night (game time to persist)

## Test Specifications

### File: `tests/test_world_save.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `WorldSave.SaveLoadRoundTrip` | Save chunk, load it back | All blocks identical |
| `WorldSave.OverwriteSave` | Save, modify, save again | Second save overwrites first correctly |
| `WorldSave.CoordToRegion` | Chunk-to-region file mapping | Chunk (33, 0) → region (1, 0) |
| `WorldSave.CoordToOffset` | Chunk-to-offset within region | Chunk (1, 2) → offset (1, 2) within region |
| `WorldSave.Compression` | Data is compressed | Saved file size < raw block data size |
| `WorldSave.FluidPreserved` | Fluid levels survive save/load | Water level 5 → save → load → still level 5 |
| `WorldSave.LightPreserved` | Light data survives save/load | Light level 14 → save → load → still 14 |
| `WorldSave.PlayerPositionSaved` | Player pos saved and loaded | Save pos (10, 64, 20) → load → same pos |
| `WorldSave.WorldSeedSaved` | World seed persists | Save seed 12345 → load → seed == 12345 |
| `WorldSave.GameTimeSaved` | Game time persists | Save time 6000 → load → time == 6000 |
| `WorldSave.NegativeCoords` | Negative chunk coords save correctly | Save chunk at (-5, -3) → load → identical |

## Design Decisions
- Region file format: header (4096 bytes, offset table) + compressed chunk data
- Each region stores 32x32 = 1024 chunks
- Region coords: `rx = floor(cx / 32)`, `rz = floor(cz / 32)`
- File naming: `region_<rx>_<rz>.dat`
- Compression: zlib deflate on chunk byte array
- World metadata (seed, time, player pos) stored in `world.dat` JSON file
- Chunks only saved when dirty flag is set
- Region directory: `saves/<world_name>/regions/`
