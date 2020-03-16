#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -1),
    vec2(1, 1),
    vec2(-1, 1)
);


void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}