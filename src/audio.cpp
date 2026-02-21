#include "vibecraft/audio.h"

#include <algorithm>
#include <cmath>

namespace vibecraft {

AudioSystem::AudioSystem(bool headless, const std::string& asset_dir)
    : headless_(headless),
      initialized_(false),
      volume_(1.0f),
      asset_dir_(asset_dir) {}

AudioSystem::~AudioSystem() { Shutdown(); }

bool AudioSystem::Init() {
    if (initialized_) {
        return true;
    }

    // In headless mode, we skip actual audio device initialization.
    // This allows testing without audio hardware.
    initialized_ = true;
    return true;
}

void AudioSystem::Shutdown() {
    if (!initialized_) {
        return;
    }

    sounds_.clear();
    initialized_ = false;
}

bool AudioSystem::IsInitialized() const { return initialized_; }

void AudioSystem::RegisterSound(const std::string& name,
                                const std::string& filename) {
    SoundEntry entry;
    entry.name = name;
    entry.file_path = asset_dir_ + "/sounds/" + filename;
    sounds_[name] = entry;
}

void AudioSystem::RegisterDefaultSounds() {
    RegisterSound("block_break", "block_break.wav");
    RegisterSound("block_place", "block_place.wav");
    RegisterSound("footstep", "footstep.wav");
    RegisterSound("splash", "splash.wav");
}

bool AudioSystem::HasSound(const std::string& name) const {
    return sounds_.find(name) != sounds_.end();
}

const std::unordered_map<std::string, SoundEntry>& AudioSystem::GetSounds()
    const {
    return sounds_;
}

std::string AudioSystem::GetSoundFilePath(const std::string& name) const {
    auto it = sounds_.find(name);
    if (it != sounds_.end()) {
        return it->second.file_path;
    }
    return "";
}

void AudioSystem::Play(const std::string& name) {
    if (!initialized_ || headless_) {
        return;
    }

    // In a full implementation, this would use miniaudio to play the sound.
    // For now, this is a no-op placeholder for non-headless mode.
}

void AudioSystem::PlayAtPosition(const std::string& name, float distance) {
    if (!initialized_ || headless_) {
        return;
    }

    float attenuation = ComputeAttenuation(distance);
    if (attenuation <= 0.0f) {
        return;  // Too far away to hear.
    }

    // In a full implementation, this would play with attenuated volume.
}

void AudioSystem::SetVolume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
}

float AudioSystem::GetVolume() const { return volume_; }

float AudioSystem::ComputeAttenuation(float distance, float max_distance) {
    if (max_distance <= 0.0f) {
        return 0.0f;
    }
    return std::max(0.0f, 1.0f - distance / max_distance);
}

}  // namespace vibecraft
