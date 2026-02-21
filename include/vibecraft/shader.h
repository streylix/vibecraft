#ifndef VIBECRAFT_SHADER_H
#define VIBECRAFT_SHADER_H

#include <string>

#include <glm/glm.hpp>

namespace vibecraft {

/// Utility for loading shader source files from disk.
/// Shader compilation and linking requires an OpenGL context and is done
/// separately in the Shader program object.
namespace shader_utils {

/// Read the entire contents of a file into a string.
/// Returns an empty string if the file cannot be opened.
std::string ReadFile(const std::string& file_path);

/// Check whether a file exists and is readable.
bool FileExists(const std::string& file_path);

}  // namespace shader_utils

/// OpenGL shader program wrapper.
///
/// Compiles vertex and fragment shaders from source strings, links them
/// into a program, and provides uniform-setting helpers.
///
/// GPU-dependent: requires an active OpenGL context for all methods
/// except the static source-loading helpers in shader_utils.
class Shader {
public:
    /// Construct an uninitialized shader (program ID = 0).
    Shader();

    /// Destructor — deletes the GL program if one was created.
    ~Shader();

    // Non-copyable.
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Movable.
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /// Compile and link a shader program from vertex and fragment source strings.
    /// Returns true on success, false on compilation/linking errors.
    /// Error messages are stored and retrievable via GetErrorLog().
    bool Compile(const std::string& vertex_source,
                 const std::string& fragment_source);

    /// Compile and link a shader program from files on disk.
    /// Returns true on success, false on errors.
    bool CompileFromFiles(const std::string& vertex_path,
                          const std::string& fragment_path);

    /// Activate this shader program for rendering.
    void Use() const;

    /// Get the OpenGL program ID.
    unsigned int GetProgramId() const;

    /// Get the last error log (compilation or linking errors).
    std::string GetErrorLog() const;

    /// Set uniform values.
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
    unsigned int program_id_;
    std::string error_log_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_SHADER_H
