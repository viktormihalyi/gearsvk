#version 450

layout (binding = 0) uniform sampler2D rgb;

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 fTexCoord;

layout (location = 0) out vec4 outcolor;

void main()
{
	ivec3 urgb = ivec3 (texture (rgb, fTexCoord * vec2(1, -1) + vec2(0, 1)).rgb * 256.0);
	outcolor = vec4 (
		( ((urgb.r *66 + urgb.g * 129 + urgb.b * 25 + 128) >> 8) + 16 ) / 256.0
		, 0, 0, 0);
}

