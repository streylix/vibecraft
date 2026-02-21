# Milestone 9 — Window, GL Context & Render Pipeline

> **Status: COMPLETE** — All 11 tests passing (8 camera + 3 shader). FPS camera, shader loading, GLFW window, GL renderer, GLSL shaders.

## Description
Create the game window using GLFW, initialize an OpenGL 3.3 Core context, and set up the basic render pipeline including camera, view/projection matrices, and shader compilation.

## Goals
- GLFW window creation with OpenGL 3.3 Core Profile
- Camera class with position, yaw, pitch, and view matrix computation
- Perspective projection matrix setup
- Shader compilation and linking utility
- Basic render loop structure

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/window.h` | Window wrapper class |
| `src/window.cpp` | GLFW window management |
| `include/vibecraft/camera.h` | Camera class declaration |
| `src/camera.cpp` | Camera implementation |
| `include/vibecraft/shader.h` | Shader loading/compilation utility |
| `src/shader.cpp` | Shader implementation |
| `include/vibecraft/renderer.h` | Render pipeline class |
| `src/renderer.cpp` | Render pipeline implementation |
| `assets/shaders/block.vert` | Block vertex shader |
| `assets/shaders/block.frag` | Block fragment shader |

## Dependencies
- **M1** — Build system (GLFW, OpenGL)
- **M2** — Math utilities (GLM operations)

## Test Specifications

### File: `tests/test_camera.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Camera.DefaultPosition` | Camera starts at origin | Position == (0, 0, 0) |
| `Camera.ViewMatrixIdentityLike` | Default camera looking down -Z | Forward vector ≈ (0, 0, -1) |
| `Camera.YawRotation` | Yaw rotates around Y axis | Yaw 90° → forward ≈ (1, 0, 0) |
| `Camera.PitchRotation` | Pitch rotates up/down | Pitch 90° → forward ≈ (0, 1, 0) |
| `Camera.PitchClamp` | Pitch clamped to [-89°, 89°] | Setting pitch to 100° results in 89° |
| `Camera.ProjectionMatrix` | Perspective projection is valid | Matrix is not zero, has correct aspect ratio influence |
| `Camera.ViewProjectionProduct` | VP matrix multiplies correctly | Result is a valid 4x4 matrix with non-zero determinant |
| `Camera.MoveForward` | Camera translates along forward vector | Position changes in forward direction |

### File: `tests/test_shader.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Shader.ValidVertexSource` | Vertex shader source compiles (string check) | Source string contains `#version 330 core` |
| `Shader.ValidFragmentSource` | Fragment shader source compiles (string check) | Source string contains `#version 330 core` |
| `Shader.ShaderFileExists` | Shader files exist on disk | `block.vert` and `block.frag` exist |

## Design Decisions
- OpenGL 3.3 Core Profile (widest macOS support for core profile)
- Camera uses Euler angles (yaw/pitch) — no quaternions needed for FPS camera
- Yaw: 0° = looking down -Z, increases counter-clockwise
- Pitch: 0° = horizontal, +90° = straight up, -90° = straight down
- FOV: 70° vertical by default
- Near/far planes: 0.1 / 1000.0
- Rendering tests avoid requiring a real GL context where possible (test math/logic only)
