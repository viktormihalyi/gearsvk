#version 450

layout (binding = 0) uniform sampler2D polymaskVertices;

layout (binding = 1) uniform P {
	vec2 value;
} patternSizeOnRetina;

layout (location = 0) out vec2 pos;

void main(void) {
   gl_Position = vec4 (texelFetch (polymaskVertices, ivec2 (int (gl_VertexIndex), 0), 0));
   pos = gl_Position.xy * patternSizeOnRetina.value * 0.5;
}

