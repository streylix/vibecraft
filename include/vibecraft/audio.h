#ifndef VIBECRAFT_AUDIO_H
#define VIBECRAFT_AUDIO_H

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace vibecraft {

/// Maximum distance (in blocks) at which a sound can be heard.
static constexpr float kMaxAudioDistance = 16.0f;

/// Information about a registered sound.
struct SoundEntry {
    std::string name;
    std::string file_path;
};

/// Audio system supporting sound registration, volume control,
/// and distance-based attenuation.
///
/// Can run in headless mode (no audio device) for testing.
class AudioSystem {
public:
    /// Construct an AudioSystem.
    /// @param headless If true, no audio device is initialized (for testing).
    /// @param asset_dir Base directory for assets (sounds are in asset_dir/sounds/).
    explicit AudioSystem(bool headless = false,
                         const std::string& asset_dir = "assets");

    /// Shut down the audio system and release resources.
    ~AudioSystem();

    // Non-copyable.
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    /// Initialize the audio engine. Returns true on success.
    bool Init();

    /// Shut down the audio engine.
    void Shutdown();

    /// Returns true if the system has been initialized.
    bool IsInitialized() const;

    /// Register a named sound with its WAV file path (relative to asset_dir/sounds/).
    void RegisterSound(const std::string& name, const std::string& filename);

    /// Register all default game sounds.
    void RegisterDefaultSounds();

    /// Returns true if a sound with the given name is registered.
    bool HasSound(const std::string& name) const;

    /// Get all registered sound entries.
    const std::unordered_map<std::string, SoundEntry>& GetSounds() const;

    /// Get the full file path for a registered sound.
    /// Returns empty string if not found.
    std::string GetSoundFilePath(const std::string& name) const;

    /// Play a sound by name. Does nothing if not found or in headless mode.
    void Play(const std::string& name);

    /// Play a sound at a given 3D position relative to the listener.
    /// Volume is attenuated based on distance.
    void PlayAtPosition(const std::string& name, float distance);

    /// Set the master volume. Clamped to [0.0, 1.0].
    void SetVolume(float volume);

    /// Get the current master volume.
    float GetVolume() const;

    /// Compute the distance attenuation factor for a given distance.
    /// Returns a value in [0.0, 1.0].
    /// Formula: max(0, 1 - distance / max_distance)
    static float ComputeAttenuation(float distance,
                                    float max_distance = kMaxAudioDistance);

private:
    bool headless_;
    bool initialized_;
    float volume_;
    std::string asset_dir_;
    std::unordered_map<std::string, SoundEntry> sounds_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_AUDIO_H
