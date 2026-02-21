# Milestone 10 — Texture Atlas & Block Shaders

> **Status: COMPLETE** — 8 tests passing + 1 GPU skip. Atlas packing, UV computation, 23 placeholder textures, chunk renderer, stb_image vendored.

## Description
Load block textures into an OpenGL texture atlas and integrate with the block shader for textured rendering of chunk meshes.

## Goals
- Load 16x16 PNG textures from disk
- Pack into a texture atlas (or texture array)
- Block shader samples correct texture per face using tex_index
- Upload chunk mesh data to GPU (VAO/VBO/EBO)
- Draw calls produce no GL errors

## Deliverables
| File | Purpose |
|------|---------|
| `include/vibecraft/texture_atlas.h` | Texture atlas class |
| `src/texture_atlas.cpp` | Atlas loading and GPU upload |
| `include/vibecraft/chunk_renderer.h` | Chunk rendering class |
| `src/chunk_renderer.cpp` | Mesh upload and draw calls |
| `assets/textures/*.png` | 16x16 block texture files |

## Dependencies
- **M6** — Mesh data to upload
- **M9** — GL context, shaders

## Test Specifications

### File: `tests/test_texture_atlas.cpp`

| Test Name | What It Verifies | Expected |
|-----------|-----------------|----------|
| `TextureAtlas.DimensionsPowerOfTwo` | Atlas dimensions are power of 2 | Width and height are powers of 2 |
| `TextureAtlas.AllBlockTexturesExist` | Required PNG files exist on disk | Every registered block's texture files exist |
| `TextureAtlas.TileSize` | Individual tile is 16x16 | tile_width == 16, tile_height == 16 |
| `TextureAtlas.UVCalculation` | UV coords for a tile index are correct | Tile 0 maps to correct atlas region |
| `TextureAtlas.MeshUploadNoThrow` | Uploading mesh data doesn't crash | No exceptions from mesh upload (mock GL context or skip if no context) |

## Design Decisions
- Use stb_image for PNG loading (header-only, no extra dependency)
- Atlas organized as a grid: 16 tiles per row, as many rows as needed
- Each tile is 16x16 pixels
- Shader uses `tex_index` to compute atlas UV offset
- Chunk meshes uploaded as static VBOs (re-uploaded when chunk is remeshed)
- stb_image.h vendored in `include/vibecraft/stb/` (single-header library)
