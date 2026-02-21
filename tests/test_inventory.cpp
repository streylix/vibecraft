#include <gtest/gtest.h>

#include "vibecraft/inventory.h"

// M18: Inventory, Hotbar & HUD

using namespace vibecraft;

TEST(Inventory, SlotCount) {
    Inventory inv;
    EXPECT_EQ(inv.GetSlotCount(), 36);
}

TEST(Inventory, EmptyByDefault) {
    Inventory inv;
    for (int i = 0; i < inv.GetSlotCount(); ++i) {
        ItemSlot slot = inv.GetSlot(i);
        EXPECT_EQ(slot.count, 0) << "Slot " << i << " should start empty";
    }
}

TEST(Inventory, SetAndGet) {
    Inventory inv;
    inv.SetSlot(0, BlockRegistry::kStone, 10);
    ItemSlot slot = inv.GetSlot(0);
    EXPECT_EQ(slot.block_id, BlockRegistry::kStone);
    EXPECT_EQ(slot.count, 10);
}

TEST(Inventory, SwapSlots) {
    Inventory inv;
    inv.SetSlot(0, BlockRegistry::kStone, 10);
    inv.SetSlot(1, BlockRegistry::kDirt, 5);
    inv.SwapSlots(0, 1);

    ItemSlot s0 = inv.GetSlot(0);
    ItemSlot s1 = inv.GetSlot(1);

    EXPECT_EQ(s0.block_id, BlockRegistry::kDirt);
    EXPECT_EQ(s0.count, 5);
    EXPECT_EQ(s1.block_id, BlockRegistry::kStone);
    EXPECT_EQ(s1.count, 10);
}

TEST(Inventory, StackToMax) {
    Inventory inv;
    // Put 60 stone in slot 0.
    inv.SetSlot(0, BlockRegistry::kStone, 60);
    // Add 10 more stone to that slot.
    uint8_t overflow = inv.AddToSlot(0, BlockRegistry::kStone, 10);
    ItemSlot slot = inv.GetSlot(0);
    // Should cap at 64, with 6 overflow.
    EXPECT_EQ(slot.count, 64);
    EXPECT_EQ(overflow, 6);
}

TEST(Inventory, StackOverflow) {
    Inventory inv;
    // Add 70 stone to an empty slot.
    uint8_t overflow = inv.AddToSlot(0, BlockRegistry::kStone, 70);
    ItemSlot slot = inv.GetSlot(0);
    // Slot should have 64 (max), 6 left over.
    EXPECT_EQ(slot.count, 64);
    EXPECT_EQ(overflow, 6);
}

TEST(Inventory, HotbarSelection) {
    Inventory inv;
    EXPECT_EQ(inv.GetSelectedSlot(), 0);  // Default is 0.
    inv.SetSelectedSlot(5);
    EXPECT_EQ(inv.GetSelectedSlot(), 5);
}

TEST(Inventory, HotbarWrapForward) {
    Inventory inv;
    inv.SetSelectedSlot(8);
    inv.NextHotbarSlot();
    EXPECT_EQ(inv.GetSelectedSlot(), 0);
}

TEST(Inventory, HotbarWrapBackward) {
    Inventory inv;
    inv.SetSelectedSlot(0);
    inv.PrevHotbarSlot();
    EXPECT_EQ(inv.GetSelectedSlot(), 8);
}

TEST(Inventory, AddToFirstEmpty) {
    Inventory inv;
    // Occupy slot 0.
    inv.SetSlot(0, BlockRegistry::kDirt, 64);
    // Add stone — should go to slot 1 (first empty).
    uint8_t leftover = inv.AddItem(BlockRegistry::kStone, 10);
    EXPECT_EQ(leftover, 0);

    ItemSlot s1 = inv.GetSlot(1);
    EXPECT_EQ(s1.block_id, BlockRegistry::kStone);
    EXPECT_EQ(s1.count, 10);
}

TEST(Inventory, AddStacksExisting) {
    Inventory inv;
    // Put 30 stone in slot 0.
    inv.SetSlot(0, BlockRegistry::kStone, 30);
    // Add 20 more stone via AddItem — should stack in slot 0.
    uint8_t leftover = inv.AddItem(BlockRegistry::kStone, 20);
    EXPECT_EQ(leftover, 0);

    ItemSlot s0 = inv.GetSlot(0);
    EXPECT_EQ(s0.block_id, BlockRegistry::kStone);
    EXPECT_EQ(s0.count, 50);
}
