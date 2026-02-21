# Milestone 12 — Input Handling & Camera Control

> **Status: COMPLETE** — All 10 tests passing. Key press/release/held detection, mouse delta, scroll, sensitivity, testable without GLFW.

## Description
Implement keyboard and mouse input handling for player control and camera movement. Detect key press/release/held states, mouse movement deltas, and scroll wheel input.

## Goals
- Key state tracking: press, release, held, with edge detection
- Mouse delta tracking for camera look
- Scroll wheel input
- Simultaneous key handling (e.g., W+A for diagonal movement)
- Configurable mouse sensitivity

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/input.h` | Input manager class |
| `src/input.cpp` | Input handling implementation |

## Dependencies
- **M9** — Window (GLFW callbacks)
- **M7** — Player (to apply movement)

## Test Specifications

### File: `tests/test_input.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Input.KeyPressDetection` | Detect key down event | After simulating key press, `IsKeyPressed(KEY_W)` returns true once |
| `Input.KeyReleaseDetection` | Detect key up event | After simulating key release, `IsKeyReleased(KEY_W)` returns true once |
| `Input.KeyHeld` | Detect key being held | While key is down, `IsKeyHeld(KEY_W)` returns true continuously |
| `Input.EdgeDetection` | Press fires only once per press | `IsKeyPressed()` returns true only on first query after press |
| `Input.MouseDelta` | Mouse movement produces delta | After mouse moves (100, 50), delta is (100, 50) |
| `Input.MouseDeltaReset` | Delta resets each frame | After reading delta, next read is (0, 0) until new movement |
| `Input.ScrollInput` | Scroll wheel detected | After scroll event, scroll value is non-zero |
| `Input.SimultaneousKeys` | Multiple keys held at once | W and A both held simultaneously → both `IsKeyHeld()` true |
| `Input.Sensitivity` | Mouse sensitivity scales delta | Sensitivity 2.0 doubles the reported delta |
| `Input.DefaultSensitivity` | Default sensitivity is 1.0 | Without configuration, sensitivity == 1.0 |

## Design Decisions
- Input state stored in arrays indexed by GLFW key codes
- Three states per key: current frame, previous frame → derive pressed/released/held
- `Update()` called once per frame to snapshot state
- Mouse delta accumulated between frames via GLFW cursor callback
- Input class is independent of camera/player — just reports state
- GLFW callbacks set through static function + pointer to Input instance
