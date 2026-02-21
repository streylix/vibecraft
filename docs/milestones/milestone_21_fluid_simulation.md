# Milestone 21 — Fluid Simulation

## Description
Implement cellular automata fluid simulation for water and lava. Water flows down and horizontally with 8 levels (0-7). Lava flows slower with 4 levels (0-3). Water + lava interactions produce cobblestone or obsidian.

## Goals
- Water flows downward first, then spreads horizontally
- Water levels 0-7 (7 = full, 0 = empty)
- Lava flows slower (updates less frequently) with max level 3
- Removing source block causes fluid to drain
- Basin filling (fluid fills depressions)
- Infinite source rule: two adjacent water sources create a third
- Water + lava interactions: flowing water + lava source = cobblestone, water source + lava = obsidian

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/fluid.h` | Fluid simulation class |
| `src/fluid.cpp` | Fluid simulation implementation |

## Dependencies
- **M4** — Chunk data (fluid level storage)
- **M3** — Block types (water/lava properties)
- **M8** — World manager (cross-chunk fluid flow)

## Test Specifications

### File: `tests/test_fluid.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Fluid.WaterFlowsDown` | Water source with air below | Water appears in block below |
| `Fluid.WaterSpreadsHorizontal` | Water on flat surface | Spreads horizontally with decreasing level |
| `Fluid.WaterLevels` | Level decreases with distance | Source = 7, adjacent = 6, next = 5, etc. |
| `Fluid.WaterRemoval` | Remove source block | Downstream water drains away |
| `Fluid.BasinFill` | Water fills a depression | Enclosed pit fills up level by level |
| `Fluid.InfiniteSource` | Two sources adjacent create third | Two water source blocks with air between → new source |
| `Fluid.LavaMaxLevel` | Lava max level is 3 | Lava source level == 3 |
| `Fluid.LavaSlower` | Lava updates less frequently | Lava spreads after more ticks than water |
| `Fluid.WaterLavaCobblestone` | Flowing water meets lava source | Creates cobblestone |
| `Fluid.WaterLavaObsidian` | Water source meets lava | Creates obsidian |
| `Fluid.WaterDoesntFlowUp` | Water below solid block | Doesn't flow upward |

## Design Decisions
- Fluid level stored in block data (separate from block ID or encoded in metadata)
- Water source blocks: level 7 (maximum), flow blocks decrease by 1 per step
- Lava source blocks: level 3, flows 1 per step
- Update order: process all fluid blocks each tick, gather pending changes, apply atomically
- Water flow priority: down first, then horizontal spread equally
- Infinite source rule: if a non-source water block has 2+ adjacent sources, it becomes a source
- Lava update rate: every 3 ticks (vs water every tick)
- Interaction: check adjacent blocks after placement for water/lava conversion
