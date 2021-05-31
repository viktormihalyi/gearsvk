#version 450

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D glyphTexture;

layout (location = 0) out vec4 outColor;

void main ()
{
	vec3 color = vec3 (1, 1, 1);
	outColor = vec4 (color.rgb, texture (glyphTexture, texCoord).r);
}

