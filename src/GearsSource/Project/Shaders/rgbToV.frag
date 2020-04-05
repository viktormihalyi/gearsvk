#version 450

layout (binding = 0) uniform sampler2D rgb;

layout (location = 0) in vec2 fTexCoord;
layout (location = 0) out vec4 outcolor;

void main() {
		ivec3 urgb = ivec3(texture(rgb, fTexCoord * vec2(1, -1) + vec2(0, 1)).rgb * 256.0);
		outcolor = vec4(
				( (( urgb.r* 112 + urgb.g * -94 + urgb.b * -18 + 128) >> 8) + 128 ) / 256.0
				, 0, 0, 0);
	}

