#include "vibecraft/debug_overlay.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace vibecraft {

DebugOverlay::DebugOverlay()
    : visible_(false),
      fps_(0),
      pos_x_(0.0f),
      pos_y_(0.0f),
      pos_z_(0.0f),
      biome_name_("Unknown") {}

void DebugOverlay::Toggle() {
    visible_ = !visible_;
}

bool DebugOverlay::IsVisible() const {
    return visible_;
}

void DebugOverlay::SetFPS(int fps) {
    fps_ = fps;
}

void DebugOverlay::SetPosition(float x, float y, float z) {
    pos_x_ = x;
    pos_y_ = y;
    pos_z_ = z;
}

void DebugOverlay::SetBiomeName(const std::string& name) {
    biome_name_ = name;
}

std::string DebugOverlay::GetFPSString() const {
    return "FPS: " + std::to_string(fps_);
}

std::string DebugOverlay::GetCoordsString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "XYZ: " << pos_x_ << " / " << pos_y_ << " / " << pos_z_;
    return oss.str();
}

std::string DebugOverlay::GetChunkString() const {
    int chunk_x = static_cast<int>(std::floor(pos_x_)) >> 4;
    int chunk_z = static_cast<int>(std::floor(pos_z_)) >> 4;
    // Handle negative coordinates: >> 4 may not floor-divide for negatives,
    // so use explicit floor division.
    chunk_x = static_cast<int>(std::floor(pos_x_ / 16.0f));
    chunk_z = static_cast<int>(std::floor(pos_z_ / 16.0f));
    return "Chunk: " + std::to_string(chunk_x) + ", " + std::to_string(chunk_z);
}

std::string DebugOverlay::GetBiomeString() const {
    return "Biome: " + biome_name_;
}

}  // namespace vibecraft
