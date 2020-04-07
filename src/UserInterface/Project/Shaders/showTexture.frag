#version 150 compatibility
uniform sampler2D rgb;

in vec2 fTexCoord;
out vec4 outcolor;

void main() {
		outcolor = texture2D(rgb, fTexCoord );
	}

