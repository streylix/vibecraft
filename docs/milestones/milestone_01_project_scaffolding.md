# Milestone 1 — Project Scaffolding & Build System

## Description
Set up the foundational build system, dependency management, and project structure for VibeCraft. This milestone ensures C++17 compilation, vcpkg-based dependency resolution, and a working Google Test harness.

## Goals
- CMake project compiles with C++17
- vcpkg manages glfw3, glm, gtest, OpenGL dependencies
- Google Test harness runs and reports results
- Project directory structure established
- `.clang-format` enforces Google C++ Style

## Deliverables
| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Top-level build configuration |
| `vcpkg.json` | Dependency manifest |
| `.clang-format` | Code style enforcement |
| `src/main.cpp` | Minimal entry point |
| `CLAUDE.md` | Project conventions and AI guidance |

## Dependencies
None — this is the foundation milestone.

## Test Specifications

### File: `tests/test_build_system.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `BuildSystem.CppStandardIs17` | `__cplusplus >= 201703L` | True |
| `BuildSystem.GlmAvailable` | GLM headers include and `glm::vec3` constructible | Compiles and vec3{1,2,3} has correct components |
| `BuildSystem.GlfwLinked` | GLFW header includes without error | `GLFW_VERSION_MAJOR >= 3` |
| `BuildSystem.GtestWorks` | Google Test itself runs | `EXPECT_TRUE(true)` passes |

## Design Decisions
- vcpkg in manifest mode (vcpkg.json) for reproducible builds
- CMake 3.20+ required for modern features
- Game code compiled as a static library (`vibecraft_lib`) shared between the game executable and tests
- `src/main.cpp` filtered out of the library to avoid duplicate `main()` with gtest
