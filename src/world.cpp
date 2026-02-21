#include "vibecraft/world.h"

#include "vibecraft/math_utils.h"

namespace vibecraft {

int World::WorldToChunkCoord(int block_coord) {
    // Integer floor division by chunk size (16).
    // For negative coords: IntFloor(-1.0f) == -1, so -1/16 gives chunk -1.
    return IntFloor(static_cast<float>(block_coord) / static_cast<float>(kChunkSizeX));
}

int World::WorldToLocalCoord(int block_coord) {
    // Euclidean mod ensures result is always in [0, 15].
    return Mod(block_coord, kChunkSizeX);
}

BlockId World::GetBlock(int bx, int by, int bz) const {
    int cx = WorldToChunkCoord(bx);
    int cz = WorldToChunkCoord(bz);

    const Chunk* chunk = GetChunk(cx, cz);
    if (chunk == nullptr) {
        return BlockRegistry::kAir;
    }

    int lx = WorldToLocalCoord(bx);
    int lz = WorldToLocalCoord(bz);
    return chunk->GetBlock(lx, by, lz);
}

void World::SetBlock(int bx, int by, int bz, BlockId id) {
    int cx = WorldToChunkCoord(bx);
    int cz = WorldToChunkCoord(bz);

    Chunk& chunk = GetOrCreateChunk(cx, cz);

    int lx = WorldToLocalCoord(bx);
    int lz = WorldToLocalCoord(bz);
    chunk.SetBlock(lx, by, lz, id);

    // Propagate dirty flag to neighbor chunks on boundary edits.
    PropagateDirty(cx, cz, lx, lz);
}

uint8_t World::GetFluidLevel(int bx, int by, int bz) const {
    int cx = WorldToChunkCoord(bx);
    int cz = WorldToChunkCoord(bz);

    const Chunk* chunk = GetChunk(cx, cz);
    if (chunk == nullptr) {
        return 0;
    }

    int lx = WorldToLocalCoord(bx);
    int lz = WorldToLocalCoord(bz);
    return chunk->GetFluidLevel(lx, by, lz);
}

void World::SetFluidLevel(int bx, int by, int bz, uint8_t level) {
    int cx = WorldToChunkCoord(bx);
    int cz = WorldToChunkCoord(bz);

    Chunk& chunk = GetOrCreateChunk(cx, cz);

    int lx = WorldToLocalCoord(bx);
    int lz = WorldToLocalCoord(bz);
    chunk.SetFluidLevel(lx, by, lz, level);
}

void World::LoadChunk(int cx, int cz) {
    ChunkCoord coord{cx, cz};
    if (chunks_.find(coord) == chunks_.end()) {
        chunks_.emplace(coord, std::make_unique<Chunk>(cx, cz));
    }
}

void World::UnloadChunk(int cx, int cz) {
    chunks_.erase(ChunkCoord{cx, cz});
}

bool World::HasChunk(int cx, int cz) const {
    return chunks_.find(ChunkCoord{cx, cz}) != chunks_.end();
}

Chunk* World::GetChunk(int cx, int cz) {
    auto it = chunks_.find(ChunkCoord{cx, cz});
    if (it == chunks_.end()) {
        return nullptr;
    }
    return it->second.get();
}

const Chunk* World::GetChunk(int cx, int cz) const {
    auto it = chunks_.find(ChunkCoord{cx, cz});
    if (it == chunks_.end()) {
        return nullptr;
    }
    return it->second.get();
}

std::size_t World::ChunkCount() const {
    return chunks_.size();
}

Chunk& World::GetOrCreateChunk(int cx, int cz) {
    ChunkCoord coord{cx, cz};
    auto it = chunks_.find(coord);
    if (it == chunks_.end()) {
        auto result = chunks_.emplace(coord, std::make_unique<Chunk>(cx, cz));
        return *result.first->second;
    }
    return *it->second;
}

void World::PropagateDirty(int cx, int cz, int local_x, int local_z) {
    // If block is at local x=0, the -X neighbor chunk might need re-meshing.
    if (local_x == 0) {
        Chunk* neighbor = GetChunk(cx - 1, cz);
        if (neighbor != nullptr) {
            neighbor->SetDirty();
        }
    }
    // If block is at local x=15, the +X neighbor chunk might need re-meshing.
    if (local_x == kChunkSizeX - 1) {
        Chunk* neighbor = GetChunk(cx + 1, cz);
        if (neighbor != nullptr) {
            neighbor->SetDirty();
        }
    }
    // If block is at local z=0, the -Z neighbor chunk might need re-meshing.
    if (local_z == 0) {
        Chunk* neighbor = GetChunk(cx, cz - 1);
        if (neighbor != nullptr) {
            neighbor->SetDirty();
        }
    }
    // If block is at local z=15, the +Z neighbor chunk might need re-meshing.
    if (local_z == kChunkSizeZ - 1) {
        Chunk* neighbor = GetChunk(cx, cz + 1);
        if (neighbor != nullptr) {
            neighbor->SetDirty();
        }
    }
}

}  // namespace vibecraft
