#ifndef VIBECRAFT_HUD_H
#define VIBECRAFT_HUD_H

#include "vibecraft/bitmap_font.h"
#include "vibecraft/inventory.h"

namespace vibecraft {

/// HUD rendering configuration.
struct HudConfig {
    int screen_width = 800;
    int screen_height = 600;
    float hotbar_slot_size = 40.0f;
    float hotbar_padding = 2.0f;
    float crosshair_size = 16.0f;
};

/// HUD overlay for rendering hotbar, crosshair, and text.
/// GPU-dependent rendering is in Render(); logic is testable separately.
class Hud {
public:
    explicit Hud(const HudConfig& config = HudConfig{});

    /// Set the screen dimensions (call on window resize).
    void SetScreenSize(int width, int height);

    /// Get the current config.
    const HudConfig& GetConfig() const { return config_; }

    /// Calculate the screen X position of a hotbar slot (0-8).
    float GetHotbarSlotX(int slot) const;

    /// Calculate the screen Y position of the hotbar.
    float GetHotbarY() const;

    /// Calculate the crosshair center position.
    float GetCrosshairX() const;
    float GetCrosshairY() const;

    /// Render the HUD overlay (requires OpenGL context).
    /// This is a no-op stub until rendering is wired up.
    void Render(const Inventory& inventory, const BitmapFont& font);

private:
    HudConfig config_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_HUD_H
