#include "Pass.h"
#include "Sequence.h"
#include "Stimulus.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include "Utils/Utils.hpp"
#include "Utils/Assert.hpp"


Pass::Pass ()
    : name ("N/A")
    , brief ("<no description>")
    , stimulus (nullptr)
    , duration (1)
    , startingFrame (0)
    , videoFileName ("")
    , rasterizationMode (RasterizationMode::fullscreen)
    , mono (true)
    , transparent (false)
    , loop (true)
{
    setShaderFunction ("intensity", "vec3 intensity(float time){return vec3(0.0, 0.0, 0.0);}");

    timelineVertexShaderSource =
        R"GLSLC0D3(
        #version 450
		uniform float frameInterval;
		uniform int startFrame;
		uniform int stride;
		void main(void) {
//			vec3 intensityRgb =	vec3(sin((gl_VertexID/2 * stride + startFrame)*frameInterval), 0, 0);
			vec3 intensityRgb =	plottedIntensity((gl_VertexID/2 * stride + startFrame)*frameInterval);
			float maxIntensity = max(intensityRgb.r, max(intensityRgb.g, intensityRgb.b));
			gl_Position	= gl_ModelViewMatrix * vec4(float((gl_VertexID+1)/2  * stride + startFrame), maxIntensity, 0.5, 1.0);
		}
		)GLSLC0D3";
    timelineFragmentShaderSource =
        R"GLSLC0D3(
        #version 450
		void main() {								
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
		}											
		)GLSLC0D3";
}

Pass::~Pass ()
{
}


float Pass::getDuration_s () const
{
    return duration * stimulus->getSequence ()->getFrameInterval_s ();
}


std::shared_ptr<Stimulus> Pass::getStimulus () const
{
    return stimulus;
}

std::shared_ptr<Sequence> Pass::getSequence () const
{
    return stimulus->getSequence ();
}


void Pass::setVideo (std::string videoFileName)
{
    this->videoFileName = videoFileName;
}

void Pass::setStimulus (std::shared_ptr<Stimulus> stimulus)
{
    this->stimulus = stimulus;
}

void Pass::onSequenceComplete ()
{
    if (stimulus->doesToneMappingInStimulusGenerator) {
        setShaderFunction ("toneMap", R"GLSLC0D3(
layout (binding = 101) uniform sampler1D gamma;
layout (binding = 102) uniform toneMapping {
    float   toneRangeMin;
    float   toneRangeMax;
    float   toneRangeMean;
    float   toneRangeVar;
    int     gammaSampleCount;
    bool    doTone;
    bool    doGamma;
};

vec3 toneMap(vec3 color) 
{
	vec3 outcolor = color;
	if (doTone) {
		if (toneRangeVar >= 0) {
			outcolor.r = 1 - 1 / (1 + exp((outcolor.r - toneRangeMean)/toneRangeVar));
			outcolor.g = 1 - 1 / (1 + exp((outcolor.g - toneRangeMean)/toneRangeVar));
			outcolor.b = 1 - 1 / (1 + exp((outcolor.b - toneRangeMean)/toneRangeVar));
		} else {
			outcolor.r = (outcolor.r - toneRangeMin) / (toneRangeMax - toneRangeMin);
			outcolor.g = (outcolor.g - toneRangeMin) / (toneRangeMax - toneRangeMin);
			outcolor.b = (outcolor.b - toneRangeMin) / (toneRangeMax - toneRangeMin);
		}
	}
	
	if (doGamma) {
	    {
		    outcolor.r = clamp(outcolor.r, 0, 1);
		    float gammaIndex = outcolor.r * (gammaSampleCount-1) + 0.5;
		    outcolor.r = texture(gamma, gammaIndex/256.0).x;
		}
		{
		    outcolor.g = clamp(outcolor.g, 0, 1);
		    float gammaIndex = outcolor.g * (gammaSampleCount-1) + 0.5;
		    outcolor.g = texture(gamma, gammaIndex/256.0).x;
		}
		{
		    outcolor.b = clamp(outcolor.b, 0, 1);
		    float gammaIndex = outcolor.b * (gammaSampleCount-1) + 0.5;
		    outcolor.b = texture(gamma, gammaIndex/256.0).x;
		}
	}
	
   	return outcolor; 
})GLSLC0D3");

    } else {
        setShaderFunction ("toneMap", "vec3 toneMap(vec3 color){return color;}");
    }

    if (stimulus->temporalProcessingStateCount > 3) {
        if (stimulus->getSequence ()->isMonochrome () && stimulus->mono && mono) {
            setShaderFunction ("temporalProcess",
                               R"GLSLC0D3(
                #version 450
				uniform mat4x4 stateTransitionMatrix[4];
				uniform sampler2DArray temporalProcessingState;
					vec3 temporalProcess(vec3 inputColor, vec2 x){
						vec4 x0_3 = vec4(inputColor.r, texture(temporalProcessingState, vec3(x, 1)).gba);
						vec4 x4_7 = texture(temporalProcessingState, vec3(x, 2)).rgba;
						vec4 newState0_3 = x0_3 * stateTransitionMatrix[0] + x4_7 * stateTransitionMatrix[1];
						vec4 newState4_7 = x0_3 * stateTransitionMatrix[2] + x4_7 * stateTransitionMatrix[3];
						gl_FragData[1] = newState0_3;
						gl_FragData[2] = newState4_7;
						return vec3(newState0_3.x, newState0_3.x, newState0_3.x);
					}
				)GLSLC0D3");
        } else {
            setShaderFunction ("temporalProcess",
                               R"GLSLC0D3(
                #version 450
				uniform mat4x4 stateTransitionMatrix[4];
				uniform sampler2DArray temporalProcessingState;
					vec3 temporalProcess(vec3 inputColor, vec2 x){
						vec4 r0_3 = vec4(inputColor.r, texture(temporalProcessingState, vec3(x, 1)).gba);
						vec4 r4_7 = texture(temporalProcessingState, vec3(x, 2)).rgba;
						vec4 newr0_3 = r0_3 * stateTransitionMatrix[0] + r4_7 * stateTransitionMatrix[1];
						vec4 newr4_7 = r0_3 * stateTransitionMatrix[2] + r4_7 * stateTransitionMatrix[3];
						gl_FragData[1] = newr0_3;
						gl_FragData[2] = newr4_7;
						vec4 g0_3 = vec4(inputColor.g, texture(temporalProcessingState, vec3(x, 3)).gba);
						vec4 g4_7 = texture(temporalProcessingState, vec3(x, 4)).rgba;
						vec4 newg0_3 = g0_3 * stateTransitionMatrix[0] + g4_7 * stateTransitionMatrix[1];
						vec4 newg4_7 = g0_3 * stateTransitionMatrix[2] + g4_7 * stateTransitionMatrix[3];
						gl_FragData[3] = newg0_3;
						gl_FragData[4] = newg4_7;
						vec4 b0_3 = vec4(inputColor.b, texture(temporalProcessingState, vec3(x, 5)).gba);
						vec4 b4_7 = texture(temporalProcessingState, vec3(x, 6)).rgba;
						vec4 newb0_3 = b0_3 * stateTransitionMatrix[0] + b4_7 * stateTransitionMatrix[1];
						vec4 newb4_7 = b0_3 * stateTransitionMatrix[2] + b4_7 * stateTransitionMatrix[3];
						gl_FragData[5] = newb0_3;
						gl_FragData[6] = newb4_7;
						return vec3(newr0_3.x, newg0_3.x, newb0_3.x);
					}
				)GLSLC0D3");
        }
    } else if (stimulus->temporalProcessingStateCount > 0)
        if (stimulus->getSequence ()->isMonochrome () && stimulus->mono && mono) {
            setShaderFunction ("temporalProcess",
                               R"GLSLC0D3(
                #version 450
				uniform mat4x4 stateTransitionMatrix;
				uniform sampler2DArray temporalProcessingState;
					vec3 temporalProcess(vec3 inputColor, vec2 x){
						vec4 newState = vec4(inputColor.r, texture(temporalProcessingState, vec3(x, 1)).gba) * stateTransitionMatrix;
						gl_FragData[1] = newState;
						return vec3(newState.x, newState.x, newState.x);
					}
				)GLSLC0D3");
        } else {
            setShaderFunction ("temporalProcess",
                               R"GLSLC0D3(
                #version 450
				uniform mat4x4 stateTransitionMatrix;
				uniform sampler2DArray temporalProcessingState;
					vec3 temporalProcess(vec3 inputColor, vec2 x){
						vec4 newr = vec4(inputColor.r, texture(temporalProcessingState, vec3(x, 1)).gba) * stateTransitionMatrix;
						gl_FragData[1] = newr;
						vec4 newg = vec4(inputColor.g, texture(temporalProcessingState, vec3(x, 2)).gba) * stateTransitionMatrix;
						gl_FragData[2] = newg;
						vec4 newb = vec4(inputColor.b, texture(temporalProcessingState, vec3(x, 3)).gba) * stateTransitionMatrix;
						gl_FragData[3] = newb;
						return vec3(newr.x, newg.x, newb.x);
					}
				)GLSLC0D3");
        }
    else
        setShaderFunction ("temporalProcess", "vec3 temporalProcess(vec3 inputColor, vec2 x){return vec3(inputColor);}");
}

void Pass::registerTemporalFunction (std::string functionName, std::string displayName)
{
    temporalShaderFunctions[displayName] = functionName;
}

std::string Pass::getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode mode) const
{
    if (GVK_VERIFY (mode == Pass::RasterizationMode::fullscreen)) {
        const std::optional<std::string> quadVert = Utils::ReadTextFile (PROJECT_ROOT / "Project" / "Shaders" / "quad.vert");
        if (GVK_VERIFY (quadVert.has_value ())) {
            return *quadVert;
        } else {
            return "";
        }
    }

    if (mode == Pass::RasterizationMode::triangles) {
        std::string s (R"GLSLCODE(
			#version 450
			uniform vec2 patternSizeOnRetina;
			uniform sampler2D vertices;
			uniform float time;
			)GLSLCODE");
        for (auto& svar : shaderColors) {
            s += "uniform vec3 ";
            s += svar.first;
            s += ";\n";
        }
        for (auto& svar : shaderVectors) {
            s += "uniform vec2 ";
            s += svar.first;
            s += ";\n";
        }
        for (auto& svar : shaderVariables) {
            s += "uniform float ";
            s += svar.first;
            s += ";\n";
        }
        for (auto gsf : geomShaderFunctions)
            s += gsf.second;
        s += stimulusGeneratorGeometryShaderMotionTransformFunction;
        s += R"GLSLCODE(
			out vec2 pos;
			out vec2 fTexCoord;
			void main(void) {
				vec2 qpos =  texelFetch(vertices, ivec2(gl_VertexID, 0), 0).xy;
				vec2 posTime = polygonMotionTransform(time) * vec3(qpos, 1.0);
				gl_Position = vec4(posTime, 0.5, 1.0);
				pos = qpos * (patternSizeOnRetina / 2.0);
			}
			)GLSLCODE";
        return s;
    }
    if (mode == Pass::RasterizationMode::quads) {
        //std::string s(R"GLSLCODE(
        //#version 150
        //uniform vec2 patternSizeOnRetina;
        //#ifndef GEARS_RANDOMS_RESOURCES
        //#define GEARS_RANDOMS_RESOURCES
        //uniform usampler2D randoms;
        //uniform vec2 cellSize;
        //uniform ivec2 randomGridSize;
        //#endif
        //#ifndef GEARS_PARTICLE_SYSTEM_RESOURCES
        //#define GEARS_PARTICLE_SYSTEM_RESOURCES
        //uniform usampler2D particles;
        //#endif
        //uniform sampler2D quads;
        //uniform float time;
        //)GLSLCODE");
        std::string s (R"GLSLCODE(
				#version 450
				void main(void) {}
				)GLSLCODE");
        return s;
    } else
        return "";
}

std::string Pass::getStimulusGeneratorGeometryShaderSource (Pass::RasterizationMode mode) const
{
    if (mode == Pass::RasterizationMode::quads) {
        std::string s (R"GLSLCODE(
			#version 450
			uniform vec2 patternSizeOnRetina;
			#ifndef GEARS_RANDOMS_RESOURCES
			#define GEARS_RANDOMS_RESOURCES
			uniform usampler2D randoms;
			uniform vec2 cellSize;
			#endif
			#ifndef GEARS_PARTICLE_SYSTEM_RESOURCES
			#define GEARS_PARTICLE_SYSTEM_RESOURCES
			uniform usampler2D particles;
			#endif
			uniform sampler2D quads;
			uniform float time;
			)GLSLCODE");
        for (auto& svar : shaderColors) {
            s += "uniform vec3 ";
            s += svar.first;
            s += ";\n";
        }
        for (auto& svar : shaderVectors) {
            s += "uniform vec2 ";
            s += svar.first;
            s += ";\n";
        }
        for (auto& svar : shaderVariables) {
            s += "uniform float ";
            s += svar.first;
            s += ";\n";
        }
        for (auto gsf : geomShaderFunctions)
            s += gsf.second;
        s += stimulusGeneratorGeometryShaderMotionTransformFunction;
        s += R"GLSLCODE(
			out vec2 pos;
			out vec2 figmotid;
			void main(void) {
				vec2 qpos =  texelFetch(quads, ivec2(gl_PrimitiveIDIn * 3 + 0, 0), 0).xy;
				vec2 qsize = texelFetch(quads, ivec2(gl_PrimitiveIDIn * 3 + 1, 0), 0).xy;
				figmotid   = texelFetch(quads, ivec2(gl_PrimitiveIDIn * 3 + 2, 0), 0).xy;
				ivec2 randomGridSize = textureSize(randoms, 0);
				ivec2 iid = ivec2(gl_PrimitiveIDIn % randomGridSize.x, gl_PrimitiveIDIn / randomGridSize.x);
			
				pos = qpos + motionTransform(qsize, time, figmotid.y, iid);
				gl_Position = vec4(pos / patternSizeOnRetina * 2, 0, 1);
				pos = qsize;
				EmitVertex();
				pos = qpos + motionTransform(qsize * vec2(-1, 1), time, figmotid.y, iid);
				gl_Position = vec4(pos / patternSizeOnRetina * 2, 0, 1);
				pos = qsize * vec2(-1, 1);
				EmitVertex();
				pos = qpos + motionTransform(qsize * vec2(1, -1), time, figmotid.y, iid);
				gl_Position = vec4(pos / patternSizeOnRetina * 2, 0, 1);
				pos = qsize * vec2(1, -1);
				EmitVertex();
				pos = qpos + motionTransform(qsize * vec2(-1, -1), time, figmotid.y, iid);
				gl_Position = vec4(pos / patternSizeOnRetina * 2, 0, 1);
				pos = qsize * vec2(-1, -1);
				EmitVertex();
				EndPrimitive();
			}
			)GLSLCODE";
        return s;
    } else
        return "";
}


static std::string GenerateUniformBlock (const uint32_t binding, const std::string& uboName, const std::vector<std::pair<std::string, std::string>>& typeNamePairs)
{
    std::stringstream ss;
    ss << "layout (binding = " << binding << ") uniform " << uboName << " { " << std::endl;;
    for (auto& [type, name] : typeNamePairs) {
       ss << "    " << type << " " << name << ";" << std::endl;
    }
       ss << "};" << std::endl;
    return ss.str ();
}


/*
static void ReplaceAll (std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty ()) {
        return;
    }

    size_t start_pos = str.find ("{");
    while ((start_pos = str.find (from, start_pos)) != std::string::npos) {
        str.replace (start_pos, from.length (), to);
        start_pos += to.length (); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
*/


std::string Pass::getStimulusGeneratorShaderSource () const
{
    static const char* GLSL_VERSION = "#version 450";
    static const char* NEWLINE = "\n";

    std::string shaderSource;
    shaderSource.reserve (1024);

    shaderSource += GLSL_VERSION;
    shaderSource += NEWLINE;

    std::vector<std::pair<std::string, std::string>> commonBlock;

    commonBlock.emplace_back ("vec2", "patternSizeOnRetina");
    commonBlock.emplace_back ("int", "swizzleForFft");
    commonBlock.emplace_back ("int", "frame");
    commonBlock.emplace_back ("float", "time");

    for (auto& svar : shaderColors) {
        commonBlock.emplace_back ("vec3", svar.first);
    }
    for (auto& svar : shaderVectors) {
        commonBlock.emplace_back ("vec2", svar.first);
    }
    for (auto& svar : shaderVariables) {
        commonBlock.emplace_back ("float", svar.first);
    }

    shaderSource += GenerateUniformBlock (1, "commonUniformBlock", commonBlock);

    for (const std::string& sfunc : shaderFunctionOrder) {
        std::string funcSource = shaderFunctions.find (sfunc)->second;
/*
        const std::vector<std::string> uniforms = { "patternSizeOnRetina", "swizzleForFft", "frame" };
        const std::string before     = funcSource;
        for (const std::string& u : uniforms) {
            ReplaceAll (funcSource, u, u + "");
        }
        const std::string after = funcSource;
        GVK_ASSERT (after == before);
*/
        shaderSource += funcSource;
        shaderSource += "\n";
    }

    return shaderSource + stimulusGeneratorShaderSource;
}


void Pass::setStimulusGeneratorShaderSource (const std::string& src)
{
    stimulusGeneratorShaderSource = src;
}


std::string Pass::ToDebugString () const
{
    std::stringstream ss;
    
    ss << "Name: " << name << std::endl;
    ss << "Brief: " << brief << std::endl;

    switch (rasterizationMode) {
        case RasterizationMode::fullscreen: ss << "RasterizationMode::fullscreen"; break;
        case RasterizationMode::quads: ss << "RasterizationMode::quads"; break;
        case RasterizationMode::triangles: ss << "RasterizationMode::triangles"; break;
        default: GVK_BREAK ("unkown RasterizationMode"); ss << "unkown RasterizationMode"; break;
    }

    ss << std::endl;

    const std::string vert = getStimulusGeneratorVertexShaderSource (rasterizationMode);
    const std::string geom = getStimulusGeneratorGeometryShaderSource (rasterizationMode);
    const std::string frag = getStimulusGeneratorShaderSource ();

    if (!vert.empty ())
        ss << "Vertex shader:" << std::endl << vert << std::endl;

    if (!geom.empty ())
        ss << "Geometry shader:" << std::endl << geom << std::endl;

    if (!frag.empty ())
        ss << "Fragment shader:" << std::endl << frag << std::endl;

    return ss.str ();
}
