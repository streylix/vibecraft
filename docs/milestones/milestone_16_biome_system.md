# Milestone 16 — Biome System

> **Status: COMPLETE** — All 10 tests passing. 5 biomes (Plains, Forest, Desert, Mountains, Tundra) with temp/moisture noise, blended height transitions, tree generation.

## Description
Implement a biome system that varies terrain surface blocks, height profile, and vegetation based on temperature and moisture noise maps.

## Goals
- At least 5 distinct biomes
- Each biome determines surface block type (grass/sand/snow/etc.)
- Biome affects terrain height profile
- Smooth transitions between biomes
- Trees generate in forested biomes but not desert

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/biome.h` | Biome definitions and BiomeMap class |
| `src/biome.cpp` | Biome implementation |
| `src/terrain_generator.cpp` | Updated terrain gen with biome integration |

## Dependencies
- **M5** — Noise generation (for biome maps)
- **M11** — Basic terrain generation (to enhance)

## Test Specifications

### File: `tests/test_biome.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Biome.AtLeastFiveBiomes` | Minimum biome count | `>= 5` biome types defined |
| `Biome.PlainsHasGrass` | Plains surface is grass | Surface block == Grass |
| `Biome.DesertHasSand` | Desert surface is sand | Surface block == Sand |
| `Biome.TundraHasSnow` | Tundra surface is snow | Surface block == Snow |
| `Biome.HeightAffectedByBiome` | Different biomes have different heights | Mountain biome avg height > plains avg height |
| `Biome.SmoothTransition` | Biome boundaries are smooth | No single-block biome patches in a smooth area |
| `Biome.TreesInForest` | Forest biome generates trees | Tree blocks (Log, Leaves) present in forest chunks |
| `Biome.NoTreesInDesert` | Desert doesn't have trees | No Log/Leaves blocks in desert chunks |
| `Biome.Deterministic` | Same seed → same biomes | Two generations with same seed produce same biome map |
| `Biome.BiomeAtCoord` | Biome query by world coordinate | `GetBiome(x, z)` returns a valid biome type |

## Biome Definitions
| Biome | Surface | Sub-surface | Height Modifier | Trees |
|-------|---------|-------------|-----------------|-------|
| Plains | Grass | Dirt | 0 (base) | Sparse |
| Forest | Grass | Dirt | +5 | Dense |
| Desert | Sand | Sand | -10 | None |
| Mountains | Stone | Stone | +30 | None |
| Tundra | Snow | Dirt | -5 | Sparse |

## Design Decisions
- Biome determined by 2D temperature and moisture noise
- Temperature noise: frequency 0.005, 1 octave
- Moisture noise: frequency 0.005, 1 octave, different seed offset
- Biome lookup: 2D table indexed by temperature/moisture ranges
- Transitions use biome blending: interpolate height between biomes in a border zone
- Trees: simple L-system (1x1x4 log trunk + 3x3x2 leaf canopy)
