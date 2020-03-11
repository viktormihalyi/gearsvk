#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 red;

layout (binding = 0) uniform sampler2D sampl;
//layout (binding = 1) uniform sampler2D sampl2;

void main() {
    //outColor = vec4 (fragColor, 1.0);
    outColor = vec4 (1.f, 0.f, 0.f, 1.f);
    //outColor = vec4 (0.f, 0.f, 1.f, 1.f);
}