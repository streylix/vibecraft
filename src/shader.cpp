#include "vibecraft/shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include <glm/gtc/type_ptr.hpp>

namespace vibecraft {

namespace shader_utils {

std::string ReadFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool FileExists(const std::string& file_path) {
    std::ifstream file(file_path);
    return file.good();
}

}  // namespace shader_utils

Shader::Shader()
    : program_id_(0) {
}

Shader::~Shader() {
    if (program_id_ != 0) {
        glDeleteProgram(program_id_);
    }
}

Shader::Shader(Shader&& other) noexcept
    : program_id_(other.program_id_),
      error_log_(std::move(other.error_log_)) {
    other.program_id_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (program_id_ != 0) {
            glDeleteProgram(program_id_);
        }
        program_id_ = other.program_id_;
        error_log_ = std::move(other.error_log_);
        other.program_id_ = 0;
    }
    return *this;
}

bool Shader::Compile(const std::string& vertex_source,
                     const std::string& fragment_source) {
    error_log_.clear();

    // Compile vertex shader.
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* v_src = vertex_source.c_str();
    glShaderSource(vertex_shader, 1, &v_src, nullptr);
    glCompileShader(vertex_shader);

    int success = 0;
    char info_log[1024];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, sizeof(info_log), nullptr, info_log);
        error_log_ = "Vertex shader compilation failed:\n";
        error_log_ += info_log;
        glDeleteShader(vertex_shader);
        return false;
    }

    // Compile fragment shader.
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* f_src = fragment_source.c_str();
    glShaderSource(fragment_shader, 1, &f_src, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
        error_log_ = "Fragment shader compilation failed:\n";
        error_log_ += info_log;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    // Link program.
    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex_shader);
    glAttachShader(program_id_, fragment_shader);
    glLinkProgram(program_id_);

    glGetProgramiv(program_id_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program_id_, sizeof(info_log), nullptr, info_log);
        error_log_ = "Shader program linking failed:\n";
        error_log_ += info_log;
        glDeleteProgram(program_id_);
        program_id_ = 0;
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    // Clean up individual shaders (they're linked into the program now).
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return true;
}

bool Shader::CompileFromFiles(const std::string& vertex_path,
                              const std::string& fragment_path) {
    std::string vertex_source = shader_utils::ReadFile(vertex_path);
    if (vertex_source.empty()) {
        error_log_ = "Failed to read vertex shader file: " + vertex_path;
        return false;
    }

    std::string fragment_source = shader_utils::ReadFile(fragment_path);
    if (fragment_source.empty()) {
        error_log_ = "Failed to read fragment shader file: " + fragment_path;
        return false;
    }

    return Compile(vertex_source, fragment_source);
}

void Shader::Use() const {
    glUseProgram(program_id_);
}

unsigned int Shader::GetProgramId() const {
    return program_id_;
}

std::string Shader::GetErrorLog() const {
    return error_log_;
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(program_id_, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(program_id_, name.c_str()), value);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program_id_, name.c_str()), 1,
                 glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program_id_, name.c_str()), 1,
                       GL_FALSE, glm::value_ptr(value));
}

}  // namespace vibecraft
