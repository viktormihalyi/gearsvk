#version 450

layout (std140, binding = 0) uniform Time {
    float time;
} time;

layout (std140, binding = 3) uniform Camera {
    mat4 viewMatrix;
    mat4 VP;
    mat4 rayDirMatrix;
    vec3 position;
} camera;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 textureCoords;
layout (location = 1) out vec3 rayDirection;

void main ()
{
    gl_Position   =  camera.VP * vec4 (position + vec2 (0 / 100.f), 0.0, 1.0);
    textureCoords = uv;
    rayDirection  = (camera.rayDirMatrix * vec4 (position, 0, 1)).xyz;
}