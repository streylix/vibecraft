# Milestone 5 — Noise Generation

> **Status: COMPLETE** — All 10 tests passing. 2D/3D Perlin noise with octave layering, deterministic from seed.

## Description
Implement Perlin/Simplex noise for procedural terrain generation. Noise is the foundation of world generation — terrain height, biomes, caves, and ore distribution all depend on coherent noise functions.

## Goals
- 2D and 3D noise functions with configurable frequency and amplitude
- Octave (fractal) noise with persistence and lacunarity
- Deterministic output for a given seed
- Output normalized to [-1, 1] range

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/noise.h` | Noise generator class declaration |
| `src/noise.cpp` | Noise implementation |

## Dependencies
- **M1** — Build system

## Test Specifications

### File: `tests/test_noise.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Noise.RangeCheck2D` | 2D noise output range | 1000 samples all in [-1, 1] |
| `Noise.RangeCheck3D` | 3D noise output range | 1000 samples all in [-1, 1] |
| `Noise.Deterministic` | Same seed + same coords = same value | Two calls with identical params produce identical output |
| `Noise.DifferentSeedsDiffer` | Different seeds produce different terrain | At least some samples differ between two seeds |
| `Noise.SmoothContinuity` | Adjacent samples are close in value | `|noise(x) - noise(x+0.01)| < threshold` for small step |
| `Noise.OctaveAmplitude` | Octave noise respects persistence | 1-octave vs 4-octave produces different detail levels |
| `Noise.OctaveCount` | More octaves = more detail | Standard deviation of samples increases or detail changes |
| `Noise.FrequencyScaling` | Higher frequency = more variation | Noise at freq 1.0 vs freq 10.0 differ in variation pattern |
| `Noise.LargeCoordinates` | Noise works at large coords | No NaN/Inf at coords like (10000, 10000) |
| `Noise.NegativeCoordinates` | Noise works at negative coords | Valid output at (-100, -100) |

## Design Decisions
- Use Perlin noise (simpler to implement, good enough for voxel terrain)
- Permutation table seeded from world seed using a deterministic shuffle
- Octave noise: `sum(amplitude * noise(freq * pos))` with persistence (amplitude multiplier) and lacunarity (frequency multiplier)
- Default octave params: 4 octaves, persistence 0.5, lacunarity 2.0
- No external noise library — implement from scratch for full control
