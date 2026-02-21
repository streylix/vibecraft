#include "vibecraft/noise.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace vibecraft {

PerlinNoise::PerlinNoise(uint32_t seed) : seed_(seed) {
    // Initialize permutation table with values 0..255.
    std::array<int, 256> base{};
    std::iota(base.begin(), base.end(), 0);

    // Deterministic shuffle using the seed.
    std::mt19937 rng(seed);
    std::shuffle(base.begin(), base.end(), rng);

    // Double the permutation table to avoid wrapping with modulo.
    for (int i = 0; i < 256; ++i) {
        perm_[i] = base[i];
        perm_[i + 256] = base[i];
    }
}

float PerlinNoise::Fade(float t) {
    // Improved Perlin fade: 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float PerlinNoise::Lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float PerlinNoise::Grad2D(int hash, float x, float y) {
    // Use the low 2 bits to select one of 4 gradient directions.
    int h = hash & 3;
    switch (h) {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        case 3: return -x - y;
        default: return 0.0f;  // unreachable
    }
}

float PerlinNoise::Grad3D(int hash, float x, float y, float z) {
    // Use the low 4 bits to select one of 12 gradient directions.
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float PerlinNoise::Noise2D(float x, float y) const {
    // Find unit grid cell containing point.
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));

    // Relative position within the cell.
    float xf = x - static_cast<float>(xi);
    float yf = y - static_cast<float>(yi);

    // Wrap to 0..255.
    xi &= 255;
    yi &= 255;

    // Fade curves for interpolation weights.
    float u = Fade(xf);
    float v = Fade(yf);

    // Hash coordinates of the 4 corners.
    int aa = perm_[perm_[xi] + yi];
    int ab = perm_[perm_[xi] + yi + 1];
    int ba = perm_[perm_[xi + 1] + yi];
    int bb = perm_[perm_[xi + 1] + yi + 1];

    // Gradient dot products at each corner, then bilinear interpolation.
    float x1 = Lerp(Grad2D(aa, xf, yf),
                     Grad2D(ba, xf - 1.0f, yf), u);
    float x2 = Lerp(Grad2D(ab, xf, yf - 1.0f),
                     Grad2D(bb, xf - 1.0f, yf - 1.0f), u);

    return Lerp(x1, x2, v);
}

float PerlinNoise::Noise3D(float x, float y, float z) const {
    // Find unit cube containing point.
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    int zi = static_cast<int>(std::floor(z));

    // Relative position within the cube.
    float xf = x - static_cast<float>(xi);
    float yf = y - static_cast<float>(yi);
    float zf = z - static_cast<float>(zi);

    // Wrap to 0..255.
    xi &= 255;
    yi &= 255;
    zi &= 255;

    // Fade curves for interpolation weights.
    float u = Fade(xf);
    float v = Fade(yf);
    float w = Fade(zf);

    // Hash coordinates of the 8 cube corners.
    int aaa = perm_[perm_[perm_[xi] + yi] + zi];
    int aba = perm_[perm_[perm_[xi] + yi + 1] + zi];
    int aab = perm_[perm_[perm_[xi] + yi] + zi + 1];
    int abb = perm_[perm_[perm_[xi] + yi + 1] + zi + 1];
    int baa = perm_[perm_[perm_[xi + 1] + yi] + zi];
    int bba = perm_[perm_[perm_[xi + 1] + yi + 1] + zi];
    int bab = perm_[perm_[perm_[xi + 1] + yi] + zi + 1];
    int bbb = perm_[perm_[perm_[xi + 1] + yi + 1] + zi + 1];

    // Gradient dot products at each corner, then trilinear interpolation.
    float x1 = Lerp(Grad3D(aaa, xf, yf, zf),
                     Grad3D(baa, xf - 1.0f, yf, zf), u);
    float x2 = Lerp(Grad3D(aba, xf, yf - 1.0f, zf),
                     Grad3D(bba, xf - 1.0f, yf - 1.0f, zf), u);
    float y1 = Lerp(x1, x2, v);

    float x3 = Lerp(Grad3D(aab, xf, yf, zf - 1.0f),
                     Grad3D(bab, xf - 1.0f, yf, zf - 1.0f), u);
    float x4 = Lerp(Grad3D(abb, xf, yf - 1.0f, zf - 1.0f),
                     Grad3D(bbb, xf - 1.0f, yf - 1.0f, zf - 1.0f), u);
    float y2 = Lerp(x3, x4, v);

    return Lerp(y1, y2, w);
}

float PerlinNoise::OctaveNoise2D(float x, float y,
                                  int octaves,
                                  float persistence,
                                  float lacunarity,
                                  float frequency) const {
    float total = 0.0f;
    float amplitude = 1.0f;
    float max_amplitude = 0.0f;  // For normalization.

    for (int i = 0; i < octaves; ++i) {
        total += amplitude * Noise2D(x * frequency, y * frequency);
        max_amplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    // Normalize to [-1, 1].
    return total / max_amplitude;
}

float PerlinNoise::OctaveNoise3D(float x, float y, float z,
                                  int octaves,
                                  float persistence,
                                  float lacunarity,
                                  float frequency) const {
    float total = 0.0f;
    float amplitude = 1.0f;
    float max_amplitude = 0.0f;  // For normalization.

    for (int i = 0; i < octaves; ++i) {
        total += amplitude * Noise3D(x * frequency, y * frequency, z * frequency);
        max_amplitude += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    // Normalize to [-1, 1].
    return total / max_amplitude;
}

}  // namespace vibecraft
