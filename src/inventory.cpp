#include "vibecraft/inventory.h"

#include <algorithm>

namespace vibecraft {

Inventory::Inventory() {
    // All slots are default-constructed as empty (air, count 0).
}

int Inventory::GetSlotCount() const {
    return kInventorySlotCount;
}

ItemSlot Inventory::GetSlot(int index) const {
    if (index < 0 || index >= kInventorySlotCount) {
        return ItemSlot{};
    }
    return slots_[index];
}

void Inventory::SetSlot(int index, BlockId block_id, uint8_t count) {
    if (index < 0 || index >= kInventorySlotCount) {
        return;
    }
    if (count == 0 || block_id == BlockRegistry::kAir) {
        slots_[index].Clear();
    } else {
        slots_[index].block_id = block_id;
        slots_[index].count = count;
    }
}

void Inventory::SwapSlots(int a, int b) {
    if (a < 0 || a >= kInventorySlotCount) return;
    if (b < 0 || b >= kInventorySlotCount) return;
    std::swap(slots_[a], slots_[b]);
}

uint8_t Inventory::AddToSlot(int index, BlockId block_id, uint8_t count) {
    if (index < 0 || index >= kInventorySlotCount) {
        return count;
    }
    if (block_id == BlockRegistry::kAir || count == 0) {
        return 0;
    }

    auto& slot = slots_[index];

    // If slot is empty, place items there.
    if (slot.IsEmpty()) {
        uint8_t to_place = std::min(count, kMaxStackSize);
        slot.block_id = block_id;
        slot.count = to_place;
        return count - to_place;
    }

    // If slot contains a different block type, can't stack.
    if (slot.block_id != block_id) {
        return count;
    }

    // Stack with existing items.
    uint8_t space = kMaxStackSize - slot.count;
    uint8_t to_add = std::min(count, space);
    slot.count += to_add;
    return count - to_add;
}

uint8_t Inventory::AddItem(BlockId block_id, uint8_t count) {
    if (block_id == BlockRegistry::kAir || count == 0) {
        return 0;
    }

    uint8_t remaining = count;

    // First pass: stack with existing matching slots.
    for (int i = 0; i < kInventorySlotCount && remaining > 0; ++i) {
        if (!slots_[i].IsEmpty() && slots_[i].block_id == block_id &&
            slots_[i].count < kMaxStackSize) {
            remaining = AddToSlot(i, block_id, remaining);
        }
    }

    // Second pass: fill first empty slots.
    for (int i = 0; i < kInventorySlotCount && remaining > 0; ++i) {
        if (slots_[i].IsEmpty()) {
            remaining = AddToSlot(i, block_id, remaining);
        }
    }

    return remaining;
}

int Inventory::GetSelectedSlot() const {
    return selected_slot_;
}

void Inventory::SetSelectedSlot(int slot) {
    if (slot >= 0 && slot < kHotbarSlotCount) {
        selected_slot_ = slot;
    }
}

void Inventory::NextHotbarSlot() {
    selected_slot_ = (selected_slot_ + 1) % kHotbarSlotCount;
}

void Inventory::PrevHotbarSlot() {
    selected_slot_ = (selected_slot_ - 1 + kHotbarSlotCount) % kHotbarSlotCount;
}

ItemSlot Inventory::GetHeldItem() const {
    return slots_[selected_slot_];
}

}  // namespace vibecraft
