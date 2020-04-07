#version 150

uniform sampler2D histogramBuffer;
uniform sampler2D frameHistogramBuffer;
uniform float newFrameWeight;
uniform float oldFramesWeight;

in vec2  fTexCoord;

out vec4 outColor;

void main() {
	vec4 data = texture(histogramBuffer, fTexCoord);
	vec4 framedata = texture(frameHistogramBuffer, fTexCoord);
	outColor = data * oldFramesWeight + framedata * newFrameWeight;
}

