#version 330 core

in vec2 v_texcoord;

out vec4 frag_color;

uniform sampler2D u_texture_atlas;
uniform float u_tile_index;
uniform float u_atlas_tiles_per_row;
uniform float u_atlas_rows;
uniform vec4 u_color;  // tint / modulate color

void main() {
    // Compute atlas UV from tile index and interpolated texcoord.
    float tile = floor(u_tile_index + 0.5);
    float col = mod(tile, u_atlas_tiles_per_row);
    float row = floor(tile / u_atlas_tiles_per_row);
    float u_tile_size = 1.0 / u_atlas_tiles_per_row;
    float v_tile_size = 1.0 / u_atlas_rows;

    vec2 tile_origin = vec2(col * u_tile_size, row * v_tile_size);
    vec2 atlas_uv = tile_origin + v_texcoord * vec2(u_tile_size, v_tile_size);

    vec4 tex_color = texture(u_texture_atlas, atlas_uv);

    // Discard fully transparent fragments.
    if (tex_color.a < 0.1) discard;

    frag_color = tex_color * u_color;
}
