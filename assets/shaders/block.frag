#version 330 core

in vec2 v_tex_coord;
in float v_tex_index;
in vec3 v_normal;
in float v_ao;
in vec3 v_frag_pos;
in float v_dist;

out vec4 frag_color;

uniform sampler2D u_texture_atlas;
uniform float u_atlas_tiles_per_row;  // e.g., 16
uniform float u_atlas_rows;           // number of tile rows in the atlas
uniform vec3 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;

// Directional sun light.
const vec3 light_dir = normalize(vec3(0.3, 1.0, 0.5));
const float ambient = 0.35;

void main() {
    // Compute atlas UV from tile index and per-face tex_coord.
    float tile = floor(v_tex_index + 0.5);
    float col = mod(tile, u_atlas_tiles_per_row);
    float row = floor(tile / u_atlas_tiles_per_row);
    float u_tile_size = 1.0 / u_atlas_tiles_per_row;
    float v_tile_size = 1.0 / u_atlas_rows;

    // Fractional part tiles across greedy-merged faces.
    vec2 within_tile = fract(v_tex_coord) * vec2(u_tile_size, v_tile_size);
    vec2 tile_origin = vec2(col * u_tile_size, row * v_tile_size);
    vec2 atlas_uv = tile_origin + within_tile;

    vec4 tex_color = texture(u_texture_atlas, atlas_uv);

    // Discard fully transparent fragments.
    if (tex_color.a < 0.1) discard;

    // Basic diffuse lighting.
    float diff = max(dot(normalize(v_normal), light_dir), 0.0);
    float light = ambient + (1.0 - ambient) * diff;

    // Apply ambient occlusion.
    light *= v_ao;

    vec3 color = tex_color.rgb * light;

    // Linear fog.
    float fog_factor = clamp((u_fog_end - v_dist) / (u_fog_end - u_fog_start), 0.0, 1.0);
    color = mix(u_fog_color, color, fog_factor);

    frag_color = vec4(color, tex_color.a);
}
