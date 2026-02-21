#ifndef VIBECRAFT_TEXTURE_ATLAS_H
#define VIBECRAFT_TEXTURE_ATLAS_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace vibecraft {

/// UV coordinates for a single tile in the texture atlas.
struct TileUVs {
    float u_min = 0.0f;
    float v_min = 0.0f;
    float u_max = 0.0f;
    float v_max = 0.0f;
};

/// Texture atlas that packs multiple 16x16 block textures into a single
/// power-of-two texture. Provides CPU-side tile layout and UV computation,
/// as well as optional GPU upload.
///
/// The atlas is arranged as a grid with a fixed number of tiles per row
/// (default 16). Atlas dimensions are always powers of two.
///
/// Usage:
///   1. Call AddTile() for each texture file to register tiles.
///   2. Call Build() to load PNGs and pack them into the atlas image.
///   3. Call UploadToGPU() when an OpenGL context is available.
///   4. Use GetUVs(tile_index) to get UV coordinates for rendering.
class TextureAtlas {
public:
    /// Default tile size in pixels.
    static constexpr int kDefaultTileSize = 16;

    /// Default tiles per row in the atlas.
    static constexpr int kDefaultTilesPerRow = 16;

    /// Construct a texture atlas.
    /// @param tile_size       Pixel dimensions of each tile (default 16).
    /// @param tiles_per_row   Number of tiles per row in the atlas (default 16).
    TextureAtlas(int tile_size = kDefaultTileSize,
                 int tiles_per_row = kDefaultTilesPerRow);

    ~TextureAtlas();

    // Non-copyable.
    TextureAtlas(const TextureAtlas&) = delete;
    TextureAtlas& operator=(const TextureAtlas&) = delete;

    // Movable.
    TextureAtlas(TextureAtlas&& other) noexcept;
    TextureAtlas& operator=(TextureAtlas&& other) noexcept;

    /// Register a texture file for a tile index.
    /// @param index       The tile index (0-based).
    /// @param file_path   Path to the 16x16 PNG file.
    void AddTile(int index, const std::string& file_path);

    /// Register all default block textures from a base directory.
    /// Uses the standard naming convention (stone.png, grass_top.png, etc.).
    /// @param texture_dir  Directory containing the PNG files.
    void AddDefaultBlockTextures(const std::string& texture_dir);

    /// Build the atlas image by loading all registered PNGs and packing them.
    /// @return True on success, false if any texture failed to load.
    bool Build();

    /// Upload the atlas image to an OpenGL texture.
    /// Requires an active OpenGL context.
    /// @return True on success.
    bool UploadToGPU();

    /// Bind the atlas texture to a texture unit.
    /// Requires an active OpenGL context.
    /// @param unit  Texture unit to bind to (default GL_TEXTURE0).
    void Bind(int unit = 0) const;

    /// Get UV coordinates for a tile index.
    /// This is a pure math operation — no GPU required.
    /// @param tile_index  Index of the tile in the atlas.
    /// @return UVs for the tile.
    TileUVs GetUVs(int tile_index) const;

    /// Get the tile size in pixels.
    int GetTileSize() const;

    /// Get the number of tiles per row.
    int GetTilesPerRow() const;

    /// Get the atlas width in pixels.
    int GetAtlasWidth() const;

    /// Get the atlas height in pixels.
    int GetAtlasHeight() const;

    /// Get the number of registered tiles.
    int GetTileCount() const;

    /// Get the OpenGL texture ID (0 if not uploaded).
    unsigned int GetTextureId() const;

    /// Check if a tile index has a registered file path.
    bool HasTile(int index) const;

    /// Get the file path for a tile index.
    const std::string& GetTilePath(int index) const;

    /// Check if the atlas has been built (image data is ready).
    bool IsBuilt() const;

    /// Get the raw atlas pixel data (RGBA, 4 bytes per pixel).
    /// Returns nullptr if not built.
    const uint8_t* GetPixelData() const;

private:
    /// Compute the atlas dimensions as powers of two based on tile count.
    void ComputeAtlasDimensions();

    /// Round up to the next power of two.
    static int NextPowerOfTwo(int value);

    int tile_size_;
    int tiles_per_row_;
    int atlas_width_;
    int atlas_height_;
    int num_rows_;

    /// Tile index -> file path mapping.
    std::vector<std::string> tile_paths_;

    /// Atlas pixel data (RGBA).
    std::vector<uint8_t> pixel_data_;

    /// OpenGL texture ID (0 = not uploaded).
    unsigned int texture_id_;

    /// Whether Build() has been called successfully.
    bool is_built_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_TEXTURE_ATLAS_H
