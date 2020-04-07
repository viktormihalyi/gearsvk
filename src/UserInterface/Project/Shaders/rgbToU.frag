#version 150 compatibility
uniform sampler2D rgb;

in vec2 fTexCoord;
out vec4 outcolor;

void main() {
		ivec3 urgb = ivec3(texture2D(rgb, fTexCoord * vec2(1, -1) + vec2(0, 1)).rgb * 256.0);
		outcolor = vec4(
				( (( urgb.r * -34 + urgb.g * -74 + urgb.b * 112 + 128) >> 8) + 128 ) / 256.0
				, 0, 0, 0);
	}

