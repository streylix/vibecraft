#ifndef VIBECRAFT_WORLD_H
#define VIBECRAFT_WORLD_H

#include <cstddef>
#include <memory>
#include <unordered_map>

#include "vibecraft/block.h"
#include "vibecraft/chunk.h"

namespace vibecraft {

/// Chunk coordinate pair (cx, cz) identifying a chunk in the world.
struct ChunkCoord {
    int cx = 0;
    int cz = 0;

    bool operator==(const ChunkCoord& other) const {
        return cx == other.cx && cz == other.cz;
    }

    bool operator!=(const ChunkCoord& other) const {
        return !(*this == other);
    }
};

/// Hash function for ChunkCoord, suitable for use in std::unordered_map.
struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& coord) const {
        // Combine hashes using a method similar to boost::hash_combine.
        std::size_t h1 = std::hash<int>{}(coord.cx);
        std::size_t h2 = std::hash<int>{}(coord.cz);
        h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        return h1;
    }
};

/// The World class manages multiple chunks and provides transparent
/// block access using world (block) coordinates. It translates between
/// world coordinates and chunk/local coordinates, handles chunk
/// loading/unloading, and propagates dirty flags to neighbor chunks
/// on boundary edits.
class World {
public:
    World() = default;

    // Non-copyable, movable.
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = default;
    World& operator=(World&&) = default;

    /// Convert a world block X or Z coordinate to a chunk coordinate.
    /// Uses IntFloor(blockCoord / 16).
    static int WorldToChunkCoord(int block_coord);

    /// Convert a world block X or Z coordinate to a local coordinate [0,15].
    /// Uses Mod(blockCoord, 16).
    static int WorldToLocalCoord(int block_coord);

    /// Get the block at world coordinates (bx, by, bz).
    /// Returns Air (0) if the chunk is not loaded or coordinates are invalid.
    BlockId GetBlock(int bx, int by, int bz) const;

    /// Set the block at world coordinates (bx, by, bz).
    /// Creates the chunk if it does not exist (lazy creation).
    /// Propagates the dirty flag to neighbor chunks if the edit is on
    /// a chunk boundary (local x=0, x=15, z=0, or z=15).
    void SetBlock(int bx, int by, int bz, BlockId id);

    /// Get the fluid level at world coordinates (bx, by, bz).
    /// Returns 0 if the chunk is not loaded or coordinates are invalid.
    uint8_t GetFluidLevel(int bx, int by, int bz) const;

    /// Set the fluid level at world coordinates (bx, by, bz).
    /// Creates the chunk if it does not exist (lazy creation).
    void SetFluidLevel(int bx, int by, int bz, uint8_t level);

    /// Load (create) an empty chunk at the given chunk coordinates.
    /// If the chunk already exists, this is a no-op.
    void LoadChunk(int cx, int cz);

    /// Unload (remove) the chunk at the given chunk coordinates.
    /// If the chunk does not exist, this is a no-op.
    void UnloadChunk(int cx, int cz);

    /// Return true if a chunk is loaded at the given chunk coordinates.
    bool HasChunk(int cx, int cz) const;

    /// Get a pointer to the chunk at the given chunk coordinates.
    /// Returns nullptr if the chunk is not loaded.
    Chunk* GetChunk(int cx, int cz);
    const Chunk* GetChunk(int cx, int cz) const;

    /// Return the number of currently loaded chunks.
    std::size_t ChunkCount() const;

private:
    /// Get or create the chunk at the given chunk coordinates.
    Chunk& GetOrCreateChunk(int cx, int cz);

    /// Propagate the dirty flag to neighbor chunks if the local coordinate
    /// is on a boundary (0 or 15 for x or z).
    void PropagateDirty(int cx, int cz, int local_x, int local_z);

    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> chunks_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_WORLD_H
