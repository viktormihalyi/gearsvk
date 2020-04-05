#version 450

layout (binding = 0) uniform sampler2D rgb;

layout (location = 0) in vec2 fTexCoord;
layout (location = 0) out vec4 outcolor;

void main() {
		outcolor = texture (rgb, fTexCoord );
	}

