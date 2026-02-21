#ifndef VIBECRAFT_LIGHTING_H
#define VIBECRAFT_LIGHTING_H

#include <queue>
#include <tuple>
#include <vector>

#include "vibecraft/block.h"
#include "vibecraft/world.h"

namespace vibecraft {

/// Entry for the BFS light propagation queue.
/// Stores world coordinates and current light level.
struct LightNode {
    int x;
    int y;
    int z;
    int level;
};

/// Entry for the BFS light removal queue.
/// Stores world coordinates and the light level that was present.
struct LightRemovalNode {
    int x;
    int y;
    int z;
    int level;
};

/// The LightingEngine handles sunlight and block light propagation
/// using BFS flood-fill. Light values are stored per-block in each Chunk
/// (upper 4 bits = sunlight, lower 4 bits = block light).
///
/// Block light is emitted by light sources (lava=15, torch=14) and
/// attenuates by 1 per block. Sunlight starts at 15 from the sky and
/// propagates straight down at full intensity through transparent blocks,
/// then spreads horizontally with -1 attenuation.
///
/// Solid blocks stop light propagation entirely. Transparent blocks
/// (glass, air, etc.) allow light to pass with normal -1 attenuation.
class LightingEngine {
public:
    /// Construct a LightingEngine that operates on the given world
    /// using the given block registry for block properties.
    LightingEngine(World& world, const BlockRegistry& registry);

    /// Propagate block light from a single light source at world coords.
    /// Reads the block at (bx, by, bz) to determine emission level,
    /// then performs BFS flood-fill to propagate light.
    void AddBlockLight(int bx, int by, int bz);

    /// Remove block light originating from (bx, by, bz).
    /// Uses reverse BFS to collect affected blocks, then re-propagates
    /// from any remaining light sources.
    void RemoveBlockLight(int bx, int by, int bz);

    /// Propagate sunlight for a single column at world (bx, bz).
    /// Sunlight starts at 15 from the top of the world and propagates
    /// downward through transparent blocks at full intensity.
    void PropagateSunlightColumn(int bx, int bz);

    /// Calculate and propagate sunlight for all loaded chunks.
    void CalculateSunlight();

    /// Calculate and propagate block light for all loaded chunks.
    /// Scans all blocks for light-emitting blocks and propagates.
    void CalculateBlockLight();

    /// Full lighting calculation: sunlight + block light for all loaded chunks.
    void CalculateAllLighting();

    /// Get the block light level at world coordinates.
    /// Returns 0 if the chunk is not loaded.
    int GetBlockLight(int bx, int by, int bz) const;

    /// Get the sunlight level at world coordinates.
    /// Returns 0 if the chunk is not loaded.
    int GetSunLight(int bx, int by, int bz) const;

    /// Set the block light level at world coordinates.
    void SetBlockLight(int bx, int by, int bz, int level);

    /// Set the sunlight level at world coordinates.
    void SetSunLight(int bx, int by, int bz, int level);

private:
    /// Check if a block at world coords is transparent (allows light).
    bool IsTransparent(int bx, int by, int bz) const;

    /// Get the light emission of the block at world coords.
    int GetEmission(int bx, int by, int bz) const;

    /// BFS propagation of block light from the queue.
    void PropagateBlockLightBFS(std::queue<LightNode>& queue);

    /// BFS propagation of sunlight from the queue.
    void PropagateSunLightBFS(std::queue<LightNode>& queue);

    World& world_;
    const BlockRegistry& registry_;

    /// The 6 cardinal directions for BFS neighbor traversal.
    static constexpr int kDirs[6][3] = {
        { 1,  0,  0},
        {-1,  0,  0},
        { 0,  1,  0},
        { 0, -1,  0},
        { 0,  0,  1},
        { 0,  0, -1}
    };
};

}  // namespace vibecraft

#endif  // VIBECRAFT_LIGHTING_H
