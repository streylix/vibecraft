# Milestone 3 — Block Type Registry

> **Status: COMPLETE** — All 15 tests passing. 21 block types registered.

## Description
Define and register all block types with their properties. The registry maps `BlockId` (uint8_t) to a `BlockType` struct containing rendering and physics metadata.

## Goals
- Centralized registry of 15-20 block types
- Each block has: name, solid flag, transparent flag, liquid flag, hardness, light emission, texture face indices
- Quick O(1) lookup by BlockId
- Air is BlockId 0

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/block.h` | BlockId typedef, BlockType struct, BlockRegistry class |
| `src/block.cpp` | Block registration and registry implementation |

## Dependencies
- **M1** — Build system

## Test Specifications

### File: `tests/test_block_registry.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `BlockRegistry.AirIsZero` | Air block is registered at id 0 | `GetBlock(0).name == "Air"` |
| `BlockRegistry.AirIsNotSolid` | Air block properties | `solid == false, transparent == true` |
| `BlockRegistry.StoneIsSolid` | Stone block properties | `solid == true, transparent == false, hardness == 1.5f` |
| `BlockRegistry.GrassProperties` | Grass block has correct textures | Different top/side/bottom tex indices |
| `BlockRegistry.WaterIsLiquid` | Water block properties | `liquid == true, solid == false, transparent == true` |
| `BlockRegistry.LavaEmitsLight` | Lava block emits light | `light_emission == 15` |
| `BlockRegistry.GlassIsTransparent` | Glass block properties | `solid == true, transparent == true` |
| `BlockRegistry.TotalBlockCount` | Minimum number of blocks | `>= 15` registered blocks |
| `BlockRegistry.AllBlocksHaveNames` | No empty names | Every registered block has non-empty name |
| `BlockRegistry.AllBlocksHaveTextures` | Valid texture indices | Every solid/transparent block has valid tex indices (>= 0) |
| `BlockRegistry.BedrockHardness` | Bedrock is unbreakable | `hardness < 0` (negative = unbreakable) |
| `BlockRegistry.OakLogProperties` | Oak Log block | `solid == true, different top vs side textures` |
| `BlockRegistry.LeavesTransparent` | Leaves block | `solid == true, transparent == true` |
| `BlockRegistry.SandProperties` | Sand block | `solid == true` |
| `BlockRegistry.TorchEmitsLight` | Torch light emission | `light_emission == 14` |

## Block List (minimum)
| Id | Name | Solid | Transparent | Liquid | Hardness | Light | Notes |
|----|------|-------|-------------|--------|----------|-------|-------|
| 0 | Air | false | true | false | 0 | 0 | |
| 1 | Stone | true | false | false | 1.5 | 0 | |
| 2 | Grass | true | false | false | 0.6 | 0 | Different top/side/bottom |
| 3 | Dirt | true | false | false | 0.5 | 0 | |
| 4 | Cobblestone | true | false | false | 2.0 | 0 | |
| 5 | Oak Planks | true | false | false | 2.0 | 0 | |
| 6 | Bedrock | true | false | false | -1.0 | 0 | Unbreakable |
| 7 | Sand | true | false | false | 0.5 | 0 | |
| 8 | Gravel | true | false | false | 0.6 | 0 | |
| 9 | Gold Ore | true | false | false | 3.0 | 0 | |
| 10 | Iron Ore | true | false | false | 3.0 | 0 | |
| 11 | Coal Ore | true | false | false | 3.0 | 0 | |
| 12 | Diamond Ore | true | false | false | 3.0 | 0 | |
| 13 | Oak Log | true | false | false | 2.0 | 0 | Different top/side |
| 14 | Oak Leaves | true | true | false | 0.2 | 0 | |
| 15 | Glass | true | true | false | 0.3 | 0 | |
| 16 | Water | false | true | true | 0 | 0 | |
| 17 | Lava | false | true | true | 0 | 15 | |
| 18 | Torch | false | true | false | 0 | 14 | |
| 19 | Snow | true | false | false | 0.1 | 0 | |
| 20 | Cactus | true | true | false | 0.4 | 0 | |

## Design Decisions
- `BlockId` is `uint8_t` — max 256 types, stored compactly in chunks
- Registry is a flat `std::array<BlockType, 256>` for O(1) lookup
- Texture indices reference positions in the texture atlas (assigned later)
- Negative hardness means unbreakable
- Air must always be id 0 (used as default/empty throughout)
- Six texture face indices: `+X, -X, +Y, -Y, +Z, -Z` (allows different faces per block like grass)
