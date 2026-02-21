# Milestone 15 — Lighting System (Sun + Block)

## Description
Implement a dual lighting system with sunlight (propagates from sky downward) and block light (emitted by torches, lava, etc.). Light values are stored per-block and propagated using BFS flood-fill.

## Goals
- Block light emission from light sources (torches = 14, lava = 15)
- Light propagation with attenuation (decreases by 1 per block)
- Solid blocks stop light propagation
- Sunlight propagates straight down from sky (level 15)
- Sunlight decreases when moving horizontally through air
- Cross-chunk light propagation
- Glass transmits light without attenuation reduction beyond normal
- Multiple light sources use max value (not additive)
- Light removal when light source is destroyed

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/lighting.h` | Lighting system class |
| `src/lighting.cpp` | Light propagation implementation |

## Dependencies
- **M4** — Chunk data (light storage)
- **M3** — Block types (emission, transparency)

## Test Specifications

### File: `tests/test_lighting.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Lighting.TorchEmission` | Torch emits light level 14 | Block at torch position has block_light == 14 |
| `Lighting.TorchSpread` | Light decreases with distance | 1 block away = 13, 2 = 12, ..., 14 away = 0 |
| `Lighting.SolidBlocksLight` | Solid block stops propagation | Light doesn't pass through stone |
| `Lighting.TorchRemoval` | Removing torch removes light | After removing torch, surrounding light updates correctly |
| `Lighting.SunlightFromSky` | Sunlight propagates from top | Exposed column has sunlight = 15 from top down |
| `Lighting.SunlightBlocked` | Solid block blocks sunlight | Block under a roof has sunlight = 0 |
| `Lighting.CrossChunkLight` | Light crosses chunk boundaries | Torch near chunk edge illuminates into neighbor chunk |
| `Lighting.GlassTransmitsLight` | Glass doesn't block light extra | Light passes through glass (decreases by 1, same as air) |
| `Lighting.MultiSourceMax` | Two light sources use max value | Overlapping lights → max(light1, light2), not sum |
| `Lighting.LavaEmission` | Lava emits light level 15 | Block at lava position has block_light == 15 |

## Design Decisions
- Each block stores 1 byte: upper 4 bits = sunlight (0-15), lower 4 bits = block light (0-15)
- BFS flood-fill propagation from each light source
- Light removal uses reverse BFS (collect affected, recalculate from remaining sources)
- Sunlight propagates straight down at full intensity, horizontal spread decreases by 1
- Glass/transparent blocks: light passes through with normal -1 attenuation
- Ambient occlusion is separate (computed during meshing), lighting is per-block
