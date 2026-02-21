# Milestone 13 — Block Interaction (Raycast / Break / Place)

## Description
Implement block raycasting for target selection, timed block breaking with hardness, and block placement on faces. This enables the core gameplay loop of mining and building.

## Goals
- Ray-voxel traversal (DDA algorithm) for block picking
- Hit detection with face normal identification
- Timed breaking based on block hardness
- Block placement on the face adjacent to the hit
- Bedrock cannot be broken
- Cannot place a block inside the player's AABB

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/raycast.h` | Ray-voxel traversal |
| `src/raycast.cpp` | Raycast implementation |
| `include/vibecraft/block_interaction.h` | Break/place logic |
| `src/block_interaction.cpp` | Block interaction implementation |

## Dependencies
- **M2** — AABB, ray intersection math
- **M8** — World block get/set
- **M12** — Input (mouse buttons)
- **M3** — Block types (hardness, solid)

## Test Specifications

### File: `tests/test_raycast.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Raycast.HitBlock` | Ray hits a block | Returns true with correct block position |
| `Raycast.MissInEmptyWorld` | Ray in empty world | Returns false |
| `Raycast.MaxDistance` | Ray beyond max range | Returns false even if block exists beyond range |
| `Raycast.NearestBlock` | Multiple blocks in path | Returns the nearest one |
| `Raycast.FaceNormalPosX` | Hit on +X face | Normal == (1, 0, 0) |
| `Raycast.FaceNormalNegY` | Hit on -Y (bottom) face | Normal == (0, -1, 0) |
| `Raycast.FaceNormalPosZ` | Hit on +Z face | Normal == (0, 0, 1) |
| `Raycast.DiagonalRay` | Ray at 45° angle | Hits correct block |

### File: `tests/test_block_interaction.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `BlockInteraction.BreakTimedByHardness` | Breaking respects hardness | Stone (1.5) takes longer than dirt (0.5) |
| `BlockInteraction.BreakCompletesAtZero` | Block breaks when timer depletes | Block replaced with air |
| `BlockInteraction.BedrockUnbreakable` | Can't break bedrock | Hardness < 0 → no break progress |
| `BlockInteraction.PlaceOnFace` | Place block adjacent to hit face | Block placed at hit_pos + normal |
| `BlockInteraction.PlaceNotInsidePlayer` | Can't place block overlapping player | Placement rejected if new block AABB overlaps player AABB |
| `BlockInteraction.BreakResetOnLookAway` | Breaking resets when target changes | Timer resets to full when aiming at different block |
| `BlockInteraction.PlaceOnlyOnSolid` | Can't place on air | Must have a solid block face to place against |

## Design Decisions
- DDA (Digital Differential Analyzer) algorithm for ray-voxel traversal
- Max raycast distance: 5.0 blocks (Minecraft-standard reach)
- Break time = hardness * base_break_time (base = 1.0 seconds)
- Negative hardness = unbreakable (bedrock)
- Placement face: block coordinate = hit_block_pos + face_normal
- Player AABB check before placement to prevent self-entombment
