#ifndef VIBECRAFT_CHUNK_H
#define VIBECRAFT_CHUNK_H

#include <array>
#include <cstdint>

#include "vibecraft/block.h"

namespace vibecraft {

/// Chunk dimensions.
constexpr int kChunkSizeX = 16;
constexpr int kChunkSizeY = 256;
constexpr int kChunkSizeZ = 16;
constexpr int kChunkVolume = kChunkSizeX * kChunkSizeY * kChunkSizeZ;  // 65,536
constexpr int kChunkColumns = kChunkSizeX * kChunkSizeZ;               // 256

/// A 16x16x256 column of blocks.
///
/// Block storage uses a flat array indexed as y * 256 + z * 16 + x (y-major
/// layout for vertical locality). The chunk also maintains a per-column
/// heightmap tracking the highest non-air block, and a dirty flag for
/// signaling that the mesh needs to be rebuilt.
class Chunk {
public:
    /// Construct a chunk at the given chunk coordinates.
    /// All blocks are initialized to Air (0). The dirty flag starts as true.
    explicit Chunk(int chunk_x = 0, int chunk_z = 0);

    /// Get the block at local coordinates (x, y, z).
    /// Returns Air (0) if coordinates are out of bounds.
    BlockId GetBlock(int x, int y, int z) const;

    /// Set the block at local coordinates (x, y, z).
    /// Out-of-bounds writes are silently ignored.
    /// Sets the dirty flag only if the block value actually changes.
    /// Updates the heightmap accordingly.
    void SetBlock(int x, int y, int z, BlockId id);

    /// Return true if the chunk has been modified since the last ClearDirty().
    bool IsDirty() const;

    /// Clear the dirty flag (typically after meshing).
    void ClearDirty();

    /// Manually set the dirty flag (e.g., when a neighbor boundary changes).
    void SetDirty();

    /// Get the highest non-air Y value for the column at (x, z).
    /// Returns -1 if the entire column is air.
    /// Returns -1 for out-of-bounds coordinates.
    int GetHeightmapValue(int x, int z) const;

    /// Get the chunk X coordinate in chunk space.
    int GetChunkX() const;

    /// Get the chunk Z coordinate in chunk space.
    int GetChunkZ() const;

    // --- Light storage ---
    // Each block stores 1 byte: upper 4 bits = sunlight (0-15),
    // lower 4 bits = block light (0-15).

    /// Get the sunlight level at local coordinates (x, y, z).
    /// Returns 0 if coordinates are out of bounds.
    int GetSunLight(int x, int y, int z) const;

    /// Set the sunlight level at local coordinates (x, y, z).
    /// Out-of-bounds writes are silently ignored.
    void SetSunLight(int x, int y, int z, int level);

    /// Get the block light level at local coordinates (x, y, z).
    /// Returns 0 if coordinates are out of bounds.
    int GetBlockLight(int x, int y, int z) const;

    /// Set the block light level at local coordinates (x, y, z).
    /// Out-of-bounds writes are silently ignored.
    void SetBlockLight(int x, int y, int z, int level);

    /// Get the raw light byte at local coordinates.
    /// Returns 0 if coordinates are out of bounds.
    uint8_t GetRawLight(int x, int y, int z) const;

    /// Set the raw light byte at local coordinates.
    void SetRawLight(int x, int y, int z, uint8_t value);

    // --- Fluid level storage ---
    // Each block stores a fluid level (0 = empty/minimum, 7 = full water source,
    // 3 = full lava source). Only meaningful when the block is Water or Lava.

    /// Get the fluid level at local coordinates (x, y, z).
    /// Returns 0 if coordinates are out of bounds.
    uint8_t GetFluidLevel(int x, int y, int z) const;

    /// Set the fluid level at local coordinates (x, y, z).
    /// Out-of-bounds writes are silently ignored.
    void SetFluidLevel(int x, int y, int z, uint8_t level);

private:
    /// Check whether local coordinates are within bounds.
    static bool InBounds(int x, int y, int z);

    /// Compute the flat array index for local coordinates.
    /// Caller must ensure coordinates are in bounds.
    static int Index(int x, int y, int z);

    /// Compute the heightmap index for a column.
    static int ColumnIndex(int x, int z);

    /// Recalculate the heightmap value for a single column by scanning downward.
    void RecalcHeightmap(int x, int z);

    std::array<BlockId, kChunkVolume> blocks_;
    std::array<uint8_t, kChunkVolume> light_;         // upper 4 bits = sun, lower 4 = block
    std::array<uint8_t, kChunkVolume> fluid_levels_;  // fluid level per block
    std::array<int16_t, kChunkColumns> heightmap_;
    bool dirty_;
    int chunk_x_;
    int chunk_z_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_CHUNK_H
