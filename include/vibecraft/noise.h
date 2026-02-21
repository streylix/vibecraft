#ifndef VIBECRAFT_NOISE_H
#define VIBECRAFT_NOISE_H

#include <array>
#include <cstdint>

namespace vibecraft {

/// Perlin noise generator with deterministic seeding.
/// Supports 2D and 3D noise, plus octave (fractal) noise
/// with configurable persistence and lacunarity.
class PerlinNoise {
public:
    /// Construct a Perlin noise generator with the given seed.
    /// The seed determines the permutation table used for hashing.
    explicit PerlinNoise(uint32_t seed = 0);

    /// Evaluate 2D Perlin noise at (x, y).
    /// Returns a value approximately in [-1, 1].
    float Noise2D(float x, float y) const;

    /// Evaluate 3D Perlin noise at (x, y, z).
    /// Returns a value approximately in [-1, 1].
    float Noise3D(float x, float y, float z) const;

    /// Evaluate 2D fractal (octave) noise at (x, y).
    /// @param x, y         Sample coordinates
    /// @param octaves      Number of noise layers to sum (default 4)
    /// @param persistence  Amplitude multiplier per octave (default 0.5)
    /// @param lacunarity   Frequency multiplier per octave (default 2.0)
    /// @param frequency    Base frequency applied to coordinates (default 1.0)
    /// Returns a value approximately in [-1, 1].
    float OctaveNoise2D(float x, float y,
                        int octaves = 4,
                        float persistence = 0.5f,
                        float lacunarity = 2.0f,
                        float frequency = 1.0f) const;

    /// Evaluate 3D fractal (octave) noise at (x, y, z).
    /// @param x, y, z      Sample coordinates
    /// @param octaves      Number of noise layers to sum (default 4)
    /// @param persistence  Amplitude multiplier per octave (default 0.5)
    /// @param lacunarity   Frequency multiplier per octave (default 2.0)
    /// @param frequency    Base frequency applied to coordinates (default 1.0)
    /// Returns a value approximately in [-1, 1].
    float OctaveNoise3D(float x, float y, float z,
                        int octaves = 4,
                        float persistence = 0.5f,
                        float lacunarity = 2.0f,
                        float frequency = 1.0f) const;

    /// Get the seed used by this noise generator.
    uint32_t GetSeed() const { return seed_; }

private:
    /// Fade curve: 6t^5 - 15t^4 + 10t^3  (Perlin's improved fade).
    static float Fade(float t);

    /// Linear interpolation.
    static float Lerp(float a, float b, float t);

    /// Compute gradient dot product for 2D noise.
    static float Grad2D(int hash, float x, float y);

    /// Compute gradient dot product for 3D noise.
    static float Grad3D(int hash, float x, float y, float z);

    /// The seed used to generate the permutation table.
    uint32_t seed_;

    /// Permutation table (doubled to avoid wrapping).
    std::array<int, 512> perm_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_NOISE_H
