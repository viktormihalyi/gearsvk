#version 450

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 fTexCoord;

void main ()
{
	
	vec4 outcolor = vec4(fig(pos, time), alphaMask(pos, time).x); 
	
	outcolor.rgb = temporalProcess(outcolor.rgb, fTexCoord);
	outcolor.rgb = toneMap(outcolor.rgb);
	
	if (swizzleForFft == 0x00020000) {
		outcolor = vec4(outcolor.r, 0, outcolor.g, 0);
	} else if (swizzleForFft == 0x00000406) {
		outcolor = vec4(outcolor.b, 0, outcolor.a, 0);
	}

	gl_FragData[0] = outcolor;
}