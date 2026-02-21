# Milestone 18 — Inventory, Hotbar & HUD

> **Status: COMPLETE** — All 14 tests passing (11 inventory + 3 bitmap font). 36-slot inventory, stack to 64, hotbar selection, 8x8 bitmap font.

## Description
Implement the inventory system (36 slots), hotbar (bottom 9 slots), and basic HUD rendering with item counts and a bitmap font.

## Goals
- 36-slot inventory (9 hotbar + 27 main)
- Item stacking up to 64
- Slot operations: set, get, swap, add with overflow
- Hotbar selection (1-9 keys, scroll wheel)
- Hotbar wraps around at boundaries
- Bitmap font for text rendering (item counts, debug info)

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/inventory.h` | Inventory class |
| `src/inventory.cpp` | Inventory implementation |
| `include/vibecraft/hud.h` | HUD rendering class |
| `src/hud.cpp` | HUD rendering implementation |
| `include/vibecraft/bitmap_font.h` | Bitmap font renderer |
| `src/bitmap_font.cpp` | Font implementation |
| `assets/textures/font.png` | Bitmap font atlas |

## Dependencies
- **M9** — Rendering context
- **M10** — Texture loading
- **M13** — Block interaction (to connect inventory to break/place)

## Test Specifications

### File: `tests/test_inventory.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `Inventory.SlotCount` | 36 total slots | `GetSlotCount() == 36` |
| `Inventory.EmptyByDefault` | All slots start empty | Every slot has count == 0 |
| `Inventory.SetAndGet` | Set/get slot contents | Set stone x10, get returns stone x10 |
| `Inventory.SwapSlots` | Swap two slots | Contents exchange correctly |
| `Inventory.StackToMax` | Add items up to stack limit | Adding 60 + 10 = 64 in slot + 6 overflow |
| `Inventory.StackOverflow` | Overflow returns remainder | Adding 70 to empty slot → 64 in slot, returns 6 |
| `Inventory.HotbarSelection` | Select hotbar slot 0-8 | `SetSelectedSlot(5)` → `GetSelectedSlot() == 5` |
| `Inventory.HotbarWrapForward` | Selection wraps 8 → 0 | Incrementing past 8 wraps to 0 |
| `Inventory.HotbarWrapBackward` | Selection wraps 0 → 8 | Decrementing past 0 wraps to 8 |
| `Inventory.AddToFirstEmpty` | Add finds first empty slot | Adding item goes to lowest empty slot |
| `Inventory.AddStacksExisting` | Add stacks with existing items | Add stone when stone already in slot → stacks |

### File: `tests/test_bitmap_font.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `BitmapFont.CharLookup` | Character UV lookup works | 'A' returns valid UV rectangle |
| `BitmapFont.DigitLookup` | Digit characters work | '0'-'9' all return valid UVs |
| `BitmapFont.SpaceCharacter` | Space has zero-width or valid UV | Space character doesn't crash |

## Design Decisions
- Inventory slot: `struct ItemSlot { BlockId block_id; uint8_t count; }`
- Stack max: 64 for all items (simplified from Minecraft's varied max stacks)
- Hotbar is slots 0-8 of the inventory array
- HUD rendered as 2D overlay using orthographic projection
- Bitmap font: ASCII characters in a grid texture, 8x8 pixels per character
- Selected hotbar slot highlighted with a different colored border
