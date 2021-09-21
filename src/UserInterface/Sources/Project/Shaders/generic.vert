#version 450

layout (binding = 2) uniform PatternSizeOnRetina {
	vec2 patternSizeOnRetina;
};

layout (location = 0) out vec2 pos;
layout (location = 1) out vec2 fTexCoord;

vec2 uvs[6] = vec2[] (
    vec2 (0.f, 0.f),
    vec2 (0.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (1.f, 1.f),
    vec2 (0.f, 0.f),
    vec2 (1.f, 0.f)
);

vec2 positions[6] = vec2[] (
    vec2 (-1.f, -1.f),
    vec2 (-1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (+1.f, +1.f),
    vec2 (-1.f, -1.f),
    vec2 (+1.f, -1.f)
);


void main() {
    gl_Position = vec4 (positions[gl_VertexIndex], 0.0, 1.0);
    fTexCoord = uvs[gl_VertexIndex];
    pos = (fTexCoord - vec2 (0.5, 0.5)) * patternSizeOnRetina;
}
