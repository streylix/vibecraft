# Milestone 24 — UI Screens, Dev Tools & Polish

## Description
Final polish milestone adding UI screen management, settings system, debug overlay (F3), ambient occlusion, fog rendering, and fixed timestep game loop.

## Goals
- Screen stack system (push/pop game screens)
- Settings with defaults, clamping, and persistence
- F3 debug overlay showing FPS, coordinates, chunk, biome
- Ambient occlusion computed during meshing
- Linear fog interpolation
- Fixed timestep accumulator for deterministic physics

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/screen.h` | Screen manager class |
| `src/screen.cpp` | Screen management implementation |
| `include/vibecraft/settings.h` | Settings class |
| `src/settings.cpp` | Settings implementation |
| `include/vibecraft/debug_overlay.h` | Debug overlay class |
| `src/debug_overlay.cpp` | Debug overlay implementation |
| `include/vibecraft/game_loop.h` | Game loop with fixed timestep |
| `src/game_loop.cpp` | Game loop implementation |

## Dependencies
- **M9** — Rendering
- **M18** — HUD/font rendering
- **M12** — Input handling
- **M15** — Lighting (AO integration)

## Test Specifications

### File: `tests/test_ui_settings.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Screen.PushPop` | Screen stack push and pop | Push 2 screens, top is second; pop, top is first |
| `Screen.EmptyStack` | Pop from empty stack | No crash, returns null/false |
| `Settings.Defaults` | Default settings values | render_distance == 8, fov == 70, volume == 1.0 |
| `Settings.ClampRenderDistance` | Render distance clamped | Setting to 100 clamps to max (e.g., 32) |
| `Settings.ClampFOV` | FOV clamped to valid range | Setting to 200 clamps to max (e.g., 120) |
| `Settings.ClampVolume` | Volume clamped [0, 1] | Setting to 1.5 clamps to 1.0 |
| `Settings.Persist` | Settings save and load | Save settings, load → identical |

### File: `tests/test_debug_overlay.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `DebugOverlay.FPSDisplay` | FPS value formatted | Given FPS 60 → string contains "60" |
| `DebugOverlay.CoordsDisplay` | Position formatted | Given pos (1.5, 64, 2.3) → string contains coordinates |
| `DebugOverlay.ChunkDisplay` | Chunk coords formatted | Given pos in chunk (0,0) → string contains "0, 0" |
| `DebugOverlay.BiomeDisplay` | Biome name formatted | Given Plains biome → string contains "Plains" |

### File: `tests/test_game_loop.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `GameLoop.AOCornerValues` | AO computed for cube corners | Corner with 3 neighbors = darkest (0.0), 0 neighbors = brightest (1.0) |
| `GameLoop.FogLinearInterp` | Fog interpolates linearly | At 50% between start/end → fog factor = 0.5 |
| `GameLoop.FixedTimestepAccumulator` | Accumulator processes correct ticks | Given 75ms elapsed at 50ms/tick → 1 tick processed, 25ms remaining |
| `GameLoop.FixedTimestepMultiple` | Multiple ticks in one frame | Given 150ms elapsed at 50ms/tick → 3 ticks processed |

## Design Decisions
- Screen stack: `std::vector<std::unique_ptr<Screen>>` with push/pop
- Screen interface: `OnEnter()`, `OnExit()`, `Update()`, `Render()`, `HandleInput()`
- Settings stored as key-value pairs, saved to JSON file
- Debug overlay toggled with F3 key
- AO: per-vertex, computed during meshing based on adjacent solid blocks
  - 4 possible values per corner: 0 (darkest, 3 neighbors), 1, 2, 3 (brightest, 0 neighbors)
  - Normalized to [0, 1]: values {0.2, 0.4, 0.7, 1.0}
- Fog: linear interpolation between fog start distance and fog end distance
  - Factor = (end - dist) / (end - start), clamped [0, 1]
  - Fog color = sky color (from M19)
- Fixed timestep: accumulator pattern
  - Accumulate real elapsed time
  - While accumulator >= tick_duration: simulate one tick, subtract tick_duration
  - Render with interpolation factor = accumulator / tick_duration
