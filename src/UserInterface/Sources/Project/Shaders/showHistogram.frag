#version 450

layout (binding = 0) uniform sampler2D histogramBuffer;

layout (binding = 1) uniform H {
	float domainMin;
	float domainMax;
} h;

layout (location = 0) in vec2  fTexCoord;

layout (location = 0) out vec4 outColor;


void main() {
//			outColor = vec4(0.5, 0.5, 0.0, 1.0);
//		else
//			outColor = vec4(1.0, 0.0, 0.0, 1.0);
//	} else {
//		if(fTexCoord.x + 0.001 > (0-domainMin) / (domainMax - domainMin) && fTexCoord.x - 0.001 < (1-domainMin) / (domainMax - domainMin))
//			outColor = vec4(0.0, 0.3, 0.0, 1.0);
//		else
//			outColor = vec4(0.0, 0.0, 0.0, 1.0);
//	}

//print mode
	float fy = fTexCoord.y * 1.2 - 0.2;
	float fx = fTexCoord.x * 1.1 - 0.05;
	float data = texture(histogramBuffer, vec2(fx, 0.5)).x;
	data -= texture(histogramBuffer, vec2(fx - 1.0/256.0, 0.5)).x;
	float topmax = texture(histogramBuffer, vec2(1.0 - 0.5/256.0, 0.5)).x;
	if(fx < 0 || fx > 1)
		outColor = vec4(0.5, 0.5, 0.5, 1.0);	// not measured
	else if(fy < 0)
		outColor = vec4(1.0, 1.0, 1.0, 1.0);	// background
	else if(data / topmax > fy / 20.0){
		if(fx + 0.001 >= (0-h.domainMin) / (h.domainMax - h.domainMin) && fx  - 0.001 <= (1-h.domainMin) / (h.domainMax - h.domainMin))
			outColor = vec4(0.0, 0.0, 0.0, 1.0);	// histo bars
		else
			outColor = vec4(1.0, 0.0, 0.0, 1.0);	// outlying bars
	} else {
		if(fx + 0.001 > (0-h.domainMin) / (h.domainMax - h.domainMin) && fx - 0.001 < (1-h.domainMin) / (h.domainMax - h.domainMin))
			outColor = vec4(0.9, 0.9, 0.9, 1.0);	// background between 0 and 1
		else
			outColor = vec4(1.0, 1.0, 1.0, 1.0);	// background
	}
}

