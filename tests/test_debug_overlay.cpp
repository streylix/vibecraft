#include <gtest/gtest.h>

#include <string>

#include "vibecraft/debug_overlay.h"

// M24: Debug Overlay

TEST(DebugOverlay, FPSDisplay) {
    vibecraft::DebugOverlay overlay;
    overlay.SetFPS(60);
    std::string fps_str = overlay.GetFPSString();
    // String should contain "60".
    EXPECT_NE(fps_str.find("60"), std::string::npos);
    // String should contain "FPS".
    EXPECT_NE(fps_str.find("FPS"), std::string::npos);
}

TEST(DebugOverlay, CoordsDisplay) {
    vibecraft::DebugOverlay overlay;
    overlay.SetPosition(1.5f, 64.0f, 2.3f);
    std::string coords = overlay.GetCoordsString();
    // Should contain the coordinate values.
    EXPECT_NE(coords.find("1.50"), std::string::npos);
    EXPECT_NE(coords.find("64.00"), std::string::npos);
    EXPECT_NE(coords.find("2.30"), std::string::npos);
}

TEST(DebugOverlay, ChunkDisplay) {
    vibecraft::DebugOverlay overlay;
    // Position (1.5, 64, 2.3) is in chunk (0, 0).
    overlay.SetPosition(1.5f, 64.0f, 2.3f);
    std::string chunk_str = overlay.GetChunkString();
    EXPECT_NE(chunk_str.find("0, 0"), std::string::npos);
}

TEST(DebugOverlay, BiomeDisplay) {
    vibecraft::DebugOverlay overlay;
    overlay.SetBiomeName("Plains");
    std::string biome_str = overlay.GetBiomeString();
    EXPECT_NE(biome_str.find("Plains"), std::string::npos);
}
