#ifndef VIBECRAFT_WORLD_SAVE_H
#define VIBECRAFT_WORLD_SAVE_H

#include <cstdint>
#include <string>

#include <glm/vec3.hpp>

#include "vibecraft/region.h"
#include "vibecraft/world.h"

namespace vibecraft {

/// World metadata stored in world.dat.
struct WorldMetadata {
    glm::vec3 player_position{0.0f, 0.0f, 0.0f};
    uint64_t world_seed = 0;
    float game_time = 0.0f;
};

/// Coordinates saving and loading of world state to disk.
///
/// Directory structure:
///   saves/<world_name>/
///     world.dat          — player pos, seed, game time (simple binary)
///     regions/
///       region_<rx>_<rz>.dat — region files
///
/// Usage:
///   WorldSave saver("saves/my_world");
///   saver.SaveWorld(world, metadata);
///   saver.LoadWorld(world, metadata);
class WorldSave {
public:
    /// Construct a WorldSave with the given save directory path.
    explicit WorldSave(const std::string& save_dir);

    /// Save all loaded chunks in the world and metadata to disk.
    /// Creates the save directory structure if it doesn't exist.
    /// Returns true on success.
    bool SaveWorld(const World& world, const WorldMetadata& metadata);

    /// Load all chunks from disk into the world, and load metadata.
    /// The world should typically be empty before calling this.
    /// Returns true on success (at least world.dat was readable).
    bool LoadWorld(World& world, WorldMetadata& metadata);

    /// Save a single chunk to its region file.
    bool SaveChunk(const Chunk& chunk);

    /// Load a single chunk from its region file.
    /// Returns nullptr if no saved data exists for this chunk.
    std::unique_ptr<Chunk> LoadChunk(int cx, int cz);

    /// Save world metadata (player position, seed, game time).
    bool SaveMetadata(const WorldMetadata& metadata);

    /// Load world metadata. Returns true on success.
    bool LoadMetadata(WorldMetadata& metadata);

    /// Get the save directory path.
    const std::string& GetSaveDir() const;

    /// Get the regions subdirectory path.
    std::string GetRegionsDir() const;

    /// Get the region file path for a given region coordinate.
    std::string GetRegionFilePath(int rx, int rz) const;

    /// Get the world.dat file path.
    std::string GetMetadataFilePath() const;

private:
    /// Ensure the save directory and regions subdirectory exist.
    bool EnsureDirectories();

    std::string save_dir_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_WORLD_SAVE_H
