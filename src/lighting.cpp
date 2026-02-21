#include "vibecraft/lighting.h"

#include <algorithm>

namespace vibecraft {

LightingEngine::LightingEngine(World& world, const BlockRegistry& registry)
    : world_(world), registry_(registry) {}

int LightingEngine::GetBlockLight(int bx, int by, int bz) const {
    if (by < 0 || by >= kChunkSizeY) {
        return 0;
    }
    int cx = World::WorldToChunkCoord(bx);
    int cz = World::WorldToChunkCoord(bz);
    const Chunk* chunk = world_.GetChunk(cx, cz);
    if (chunk == nullptr) {
        return 0;
    }
    int lx = World::WorldToLocalCoord(bx);
    int lz = World::WorldToLocalCoord(bz);
    return chunk->GetBlockLight(lx, by, lz);
}

int LightingEngine::GetSunLight(int bx, int by, int bz) const {
    if (by < 0 || by >= kChunkSizeY) {
        return 0;
    }
    int cx = World::WorldToChunkCoord(bx);
    int cz = World::WorldToChunkCoord(bz);
    const Chunk* chunk = world_.GetChunk(cx, cz);
    if (chunk == nullptr) {
        return 0;
    }
    int lx = World::WorldToLocalCoord(bx);
    int lz = World::WorldToLocalCoord(bz);
    return chunk->GetSunLight(lx, by, lz);
}

void LightingEngine::SetBlockLight(int bx, int by, int bz, int level) {
    if (by < 0 || by >= kChunkSizeY) {
        return;
    }
    int cx = World::WorldToChunkCoord(bx);
    int cz = World::WorldToChunkCoord(bz);
    Chunk* chunk = world_.GetChunk(cx, cz);
    if (chunk == nullptr) {
        return;
    }
    int lx = World::WorldToLocalCoord(bx);
    int lz = World::WorldToLocalCoord(bz);
    chunk->SetBlockLight(lx, by, lz, level);
}

void LightingEngine::SetSunLight(int bx, int by, int bz, int level) {
    if (by < 0 || by >= kChunkSizeY) {
        return;
    }
    int cx = World::WorldToChunkCoord(bx);
    int cz = World::WorldToChunkCoord(bz);
    Chunk* chunk = world_.GetChunk(cx, cz);
    if (chunk == nullptr) {
        return;
    }
    int lx = World::WorldToLocalCoord(bx);
    int lz = World::WorldToLocalCoord(bz);
    chunk->SetSunLight(lx, by, lz, level);
}

bool LightingEngine::IsTransparent(int bx, int by, int bz) const {
    BlockId id = world_.GetBlock(bx, by, bz);
    return registry_.GetBlock(id).transparent;
}

int LightingEngine::GetEmission(int bx, int by, int bz) const {
    BlockId id = world_.GetBlock(bx, by, bz);
    return registry_.GetBlock(id).light_emission;
}

void LightingEngine::AddBlockLight(int bx, int by, int bz) {
    int emission = GetEmission(bx, by, bz);
    if (emission <= 0) {
        return;
    }

    // Set the light level at the source position.
    SetBlockLight(bx, by, bz, emission);

    // Enqueue for BFS propagation.
    std::queue<LightNode> queue;
    queue.push({bx, by, bz, emission});
    PropagateBlockLightBFS(queue);
}

void LightingEngine::RemoveBlockLight(int bx, int by, int bz) {
    int old_level = GetBlockLight(bx, by, bz);
    if (old_level <= 0) {
        return;
    }

    // BFS removal: collect all blocks that were lit by this source.
    std::queue<LightRemovalNode> removal_queue;
    std::queue<LightNode> relight_queue;

    removal_queue.push({bx, by, bz, old_level});
    SetBlockLight(bx, by, bz, 0);

    while (!removal_queue.empty()) {
        auto node = removal_queue.front();
        removal_queue.pop();

        for (const auto& dir : kDirs) {
            int nx = node.x + dir[0];
            int ny = node.y + dir[1];
            int nz = node.z + dir[2];

            if (ny < 0 || ny >= kChunkSizeY) {
                continue;
            }

            // Check if this neighbor chunk is loaded.
            int cx = World::WorldToChunkCoord(nx);
            int cz = World::WorldToChunkCoord(nz);
            if (!world_.HasChunk(cx, cz)) {
                continue;
            }

            int neighbor_level = GetBlockLight(nx, ny, nz);
            if (neighbor_level > 0 && neighbor_level < node.level) {
                // This neighbor was lit by the removed source (its level is
                // less than the node's level, so it was downstream).
                SetBlockLight(nx, ny, nz, 0);
                removal_queue.push({nx, ny, nz, neighbor_level});
            } else if (neighbor_level >= node.level) {
                // This neighbor still has light from another source.
                // Re-propagate from it.
                relight_queue.push({nx, ny, nz, neighbor_level});
            }
        }
    }

    // Re-propagate from remaining light sources.
    PropagateBlockLightBFS(relight_queue);
}

void LightingEngine::PropagateBlockLightBFS(std::queue<LightNode>& queue) {
    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        for (const auto& dir : kDirs) {
            int nx = node.x + dir[0];
            int ny = node.y + dir[1];
            int nz = node.z + dir[2];

            if (ny < 0 || ny >= kChunkSizeY) {
                continue;
            }

            // Check if neighbor chunk is loaded.
            int cx = World::WorldToChunkCoord(nx);
            int cz = World::WorldToChunkCoord(nz);
            if (!world_.HasChunk(cx, cz)) {
                continue;
            }

            // Light cannot pass through opaque (non-transparent) blocks.
            if (!IsTransparent(nx, ny, nz)) {
                continue;
            }

            int new_level = node.level - 1;
            if (new_level <= 0) {
                continue;
            }

            int current = GetBlockLight(nx, ny, nz);
            if (new_level > current) {
                SetBlockLight(nx, ny, nz, new_level);
                queue.push({nx, ny, nz, new_level});
            }
        }
    }
}

void LightingEngine::PropagateSunlightColumn(int bx, int bz) {
    // Propagate sunlight straight down from the top.
    // Sunlight starts at 15 and propagates through transparent blocks.
    int level = 15;
    for (int y = kChunkSizeY - 1; y >= 0; --y) {
        if (y < 0 || y >= kChunkSizeY) {
            break;
        }

        int cx = World::WorldToChunkCoord(bx);
        int cz = World::WorldToChunkCoord(bz);
        if (!world_.HasChunk(cx, cz)) {
            break;
        }

        if (!IsTransparent(bx, y, bz)) {
            // Solid block stops sunlight entirely.
            level = 0;
        }

        SetSunLight(bx, y, bz, level);
    }
}

void LightingEngine::PropagateSunLightBFS(std::queue<LightNode>& queue) {
    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();

        for (const auto& dir : kDirs) {
            int nx = node.x + dir[0];
            int ny = node.y + dir[1];
            int nz = node.z + dir[2];

            if (ny < 0 || ny >= kChunkSizeY) {
                continue;
            }

            int cx = World::WorldToChunkCoord(nx);
            int cz = World::WorldToChunkCoord(nz);
            if (!world_.HasChunk(cx, cz)) {
                continue;
            }

            if (!IsTransparent(nx, ny, nz)) {
                continue;
            }

            // Sunlight going straight down doesn't attenuate.
            int new_level;
            if (dir[1] == -1 && dir[0] == 0 && dir[2] == 0) {
                new_level = node.level;  // Straight down: no attenuation
            } else {
                new_level = node.level - 1;  // Horizontal/upward: attenuate
            }

            if (new_level <= 0) {
                continue;
            }

            int current = GetSunLight(nx, ny, nz);
            if (new_level > current) {
                SetSunLight(nx, ny, nz, new_level);
                queue.push({nx, ny, nz, new_level});
            }
        }
    }
}

void LightingEngine::CalculateSunlight() {
    // First pass: propagate sunlight straight down in each column.
    // We need to iterate over all loaded chunks.
    // Collect chunk coordinates first to avoid iterator invalidation.
    std::vector<std::pair<int, int>> chunk_coords;
    for (int cx = -100; cx <= 100; ++cx) {
        for (int cz = -100; cz <= 100; ++cz) {
            if (world_.HasChunk(cx, cz)) {
                chunk_coords.emplace_back(cx, cz);
            }
        }
    }

    // Propagate sunlight straight down in all columns.
    for (auto& [cx, cz] : chunk_coords) {
        int base_x = cx * kChunkSizeX;
        int base_z = cz * kChunkSizeZ;
        for (int lx = 0; lx < kChunkSizeX; ++lx) {
            for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                PropagateSunlightColumn(base_x + lx, base_z + lz);
            }
        }
    }

    // Second pass: BFS horizontal spread of sunlight.
    std::queue<LightNode> queue;
    for (auto& [cx, cz] : chunk_coords) {
        Chunk* chunk = world_.GetChunk(cx, cz);
        if (chunk == nullptr) continue;
        int base_x = cx * kChunkSizeX;
        int base_z = cz * kChunkSizeZ;
        for (int lx = 0; lx < kChunkSizeX; ++lx) {
            for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                for (int y = 0; y < kChunkSizeY; ++y) {
                    int sun = chunk->GetSunLight(lx, y, lz);
                    if (sun > 1) {
                        queue.push({base_x + lx, y, base_z + lz, sun});
                    }
                }
            }
        }
    }
    PropagateSunLightBFS(queue);
}

void LightingEngine::CalculateBlockLight() {
    // Scan all loaded chunks for light-emitting blocks.
    std::vector<std::pair<int, int>> chunk_coords;
    for (int cx = -100; cx <= 100; ++cx) {
        for (int cz = -100; cz <= 100; ++cz) {
            if (world_.HasChunk(cx, cz)) {
                chunk_coords.emplace_back(cx, cz);
            }
        }
    }

    for (auto& [cx, cz] : chunk_coords) {
        Chunk* chunk = world_.GetChunk(cx, cz);
        if (chunk == nullptr) continue;
        int base_x = cx * kChunkSizeX;
        int base_z = cz * kChunkSizeZ;
        for (int lx = 0; lx < kChunkSizeX; ++lx) {
            for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                for (int y = 0; y < kChunkSizeY; ++y) {
                    BlockId id = chunk->GetBlock(lx, y, lz);
                    int emission = registry_.GetBlock(id).light_emission;
                    if (emission > 0) {
                        AddBlockLight(base_x + lx, y, base_z + lz);
                    }
                }
            }
        }
    }
}

void LightingEngine::CalculateAllLighting() {
    CalculateSunlight();
    CalculateBlockLight();
}

}  // namespace vibecraft
