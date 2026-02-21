# Milestone 23 — Particle System & Weather

## Description
Implement a particle system for visual effects (block breaking, torch sparks) and a weather system (clear, rain, snow) that varies by biome.

## Goals
- Particle emitter with configurable lifetime, gravity, and color
- Maximum particle count to prevent performance issues
- Block break particles colored to match broken block
- Weather states: clear, rain, snow
- Weather varies by biome (no rain in desert, snow in tundra)
- Particle update (position, lifetime) each frame

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/particle.h` | Particle system class |
| `src/particle.cpp` | Particle implementation |
| `include/vibecraft/weather.h` | Weather system class |
| `src/weather.cpp` | Weather implementation |
| `assets/shaders/particle.vert` | Particle vertex shader |
| `assets/shaders/particle.frag` | Particle fragment shader |

## Dependencies
- **M9** — Rendering pipeline
- **M16** — Biome system (weather per biome)
- **M3** — Block types (particle colors)

## Test Specifications

### File: `tests/test_particles.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Particles.Emit` | Emitter creates particles | After emit, particle count > 0 |
| `Particles.UpdatePosition` | Particles move over time | Position changes after update |
| `Particles.Lifetime` | Particles expire | After lifetime elapsed, particle removed |
| `Particles.Gravity` | Particles fall with gravity | Y position decreases over time |
| `Particles.MaxLimit` | Particle count capped | Can't exceed max particles (e.g., 1000) |
| `Particles.BlockBreakColor` | Break particles match block color | Stone break → gray particles |

### File: `tests/test_weather.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Weather.DefaultClear` | Weather starts clear | Initial state == Clear |
| `Weather.RainState` | Rain can be activated | SetWeather(Rain) → GetWeather() == Rain |
| `Weather.SnowState` | Snow can be activated | SetWeather(Snow) → GetWeather() == Snow |
| `Weather.BiomeDesertNoRain` | Desert doesn't get rain | GetWeatherForBiome(Desert) != Rain |
| `Weather.BiomeTundraSnow` | Tundra gets snow not rain | GetWeatherForBiome(Tundra) == Snow |

## Design Decisions
- Particles: simple point sprites or small quads
- Particle struct: position, velocity, color, lifetime, age
- Pool allocator: pre-allocate max particles, reuse expired slots
- Max particles: 1000 (configurable)
- Block break: emit 10-20 particles with random velocity, colored by block texture dominant color
- Weather particles: rain = blue streaks, snow = white dots
- Weather transition: fade between states over 5 seconds
- Rain/snow only rendered within 20 blocks of player for performance
