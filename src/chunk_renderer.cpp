#include "vibecraft/chunk_renderer.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "vibecraft/shader.h"
#include "vibecraft/texture_atlas.h"

namespace vibecraft {

ChunkRenderer::ChunkRenderer() = default;

ChunkRenderer::~ChunkRenderer() {
    // Note: Do not call Clear() here because the GL context may already
    // be destroyed. The game loop should call Clear() before shutdown.
}

void ChunkRenderer::UploadChunkMesh(int cx, int cz, const MeshData& mesh) {
    glm::ivec2 key(cx, cz);

    // Free existing mesh if present.
    auto it = meshes_.find(key);
    if (it != meshes_.end()) {
        FreeMesh(it->second);
        meshes_.erase(it);
    }

    if (mesh.Empty()) return;

    ChunkGPUMesh gpu_mesh;
    gpu_mesh.index_count = static_cast<uint32_t>(mesh.indices.size());

    // Generate VAO, VBO, EBO.
    glGenVertexArrays(1, &gpu_mesh.vao);
    glGenBuffers(1, &gpu_mesh.vbo);
    glGenBuffers(1, &gpu_mesh.ebo);

    glBindVertexArray(gpu_mesh.vao);

    // Upload vertex data.
    glBindBuffer(GL_ARRAY_BUFFER, gpu_mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vertex)),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    // Upload index data.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(uint32_t)),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    // Set vertex attribute pointers.
    // Vertex layout: position(3f), tex_coord(2f), tex_index(1f), normal(3f), ao(1f)
    // Total: 10 floats = 40 bytes.

    // Attribute 0: position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    // Attribute 1: tex_coord (vec2)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, tex_coord)));

    // Attribute 2: tex_index (float)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, tex_index)));

    // Attribute 3: normal (vec3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    // Attribute 4: ao (float)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, ao)));

    // Unbind VAO (keep EBO bound to VAO).
    glBindVertexArray(0);

    meshes_[key] = gpu_mesh;
}

void ChunkRenderer::RemoveChunkMesh(int cx, int cz) {
    auto it = meshes_.find(glm::ivec2(cx, cz));
    if (it != meshes_.end()) {
        FreeMesh(it->second);
        meshes_.erase(it);
    }
}

void ChunkRenderer::DrawAll(const Shader& shader, const TextureAtlas& atlas,
                             const glm::mat4& view, const glm::mat4& proj) {
    shader.Use();
    shader.SetMat4("u_view", view);
    shader.SetMat4("u_projection", proj);
    shader.SetInt("u_texture_atlas", 0);

    atlas.Bind(0);

    for (const auto& [coords, gpu_mesh] : meshes_) {
        if (!gpu_mesh.IsValid()) continue;

        // Model matrix: translate chunk to world position.
        glm::mat4 model = glm::translate(glm::mat4(1.0f),
            glm::vec3(coords.x * 16.0f, 0.0f, coords.y * 16.0f));
        shader.SetMat4("u_model", model);

        glBindVertexArray(gpu_mesh.vao);
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(gpu_mesh.index_count),
                       GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);
}

void ChunkRenderer::DrawChunk(int cx, int cz, const Shader& shader) {
    auto it = meshes_.find(glm::ivec2(cx, cz));
    if (it == meshes_.end() || !it->second.IsValid()) return;

    const auto& gpu_mesh = it->second;

    glm::mat4 model = glm::translate(glm::mat4(1.0f),
        glm::vec3(cx * 16.0f, 0.0f, cz * 16.0f));
    shader.SetMat4("u_model", model);

    glBindVertexArray(gpu_mesh.vao);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(gpu_mesh.index_count),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

int ChunkRenderer::GetMeshCount() const {
    return static_cast<int>(meshes_.size());
}

bool ChunkRenderer::HasMesh(int cx, int cz) const {
    return meshes_.count(glm::ivec2(cx, cz)) > 0;
}

void ChunkRenderer::Clear() {
    for (auto& [coords, mesh] : meshes_) {
        FreeMesh(mesh);
    }
    meshes_.clear();
}

void ChunkRenderer::FreeMesh(ChunkGPUMesh& mesh) {
    if (mesh.vao != 0) {
        glDeleteVertexArrays(1, &mesh.vao);
    }
    if (mesh.vbo != 0) {
        glDeleteBuffers(1, &mesh.vbo);
    }
    if (mesh.ebo != 0) {
        glDeleteBuffers(1, &mesh.ebo);
    }
    mesh = ChunkGPUMesh{};
}

}  // namespace vibecraft
