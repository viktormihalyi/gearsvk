#version 450

layout (binding = 0) uniform sampler2D histogramBuffer;
layout (binding = 1) uniform sampler2D frameHistogramBuffer;

layout (binding = 0) uniform HalfHistogram {
	float newFrameWeight;
	float oldFramesWeight;
} h;

layout (location = 0) in vec2  fTexCoord;

layout (location = 0) out vec4 outColor;

void main() {
	vec4 data = texture(histogramBuffer, fTexCoord);
	vec4 framedata = texture(frameHistogramBuffer, fTexCoord);
	outColor = data * h.oldFramesWeight + framedata * h.newFrameWeight;
}

