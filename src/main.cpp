#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vibecraft/block.h"
#include "vibecraft/block_interaction.h"
#include "vibecraft/camera.h"
#include "vibecraft/chunk.h"
#include "vibecraft/chunk_mesher.h"
#include "vibecraft/chunk_renderer.h"
#include "vibecraft/debug_overlay.h"
#include "vibecraft/game_loop.h"
#include "vibecraft/input.h"
#include "vibecraft/inventory.h"
#include "vibecraft/player.h"
#include "vibecraft/raycast.h"
#include "vibecraft/renderer.h"
#include "vibecraft/shader.h"
#include "vibecraft/sky.h"
#include "vibecraft/terrain_generator.h"
#include "vibecraft/texture_atlas.h"
#include "vibecraft/window.h"
#include "vibecraft/world.h"

// GLFW key codes used.
static constexpr int kKeyW = GLFW_KEY_W;
static constexpr int kKeyA = GLFW_KEY_A;
static constexpr int kKeyS = GLFW_KEY_S;
static constexpr int kKeyD = GLFW_KEY_D;
static constexpr int kKeySpace = GLFW_KEY_SPACE;
static constexpr int kKeyEscape = GLFW_KEY_ESCAPE;
static constexpr int kKeyF3 = GLFW_KEY_F3;
static constexpr int kKeyF4 = GLFW_KEY_F4;

static constexpr int kRenderDistance = 2;  // chunks
static constexpr float kMoveSpeed = 6.0f;  // blocks per second
static constexpr float kMouseSensitivity = 0.1f;
static constexpr uint32_t kWorldSeed = 42;

// Global pointers for GLFW callbacks.
static vibecraft::Input* g_input = nullptr;
static vibecraft::Window* g_window = nullptr;

// Mouse button state (tracked via GLFW callback).
static bool g_mouse_left_held = false;
static bool g_mouse_right_pressed = false;  // edge-triggered: true for one frame

static void KeyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/,
                         int action, int /*mods*/) {
    if (g_input && key >= 0 && key < vibecraft::Input::kMaxKeys) {
        g_input->SetKeyState(key, action != GLFW_RELEASE);
    }
}

static void CursorPosCallback(GLFWwindow* /*window*/, double x, double y) {
    if (g_input) {
        g_input->SetMousePosition(x, y);
    }
}

static void ScrollCallback(GLFWwindow* /*window*/, double xoff, double yoff) {
    if (g_input) {
        g_input->SetScrollOffset(xoff, yoff);
    }
}

static void MouseButtonCallback(GLFWwindow* /*window*/, int button, int action,
                                  int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_mouse_left_held = (action != GLFW_RELEASE);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        g_mouse_right_pressed = true;
    }
}

// GL calls needed for crosshair rendering are included via OpenGL/gl3.h above.

// Framebuffer resize handled by Renderer.SetViewport each frame.

static std::string BiomeNameString(vibecraft::BiomeType type) {
    switch (type) {
        case vibecraft::BiomeType::kPlains: return "Plains";
        case vibecraft::BiomeType::kForest: return "Forest";
        case vibecraft::BiomeType::kDesert: return "Desert";
        case vibecraft::BiomeType::kMountains: return "Mountains";
        case vibecraft::BiomeType::kTundra: return "Tundra";
        default: return "Unknown";
    }
}

int main() {
    std::cout << "VibeCraft — starting up...\n";

    // --- Systems ---
    vibecraft::Window window(1280, 720, "VibeCraft");
    g_window = &window;

    window.SetCursorEnabled(false);

    vibecraft::Input input;
    g_input = &input;
    input.SetSensitivity(kMouseSensitivity);

    // Install GLFW callbacks manually to avoid user-pointer conflicts.
    glfwSetKeyCallback(window.GetHandle(), KeyCallback);
    glfwSetCursorPosCallback(window.GetHandle(), CursorPosCallback);
    glfwSetScrollCallback(window.GetHandle(), ScrollCallback);
    glfwSetMouseButtonCallback(window.GetHandle(), MouseButtonCallback);
    // Framebuffer resize is handled by renderer.SetViewport each frame.

    vibecraft::Renderer renderer;
    renderer.Init();
    renderer.SetViewport(0, 0, window.GetWidth(), window.GetHeight());

    vibecraft::BlockRegistry registry;

    // Shader.
    vibecraft::Shader block_shader;
    if (!block_shader.CompileFromFiles("assets/shaders/block.vert",
                                        "assets/shaders/block.frag")) {
        std::cerr << "Shader error: " << block_shader.GetErrorLog() << "\n";
        return 1;
    }
    std::cout << "Shaders compiled.\n";

    // Texture atlas.
    vibecraft::TextureAtlas atlas;
    atlas.AddDefaultBlockTextures("assets/textures");
    if (!atlas.Build()) {
        std::cerr << "Failed to build texture atlas\n";
        return 1;
    }
    if (!atlas.UploadToGPU()) {
        std::cerr << "Failed to upload atlas to GPU\n";
        return 1;
    }

    // World & terrain.
    vibecraft::World world;
    vibecraft::TerrainGenerator terrain_gen(kWorldSeed);

    // Player — spawn at world center, find surface height.
    int spawn_height = terrain_gen.GetHeight(0, 0) + 2;
    vibecraft::Player player(glm::vec3(0.5f, static_cast<float>(spawn_height), 0.5f));

    // Camera.
    vibecraft::Camera camera(player.GetEyePosition());
    camera.SetAspectRatio(window.GetAspectRatio());

    // Sky.
    vibecraft::Sky sky;
    sky.SetTime(6000);  // Start at noon.

    // Debug overlay.
    vibecraft::DebugOverlay debug_overlay;

    // Chunk renderer.
    vibecraft::ChunkRenderer chunk_renderer;

    // Block interaction and inventory.
    vibecraft::BlockInteraction block_interaction;
    vibecraft::Inventory inventory;
    // Seed the hotbar with some common block types for placing.
    inventory.SetSlot(0, vibecraft::BlockRegistry::kStone, 64);
    inventory.SetSlot(1, vibecraft::BlockRegistry::kDirt, 64);
    inventory.SetSlot(2, vibecraft::BlockRegistry::kCobblestone, 64);
    inventory.SetSlot(3, vibecraft::BlockRegistry::kOakPlanks, 64);
    inventory.SetSlot(4, vibecraft::BlockRegistry::kSand, 64);
    inventory.SetSlot(5, vibecraft::BlockRegistry::kGlass, 64);
    inventory.SetSlot(6, vibecraft::BlockRegistry::kOakLog, 64);
    inventory.SetSlot(7, vibecraft::BlockRegistry::kGrass, 64);
    inventory.SetSlot(8, vibecraft::BlockRegistry::kOakLeaves, 64);

    // --- HUD: crosshair setup ---
    vibecraft::Shader hud_shader;
    if (!hud_shader.CompileFromFiles("assets/shaders/hud.vert",
                                      "assets/shaders/hud.frag")) {
        std::cerr << "HUD shader error: " << hud_shader.GetErrorLog() << "\n";
        return 1;
    }

    // Crosshair geometry: two lines forming a "+" at the origin.
    // We'll transform them to screen center via the projection uniform.
    unsigned int crosshair_vao = 0, crosshair_vbo = 0;
    {
        // We'll update the vertex data each frame based on screen size.
        glGenVertexArrays(1, &crosshair_vao);
        glGenBuffers(1, &crosshair_vbo);
        glBindVertexArray(crosshair_vao);
        glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo);
        // Reserve space for 4 vertices (2 lines x 2 endpoints, each vec2).
        glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), nullptr,
                     GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                              nullptr);
        glBindVertexArray(0);
    }

    // Game loop timing.
    vibecraft::GameLoop game_loop;
    auto last_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;
    float fps_timer = 0.0f;
    int current_fps = 0;

    // --- Chunk management helpers ---
    auto generate_and_mesh_chunk = [&](int cx, int cz) {
        world.LoadChunk(cx, cz);
        vibecraft::Chunk* chunk = world.GetChunk(cx, cz);
        if (chunk) {
            terrain_gen.GenerateChunk(*chunk);
            chunk->ClearDirty();
        }
    };

    auto mesh_chunk = [&](int cx, int cz) {
        const vibecraft::Chunk* chunk = world.GetChunk(cx, cz);
        if (!chunk) return;

        vibecraft::NeighborData neighbors;
        neighbors.pos_x = world.GetChunk(cx + 1, cz);
        neighbors.neg_x = world.GetChunk(cx - 1, cz);
        neighbors.pos_z = world.GetChunk(cx, cz + 1);
        neighbors.neg_z = world.GetChunk(cx, cz - 1);

        vibecraft::MeshData mesh =
            vibecraft::BuildGreedyMesh(*chunk, neighbors, registry);
        if (!mesh.Empty()) {
            chunk_renderer.UploadChunkMesh(cx, cz, mesh);
        }
    };

    // Generate initial chunks around spawn.
    int total_chunks = (2 * kRenderDistance + 1) * (2 * kRenderDistance + 1);
    int generated = 0;
    std::cout << "Generating " << total_chunks << " chunks..." << std::flush;
    for (int cx = -kRenderDistance; cx <= kRenderDistance; ++cx) {
        for (int cz = -kRenderDistance; cz <= kRenderDistance; ++cz) {
            generate_and_mesh_chunk(cx, cz);
            ++generated;
        }
        std::cout << " " << generated << std::flush;
    }
    std::cout << "\nMeshing..." << std::flush;
    int meshed = 0;
    for (int cx = -kRenderDistance; cx <= kRenderDistance; ++cx) {
        for (int cz = -kRenderDistance; cz <= kRenderDistance; ++cz) {
            mesh_chunk(cx, cz);
            ++meshed;
            std::cout << " " << meshed << std::flush;
        }
    }
    std::cout << " done!\nReady! Loaded " << world.ChunkCount() << " chunks.\n";

    // Block query for player physics.
    auto block_query = [&](int x, int y, int z) -> vibecraft::BlockId {
        return world.GetBlock(x, y, z);
    };

    bool wireframe = false;

    // --- Main loop ---
    while (window.IsOpen()) {
        // Timing.
        auto now = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        // Cap delta to avoid spiral of death.
        if (elapsed > 0.25f) elapsed = 0.25f;

        // FPS counter.
        frame_count++;
        fps_timer += elapsed;
        if (fps_timer >= 1.0f) {
            current_fps = frame_count;
            frame_count = 0;
            fps_timer -= 1.0f;
        }

        // Poll input.
        window.PollEvents();
        input.Update();

        // Escape to quit.
        if (input.IsKeyPressed(kKeyEscape)) {
            break;
        }

        // F3 toggle debug overlay.
        if (input.IsKeyPressed(kKeyF3)) {
            debug_overlay.Toggle();
        }

        // F4 toggle wireframe.
        if (input.IsKeyPressed(kKeyF4)) {
            wireframe = !wireframe;
            renderer.SetWireframe(wireframe);
        }

        // Mouse look.
        double mdx, mdy;
        input.GetMouseDelta(mdx, mdy);
        camera.SetYaw(camera.GetYaw() + static_cast<float>(mdx));
        camera.SetPitch(camera.GetPitch() - static_cast<float>(mdy));

        // Fixed timestep simulation.
        int ticks = game_loop.Accumulate(elapsed);
        for (int t = 0; t < ticks; ++t) {
            // Player movement input (apply velocity each tick).
            glm::vec3 move_dir(0.0f);
            glm::vec3 forward = camera.GetForward();
            glm::vec3 right = camera.GetRight();
            // Flatten forward for movement (no flying).
            forward.y = 0.0f;
            if (glm::length(forward) > 0.001f) forward = glm::normalize(forward);

            if (input.IsKeyHeld(kKeyW)) move_dir += forward;
            if (input.IsKeyHeld(kKeyS)) move_dir -= forward;
            if (input.IsKeyHeld(kKeyD)) move_dir += right;
            if (input.IsKeyHeld(kKeyA)) move_dir -= right;

            if (glm::length(move_dir) > 0.001f) {
                move_dir = glm::normalize(move_dir) * kMoveSpeed;
            }

            glm::vec3 vel = player.GetVelocity();
            vel.x = move_dir.x;
            vel.z = move_dir.z;
            player.SetVelocity(vel);

            if (input.IsKeyHeld(kKeySpace)) {
                player.Jump();
            }

            player.Update(block_query, registry);
            sky.Tick();
        }

        // Sync camera to player eye position.
        camera.SetPosition(player.GetEyePosition());
        camera.SetAspectRatio(window.GetAspectRatio());

        // --- Hotbar selection via scroll wheel ---
        {
            double scroll_dx, scroll_dy;
            input.GetScrollDelta(scroll_dx, scroll_dy);
            if (scroll_dy > 0.0) {
                inventory.PrevHotbarSlot();
            } else if (scroll_dy < 0.0) {
                inventory.NextHotbarSlot();
            }
        }

        // --- Block interaction: raycast + breaking/placing ---
        {
            auto block_set = [&](int x, int y, int z, vibecraft::BlockId id) {
                world.SetBlock(x, y, z, id);
            };

            vibecraft::RaycastResult ray_hit = vibecraft::CastRay(
                camera.GetPosition(), camera.GetForward(),
                vibecraft::kMaxRaycastDistance, block_query, registry);

            if (g_mouse_left_held && ray_hit.hit) {
                vibecraft::BlockId hit_block = world.GetBlock(
                    ray_hit.block_position.x,
                    ray_hit.block_position.y,
                    ray_hit.block_position.z);
                bool broken = block_interaction.UpdateBreaking(
                    ray_hit.block_position, hit_block, registry,
                    elapsed, block_set);
                if (broken) {
                    // Add the broken block to inventory.
                    inventory.AddItem(hit_block, 1);
                }
            } else {
                block_interaction.ResetBreaking();
            }

            if (g_mouse_right_pressed && ray_hit.hit) {
                vibecraft::ItemSlot held = inventory.GetHeldItem();
                if (!held.IsEmpty()) {
                    bool placed = vibecraft::BlockInteraction::PlaceBlock(
                        ray_hit.block_position, ray_hit.face_normal,
                        held.block_id, player.GetAABB(),
                        block_query, block_set, registry);
                    if (placed) {
                        // Decrement the count in the selected hotbar slot.
                        int sel = inventory.GetSelectedSlot();
                        vibecraft::ItemSlot slot = inventory.GetSlot(sel);
                        if (slot.count > 1) {
                            inventory.SetSlot(sel, slot.block_id,
                                              slot.count - 1);
                        } else {
                            inventory.SetSlot(sel,
                                              vibecraft::BlockRegistry::kAir, 0);
                        }
                    }
                }
            }
            g_mouse_right_pressed = false;  // Consume the edge-trigger.
        }

        // --- Chunk loading/unloading around player ---
        int player_cx = vibecraft::World::WorldToChunkCoord(
            static_cast<int>(std::floor(player.GetPosition().x)));
        int player_cz = vibecraft::World::WorldToChunkCoord(
            static_cast<int>(std::floor(player.GetPosition().z)));

        // Load new chunks.
        for (int cx = player_cx - kRenderDistance; cx <= player_cx + kRenderDistance; ++cx) {
            for (int cz = player_cz - kRenderDistance; cz <= player_cz + kRenderDistance; ++cz) {
                if (!world.HasChunk(cx, cz)) {
                    generate_and_mesh_chunk(cx, cz);
                    mesh_chunk(cx, cz);
                }
            }
        }

        // Remesh dirty chunks.
        for (int cx = player_cx - kRenderDistance; cx <= player_cx + kRenderDistance; ++cx) {
            for (int cz = player_cz - kRenderDistance; cz <= player_cz + kRenderDistance; ++cz) {
                vibecraft::Chunk* chunk = world.GetChunk(cx, cz);
                if (chunk && chunk->IsDirty()) {
                    mesh_chunk(cx, cz);
                    chunk->ClearDirty();
                }
            }
        }

        // --- Render ---
        glm::vec3 sky_color = sky.GetSkyColor();
        renderer.SetClearColor(sky_color.r, sky_color.g, sky_color.b);
        renderer.SetViewport(0, 0, window.GetWidth(), window.GetHeight());
        renderer.BeginFrame();

        // Set up shader uniforms.
        block_shader.Use();
        block_shader.SetFloat("u_atlas_tiles_per_row",
                               static_cast<float>(atlas.GetTilesPerRow()));
        block_shader.SetVec3("u_fog_color", sky_color);
        block_shader.SetFloat("u_fog_start",
                               static_cast<float>((kRenderDistance - 2) * 16));
        block_shader.SetFloat("u_fog_end",
                               static_cast<float>(kRenderDistance * 16));

        // Draw all chunks.
        chunk_renderer.DrawAll(block_shader, atlas,
                                camera.GetViewMatrix(),
                                camera.GetProjectionMatrix());

        // --- Draw crosshair ---
        {
            float sw = static_cast<float>(window.GetWidth());
            float sh = static_cast<float>(window.GetHeight());
            float cx = sw / 2.0f;
            float cy = sh / 2.0f;
            float half_size = 10.0f;

            float crosshair_verts[] = {
                cx - half_size, cy,   // horizontal line left
                cx + half_size, cy,   // horizontal line right
                cx, cy - half_size,   // vertical line bottom
                cx, cy + half_size,   // vertical line top
            };

            glDisable(GL_DEPTH_TEST);
            glLineWidth(2.0f);

            hud_shader.Use();
            glm::mat4 ortho = glm::ortho(0.0f, sw, 0.0f, sh);
            hud_shader.SetMat4("u_projection", ortho);
            hud_shader.SetVec3("u_color", glm::vec3(1.0f, 1.0f, 1.0f));
            // Set the vec4 u_color uniform manually via the shader's SetFloat
            // approach — but Shader only has SetVec3. We need the alpha too.
            // Use the vec3 overload and set alpha separately... Actually the
            // fragment shader uses vec4 u_color, but Shader has no SetVec4.
            // We can use SetVec3 for the color and rely on the uniform
            // default or use a workaround. Let's just treat the alpha as 1.0
            // by setting u_color as a vec4 via the program directly.
            {
                int loc = glGetUniformLocation(hud_shader.GetProgramId(),
                                               "u_color");
                if (loc >= 0) {
                    glUniform4f(loc, 1.0f, 1.0f, 1.0f, 1.0f);
                }
            }

            glBindVertexArray(crosshair_vao);
            glBindBuffer(GL_ARRAY_BUFFER, crosshair_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(crosshair_verts),
                            crosshair_verts);
            glDrawArrays(GL_LINES, 0, 4);
            glBindVertexArray(0);

            glEnable(GL_DEPTH_TEST);
        }

        renderer.EndFrame();

        // Debug overlay (print to console every second for now).
        if (debug_overlay.IsVisible() && fps_timer < elapsed) {
            debug_overlay.SetFPS(current_fps);
            glm::vec3 pos = player.GetPosition();
            debug_overlay.SetPosition(pos.x, pos.y, pos.z);
            vibecraft::BiomeType biome = terrain_gen.GetBiome(
                static_cast<int>(std::floor(pos.x)),
                static_cast<int>(std::floor(pos.z)));
            debug_overlay.SetBiomeName(BiomeNameString(biome));

            std::cout << "\r" << debug_overlay.GetFPSString()
                      << " | " << debug_overlay.GetCoordsString()
                      << " | " << debug_overlay.GetChunkString()
                      << " | " << debug_overlay.GetBiomeString()
                      << "     " << std::flush;
        }

        window.SwapBuffers();
    }

    // Cleanup.
    std::cout << "\nShutting down...\n";
    chunk_renderer.Clear();
    if (crosshair_vbo) glDeleteBuffers(1, &crosshair_vbo);
    if (crosshair_vao) glDeleteVertexArrays(1, &crosshair_vao);

    return 0;
}
