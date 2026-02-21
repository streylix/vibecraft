# Milestone 2 — Math Utilities & AABB Primitives

> **Status: COMPLETE** — All 23 tests passing (12 MathUtils + 11 AABB)

## Description
Implement core math utilities and axis-aligned bounding box (AABB) primitives used throughout the engine for collision detection, physics, and spatial queries.

## Goals
- Robust AABB implementation with overlap, containment, and intersection tests
- Ray-AABB intersection for block picking
- Swept AABB collision for player movement
- Common math helpers (lerp, clamp, intFloor, mod)

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/math_utils.h` | Math helper functions |
| `src/math_utils.cpp` | Math helper implementations |
| `include/vibecraft/aabb.h` | AABB class declaration |
| `src/aabb.cpp` | AABB implementation |

## Dependencies
- **M1** — Build system and GLM available

## Test Specifications

### File: `tests/test_math_utils.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `MathUtils.LerpZero` | `Lerp(0, 10, 0.0f)` | `0.0f` |
| `MathUtils.LerpHalf` | `Lerp(0, 10, 0.5f)` | `5.0f` |
| `MathUtils.LerpOne` | `Lerp(0, 10, 1.0f)` | `10.0f` |
| `MathUtils.ClampBelow` | `Clamp(-5, 0, 10)` | `0` |
| `MathUtils.ClampAbove` | `Clamp(15, 0, 10)` | `10` |
| `MathUtils.ClampInRange` | `Clamp(5, 0, 10)` | `5` |
| `MathUtils.IntFloorPositive` | `IntFloor(3.7f)` | `3` |
| `MathUtils.IntFloorNegative` | `IntFloor(-0.1f)` | `-1` |
| `MathUtils.IntFloorExact` | `IntFloor(4.0f)` | `4` |
| `MathUtils.ModPositive` | `Mod(17, 16)` | `1` |
| `MathUtils.ModNegative` | `Mod(-1, 16)` | `15` |
| `MathUtils.ModZero` | `Mod(0, 16)` | `0` |

### File: `tests/test_aabb.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `AABB.Overlap` | Two overlapping AABBs | `Overlaps()` returns true |
| `AABB.NoOverlap` | Two separated AABBs | `Overlaps()` returns false |
| `AABB.TouchingNotOverlapping` | AABBs share a face | `Overlaps()` returns false (open intervals) |
| `AABB.ContainsPoint` | Point inside AABB | `Contains(point)` returns true |
| `AABB.DoesNotContainPoint` | Point outside AABB | `Contains(point)` returns false |
| `AABB.RayHit` | Ray aimed at AABB | `RayIntersect()` returns true, t > 0 |
| `AABB.RayMiss` | Ray aimed away from AABB | `RayIntersect()` returns false |
| `AABB.RayFromInside` | Ray origin inside AABB | `RayIntersect()` returns true, t = 0 |
| `AABB.SweptCollision` | Moving AABB into stationary AABB | Returns collision time in [0,1] and correct normal |
| `AABB.SweptNoCollision` | Moving AABB away from stationary AABB | Returns t = 1.0 (no collision) |
| `AABB.SweptEdgeSlide` | Moving AABB slides along edge | Correct collision normal for slide direction |

## Design Decisions
- AABB stores `min` and `max` as `glm::vec3`
- Open intervals for overlap (touching faces don't count as overlapping)
- Swept collision returns `SweptResult{float t, glm::vec3 normal}` for physics integration
- `IntFloor` uses `static_cast<int>(floor(x))` to handle negative coords correctly
- `Mod` always returns non-negative result (Euclidean modulo)
