#version 450

layout (binding = 0) uniform sampler2D inputBuffer;

layout (binding = 1) uniform Histogram {
	float histogramLevels;
	float domainMin;
	float domainMax;
	uvec2 sampleCount;
} h;

float I (vec2 coord){
	vec4 color = texture(inputBuffer, coord);
	return( dot(color.rgb, vec3(0.299, 0.587, 0.114)) );
}

void main(void){
	//vec2 resolution = textureSize(inputBuffer, 0);
	int px = gl_VertexIndex % int(h.sampleCount.x);
	int py = gl_VertexIndex / int(h.sampleCount.x);
	float luminance = I( vec2( px + (gl_VertexIndex*3631 & 0xf) * 0.0625, py + (gl_VertexIndex*3119 & 0xf) * 0.0625) / h.sampleCount );
//	float luminance = I( vec2( px , py ) / h.sampleCount );
	luminance = (luminance - h.domainMin) / (h.domainMax - h.domainMin);
	//gl_Position = vec4(2.0 * (luminance /*/ (1.0 + 1.0 / h.histogramLevels) + 0.5 / h.histogramLevels*/ - 0.5) , 0.5, 0.0, 1.0);
	gl_Position = vec4(2.0 * (luminance - 0.5) , 0.5, 0.0, 1.0);

	//gl_Position = vec4(luminance * 2.0 - 1.0, 0.5, 0.0, 1.0);
}

