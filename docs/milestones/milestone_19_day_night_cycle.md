# Milestone 19 — Day/Night Cycle & Sky

## Description
Implement a day/night cycle with time progression, sun/moon positioning, ambient light changes, sky color transitions, and fog that matches the sky color.

## Goals
- Game time advances and wraps (0-24000 ticks, 20 min real-time cycle)
- Sun angle based on time (noon = directly overhead, midnight = directly below)
- Ambient light varies: day = 15, night = 4
- Sky color transitions smoothly (blue during day, dark blue/black at night, orange at sunset/sunrise)
- Fog color matches sky color for seamless horizon

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/sky.h` | Sky and day/night cycle class |
| `src/sky.cpp` | Sky implementation |
| `assets/shaders/sky.vert` | Sky vertex shader |
| `assets/shaders/sky.frag` | Sky fragment shader |

## Dependencies
- **M9** — Rendering pipeline
- **M15** — Lighting system (ambient light level)

## Test Specifications

### File: `tests/test_day_night.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `DayNight.TimeAdvances` | Time increases each tick | After N ticks, time == N |
| `DayNight.TimeWraps` | Time wraps at 24000 | At tick 24001, time == 1 |
| `DayNight.SunAngleNoon` | Sun at noon (6000 ticks) | Sun angle ≈ 90° (directly overhead) |
| `DayNight.SunAngleMidnight` | Sun at midnight (18000 ticks) | Sun angle ≈ 270° (directly below) |
| `DayNight.AmbientLightDay` | Daytime ambient light | At noon, ambient == 15 |
| `DayNight.AmbientLightNight` | Nighttime ambient light | At midnight, ambient == 4 |
| `DayNight.SkyColorDay` | Sky is blue during day | Sky color ≈ (0.5, 0.7, 1.0) at noon |
| `DayNight.SkyColorNight` | Sky is dark at night | Sky color ≈ (0.01, 0.01, 0.05) at midnight |
| `DayNight.SkyColorTransition` | Smooth transition at sunset | Sky color between day and night values at dusk (12000) |
| `DayNight.FogMatchesSky` | Fog color equals sky color | `GetFogColor() == GetSkyColor()` at any time |

## Design Decisions
- Time unit: ticks (0-24000), where 6000 = noon, 18000 = midnight
- Full day = 24000 ticks = 20 minutes at 20 ticks/second
- Sun angle = `(time / 24000) * 360°` with offset so noon = overhead
- Ambient light: lerp between 15 (day) and 4 (night) based on sun angle
- Sky color: lerp between day/sunset/night colors based on time ranges
- Fog distance and color set to match sky for horizon blending
- Sun/moon rendered as simple textured quads on a skybox
