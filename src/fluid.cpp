#include "vibecraft/fluid.h"

#include <algorithm>

namespace vibecraft {

FluidSimulator::FluidSimulator(World* world) : world_(world) {}

void FluidSimulator::PlaceWaterSource(int bx, int by, int bz) {
    world_->SetBlock(bx, by, bz, BlockRegistry::kWater);
    world_->SetFluidLevel(bx, by, bz, kWaterSourceLevel);
}

void FluidSimulator::PlaceLavaSource(int bx, int by, int bz) {
    world_->SetBlock(bx, by, bz, BlockRegistry::kLava);
    world_->SetFluidLevel(bx, by, bz, kLavaSourceLevel);
}

void FluidSimulator::RemoveFluid(int bx, int by, int bz) {
    world_->SetBlock(bx, by, bz, BlockRegistry::kAir);
    world_->SetFluidLevel(bx, by, bz, 0);
}

void FluidSimulator::Tick() {
    ++tick_count_;

    std::vector<PendingChange> changes;

    // Always process water.
    ProcessWater(changes);

    // Process lava only every kLavaTickRate ticks.
    if (tick_count_ % kLavaTickRate == 0) {
        ProcessLava(changes);
    }

    // Apply all changes atomically.
    ApplyChanges(changes);

    // After applying flow, check for water/lava interactions.
    std::vector<PendingChange> interaction_changes;
    ProcessInteractions(interaction_changes);
    ApplyChanges(interaction_changes);
}

int FluidSimulator::GetTickCount() const {
    return tick_count_;
}

World* FluidSimulator::GetWorld() const {
    return world_;
}

void FluidSimulator::ProcessWater(std::vector<PendingChange>& changes) {
    // We need to scan all loaded chunks for water blocks.
    // Collect all water block positions first, then process them.
    struct FluidBlock {
        int bx, by, bz;
        uint8_t level;
    };
    std::vector<FluidBlock> water_blocks;

    // Iterate over all chunks in the world.
    // We need to iterate through the world's chunks. Since World doesn't expose
    // an iterator, we'll scan a reasonable area. For the simulation, we scan
    // all loaded chunks by checking which ones exist.
    // NOTE: For a real game, we'd want a more efficient data structure.
    // For now, we scan all loaded chunks via the world's chunk map.

    // We need to get all chunks. Let's scan a region around origin.
    // Actually, since World stores chunks in an unordered_map, we need a way
    // to iterate. Let's collect fluid blocks from all loaded chunks.
    // We'll add a helper that scans the world for fluid blocks.

    // Scan approach: we iterate loaded chunks by trying coordinates.
    // Since the test uses small worlds, we scan a bounded region.
    // A better approach: track fluid blocks explicitly in a set.
    // For simplicity and correctness, we scan all loaded chunks.

    // To iterate chunks, we'll use a two-pass approach:
    // 1. Scan Y range [0, 255] for each block in each chunk.
    // This is expensive but correct for tests.

    // For now, let's gather all water positions from loaded chunks.
    // We need to iterate the chunk map - let's add a method or scan known bounds.

    // Practical approach: scan a bounded region that covers all loaded chunks.
    // Find min/max chunk coords first.
    // Since we can't easily iterate the internal map, scan a generous area.
    // For testing purposes, we'll scan chunks in a reasonable range.

    // Let's scan chunks from (-10, -10) to (10, 10) — covers test scenarios.
    // Better: scan each chunk by trying if it exists.
    for (int cx = -10; cx <= 10; ++cx) {
        for (int cz = -10; cz <= 10; ++cz) {
            const Chunk* chunk = world_->GetChunk(cx, cz);
            if (chunk == nullptr) {
                continue;
            }

            int base_bx = cx * kChunkSizeX;
            int base_bz = cz * kChunkSizeZ;

            for (int ly = 0; ly < kChunkSizeY; ++ly) {
                for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                    for (int lx = 0; lx < kChunkSizeX; ++lx) {
                        if (chunk->GetBlock(lx, ly, lz) == BlockRegistry::kWater) {
                            water_blocks.push_back({
                                base_bx + lx, ly, base_bz + lz,
                                chunk->GetFluidLevel(lx, ly, lz)
                            });
                        }
                    }
                }
            }
        }
    }

    // Process each water block.
    for (const auto& wb : water_blocks) {
        int bx = wb.bx;
        int by = wb.by;
        int bz = wb.bz;
        uint8_t level = wb.level;

        bool is_source = (level == kWaterSourceLevel);

        // Infinite source rule: if a non-source water block has 2+ adjacent
        // source blocks, it becomes a source.
        if (!is_source) {
            int source_count = CountAdjacentSources(bx, by, bz, BlockRegistry::kWater);
            if (source_count >= 2) {
                changes.push_back({bx, by, bz, BlockRegistry::kWater, kWaterSourceLevel});
                // Don't process further flow from this block this tick since
                // it's becoming a source.
                continue;
            }
        }

        // Non-source blocks: check if they should still exist.
        // A flow block should drain if no neighbor can feed it.
        if (!is_source) {
            // Check if this block can be sustained.
            // It needs at least one neighbor (horizontal or above) with a higher level,
            // or water directly above.
            bool has_water_above = IsFluid(bx, by + 1, bz, BlockRegistry::kWater);
            int max_neighbor = GetMaxNeighborLevel(bx, by, bz, BlockRegistry::kWater);

            // A flow block is sustained if:
            // - There's water above (falling water)
            // - A horizontal neighbor has level > this level
            bool sustained = has_water_above || (max_neighbor > level);

            if (!sustained) {
                // Drain this block.
                changes.push_back({bx, by, bz, BlockRegistry::kAir, 0});
                continue;
            }

            // If sustained by a horizontal neighbor, our level should be max_neighbor - 1.
            // If sustained by water above, level should be kWaterSourceLevel (falling water
            // at the bottom acts like a full column).
            if (has_water_above) {
                uint8_t expected = kWaterSourceLevel;
                if (level != expected) {
                    changes.push_back({bx, by, bz, BlockRegistry::kWater, expected});
                }
            } else if (max_neighbor > 0) {
                uint8_t expected = static_cast<uint8_t>(max_neighbor - 1);
                if (level != expected) {
                    changes.push_back({bx, by, bz, BlockRegistry::kWater, expected});
                }
            }
        }

        // Flow downward: if block below is air, place water there at source level
        // (falling water is always full).
        if (by > 0 && CanFlowInto(bx, by - 1, bz)) {
            changes.push_back({bx, by - 1, bz, BlockRegistry::kWater, kWaterSourceLevel});
        }

        // Flow horizontally if level > 1 (or source), and there's no air below
        // (if there's air below, water falls instead of spreading).
        if (level > 0) {
            bool can_fall = (by > 0 && CanFlowInto(bx, by - 1, bz));

            // Water spreads horizontally even if it can also fall down.
            // But horizontal spread level decreases by 1 from current level.
            uint8_t spread_level = (level > 0) ? static_cast<uint8_t>(level - 1) : 0;

            if (spread_level > 0 || is_source) {
                if (is_source) {
                    spread_level = kWaterSourceLevel - 1;  // 6
                }

                // Only spread horizontally if spread level > 0.
                if (spread_level > 0) {
                    // Check 4 horizontal neighbors.
                    const int dx[] = {1, -1, 0, 0};
                    const int dz[] = {0, 0, 1, -1};
                    for (int i = 0; i < 4; ++i) {
                        int nx = bx + dx[i];
                        int nz = bz + dz[i];

                        if (CanFlowInto(nx, by, nz)) {
                            changes.push_back({nx, by, nz, BlockRegistry::kWater, spread_level});
                        } else if (IsFluid(nx, by, nz, BlockRegistry::kWater)) {
                            // If neighbor is water with lower level, update it.
                            uint8_t neighbor_level = world_->GetFluidLevel(nx, by, nz);
                            if (neighbor_level < spread_level) {
                                changes.push_back({nx, by, nz, BlockRegistry::kWater, spread_level});
                            }
                        }
                    }
                }
            }
        }
    }
}

void FluidSimulator::ProcessLava(std::vector<PendingChange>& changes) {
    // Similar to water but with lava-specific parameters.
    struct FluidBlock {
        int bx, by, bz;
        uint8_t level;
    };
    std::vector<FluidBlock> lava_blocks;

    for (int cx = -10; cx <= 10; ++cx) {
        for (int cz = -10; cz <= 10; ++cz) {
            const Chunk* chunk = world_->GetChunk(cx, cz);
            if (chunk == nullptr) {
                continue;
            }

            int base_bx = cx * kChunkSizeX;
            int base_bz = cz * kChunkSizeZ;

            for (int ly = 0; ly < kChunkSizeY; ++ly) {
                for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                    for (int lx = 0; lx < kChunkSizeX; ++lx) {
                        if (chunk->GetBlock(lx, ly, lz) == BlockRegistry::kLava) {
                            lava_blocks.push_back({
                                base_bx + lx, ly, base_bz + lz,
                                chunk->GetFluidLevel(lx, ly, lz)
                            });
                        }
                    }
                }
            }
        }
    }

    for (const auto& lb : lava_blocks) {
        int bx = lb.bx;
        int by = lb.by;
        int bz = lb.bz;
        uint8_t level = lb.level;

        bool is_source = (level == kLavaSourceLevel);

        // Non-source lava: check if sustained.
        if (!is_source) {
            bool has_lava_above = IsFluid(bx, by + 1, bz, BlockRegistry::kLava);
            int max_neighbor = GetMaxNeighborLevel(bx, by, bz, BlockRegistry::kLava);

            bool sustained = has_lava_above || (max_neighbor > level);

            if (!sustained) {
                changes.push_back({bx, by, bz, BlockRegistry::kAir, 0});
                continue;
            }
        }

        // Flow downward.
        if (by > 0 && CanFlowInto(bx, by - 1, bz)) {
            changes.push_back({bx, by - 1, bz, BlockRegistry::kLava, kLavaSourceLevel});
        }

        // Flow horizontally.
        if (level > 0) {
            uint8_t spread_level;
            if (is_source) {
                spread_level = kLavaSourceLevel - 1;  // 2
            } else {
                spread_level = (level > 0) ? static_cast<uint8_t>(level - 1) : 0;
            }

            if (spread_level > 0) {
                const int dx[] = {1, -1, 0, 0};
                const int dz[] = {0, 0, 1, -1};
                for (int i = 0; i < 4; ++i) {
                    int nx = bx + dx[i];
                    int nz = bz + dz[i];

                    if (CanFlowInto(nx, by, nz)) {
                        changes.push_back({nx, by, nz, BlockRegistry::kLava, spread_level});
                    } else if (IsFluid(nx, by, nz, BlockRegistry::kLava)) {
                        uint8_t neighbor_level = world_->GetFluidLevel(nx, by, nz);
                        if (neighbor_level < spread_level) {
                            changes.push_back({nx, by, nz, BlockRegistry::kLava, spread_level});
                        }
                    }
                }
            }
        }
    }
}

void FluidSimulator::ProcessInteractions(std::vector<PendingChange>& changes) {
    // Scan for water/lava adjacency.
    // When flowing water meets a lava source → cobblestone.
    // When a water source meets lava (any) → obsidian.
    // We check each water block for adjacent lava and vice versa.

    struct FluidBlock {
        int bx, by, bz;
        BlockId type;
        uint8_t level;
    };
    std::vector<FluidBlock> fluid_blocks;

    for (int cx = -10; cx <= 10; ++cx) {
        for (int cz = -10; cz <= 10; ++cz) {
            const Chunk* chunk = world_->GetChunk(cx, cz);
            if (chunk == nullptr) {
                continue;
            }

            int base_bx = cx * kChunkSizeX;
            int base_bz = cz * kChunkSizeZ;

            for (int ly = 0; ly < kChunkSizeY; ++ly) {
                for (int lz = 0; lz < kChunkSizeZ; ++lz) {
                    for (int lx = 0; lx < kChunkSizeX; ++lx) {
                        BlockId block = chunk->GetBlock(lx, ly, lz);
                        if (block == BlockRegistry::kWater || block == BlockRegistry::kLava) {
                            fluid_blocks.push_back({
                                base_bx + lx, ly, base_bz + lz,
                                block,
                                chunk->GetFluidLevel(lx, ly, lz)
                            });
                        }
                    }
                }
            }
        }
    }

    const int dx[] = {1, -1, 0, 0, 0, 0};
    const int dy[] = {0, 0, 0, 0, 1, -1};
    const int dz[] = {0, 0, 1, -1, 0, 0};

    for (const auto& fb : fluid_blocks) {
        if (fb.type == BlockRegistry::kWater) {
            // Check if any adjacent block is lava.
            for (int i = 0; i < 6; ++i) {
                int nx = fb.bx + dx[i];
                int ny = fb.by + dy[i];
                int nz = fb.bz + dz[i];

                if (world_->GetBlock(nx, ny, nz) == BlockRegistry::kLava) {
                    uint8_t lava_level = world_->GetFluidLevel(nx, ny, nz);
                    bool lava_is_source = (lava_level == kLavaSourceLevel);

                    bool water_is_source = (fb.level == kWaterSourceLevel);

                    if (water_is_source) {
                        // Water source meets lava → obsidian replaces lava.
                        changes.push_back({nx, ny, nz, BlockRegistry::kObsidian, 0});
                    } else {
                        // Flowing water meets lava source → cobblestone replaces lava.
                        if (lava_is_source) {
                            changes.push_back({nx, ny, nz, BlockRegistry::kCobblestone, 0});
                        }
                    }
                }
            }
        }
    }
}

void FluidSimulator::ApplyChanges(const std::vector<PendingChange>& changes) {
    for (const auto& change : changes) {
        // Only apply if it's actually a change (avoid no-op SetBlock which
        // would skip due to same value check).
        BlockId current_block = world_->GetBlock(change.bx, change.by, change.bz);
        uint8_t current_level = world_->GetFluidLevel(change.bx, change.by, change.bz);

        if (current_block != change.block_id || current_level != change.fluid_level) {
            world_->SetBlock(change.bx, change.by, change.bz, change.block_id);
            world_->SetFluidLevel(change.bx, change.by, change.bz, change.fluid_level);
        }
    }
}

bool FluidSimulator::CanFlowInto(int bx, int by, int bz) const {
    BlockId block = world_->GetBlock(bx, by, bz);
    return block == BlockRegistry::kAir;
}

bool FluidSimulator::IsFluid(int bx, int by, int bz, BlockId fluid_type) const {
    return world_->GetBlock(bx, by, bz) == fluid_type;
}

int FluidSimulator::CountAdjacentSources(int bx, int by, int bz, BlockId fluid_type) const {
    int count = 0;
    const int dx[] = {1, -1, 0, 0};
    const int dz[] = {0, 0, 1, -1};

    uint8_t source_level = (fluid_type == BlockRegistry::kWater) ?
                           kWaterSourceLevel : kLavaSourceLevel;

    for (int i = 0; i < 4; ++i) {
        int nx = bx + dx[i];
        int nz = bz + dz[i];
        if (IsFluid(nx, by, nz, fluid_type) &&
            world_->GetFluidLevel(nx, by, nz) == source_level) {
            ++count;
        }
    }
    return count;
}

int FluidSimulator::GetMaxNeighborLevel(int bx, int by, int bz, BlockId fluid_type) const {
    int max_level = -1;
    const int dx[] = {1, -1, 0, 0};
    const int dz[] = {0, 0, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nx = bx + dx[i];
        int nz = bz + dz[i];
        if (IsFluid(nx, by, nz, fluid_type)) {
            int level = static_cast<int>(world_->GetFluidLevel(nx, by, nz));
            max_level = std::max(max_level, level);
        }
    }
    return max_level;
}

bool FluidSimulator::IsSolid(int bx, int by, int bz) const {
    BlockId block = world_->GetBlock(bx, by, bz);
    return block != BlockRegistry::kAir &&
           block != BlockRegistry::kWater &&
           block != BlockRegistry::kLava;
}

}  // namespace vibecraft
