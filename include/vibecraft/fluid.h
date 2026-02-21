#ifndef VIBECRAFT_FLUID_H
#define VIBECRAFT_FLUID_H

#include <vector>

#include "vibecraft/block.h"
#include "vibecraft/world.h"

namespace vibecraft {

/// Fluid simulation using cellular automata.
///
/// Water uses levels 0-7 (7 = full source block).
/// Lava uses levels 0-3 (3 = full source block).
///
/// Water is processed every tick. Lava is processed every 3 ticks.
/// Each tick, the simulator scans all fluid blocks, computes pending
/// changes, and applies them atomically.
class FluidSimulator {
public:
    /// Water source level (maximum).
    static constexpr uint8_t kWaterSourceLevel = 7;

    /// Lava source level (maximum).
    static constexpr uint8_t kLavaSourceLevel = 3;

    /// Number of ticks between lava updates.
    static constexpr int kLavaTickRate = 3;

    /// Construct a fluid simulator operating on the given world.
    /// The world pointer must remain valid for the lifetime of the simulator.
    explicit FluidSimulator(World* world);

    /// Place a water source block at the given world coordinates.
    /// Sets the block to Water with source level.
    void PlaceWaterSource(int bx, int by, int bz);

    /// Place a lava source block at the given world coordinates.
    /// Sets the block to Lava with source level.
    void PlaceLavaSource(int bx, int by, int bz);

    /// Remove a fluid block at the given world coordinates.
    /// Sets the block to Air with level 0.
    void RemoveFluid(int bx, int by, int bz);

    /// Advance the simulation by one tick.
    /// Water is processed every tick. Lava every kLavaTickRate ticks.
    void Tick();

    /// Get the current tick counter.
    int GetTickCount() const;

    /// Get the world this simulator operates on.
    World* GetWorld() const;

private:
    /// A pending block change to apply atomically.
    struct PendingChange {
        int bx, by, bz;
        BlockId block_id;
        uint8_t fluid_level;
    };

    /// Process all water blocks and generate pending changes.
    void ProcessWater(std::vector<PendingChange>& changes);

    /// Process all lava blocks and generate pending changes.
    void ProcessLava(std::vector<PendingChange>& changes);

    /// Check water+lava interactions and generate pending changes.
    void ProcessInteractions(std::vector<PendingChange>& changes);

    /// Apply all pending changes to the world.
    void ApplyChanges(const std::vector<PendingChange>& changes);

    /// Check if a position can be replaced by flowing fluid.
    /// Returns true if the block at (bx, by, bz) is Air.
    bool CanFlowInto(int bx, int by, int bz) const;

    /// Check if a position contains a fluid of the given type.
    bool IsFluid(int bx, int by, int bz, BlockId fluid_type) const;

    /// Count adjacent source blocks of the given fluid type (horizontal only).
    int CountAdjacentSources(int bx, int by, int bz, BlockId fluid_type) const;

    /// Get the maximum fluid level among horizontal neighbors of the given type.
    /// Returns -1 if no neighbors of this type exist.
    int GetMaxNeighborLevel(int bx, int by, int bz, BlockId fluid_type) const;

    /// Check if a block is solid (not air, not liquid).
    bool IsSolid(int bx, int by, int bz) const;

    World* world_;
    int tick_count_ = 0;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_FLUID_H
