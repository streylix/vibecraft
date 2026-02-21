# Milestone 7 — Player Physics

## Description
Implement player physics including gravity, jumping, ground detection, and collision response against the voxel world. The player is represented as an AABB that moves through the block grid.

## Goals
- Gravity pulls the player down at 32 blocks/s²
- Terminal velocity caps falling speed at 78 blocks/s
- Jump impulse of 9.2 blocks/s (reach ~1.25 blocks height)
- AABB collision with solid blocks (wall, floor, ceiling)
- Water blocks are passable (no collision)
- Grounded detection for jump eligibility

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/player.h` | Player class declaration |
| `src/player.cpp` | Player physics implementation |

## Dependencies
- **M2** — AABB and swept collision
- **M3** — Block types (solid/liquid lookup)
- **M4** — Chunk data (world query)

## Test Specifications

### File: `tests/test_player_physics.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `PlayerPhysics.GravityAccelerates` | Player velocity increases downward over time | After 1 tick at 20 tps, vy ≈ -1.6 (32/20) |
| `PlayerPhysics.LandingOnGround` | Player stops when hitting solid block below | Player y stops at block top, vy = 0, grounded = true |
| `PlayerPhysics.TerminalVelocity` | Falling speed caps out | After many ticks, abs(vy) <= 78.0 |
| `PlayerPhysics.JumpFromGround` | Jump sets upward velocity | When grounded, jump → vy = 9.2 |
| `PlayerPhysics.NoJumpInAir` | Can't jump while airborne | When not grounded, jump has no effect |
| `PlayerPhysics.WallCollision` | Player can't walk through solid blocks | Moving into wall stops at wall face |
| `PlayerPhysics.CeilingCollision` | Player can't jump through solid blocks above | Hitting ceiling sets vy = 0 |
| `PlayerPhysics.WaterPassthrough` | Player moves through water blocks | No collision with water blocks |
| `PlayerPhysics.PlayerAABBSize` | Player has correct dimensions | Width = 0.6, Height = 1.8 |
| `PlayerPhysics.GroundedOnEdge` | Player on edge of block is still grounded | Standing on block edge → grounded = true |
| `PlayerPhysics.FallingNotGrounded` | Player in air is not grounded | After walking off edge → grounded = false |

## Design Decisions
- Player AABB: 0.6 wide (x,z), 1.8 tall (y), centered at feet
- Eye position: player position + (0, 1.62, 0)
- Physics uses fixed timestep of 1/20 second (matches game tick rate)
- Collision resolution: swept AABB against nearby blocks, resolve axis by axis
- Check blocks in a 3x4x3 region around the player for collisions
- Gravity constant: 32 blocks/s², terminal velocity: 78 blocks/s
- Jump velocity: 9.2 blocks/s upward
