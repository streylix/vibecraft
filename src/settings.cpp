#include "vibecraft/settings.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

namespace vibecraft {

Settings::Settings()
    : render_distance_(kDefaultRenderDistance),
      fov_(kDefaultFOV),
      volume_(kDefaultVolume) {}

int Settings::GetRenderDistance() const {
    return render_distance_;
}

void Settings::SetRenderDistance(int distance) {
    render_distance_ = std::clamp(distance, kMinRenderDistance, kMaxRenderDistance);
}

int Settings::GetFOV() const {
    return fov_;
}

void Settings::SetFOV(int fov) {
    fov_ = std::clamp(fov, kMinFOV, kMaxFOV);
}

float Settings::GetVolume() const {
    return volume_;
}

void Settings::SetVolume(float volume) {
    volume_ = std::clamp(volume, kMinVolume, kMaxVolume);
}

bool Settings::Save(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << "render_distance=" << render_distance_ << "\n";
    file << "fov=" << fov_ << "\n";
    file << "volume=" << volume_ << "\n";
    return file.good();
}

bool Settings::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments.
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        if (key == "render_distance") {
            SetRenderDistance(std::stoi(value));
        } else if (key == "fov") {
            SetFOV(std::stoi(value));
        } else if (key == "volume") {
            SetVolume(std::stof(value));
        }
    }

    return true;
}

}  // namespace vibecraft
