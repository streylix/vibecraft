#include "vibecraft/chunk.h"

#include <algorithm>

namespace vibecraft {

Chunk::Chunk(int chunk_x, int chunk_z)
    : dirty_(true), chunk_x_(chunk_x), chunk_z_(chunk_z) {
    blocks_.fill(BlockRegistry::kAir);
    light_.fill(0);
    fluid_levels_.fill(0);
    heightmap_.fill(-1);
}

BlockId Chunk::GetBlock(int x, int y, int z) const {
    if (!InBounds(x, y, z)) {
        return BlockRegistry::kAir;
    }
    return blocks_[Index(x, y, z)];
}

void Chunk::SetBlock(int x, int y, int z, BlockId id) {
    if (!InBounds(x, y, z)) {
        return;
    }

    int idx = Index(x, y, z);
    BlockId old = blocks_[idx];
    if (old == id) {
        return;  // No change — don't dirty or update heightmap.
    }

    blocks_[idx] = id;
    dirty_ = true;

    // Update heightmap.
    int col = ColumnIndex(x, z);
    if (id != BlockRegistry::kAir) {
        // Placed a non-air block: heightmap may increase.
        if (y > heightmap_[col]) {
            heightmap_[col] = static_cast<int16_t>(y);
        }
    } else {
        // Removed a block (set to air): heightmap may decrease.
        if (y == heightmap_[col]) {
            RecalcHeightmap(x, z);
        }
    }
}

bool Chunk::IsDirty() const {
    return dirty_;
}

void Chunk::ClearDirty() {
    dirty_ = false;
}

void Chunk::SetDirty() {
    dirty_ = true;
}

int Chunk::GetHeightmapValue(int x, int z) const {
    if (x < 0 || x >= kChunkSizeX || z < 0 || z >= kChunkSizeZ) {
        return -1;
    }
    return static_cast<int>(heightmap_[ColumnIndex(x, z)]);
}

int Chunk::GetChunkX() const {
    return chunk_x_;
}

int Chunk::GetChunkZ() const {
    return chunk_z_;
}

int Chunk::GetSunLight(int x, int y, int z) const {
    if (!InBounds(x, y, z)) {
        return 0;
    }
    return (light_[Index(x, y, z)] >> 4) & 0x0F;
}

void Chunk::SetSunLight(int x, int y, int z, int level) {
    if (!InBounds(x, y, z)) {
        return;
    }
    int idx = Index(x, y, z);
    light_[idx] = static_cast<uint8_t>((light_[idx] & 0x0F) | ((level & 0x0F) << 4));
}

int Chunk::GetBlockLight(int x, int y, int z) const {
    if (!InBounds(x, y, z)) {
        return 0;
    }
    return light_[Index(x, y, z)] & 0x0F;
}

void Chunk::SetBlockLight(int x, int y, int z, int level) {
    if (!InBounds(x, y, z)) {
        return;
    }
    int idx = Index(x, y, z);
    light_[idx] = static_cast<uint8_t>((light_[idx] & 0xF0) | (level & 0x0F));
}

uint8_t Chunk::GetRawLight(int x, int y, int z) const {
    if (!InBounds(x, y, z)) {
        return 0;
    }
    return light_[Index(x, y, z)];
}

void Chunk::SetRawLight(int x, int y, int z, uint8_t value) {
    if (!InBounds(x, y, z)) {
        return;
    }
    light_[Index(x, y, z)] = value;
}

uint8_t Chunk::GetFluidLevel(int x, int y, int z) const {
    if (!InBounds(x, y, z)) {
        return 0;
    }
    return fluid_levels_[Index(x, y, z)];
}

void Chunk::SetFluidLevel(int x, int y, int z, uint8_t level) {
    if (!InBounds(x, y, z)) {
        return;
    }
    fluid_levels_[Index(x, y, z)] = level;
}

bool Chunk::InBounds(int x, int y, int z) {
    return x >= 0 && x < kChunkSizeX &&
           y >= 0 && y < kChunkSizeY &&
           z >= 0 && z < kChunkSizeZ;
}

int Chunk::Index(int x, int y, int z) {
    return y * kChunkSizeX * kChunkSizeZ + z * kChunkSizeX + x;
}

int Chunk::ColumnIndex(int x, int z) {
    return z * kChunkSizeX + x;
}

void Chunk::RecalcHeightmap(int x, int z) {
    int col = ColumnIndex(x, z);
    for (int y = kChunkSizeY - 1; y >= 0; --y) {
        if (blocks_[Index(x, y, z)] != BlockRegistry::kAir) {
            heightmap_[col] = static_cast<int16_t>(y);
            return;
        }
    }
    heightmap_[col] = -1;  // Entire column is air.
}

}  // namespace vibecraft
