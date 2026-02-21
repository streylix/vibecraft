#ifndef VIBECRAFT_DEBUG_OVERLAY_H
#define VIBECRAFT_DEBUG_OVERLAY_H

#include <string>

namespace vibecraft {

/// Debug overlay data provider (F3 screen).
///
/// All methods are pure data computation — no GPU required.
/// This class formats debug information into strings suitable for display.
class DebugOverlay {
public:
    DebugOverlay();

    /// Toggle the overlay on or off.
    void Toggle();

    /// Return true if the overlay is currently visible.
    bool IsVisible() const;

    /// Set the current FPS value.
    void SetFPS(int fps);

    /// Set the player world position.
    void SetPosition(float x, float y, float z);

    /// Set the current biome name.
    void SetBiomeName(const std::string& name);

    /// Get the FPS display string (e.g., "FPS: 60").
    std::string GetFPSString() const;

    /// Get the position display string (e.g., "XYZ: 1.50 / 64.00 / 2.30").
    std::string GetCoordsString() const;

    /// Get the chunk coordinates display string (e.g., "Chunk: 0, 0").
    std::string GetChunkString() const;

    /// Get the biome display string (e.g., "Biome: Plains").
    std::string GetBiomeString() const;

private:
    bool visible_;
    int fps_;
    float pos_x_;
    float pos_y_;
    float pos_z_;
    std::string biome_name_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_DEBUG_OVERLAY_H
