# Milestone 17 — Cave Generation & Ore Distribution

## Description
Add cave carving using 3D noise and distribute ores at appropriate depth ranges. Caves create underground exploration spaces, and ores provide mining targets.

## Goals
- Cave carving using 3D noise thresholding
- Caves are deterministic per seed
- Caves don't replace bedrock (y=0)
- Ore distribution by depth (diamond < y=16, gold < y=32, etc.)
- Ore veins are clusters (3-8 blocks)
- Ores only replace stone (not air, dirt, etc.)

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/cave_generator.h` | Cave generation |
| `src/cave_generator.cpp` | Cave carving implementation |
| `include/vibecraft/ore_generator.h` | Ore distribution |
| `src/ore_generator.cpp` | Ore placement implementation |
| `src/terrain_generator.cpp` | Updated to integrate caves and ores |

## Dependencies
- **M5** — Noise generation
- **M11** — Basic terrain (to carve into)
- **M3** — Block types (ore block ids)

## Test Specifications

### File: `tests/test_caves_ores.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Caves.CavesExist` | Caves are generated | Air blocks exist below surface that aren't from terrain |
| `Caves.Deterministic` | Same seed = same caves | Two generations produce identical cave patterns |
| `Caves.DontReplaceBedrock` | Bedrock preserved | y=0 is still bedrock after cave generation |
| `Caves.CaveSize` | Caves have reasonable size | Cave regions are multi-block, not single-block holes |
| `Ores.DiamondBelowY16` | Diamond ore only below y=16 | No diamond ore at y >= 16 |
| `Ores.GoldBelowY32` | Gold ore only below y=32 | No gold ore at y >= 32 |
| `Ores.IronBelowY64` | Iron ore only below y=64 | No iron ore at y >= 64 |
| `Ores.CoalAnywhere` | Coal ore at any height | Coal ore can appear at various heights |
| `Ores.OreVeins` | Ores appear in clusters | At least some ore blocks have adjacent ore of same type |
| `Ores.OresOnlyInStone` | Ores only replace stone | No ore block adjacent to air/dirt without stone context |
| `Ores.Deterministic` | Same seed = same ore placement | Two generations identical |

## Design Decisions
- Cave generation: 3D noise, threshold at > 0.6 = air
- Cave noise frequency: 0.05 for worm-like tunnels
- Second noise layer at different frequency for cave variety
- Ore distribution table:
  - Coal: y=0-128, vein size 3-8, frequency high
  - Iron: y=0-64, vein size 3-6, frequency medium
  - Gold: y=0-32, vein size 3-5, frequency low
  - Diamond: y=0-16, vein size 2-4, frequency very low
- Ore veins: place seed block, BFS outward with decreasing probability
- Order: terrain → caves → ores (ores can be in cave walls)
