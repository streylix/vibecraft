#ifndef VIBECRAFT_SETTINGS_H
#define VIBECRAFT_SETTINGS_H

#include <string>

namespace vibecraft {

/// Game settings with defaults, clamping, and file persistence.
///
/// Settings are stored as simple key-value pairs and serialized to a
/// plain-text file format (one key=value per line).
class Settings {
public:
    /// Default values.
    static constexpr int kDefaultRenderDistance = 8;
    static constexpr int kDefaultFOV = 70;
    static constexpr float kDefaultVolume = 1.0f;

    /// Clamping ranges.
    static constexpr int kMinRenderDistance = 2;
    static constexpr int kMaxRenderDistance = 32;
    static constexpr int kMinFOV = 30;
    static constexpr int kMaxFOV = 120;
    static constexpr float kMinVolume = 0.0f;
    static constexpr float kMaxVolume = 1.0f;

    /// Construct with default values.
    Settings();

    /// Get the render distance in chunks.
    int GetRenderDistance() const;

    /// Set the render distance. Value is clamped to [kMinRenderDistance, kMaxRenderDistance].
    void SetRenderDistance(int distance);

    /// Get the field of view in degrees.
    int GetFOV() const;

    /// Set the field of view. Value is clamped to [kMinFOV, kMaxFOV].
    void SetFOV(int fov);

    /// Get the master volume [0.0, 1.0].
    float GetVolume() const;

    /// Set the master volume. Value is clamped to [0.0, 1.0].
    void SetVolume(float volume);

    /// Save settings to a file. Returns true on success.
    bool Save(const std::string& path) const;

    /// Load settings from a file. Returns true on success.
    /// Missing keys keep their current (default) values.
    bool Load(const std::string& path);

private:
    int render_distance_;
    int fov_;
    float volume_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_SETTINGS_H
