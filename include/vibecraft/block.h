#ifndef VIBECRAFT_BLOCK_H
#define VIBECRAFT_BLOCK_H

#include <array>
#include <cstdint>
#include <string>

namespace vibecraft {

/// Block identifier. Max 256 block types (0-255).
using BlockId = uint8_t;

/// Texture face indices for a block.
/// Order: +X, -X, +Y (top), -Y (bottom), +Z, -Z
struct BlockFaces {
    int pos_x = -1;
    int neg_x = -1;
    int pos_y = -1;  // top
    int neg_y = -1;  // bottom
    int pos_z = -1;
    int neg_z = -1;

    /// Construct with all faces using the same texture index.
    static BlockFaces All(int index) {
        return {index, index, index, index, index, index};
    }

    /// Construct with separate top, bottom, and side textures.
    static BlockFaces TopBottomSide(int top, int bottom, int side) {
        return {side, side, top, bottom, side, side};
    }

    /// Construct with separate top and side textures (bottom same as top).
    static BlockFaces TopSide(int top, int side) {
        return {side, side, top, top, side, side};
    }
};

/// Properties for a single block type.
struct BlockType {
    std::string name;
    bool solid = false;
    bool transparent = false;
    bool liquid = false;
    float hardness = 0.0f;
    int light_emission = 0;
    BlockFaces faces;
};

/// Centralized registry of all block types.
/// Provides O(1) lookup by BlockId using a flat array.
class BlockRegistry {
public:
    /// Construct the registry and register all default block types.
    BlockRegistry();

    /// Get the block type for a given id. Returns a const reference.
    const BlockType& GetBlock(BlockId id) const;

    /// Return the number of registered blocks (those with non-empty names).
    int GetRegisteredCount() const;

    /// Named block id constants for convenience.
    static constexpr BlockId kAir = 0;
    static constexpr BlockId kStone = 1;
    static constexpr BlockId kGrass = 2;
    static constexpr BlockId kDirt = 3;
    static constexpr BlockId kCobblestone = 4;
    static constexpr BlockId kOakPlanks = 5;
    static constexpr BlockId kBedrock = 6;
    static constexpr BlockId kSand = 7;
    static constexpr BlockId kGravel = 8;
    static constexpr BlockId kGoldOre = 9;
    static constexpr BlockId kIronOre = 10;
    static constexpr BlockId kCoalOre = 11;
    static constexpr BlockId kDiamondOre = 12;
    static constexpr BlockId kOakLog = 13;
    static constexpr BlockId kOakLeaves = 14;
    static constexpr BlockId kGlass = 15;
    static constexpr BlockId kWater = 16;
    static constexpr BlockId kLava = 17;
    static constexpr BlockId kTorch = 18;
    static constexpr BlockId kSnow = 19;
    static constexpr BlockId kCactus = 20;

private:
    /// Register a block type at the given id.
    void Register(BlockId id, BlockType type);

    /// Register all default block types.
    void RegisterDefaults();

    std::array<BlockType, 256> blocks_;
    int registered_count_ = 0;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_BLOCK_H
