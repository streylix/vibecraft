#include <gtest/gtest.h>

#include "vibecraft/bitmap_font.h"

// M18: Bitmap Font

using namespace vibecraft;

TEST(BitmapFont, CharLookup) {
    BitmapFont font;
    GlyphUV uv = font.GetCharUV('A');
    // 'A' is ASCII 65: col = 65 % 16 = 1, row = 65 / 16 = 4
    EXPECT_TRUE(uv.IsValid());
    EXPECT_GT(uv.u1, uv.u0);
    EXPECT_GT(uv.v1, uv.v0);
    // Verify approximate expected UV values.
    EXPECT_NEAR(uv.u0, 1.0f / 16.0f, 1e-5f);
    EXPECT_NEAR(uv.v0, 4.0f / 16.0f, 1e-5f);
    EXPECT_NEAR(uv.u1, 2.0f / 16.0f, 1e-5f);
    EXPECT_NEAR(uv.v1, 5.0f / 16.0f, 1e-5f);
}

TEST(BitmapFont, DigitLookup) {
    BitmapFont font;
    // '0' is ASCII 48 through '9' is ASCII 57.
    for (char c = '0'; c <= '9'; ++c) {
        GlyphUV uv = font.GetCharUV(c);
        EXPECT_TRUE(uv.IsValid()) << "Digit '" << c << "' should have valid UV";
        EXPECT_GT(uv.u1, uv.u0) << "Digit '" << c << "'";
        EXPECT_GT(uv.v1, uv.v0) << "Digit '" << c << "'";
    }
}

TEST(BitmapFont, SpaceCharacter) {
    BitmapFont font;
    // Space is ASCII 32: should not crash and should return a valid UV rect.
    GlyphUV uv = font.GetCharUV(' ');
    EXPECT_TRUE(uv.IsValid());
    // Space at ASCII 32: col = 0, row = 2
    EXPECT_NEAR(uv.u0, 0.0f / 16.0f, 1e-5f);
    EXPECT_NEAR(uv.v0, 2.0f / 16.0f, 1e-5f);
}
