#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_tex_coord;
layout (location = 2) in float a_tex_index;
layout (location = 3) in vec3 a_normal;
layout (location = 4) in float a_ao;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;

out vec2 v_tex_coord;
out float v_tex_index;
out vec3 v_normal;
out float v_ao;
out vec3 v_frag_pos;
out float v_dist;

void main() {
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    vec4 view_pos = u_view * world_pos;
    gl_Position = u_projection * view_pos;

    v_tex_coord = a_tex_coord;
    v_tex_index = a_tex_index;
    v_normal = mat3(u_model) * a_normal;
    v_ao = a_ao;
    v_frag_pos = world_pos.xyz;
    v_dist = length(view_pos.xyz);
}
