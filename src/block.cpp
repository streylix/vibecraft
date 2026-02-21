#include "vibecraft/block.h"

namespace vibecraft {

BlockRegistry::BlockRegistry() {
    // Initialize all blocks to default (empty name, not solid, etc.)
    for (auto& block : blocks_) {
        block = BlockType{};
    }
    RegisterDefaults();
}

const BlockType& BlockRegistry::GetBlock(BlockId id) const {
    return blocks_[id];
}

int BlockRegistry::GetRegisteredCount() const {
    return registered_count_;
}

void BlockRegistry::Register(BlockId id, BlockType type) {
    blocks_[id] = std::move(type);
    ++registered_count_;
}

void BlockRegistry::RegisterDefaults() {
    // Texture indices are placeholders for now — they reference positions in
    // the texture atlas that will be created in a later milestone.
    // Each unique texture gets a unique index starting from 0.

    // Texture index assignments (placeholder):
    // 0  = stone
    // 1  = grass_top
    // 2  = grass_side
    // 3  = dirt
    // 4  = cobblestone
    // 5  = oak_planks
    // 6  = bedrock
    // 7  = sand
    // 8  = gravel
    // 9  = gold_ore
    // 10 = iron_ore
    // 11 = coal_ore
    // 12 = diamond_ore
    // 13 = oak_log_top
    // 14 = oak_log_side
    // 15 = oak_leaves
    // 16 = glass
    // 17 = water
    // 18 = lava
    // 19 = torch
    // 20 = snow
    // 21 = cactus_top
    // 22 = cactus_side

    // 0: Air — not solid, transparent, no textures needed
    Register(kAir, BlockType{
        "Air",        // name
        false,        // solid
        true,         // transparent
        false,        // liquid
        0.0f,         // hardness
        0,            // light_emission
        BlockFaces{}  // no textures (all -1)
    });

    // 1: Stone
    Register(kStone, BlockType{
        "Stone", true, false, false, 1.5f, 0,
        BlockFaces::All(0)
    });

    // 2: Grass — different top (grass_top), bottom (dirt), sides (grass_side)
    Register(kGrass, BlockType{
        "Grass", true, false, false, 0.6f, 0,
        BlockFaces::TopBottomSide(1, 3, 2)
    });

    // 3: Dirt
    Register(kDirt, BlockType{
        "Dirt", true, false, false, 0.5f, 0,
        BlockFaces::All(3)
    });

    // 4: Cobblestone
    Register(kCobblestone, BlockType{
        "Cobblestone", true, false, false, 2.0f, 0,
        BlockFaces::All(4)
    });

    // 5: Oak Planks
    Register(kOakPlanks, BlockType{
        "Oak Planks", true, false, false, 2.0f, 0,
        BlockFaces::All(5)
    });

    // 6: Bedrock — negative hardness means unbreakable
    Register(kBedrock, BlockType{
        "Bedrock", true, false, false, -1.0f, 0,
        BlockFaces::All(6)
    });

    // 7: Sand
    Register(kSand, BlockType{
        "Sand", true, false, false, 0.5f, 0,
        BlockFaces::All(7)
    });

    // 8: Gravel
    Register(kGravel, BlockType{
        "Gravel", true, false, false, 0.6f, 0,
        BlockFaces::All(8)
    });

    // 9: Gold Ore
    Register(kGoldOre, BlockType{
        "Gold Ore", true, false, false, 3.0f, 0,
        BlockFaces::All(9)
    });

    // 10: Iron Ore
    Register(kIronOre, BlockType{
        "Iron Ore", true, false, false, 3.0f, 0,
        BlockFaces::All(10)
    });

    // 11: Coal Ore
    Register(kCoalOre, BlockType{
        "Coal Ore", true, false, false, 3.0f, 0,
        BlockFaces::All(11)
    });

    // 12: Diamond Ore
    Register(kDiamondOre, BlockType{
        "Diamond Ore", true, false, false, 3.0f, 0,
        BlockFaces::All(12)
    });

    // 13: Oak Log — different top (log_top) vs sides (log_side)
    Register(kOakLog, BlockType{
        "Oak Log", true, false, false, 2.0f, 0,
        BlockFaces::TopSide(13, 14)
    });

    // 14: Oak Leaves — solid but transparent (for rendering)
    Register(kOakLeaves, BlockType{
        "Oak Leaves", true, true, false, 0.2f, 0,
        BlockFaces::All(15)
    });

    // 15: Glass — solid but transparent
    Register(kGlass, BlockType{
        "Glass", true, true, false, 0.3f, 0,
        BlockFaces::All(16)
    });

    // 16: Water — not solid, transparent, liquid
    Register(kWater, BlockType{
        "Water", false, true, true, 0.0f, 0,
        BlockFaces::All(17)
    });

    // 17: Lava — not solid, transparent, liquid, emits max light
    Register(kLava, BlockType{
        "Lava", false, true, true, 0.0f, 15,
        BlockFaces::All(18)
    });

    // 18: Torch — not solid, transparent, emits light level 14
    Register(kTorch, BlockType{
        "Torch", false, true, false, 0.0f, 14,
        BlockFaces::All(19)
    });

    // 19: Snow
    Register(kSnow, BlockType{
        "Snow", true, false, false, 0.1f, 0,
        BlockFaces::All(20)
    });

    // 20: Cactus — solid, transparent (for rendering side cutouts)
    Register(kCactus, BlockType{
        "Cactus", true, true, false, 0.4f, 0,
        BlockFaces::TopBottomSide(21, 21, 22)
    });

    // 21: Obsidian — very hard, created by water+lava interactions
    Register(kObsidian, BlockType{
        "Obsidian", true, false, false, 50.0f, 0,
        BlockFaces::All(23)
    });
}

}  // namespace vibecraft
