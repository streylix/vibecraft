#include "vibecraft/hud.h"

namespace vibecraft {

Hud::Hud(const HudConfig& config) : config_(config) {}

void Hud::SetScreenSize(int width, int height) {
    config_.screen_width = width;
    config_.screen_height = height;
}

float Hud::GetHotbarSlotX(int slot) const {
    // Center the hotbar horizontally.
    float total_width = kHotbarSlotCount * config_.hotbar_slot_size +
                        (kHotbarSlotCount - 1) * config_.hotbar_padding;
    float start_x = (static_cast<float>(config_.screen_width) - total_width) / 2.0f;
    return start_x + slot * (config_.hotbar_slot_size + config_.hotbar_padding);
}

float Hud::GetHotbarY() const {
    // Hotbar at the bottom of the screen with a small margin.
    return static_cast<float>(config_.screen_height) - config_.hotbar_slot_size - 10.0f;
}

float Hud::GetCrosshairX() const {
    return static_cast<float>(config_.screen_width) / 2.0f;
}

float Hud::GetCrosshairY() const {
    return static_cast<float>(config_.screen_height) / 2.0f;
}

void Hud::Render(const Inventory& /*inventory*/, const BitmapFont& /*font*/) {
    // GPU-dependent rendering stub.
    // Will be implemented when wiring up the OpenGL rendering pipeline.
    // This method requires an active OpenGL context.
}

}  // namespace vibecraft
