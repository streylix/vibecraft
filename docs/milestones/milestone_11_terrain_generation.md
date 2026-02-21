# Milestone 11 — Terrain Generation (Basic)

> **Status: COMPLETE** — All 10 tests passing. Perlin noise height, bedrock/stone/dirt/grass layers, height range [40,120], seamless borders.

## Description
Generate basic Minecraft-like terrain using noise functions. Create rolling hills with grass/dirt/stone layers and bedrock floor.

## Goals
- Bedrock at y=0 (always)
- Grass surface block at terrain height
- 4 blocks of dirt below grass
- Stone fills the rest down to bedrock
- Air above terrain
- Terrain height varies between y=40 and y=120
- Deterministic terrain from world seed
- Chunk border continuity (no seams between chunks)

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/terrain_generator.h` | TerrainGenerator class |
| `src/terrain_generator.cpp` | Terrain generation implementation |

## Dependencies
- **M5** — Noise generation
- **M8** — World/chunk manager (to populate chunks)

## Test Specifications

### File: `tests/test_terrain_gen.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `TerrainGen.BedrockAtZero` | y=0 is always bedrock | `GetBlock(x, 0, z) == BlockId::Bedrock` for any x,z |
| `TerrainGen.AirAtTop` | y=255 is always air | `GetBlock(x, 255, z) == BlockId::Air` for any x,z |
| `TerrainGen.GrassSurface` | Topmost solid block is grass | At any column, highest non-air block is Grass |
| `TerrainGen.DirtBelowGrass` | 4 blocks of dirt under grass | Blocks surface-1 through surface-4 are Dirt |
| `TerrainGen.StoneBelowDirt` | Stone fills below dirt to bedrock | Blocks surface-5 down to y=1 are Stone |
| `TerrainGen.Deterministic` | Same seed produces same terrain | Two generations with same seed are identical |
| `TerrainGen.DifferentSeeds` | Different seeds produce different terrain | At least some blocks differ between seeds |
| `TerrainGen.HeightRange` | Terrain height within expected range | Surface heights all in [40, 120] |
| `TerrainGen.HeightVariation` | Terrain isn't flat | Standard deviation of heights > 5.0 |
| `TerrainGen.ChunkBorderContinuity` | No seams at chunk boundaries | Height at chunk border matches across chunks |

## Design Decisions
- TerrainGenerator takes a world seed in constructor
- `GenerateChunk(Chunk& chunk)` fills a chunk based on its chunk coordinates
- Height computed as: `base_height + noise2D(x * freq, z * freq) * amplitude`
- Default: base_height = 64, amplitude = 40, frequency = 0.01
- Layers: bedrock(y=0), stone(y=1 to surface-5), dirt(surface-4 to surface-1), grass(surface)
- Noise uses world-space block coordinates for seamless chunks
