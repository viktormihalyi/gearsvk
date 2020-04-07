#version 150

uniform sampler2D inputBuffer;
uniform float histogramLevels;
uniform float domainMin;
uniform float domainMax;
uniform uvec2 sampleCount;

float I (vec2 coord){
	vec4 color = texture(inputBuffer, coord);
	return( dot(color.rgb, vec3(0.299, 0.587, 0.114)) );
}

void main(void){
	//vec2 resolution = textureSize(inputBuffer, 0);
	int px = gl_VertexID % int(sampleCount.x);
	int py = gl_VertexID / int(sampleCount.x);
	float luminance = I( vec2( px + (gl_VertexID*3631 & 0xf) * 0.0625, py + (gl_VertexID*3119 & 0xf) * 0.0625) / sampleCount );
//	float luminance = I( vec2( px , py ) / sampleCount );
	luminance = (luminance - domainMin) / (domainMax - domainMin);
	//gl_Position = vec4(2.0 * (luminance /*/ (1.0 + 1.0 / histogramLevels) + 0.5 / histogramLevels*/ - 0.5) , 0.5, 0.0, 1.0);
	gl_Position = vec4(2.0 * (luminance - 0.5) , 0.5, 0.0, 1.0);

	//gl_Position = vec4(luminance * 2.0 - 1.0, 0.5, 0.0, 1.0);
}

