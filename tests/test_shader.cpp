#include <gtest/gtest.h>

#include <string>

#include "vibecraft/shader.h"

// M9: Shaders

using vibecraft::shader_utils::FileExists;
using vibecraft::shader_utils::ReadFile;

// Path to shader assets — relative to the build directory or project root.
// Tests are run from the build directory, so we look relative to the source dir.
// CMake sets the working directory to the build dir, so use the source tree path.
static const std::string kShaderDir = VIBECRAFT_ASSET_DIR "/shaders/";

TEST(Shader, ValidVertexSource) {
    std::string source = ReadFile(kShaderDir + "block.vert");
    ASSERT_FALSE(source.empty()) << "Could not read block.vert";
    EXPECT_NE(source.find("#version 330 core"), std::string::npos)
        << "Vertex shader must contain '#version 330 core'";
    // Should contain main function.
    EXPECT_NE(source.find("void main()"), std::string::npos)
        << "Vertex shader must contain 'void main()'";
}

TEST(Shader, ValidFragmentSource) {
    std::string source = ReadFile(kShaderDir + "block.frag");
    ASSERT_FALSE(source.empty()) << "Could not read block.frag";
    EXPECT_NE(source.find("#version 330 core"), std::string::npos)
        << "Fragment shader must contain '#version 330 core'";
    // Should contain main function.
    EXPECT_NE(source.find("void main()"), std::string::npos)
        << "Fragment shader must contain 'void main()'";
}

TEST(Shader, ShaderFileExists) {
    EXPECT_TRUE(FileExists(kShaderDir + "block.vert"))
        << "block.vert should exist at: " << kShaderDir + "block.vert";
    EXPECT_TRUE(FileExists(kShaderDir + "block.frag"))
        << "block.frag should exist at: " << kShaderDir + "block.frag";
}
