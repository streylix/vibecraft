# VibeCraft — Project Conventions & AI Guidance

## Build & Run

```bash
# Configure (first time or after dependency changes)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build

# Run game
./build/vibecraft

# Run all tests
ctest --test-dir build --output-on-failure

# Run a single test binary
./build/test_math_utils
```

## Project Architecture

### Directory Layout

| Path | Purpose |
|------|---------|
| `src/` | Game source files (.cpp) |
| `include/vibecraft/` | Public headers (.h) — all headers go here |
| `tests/` | Google Test files (test_*.cpp) |
| `assets/textures/` | 16x16 pixel-art PNGs |
| `assets/shaders/` | GLSL vertex/fragment shaders |
| `assets/sounds/` | WAV sound effects |
| `docs/milestones/` | Milestone specification documents |

### Coordinate System

- **Y-up** OpenGL convention throughout
- **World coords:** `float` (player position, physics)
- **Block coords:** `int` (discrete voxel grid)
- **Chunk coords:** `int`, derived as `floor(blockCoord / 16)`
- **Local coords:** `int` in [0,15] within a chunk (x,z) and [0,255] (y)
- Block at world float position: `floor()` to get block coord

### Block Storage

- `uint8_t` BlockId — max 256 block types
- Chunk = 16 x 16 x 256 column = 65,536 blocks
- Index: `y * 16 * 16 + z * 16 + x` (y-major for vertical locality)
- Each chunk tracks a heightmap (highest non-air y per column)

### Rendering Pipeline

- OpenGL 3.3 Core Profile
- Texture atlas: all block faces in a single texture
- Per-chunk mesh with face culling (then greedy meshing upgrade)
- Vertex format: position (3f), texcoord (2f), texIndex (1f), normal (3f), AO (1f)

### Game Loop

- Fixed tick rate: 20 ticks/second (50ms per tick)
- Rendering decoupled, uses interpolation for smooth visuals
- Single-threaded to start

### Naming Conventions (Google C++ Style)

- Classes/Structs: `PascalCase`
- Functions/Methods: `PascalCase` (e.g., `GetBlock()`)
- Variables: `snake_case`
- Member variables: `snake_case_` (trailing underscore)
- Constants: `kPascalCase`
- Namespaces: `snake_case`
- Files: `snake_case.cpp`, `snake_case.h`
- Test files: `test_<module>.cpp`
- Namespace: `vibecraft`

### Testing Conventions

- Framework: Google Test
- One test file per module/milestone area
- Test fixtures named `<Module>Test`
- Use `GTEST_SKIP()` for tests of unimplemented milestones
- Tests must never require a GPU/window (mock or skip rendering tests)
- Deterministic: no random seeds without explicit control

## Milestones

The project is built in 24 incremental milestones across 6 phases.
See `docs/milestones/` for detailed specifications.
Each milestone has strict test specs that must pass before proceeding.

## Key Design Rules

1. **No global state.** Pass dependencies explicitly.
2. **Chunk meshing** receives neighbor data as a parameter, never holds a world reference.
3. **Fluids** use cellular automata: water levels 0-7, lava levels 0-3.
4. **Lighting** uses BFS flood-fill, stored per-block (4 bits sun + 4 bits block light).
5. **World persistence** uses region files (32x32 chunks per file) with zlib compression.
