#version 450

layout (binding = 0) uniform sampler2D polymaskVertices;

layout (binding = 1) uniform P {
	vec2 value;
} patternSizeOnRetina;

out vec2 pos;

void main(void) {
   gl_Position = texelFetch(polymaskVertices, ivec2(gl_VertexIndex, 0));
   pos = gl_Position * patternSizeOnRetina.value * 0.5;
}

