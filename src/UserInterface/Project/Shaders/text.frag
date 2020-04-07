#version 450

layout (binding = 0) uniform sampler2D glyphTexture;

layout (location = 0) out vec4 outColor;

void main() {
		outColor = 
			vec4(gl_Color.rgb, texture(glyphTexture, gl_TexCoord[0].xy).r);
		//gl_TexCoord[0]; //vec4(0.5, 0.7, 1.0, 1.0);
	}

