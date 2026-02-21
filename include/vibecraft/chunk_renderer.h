#ifndef VIBECRAFT_CHUNK_RENDERER_H
#define VIBECRAFT_CHUNK_RENDERER_H

#include <cstdint>
#include <unordered_map>
#include <utility>

#include <glm/glm.hpp>

#include "vibecraft/mesh.h"

namespace vibecraft {

class Shader;
class TextureAtlas;

/// GPU-side storage for a single chunk's mesh.
/// Holds the VAO, VBO, and EBO created from MeshData.
struct ChunkGPUMesh {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    uint32_t index_count = 0;

    /// Returns true if the GPU mesh has been uploaded.
    bool IsValid() const { return vao != 0; }
};

/// Hash function for glm::ivec2 (chunk coordinates).
struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const {
        size_t h1 = std::hash<int>()(v.x);
        size_t h2 = std::hash<int>()(v.y);
        return h1 ^ (h2 << 16) ^ (h2 >> 16);
    }
};

/// Renders chunk meshes using OpenGL.
///
/// Manages GPU buffers (VAO/VBO/EBO) for chunk meshes. Each chunk is
/// identified by its (cx, cz) chunk coordinates.
///
/// All methods require an active OpenGL context.
class ChunkRenderer {
public:
    ChunkRenderer();
    ~ChunkRenderer();

    // Non-copyable.
    ChunkRenderer(const ChunkRenderer&) = delete;
    ChunkRenderer& operator=(const ChunkRenderer&) = delete;

    /// Upload mesh data for a chunk. If the chunk already has a mesh,
    /// the old GPU buffers are freed and replaced.
    /// @param cx, cz   Chunk coordinates.
    /// @param mesh      Mesh data (vertices + indices) to upload.
    void UploadChunkMesh(int cx, int cz, const MeshData& mesh);

    /// Remove and free GPU buffers for a chunk.
    /// @param cx, cz   Chunk coordinates.
    void RemoveChunkMesh(int cx, int cz);

    /// Draw all uploaded chunk meshes.
    /// @param shader   The shader to use (must already be Use()'d).
    /// @param atlas    The texture atlas to bind.
    /// @param view     View matrix.
    /// @param proj     Projection matrix.
    void DrawAll(const Shader& shader, const TextureAtlas& atlas,
                 const glm::mat4& view, const glm::mat4& proj);

    /// Draw a single chunk mesh.
    /// @param cx, cz   Chunk coordinates.
    /// @param shader   The shader to use.
    void DrawChunk(int cx, int cz, const Shader& shader);

    /// Get the number of uploaded chunk meshes.
    int GetMeshCount() const;

    /// Check if a chunk has an uploaded mesh.
    bool HasMesh(int cx, int cz) const;

    /// Free all GPU resources.
    void Clear();

private:
    /// Free a single ChunkGPUMesh's GPU buffers.
    void FreeMesh(ChunkGPUMesh& mesh);

    /// Map from chunk coordinates to GPU mesh data.
    std::unordered_map<glm::ivec2, ChunkGPUMesh, IVec2Hash> meshes_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_CHUNK_RENDERER_H
