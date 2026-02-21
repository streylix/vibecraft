#include "vibecraft/bitmap_font.h"

namespace vibecraft {

GlyphUV BitmapFont::GetCharUV(char c) const {
    // Treat char as unsigned to get correct ASCII index (0-255).
    unsigned char uc = static_cast<unsigned char>(c);

    int col = uc % kCharsPerRow;
    int row = uc / kCharsPerRow;

    float u0 = static_cast<float>(col) / static_cast<float>(kCharsPerRow);
    float v0 = static_cast<float>(row) / static_cast<float>(kCharRows);
    float u1 = static_cast<float>(col + 1) / static_cast<float>(kCharsPerRow);
    float v1 = static_cast<float>(row + 1) / static_cast<float>(kCharRows);

    return GlyphUV{u0, v0, u1, v1};
}

int BitmapFont::MeasureString(const std::string& text) const {
    return static_cast<int>(text.size()) * kCharWidth;
}

}  // namespace vibecraft
