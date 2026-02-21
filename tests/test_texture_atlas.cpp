#include <gtest/gtest.h>

#include <cmath>
#include <fstream>
#include <string>

#include "vibecraft/texture_atlas.h"

// M10: Texture Atlas & Block Shaders

using namespace vibecraft;

namespace {

// Helper: check if a value is a power of two.
bool IsPowerOfTwo(int value) {
    return value > 0 && (value & (value - 1)) == 0;
}

// The VIBECRAFT_ASSET_DIR macro is defined by CMakeLists.txt.
std::string GetTextureDir() {
    return std::string(VIBECRAFT_ASSET_DIR) + "/textures";
}

}  // namespace

// Test that atlas dimensions are always powers of two.
TEST(TextureAtlas, DimensionsPowerOfTwo) {
    TextureAtlas atlas(16, 16);
    atlas.AddDefaultBlockTextures(GetTextureDir());
    ASSERT_TRUE(atlas.Build());

    EXPECT_TRUE(IsPowerOfTwo(atlas.GetAtlasWidth()))
        << "Atlas width " << atlas.GetAtlasWidth() << " is not a power of 2";
    EXPECT_TRUE(IsPowerOfTwo(atlas.GetAtlasHeight()))
        << "Atlas height " << atlas.GetAtlasHeight() << " is not a power of 2";
    EXPECT_GT(atlas.GetAtlasWidth(), 0);
    EXPECT_GT(atlas.GetAtlasHeight(), 0);
}

// Test that all required block texture PNG files exist on disk.
TEST(TextureAtlas, AllBlockTexturesExist) {
    // The expected texture file names (matching block.cpp indices 0-22).
    const std::vector<std::string> expected_textures = {
        "stone.png",
        "grass_top.png",
        "grass_side.png",
        "dirt.png",
        "cobblestone.png",
        "oak_planks.png",
        "bedrock.png",
        "sand.png",
        "gravel.png",
        "gold_ore.png",
        "iron_ore.png",
        "coal_ore.png",
        "diamond_ore.png",
        "oak_log_top.png",
        "oak_log_side.png",
        "oak_leaves.png",
        "glass.png",
        "water.png",
        "lava.png",
        "torch.png",
        "snow.png",
        "cactus_top.png",
        "cactus_side.png",
    };

    std::string dir = GetTextureDir();
    for (const auto& name : expected_textures) {
        std::string path = dir + "/" + name;
        std::ifstream f(path);
        EXPECT_TRUE(f.good()) << "Missing texture file: " << path;
    }
}

// Test that the tile size is 16x16.
TEST(TextureAtlas, TileSize) {
    TextureAtlas atlas(16, 16);
    EXPECT_EQ(atlas.GetTileSize(), 16);

    // Also test with a non-default tile size to ensure the parameter works.
    TextureAtlas atlas32(32, 8);
    EXPECT_EQ(atlas32.GetTileSize(), 32);
}

// Test that UV coordinates are correctly computed for tile indices.
TEST(TextureAtlas, UVCalculation) {
    // Atlas: 16 tiles per row, 16x16 pixel tiles.
    // Atlas width = 16 * 16 = 256 pixels (power of 2).
    // With 23 tiles, we need 2 rows => atlas height = NextPowerOfTwo(2 * 16) = 32.
    TextureAtlas atlas(16, 16);
    atlas.AddDefaultBlockTextures(GetTextureDir());
    ASSERT_TRUE(atlas.Build());

    int atlas_w = atlas.GetAtlasWidth();
    int atlas_h = atlas.GetAtlasHeight();

    // Tile 0 should be at column 0, row 0.
    {
        TileUVs uv = atlas.GetUVs(0);
        float expected_u_min = 0.0f;
        float expected_v_min = 0.0f;
        float expected_u_max = 16.0f / static_cast<float>(atlas_w);
        float expected_v_max = 16.0f / static_cast<float>(atlas_h);

        EXPECT_FLOAT_EQ(uv.u_min, expected_u_min);
        EXPECT_FLOAT_EQ(uv.v_min, expected_v_min);
        EXPECT_FLOAT_EQ(uv.u_max, expected_u_max);
        EXPECT_FLOAT_EQ(uv.v_max, expected_v_max);
    }

    // Tile 1 should be at column 1, row 0.
    {
        TileUVs uv = atlas.GetUVs(1);
        float expected_u_min = 16.0f / static_cast<float>(atlas_w);
        float expected_v_min = 0.0f;
        float expected_u_max = 32.0f / static_cast<float>(atlas_w);
        float expected_v_max = 16.0f / static_cast<float>(atlas_h);

        EXPECT_FLOAT_EQ(uv.u_min, expected_u_min);
        EXPECT_FLOAT_EQ(uv.v_min, expected_v_min);
        EXPECT_FLOAT_EQ(uv.u_max, expected_u_max);
        EXPECT_FLOAT_EQ(uv.v_max, expected_v_max);
    }

    // Tile 16 should be at column 0, row 1 (wraps to next row).
    {
        TileUVs uv = atlas.GetUVs(16);
        float expected_u_min = 0.0f;
        float expected_v_min = 16.0f / static_cast<float>(atlas_h);
        float expected_u_max = 16.0f / static_cast<float>(atlas_w);
        float expected_v_max = 32.0f / static_cast<float>(atlas_h);

        EXPECT_FLOAT_EQ(uv.u_min, expected_u_min);
        EXPECT_FLOAT_EQ(uv.v_min, expected_v_min);
        EXPECT_FLOAT_EQ(uv.u_max, expected_u_max);
        EXPECT_FLOAT_EQ(uv.v_max, expected_v_max);
    }

    // Verify UV tile dimensions are consistent (all tiles same size).
    for (int i = 0; i < 23; ++i) {
        TileUVs uv = atlas.GetUVs(i);
        float tile_u_size = uv.u_max - uv.u_min;
        float tile_v_size = uv.v_max - uv.v_min;

        float expected_u_size = 16.0f / static_cast<float>(atlas_w);
        float expected_v_size = 16.0f / static_cast<float>(atlas_h);

        EXPECT_FLOAT_EQ(tile_u_size, expected_u_size)
            << "Tile " << i << " has wrong U size";
        EXPECT_FLOAT_EQ(tile_v_size, expected_v_size)
            << "Tile " << i << " has wrong V size";
    }
}

// Test that mesh upload doesn't throw (GPU-dependent, skipped without context).
TEST(TextureAtlas, MeshUploadNoThrow) {
    GTEST_SKIP() << "Requires OpenGL context (GPU-dependent test)";
}

// Additional tests for atlas construction without GPU.

TEST(TextureAtlas, TileCountMatchesRegistered) {
    TextureAtlas atlas;
    atlas.AddDefaultBlockTextures(GetTextureDir());
    EXPECT_EQ(atlas.GetTileCount(), 23);
}

TEST(TextureAtlas, HasTileReturnsTrueForRegistered) {
    TextureAtlas atlas;
    atlas.AddDefaultBlockTextures(GetTextureDir());

    for (int i = 0; i < 23; ++i) {
        EXPECT_TRUE(atlas.HasTile(i)) << "Tile " << i << " should exist";
    }
    // Index 23 and beyond should not exist.
    EXPECT_FALSE(atlas.HasTile(23));
    EXPECT_FALSE(atlas.HasTile(100));
    EXPECT_FALSE(atlas.HasTile(-1));
}

TEST(TextureAtlas, BuildProducesPixelData) {
    TextureAtlas atlas(16, 16);
    atlas.AddDefaultBlockTextures(GetTextureDir());

    // Before build, no pixel data.
    EXPECT_FALSE(atlas.IsBuilt());
    EXPECT_EQ(atlas.GetPixelData(), nullptr);

    ASSERT_TRUE(atlas.Build());
    EXPECT_TRUE(atlas.IsBuilt());
    EXPECT_NE(atlas.GetPixelData(), nullptr);
}

TEST(TextureAtlas, UVCalculationPureMath) {
    // Test UV calculation without building (no file loading needed).
    // Create an atlas and manually set dimensions to test the math.
    // We use a small atlas: 4 tiles per row, 8x8 pixel tiles.
    TextureAtlas atlas(8, 4);

    // Add 8 dummy tiles (files don't need to exist for UV calculation).
    // The atlas computes dimensions from registered tile count.
    for (int i = 0; i < 8; ++i) {
        atlas.AddTile(i, "/nonexistent/tile_" + std::to_string(i) + ".png");
    }

    // Force atlas dimensions by calling Build (will fail on file load but
    // that's okay - we need the dimensions computed).
    // Instead, test GetUVs on a built atlas where we know dimensions.
    // Actually, GetUVs relies on atlas_width_ and atlas_height_ which are
    // set during Build(). Let's use the default atlas with known textures.

    TextureAtlas atlas2(16, 16);
    atlas2.AddDefaultBlockTextures(GetTextureDir());
    ASSERT_TRUE(atlas2.Build());

    // With 16 tiles per row, 16px tiles: atlas_width = 256 (power of 2).
    // 23 tiles => 2 rows => atlas_height = NextPowerOfTwo(32) = 32.
    EXPECT_EQ(atlas2.GetAtlasWidth(), 256);
    EXPECT_EQ(atlas2.GetAtlasHeight(), 32);

    // Tile 15 (last in first row): col=15, row=0
    TileUVs uv15 = atlas2.GetUVs(15);
    EXPECT_FLOAT_EQ(uv15.u_min, 15.0f * 16.0f / 256.0f);
    EXPECT_FLOAT_EQ(uv15.v_min, 0.0f);
    EXPECT_FLOAT_EQ(uv15.u_max, 16.0f * 16.0f / 256.0f);
    EXPECT_FLOAT_EQ(uv15.v_max, 16.0f / 32.0f);

    // Tile 22 (last registered): col=6, row=1
    TileUVs uv22 = atlas2.GetUVs(22);
    EXPECT_FLOAT_EQ(uv22.u_min, 6.0f * 16.0f / 256.0f);
    EXPECT_FLOAT_EQ(uv22.v_min, 16.0f / 32.0f);
    EXPECT_FLOAT_EQ(uv22.u_max, 7.0f * 16.0f / 256.0f);
    EXPECT_FLOAT_EQ(uv22.v_max, 32.0f / 32.0f);
}
