#version 330 core

in vec2 v_tex_coord;
in float v_tex_index;
in vec3 v_normal;
in float v_ao;
in vec3 v_frag_pos;

out vec4 frag_color;

uniform sampler2D u_texture_atlas;

// Simple directional light for basic shading.
const vec3 light_dir = normalize(vec3(0.3, 1.0, 0.5));
const float ambient = 0.4;

void main() {
    // Basic diffuse lighting.
    float diff = max(dot(normalize(v_normal), light_dir), 0.0);
    float light = ambient + (1.0 - ambient) * diff;

    // Apply ambient occlusion.
    light *= v_ao;

    // Sample texture (placeholder: solid white until atlas is set up).
    vec4 tex_color = vec4(1.0);
    // tex_color = texture(u_texture_atlas, v_tex_coord);

    frag_color = vec4(tex_color.rgb * light, tex_color.a);
}
