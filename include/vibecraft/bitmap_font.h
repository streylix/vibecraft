#ifndef VIBECRAFT_BITMAP_FONT_H
#define VIBECRAFT_BITMAP_FONT_H

#include <string>

namespace vibecraft {

/// UV rectangle for a single character glyph.
struct GlyphUV {
    float u0 = 0.0f;  ///< Left edge U coordinate
    float v0 = 0.0f;  ///< Top edge V coordinate
    float u1 = 0.0f;  ///< Right edge U coordinate
    float v1 = 0.0f;  ///< Bottom edge V coordinate

    /// Returns true if this is a valid (non-zero area) UV rectangle.
    bool IsValid() const {
        return u1 > u0 && v1 > v0;
    }
};

/// Bitmap font for rendering text using an ASCII grid texture.
/// Assumes a 16x16 character grid (256 characters), each cell 8x8 pixels.
/// Characters are laid out in ASCII order starting from character 0.
class BitmapFont {
public:
    /// Number of characters per row in the font atlas.
    static constexpr int kCharsPerRow = 16;

    /// Number of rows in the font atlas.
    static constexpr int kCharRows = 16;

    /// Pixel width of each character cell.
    static constexpr int kCharWidth = 8;

    /// Pixel height of each character cell.
    static constexpr int kCharHeight = 8;

    /// Total atlas width in pixels.
    static constexpr int kAtlasWidth = kCharsPerRow * kCharWidth;   // 128

    /// Total atlas height in pixels.
    static constexpr int kAtlasHeight = kCharRows * kCharHeight;    // 128

    BitmapFont() = default;

    /// Get the UV rectangle for a character. Works for any ASCII char.
    GlyphUV GetCharUV(char c) const;

    /// Measure the width of a string in pixels (no kerning, 8px per char).
    int MeasureString(const std::string& text) const;

    /// Get the character width in pixels.
    int GetCharWidth() const { return kCharWidth; }

    /// Get the character height in pixels.
    int GetCharHeight() const { return kCharHeight; }
};

}  // namespace vibecraft

#endif  // VIBECRAFT_BITMAP_FONT_H
