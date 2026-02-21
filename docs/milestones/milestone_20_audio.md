# Milestone 20 — Audio System

> **Status: COMPLETE** — All 9 tests passing. Sound registry, volume clamping, distance attenuation, headless mode for testing, 4 WAV placeholders.

## Description
Implement a basic audio system for playing sound effects (block break/place, footsteps, ambient sounds) with volume control and distance-based attenuation.

## Goals
- Audio engine initialization and shutdown
- All game sounds registered and backed by real WAV files
- Sound playback without crashes
- Volume clamping to [0.0, 1.0]
- Distance-based attenuation for positional audio

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/audio.h` | Audio system class |
| `src/audio.cpp` | Audio implementation |
| `assets/sounds/*.wav` | Sound effect files |

## Dependencies
- **M1** — Build system
- **M13** — Block interaction (trigger sounds on break/place)

## Test Specifications

### File: `tests/test_audio.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Audio.InitShutdown` | System initializes and shuts down cleanly | No crashes or leaks |
| `Audio.AllSoundsRegistered` | All expected sounds are registered | Break, place, footstep, ambient sounds all in registry |
| `Audio.SoundFilesExist` | WAV files exist on disk | Every registered sound points to an existing file |
| `Audio.PlayNoThrow` | Playing a sound doesn't throw | `Play("block_break")` completes without exception |
| `Audio.VolumeClampLow` | Volume below 0 clamped to 0 | `SetVolume(-0.5)` → effective volume == 0.0 |
| `Audio.VolumeClampHigh` | Volume above 1 clamped to 1 | `SetVolume(1.5)` → effective volume == 1.0 |
| `Audio.DistanceAttenuation` | Sound quieter at distance | Volume at distance 10 < volume at distance 1 |
| `Audio.ZeroDistanceFullVolume` | Sound at distance 0 = full volume | Attenuation factor == 1.0 at distance 0 |
| `Audio.MaxDistanceSilent` | Sound beyond max distance is silent | Attenuation factor == 0.0 beyond max distance |

## Sound List
| Name | File | Trigger |
|------|------|---------|
| block_break | block_break.wav | Breaking a block |
| block_place | block_place.wav | Placing a block |
| footstep | footstep.wav | Walking |
| splash | splash.wav | Entering water |

## Design Decisions
- Use miniaudio (header-only, cross-platform) for audio playback
- Sound registry maps string names to file paths
- Distance attenuation: `volume * max(0, 1 - distance / max_distance)`
- Max audio distance: 16 blocks
- Multiple sounds can play simultaneously (fire-and-forget)
- Audio system can be disabled (headless mode for testing)
