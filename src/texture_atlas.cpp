#include "vibecraft/texture_atlas.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

#include "vibecraft/stb/stb_image.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace vibecraft {

namespace {

// Empty string returned for invalid tile lookups.
const std::string kEmptyString;

// Default texture names matching block registry indices.
// Order must match the texture index assignments in block.cpp.
const std::array<std::string, 23> kDefaultTextureNames = {
    "stone",         // 0
    "grass_top",     // 1
    "grass_side",    // 2
    "dirt",          // 3
    "cobblestone",   // 4
    "oak_planks",    // 5
    "bedrock",       // 6
    "sand",          // 7
    "gravel",        // 8
    "gold_ore",      // 9
    "iron_ore",      // 10
    "coal_ore",      // 11
    "diamond_ore",   // 12
    "oak_log_top",   // 13
    "oak_log_side",  // 14
    "oak_leaves",    // 15
    "glass",         // 16
    "water",         // 17
    "lava",          // 18
    "torch",         // 19
    "snow",          // 20
    "cactus_top",    // 21
    "cactus_side"    // 22
};

}  // namespace

TextureAtlas::TextureAtlas(int tile_size, int tiles_per_row)
    : tile_size_(tile_size),
      tiles_per_row_(tiles_per_row),
      atlas_width_(0),
      atlas_height_(0),
      num_rows_(0),
      texture_id_(0),
      is_built_(false) {
}

TextureAtlas::~TextureAtlas() {
    // Note: We intentionally do NOT delete the GL texture here because
    // the destructor may be called after the GL context is destroyed.
    // The ChunkRenderer or game loop should manage GL resource cleanup.
}

TextureAtlas::TextureAtlas(TextureAtlas&& other) noexcept
    : tile_size_(other.tile_size_),
      tiles_per_row_(other.tiles_per_row_),
      atlas_width_(other.atlas_width_),
      atlas_height_(other.atlas_height_),
      num_rows_(other.num_rows_),
      tile_paths_(std::move(other.tile_paths_)),
      pixel_data_(std::move(other.pixel_data_)),
      texture_id_(other.texture_id_),
      is_built_(other.is_built_) {
    other.texture_id_ = 0;
    other.is_built_ = false;
}

TextureAtlas& TextureAtlas::operator=(TextureAtlas&& other) noexcept {
    if (this != &other) {
        tile_size_ = other.tile_size_;
        tiles_per_row_ = other.tiles_per_row_;
        atlas_width_ = other.atlas_width_;
        atlas_height_ = other.atlas_height_;
        num_rows_ = other.num_rows_;
        tile_paths_ = std::move(other.tile_paths_);
        pixel_data_ = std::move(other.pixel_data_);
        texture_id_ = other.texture_id_;
        is_built_ = other.is_built_;
        other.texture_id_ = 0;
        other.is_built_ = false;
    }
    return *this;
}

void TextureAtlas::AddTile(int index, const std::string& file_path) {
    if (index < 0) return;

    if (static_cast<size_t>(index) >= tile_paths_.size()) {
        tile_paths_.resize(static_cast<size_t>(index) + 1);
    }
    tile_paths_[static_cast<size_t>(index)] = file_path;
}

void TextureAtlas::AddDefaultBlockTextures(const std::string& texture_dir) {
    for (size_t i = 0; i < kDefaultTextureNames.size(); ++i) {
        std::string path = texture_dir + "/" + kDefaultTextureNames[i] + ".png";
        AddTile(static_cast<int>(i), path);
    }
}

bool TextureAtlas::Build() {
    if (tile_paths_.empty()) {
        std::cerr << "TextureAtlas::Build: No tiles registered.\n";
        return false;
    }

    ComputeAtlasDimensions();

    // Allocate atlas pixel data (RGBA, initialized to transparent black).
    size_t total_bytes = static_cast<size_t>(atlas_width_) *
                         static_cast<size_t>(atlas_height_) * 4;
    pixel_data_.resize(total_bytes, 0);

    bool all_loaded = true;

    for (size_t i = 0; i < tile_paths_.size(); ++i) {
        if (tile_paths_[i].empty()) continue;

        int w = 0, h = 0, channels = 0;
        unsigned char* data = stbi_load(tile_paths_[i].c_str(),
                                        &w, &h, &channels, 4);
        if (!data) {
            std::cerr << "TextureAtlas::Build: Failed to load '"
                      << tile_paths_[i] << "': " << stbi_failure_reason()
                      << "\n";
            all_loaded = false;
            continue;
        }

        if (w != tile_size_ || h != tile_size_) {
            std::cerr << "TextureAtlas::Build: Texture '" << tile_paths_[i]
                      << "' is " << w << "x" << h << " but expected "
                      << tile_size_ << "x" << tile_size_ << "\n";
            stbi_image_free(data);
            all_loaded = false;
            continue;
        }

        // Compute position in atlas grid.
        int col = static_cast<int>(i) % tiles_per_row_;
        int row = static_cast<int>(i) / tiles_per_row_;
        int pixel_x = col * tile_size_;
        int pixel_y = row * tile_size_;

        // Copy tile pixels into atlas.
        for (int ty = 0; ty < tile_size_; ++ty) {
            int src_offset = ty * tile_size_ * 4;
            int dst_offset = ((pixel_y + ty) * atlas_width_ + pixel_x) * 4;
            std::memcpy(&pixel_data_[static_cast<size_t>(dst_offset)],
                        &data[src_offset],
                        static_cast<size_t>(tile_size_) * 4);
        }

        stbi_image_free(data);
    }

    is_built_ = true;
    return all_loaded;
}

bool TextureAtlas::UploadToGPU() {
    if (!is_built_) {
        std::cerr << "TextureAtlas::UploadToGPU: Atlas not built yet.\n";
        return false;
    }

    if (texture_id_ != 0) {
        glDeleteTextures(1, &texture_id_);
    }

    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    // Set texture parameters for pixel art (nearest filtering, no mipmaps).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload pixel data.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 atlas_width_, atlas_height_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixel_data_.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture_id_ != 0;
}

void TextureAtlas::Bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
}

TileUVs TextureAtlas::GetUVs(int tile_index) const {
    if (atlas_width_ == 0 || atlas_height_ == 0) {
        return {};
    }

    int col = tile_index % tiles_per_row_;
    int row = tile_index / tiles_per_row_;

    float atlas_w = static_cast<float>(atlas_width_);
    float atlas_h = static_cast<float>(atlas_height_);

    float u_min = static_cast<float>(col * tile_size_) / atlas_w;
    float v_min = static_cast<float>(row * tile_size_) / atlas_h;
    float u_max = static_cast<float>((col + 1) * tile_size_) / atlas_w;
    float v_max = static_cast<float>((row + 1) * tile_size_) / atlas_h;

    return {u_min, v_min, u_max, v_max};
}

int TextureAtlas::GetTileSize() const {
    return tile_size_;
}

int TextureAtlas::GetTilesPerRow() const {
    return tiles_per_row_;
}

int TextureAtlas::GetAtlasWidth() const {
    return atlas_width_;
}

int TextureAtlas::GetAtlasHeight() const {
    return atlas_height_;
}

int TextureAtlas::GetTileCount() const {
    int count = 0;
    for (const auto& path : tile_paths_) {
        if (!path.empty()) {
            ++count;
        }
    }
    return count;
}

unsigned int TextureAtlas::GetTextureId() const {
    return texture_id_;
}

bool TextureAtlas::HasTile(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= tile_paths_.size()) {
        return false;
    }
    return !tile_paths_[static_cast<size_t>(index)].empty();
}

const std::string& TextureAtlas::GetTilePath(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= tile_paths_.size()) {
        return kEmptyString;
    }
    return tile_paths_[static_cast<size_t>(index)];
}

bool TextureAtlas::IsBuilt() const {
    return is_built_;
}

const uint8_t* TextureAtlas::GetPixelData() const {
    return is_built_ ? pixel_data_.data() : nullptr;
}

void TextureAtlas::ComputeAtlasDimensions() {
    int max_index = static_cast<int>(tile_paths_.size()) - 1;
    num_rows_ = (max_index / tiles_per_row_) + 1;

    // Atlas width is always tiles_per_row * tile_size, rounded to power of 2.
    atlas_width_ = NextPowerOfTwo(tiles_per_row_ * tile_size_);

    // Atlas height is num_rows * tile_size, rounded to power of 2.
    atlas_height_ = NextPowerOfTwo(num_rows_ * tile_size_);
}

int TextureAtlas::NextPowerOfTwo(int value) {
    if (value <= 0) return 1;
    // If already a power of two, return as-is.
    if ((value & (value - 1)) == 0) return value;
    int power = 1;
    while (power < value) {
        power <<= 1;
    }
    return power;
}

}  // namespace vibecraft
