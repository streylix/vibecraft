# VibeCraft

My vibathon submission that tries to recreate minecraft purely from vibe coding. This was done by first planning out a 20 step phase with each phase a document detailing its implementation and tests. Then using a claude-code session, I instructed it to use a separate agent to implement each step, write the tests and verify the implementation worked. In just one claude chat, it was able to implement all 20 in just under 2 hours. The result is 100% vibe coded. There were some fixes that had to be made after all steps completed however the current state it is in is playable. The point of this is to demonstrate how effective vibe coding can be with proper guidance and preparation.

![macOS](https://img.shields.io/badge/platform-macOS-lightgrey)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![OpenGL 3.3](https://img.shields.io/badge/OpenGL-3.3-green)

## Features

- Procedural terrain generation with Perlin noise
- 5 biomes (Plains, Forest, Desert, Mountains, Tundra)
- Caves and ore generation
- Block breaking and placing
- Greedy meshing for optimized chunk rendering
- Day/night cycle with dynamic sky colors
- Player physics with gravity, jumping, and collision
- Hotbar HUD with block texture icons
- BFS flood-fill lighting (sun + block light)
- Fluid simulation (water and lava)
- World save/load with region files and zlib compression

## Prerequisites

- **CMake** 3.20+
- **C++17** compiler (Clang on macOS)
- **vcpkg** (included as a submodule)

Dependencies (managed by vcpkg):
- GLFW 3
- GLM
- Google Test
- zlib

## Build & Run

```bash
# Clone the repo (--recursive pulls the vcpkg submodule)
git clone --recursive https://github.com/streylix/vibecraft.git
cd vibecraft

# Bootstrap vcpkg (first time only)
./vcpkg/bootstrap-vcpkg.sh

# Configure
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build

# Run
./build/vibecraft
```

## Controls

| Key | Action |
|-----|--------|
| WASD | Move |
| Mouse | Look around |
| Space | Jump |
| Left click (hold) | Break block |
| Right click | Place block |
| 1-9 | Select hotbar slot |
| Scroll wheel | Cycle hotbar |
| Escape | Quit |
| F3 | Toggle debug overlay |
| F4 | Toggle wireframe |

## Running Tests

```bash
ctest --test-dir build --output-on-failure
```

## Project Structure

```
src/                  Game source files
include/vibecraft/    Public headers
tests/                Google Test files
assets/
  shaders/            GLSL vertex/fragment shaders
  textures/           16x16 pixel-art block textures
  sounds/             Sound effects
docs/milestones/      Milestone specifications
```

## License

MIT
