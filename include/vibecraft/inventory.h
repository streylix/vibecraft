#ifndef VIBECRAFT_INVENTORY_H
#define VIBECRAFT_INVENTORY_H

#include <array>
#include <cstdint>

#include "vibecraft/block.h"

namespace vibecraft {

/// Represents one inventory slot containing a block type and a count.
struct ItemSlot {
    BlockId block_id = BlockRegistry::kAir;
    uint8_t count = 0;

    /// Returns true if this slot is empty (count == 0 or air).
    bool IsEmpty() const { return count == 0 || block_id == BlockRegistry::kAir; }

    /// Clear this slot.
    void Clear() {
        block_id = BlockRegistry::kAir;
        count = 0;
    }
};

/// Maximum number of items in a single stack.
static constexpr uint8_t kMaxStackSize = 64;

/// Total inventory slots: 9 hotbar + 27 main storage.
static constexpr int kInventorySlotCount = 36;

/// Number of hotbar slots.
static constexpr int kHotbarSlotCount = 9;

/// Inventory system with 36 slots (0-8 hotbar, 9-35 main storage).
class Inventory {
public:
    Inventory();

    /// Returns the total number of slots.
    int GetSlotCount() const;

    /// Get the item in a slot. Returns empty slot if index out of range.
    ItemSlot GetSlot(int index) const;

    /// Set the contents of a slot. Does nothing if index out of range.
    void SetSlot(int index, BlockId block_id, uint8_t count);

    /// Swap the contents of two slots.
    void SwapSlots(int a, int b);

    /// Add count items of block_id to a specific slot, respecting the max
    /// stack size. Returns the number of items that could not fit (overflow).
    uint8_t AddToSlot(int index, BlockId block_id, uint8_t count);

    /// Add items to the inventory, first stacking with existing matching
    /// slots, then filling the first empty slot. Returns leftover count.
    uint8_t AddItem(BlockId block_id, uint8_t count);

    /// Get the currently selected hotbar slot index (0-8).
    int GetSelectedSlot() const;

    /// Set the selected hotbar slot (clamped to 0-8).
    void SetSelectedSlot(int slot);

    /// Increment hotbar selection (wraps 8 -> 0).
    void NextHotbarSlot();

    /// Decrement hotbar selection (wraps 0 -> 8).
    void PrevHotbarSlot();

    /// Get the item currently held (selected hotbar slot).
    ItemSlot GetHeldItem() const;

private:
    std::array<ItemSlot, kInventorySlotCount> slots_;
    int selected_slot_ = 0;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_INVENTORY_H
