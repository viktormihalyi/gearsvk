#include "core/filter/SpatialFilter.h"
#include "core/SequenceRenderer.h"
#include <ctime>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdafx.h>

SpatialFilter::SpatialFilter ()
    : width_um (500), height_um (500), useFft (true), separable (false), horizontalSampleCount (16), verticalSampleCount (16), kernelGivenInFrequencyDomain (false), showFft (false), stimulusGivenInFrequencyDomain (false), uniqueId (0), fftSwizzleMask (0)
{
    kernelFuncSource =
        R"GLSLC0D3(
        	#version 450
			in vec2 pos;
			out vec4 outcolor;
			uniform vec2 texelSize_um;
			uniform vec2 patternSizeOnRetina;
			uniform bool kernelGivenInFrequencyDomain;
			void main() {
				vec4 kernelInTex = vec4(0, 0, 0, 0);
				for(int u=-4; u<=4; u++)
					for(int v=-4; v<=4; v++){
						vec2 px = pos + vec2(u, v) / 8.5 * texelSize_um;
						if(kernelGivenInFrequencyDomain)
						{
							px = mod(px + patternSizeOnRetina, patternSizeOnRetina) - patternSizeOnRetina * 0.5;
						}
						kernelInTex += kernel(px);
					}
				kernelInTex /= 81.0;
				outcolor = mix(vec4(plotDarkColor.rgb, 0), vec4(plotBrightColor, 1), (kernelInTex - vec4(plotMin))/vec4(plotMax-plotMin));
				int cq = (int(gl_FragCoord.x) % 2) ^ (int(gl_FragCoord.y) % 2);
				if(kernelGivenInFrequencyDomain && (cq == 0) )
					outcolor = -outcolor;
			}
		)GLSLC0D3";
    setShaderVariable ("plotMin", 0);
    setShaderVariable ("plotMax", 1);
    setShaderColor ("plotDarkColor", -2, 0, 0, 0);
    setShaderColor ("plotBrightColor", -2, 1, 1, 1);
    kernelProfileVertexSource =
        "	#version 450\n"
        "	uniform vec2 patternSizeOnRetina;		 \n"
        "	void main(void) {		 \n"
        "		float x = float(gl_VertexID)/256.0 - 1.0;			\n"
        "		gl_Position	= vec4(x, (kernel(vec2(x*patternSizeOnRetina.x*0.5, 0)).r - plotMin)/(plotMax-plotMin)*2.0-1, 0.5, 1.0);		\n"
        "	}																										\n";
    kernelProfileFragmentSource =
        "	#version 450\n"
        "	void main() {																							\n"
        "		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);															\n"
        "	}																										\n";
    spatialDomainConvolutionShaderSource =
        "	#version 450\n"
        "	#extension GL_ARB_texture_rectangle : enable            \n"
        "	precision highp float;																  \n"
        "	uniform sampler2D original;																\n"
        "	uniform sampler2D kernel;															  \n"
        "	uniform vec2 patternSizeOnRetina;																\n"
        "	uniform vec2 kernelSizeOnRetina;																\n"
        "	uniform vec2 step;																\n"
        "	uniform bool combine;																\n"
        "	in vec2 fTexCoord;																	\n"
        "	out vec4 outcolor;																	\n"
        "	void main() {																		 \n"
        //		"			outcolor = texture2D(kernel, fTexCoord) * vec4(1,1, 1, 1); 								\n"
        "		outcolor = vec4(0, 0, 0, 0);																		 \n"
        "		for(float i=-8; i<=8.01; i+=1.0)																		\n"
        "		{																		\n"
        "			vec2 sample = step * i;																		\n"
        "			if(combine)			\n"
        "				outcolor += 				\n"
        "					texture2D(original, fTexCoord + sample / patternSizeOnRetina).rgba *				\n"
        "					texture2D(kernel, vec2(0.5, 0.5) - sample / kernelSizeOnRetina).xxyy; 								\n"
        "			else			\n"
        "				outcolor += texture2D(original, fTexCoord + sample / patternSizeOnRetina).gbgb				\n"
        //		"				outcolor += vec4(1, 1, 1, 1)				\n"
        "					* texture2D(kernel, vec2(0.5, 0.5) - sample / kernelSizeOnRetina).xxyy; 								\n"
        "		}																		\n"
        "			if(combine)			\n"
        "				outcolor = vec4(-outcolor.rgrg  + outcolor.baba ) * 50;					\n"
        //		"				outcolor = (outcolor.ba ).rrgg;					\n"
        //		"				outcolor = (outcolor.rg).rrgg;					\n"
        "	}								\n";
}

void SpatialFilter::makeUnique ()
{
    static int uniqueCounter = 1;
    if (uniqueId == 0)
        uniqueId = uniqueCounter++;
}