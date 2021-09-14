#include "Stimulus.h"

#include "Pass.h"
#include "Sequence.h"
#include "SpatialFilter.h"
#include "Utils/Assert.hpp"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <limits>
#include <sstream>
#include <cstring>


Stimulus::Stimulus ()
    : name ("N/A")
    , sequence (nullptr)
    , brief ("<no description>")
    , duration (1)
    , startingFrame (0)
    , spatialFilter (nullptr)
    , particleGridWidth (0)
    , particleGridHeight (0)
    , gammaSamplesCount (2)
    , toneRangeMin (0.f)
    , toneRangeMax (1.f)
    , toneRangeMean (0.5f)
    , toneRangeVar (-0.25f)
    , toneMappingMode (ToneMappingMode::LINEAR)
    , usesForwardRendering (false)
    , fullScreenTemporalFiltering (false)
    , mono (true)
    , doesToneMappingInStimulusGenerator (true)
    , requiresClearing (false)
    , clearColor (0.5f, 0.5f, 0.5f)
    , doesDynamicToneMapping (false)
    , computesFullAverageForHistogram (true)
    , stretchFactor (1.f)
    , meanOffset (0.f)
    , histogramMeasurementImpedance (0.95f)
    , rngCompute_workGroupSizeX (0)
    , rngCompute_workGroupSizeY (0)
    , rngCompute_seed (0)
    , rngCompute_multiLayer (false)
{
    measuredToneRangeMin = std::numeric_limits<float>::quiet_NaN ();
    measuredToneRangeMax = std::numeric_limits<float>::quiet_NaN ();
    measuredMean         = std::numeric_limits<float>::quiet_NaN ();
    measuredVariance     = std::numeric_limits<float>::quiet_NaN ();

    gamma[0] = 0.0f;
    gamma[1] = 1.0f;

    for (unsigned int i = 0; i < 63; i++)
        temporalWeights[i] = 0.0f;
    temporalWeights[63]          = 1.0f;
    temporalMemoryLength         = 0;
    temporalProcessingStateCount = 0;
    temporalWeightMax            = 1.0f;
    temporalWeightMin            = 0.0f;

    temporalFilterFuncSource =
        "float temporalWeight(int i) { if(i==0) return 1.0; else return 0.0; } \n";

    linearDynamicToneShaderSource    = R"GLSLC0D3(
            #version 450
			uniform sampler2D stimulusImage;
			uniform float toneRangeMin;
			uniform float toneRangeMax;
			uniform int gammaSampleCount;
			in vec2 fTexCoord;																						
			out vec4 outcolor;																						
			void main() {																							
				outcolor = texture(stimulusImage, fTexCoord);	
				outcolor.r = (outcolor.r - toneRangeMin) / (toneRangeMax - toneRangeMin);				
				outcolor.g = (outcolor.g - toneRangeMin) / (toneRangeMax - toneRangeMin);				
				outcolor.b = (outcolor.b - toneRangeMin) / (toneRangeMax - toneRangeMin);				
				outcolor.w = (outcolor.w - toneRangeMin) / (toneRangeMax - toneRangeMin);				
																													
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
			)GLSLC0D3";
    erfDynamicToneShaderSource       = R"GLSLC0D3(
            #version 450
			uniform sampler2D stimulusImage;
			uniform float toneRangeMean;
			uniform float toneRangeVar;
			uniform int gammaSampleCount;
			in vec2 fTexCoord;																						
			out vec4 outcolor;																						
			void main() {																							
				outcolor = texture(stimulusImage, fTexCoord);	
				outcolor.r = 1 - 1 / (1 + exp((outcolor.r - toneRangeMean)/toneRangeVar));					
				outcolor.g = 1 - 1 / (1 + exp((outcolor.g - toneRangeMean)/toneRangeVar));					
				outcolor.b = 1 - 1 / (1 + exp((outcolor.b - toneRangeMean)/toneRangeVar));					
				outcolor.w = 1 - 1 / (1 + exp((outcolor.w - toneRangeMean)/toneRangeVar));					
																													
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
			)GLSLC0D3";
    equalizedDynamicToneShaderSource = R"GLSLC0D3(
            #version 450
			uniform sampler2D stimulusImage;
			uniform sampler2D histogram;
			uniform vec2 histogramLimits;
			uniform float toneRangeMin;
			uniform float toneRangeMax;
			uniform int gammaSampleCount;
			in vec2 fTexCoord;																						
			out vec4 outcolor;																						
			void main() {
				vec4 origcolor = 	texture(stimulusImage, fTexCoord);																						
				float lum = dot(origcolor.rgb, vec3(0.299, 0.587, 0.114));
				float pb = dot(origcolor.rgb, vec3(-0.168736, -0.331264, 0.5));
				float pr = dot(origcolor.rgb, vec3(0.5, -0.418688, -0.081312));
				lum -= histogramLimits.x;
				lum *=  histogramLimits.y;
				lum = texture(histogram, vec2(lum, 0.5)).x / texture(histogram, vec2(1.0 - 0.5/256.0, 0.5)).x;
				outcolor.r = lum  + 1.402 * pr;
				outcolor.g = lum  - 0.344136 * pb - 0.714136 * pr;
				outcolor.b = lum + 1.772 * pb;
				outcolor.w = origcolor.w;
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
			)GLSLC0D3";

    temporalFilterShaderSource =
        R"GLSLC0D3(
            #version 450
			uniform sampler2DArray stimulusQueue;																	
			uniform int currentSlice;																				
			uniform int memoryLength;																				
			uniform int queueLength;																				
			uniform float toneRangeMin;																			
			uniform float toneRangeMax;																			
			uniform float toneRangeMean;																			
			uniform float toneRangeVar;																			
			uniform int gammaSampleCount;																			
			uniform bool doGamma;
			uniform bool doTone;
			in vec2 fTexCoord;																						
			out vec4 outcolor;																						
			void main() {																							
				vec4 a = vec4(0, 0, 0, 0);																			
				for(int i=0; i<memoryLength; i++)	{
					float w = temporalWeight(i);
					a.xyz += texture(stimulusQueue, vec3(fTexCoord, (currentSlice+queueLength-i)%queueLength)).xyz * w;	
				}
				outcolor = a;																						
				if(doTone){
					if(toneRangeVar >= 0)																			
					{																									
						outcolor.r = 1 - 1 / (1 + exp((outcolor.r - toneRangeMean)/toneRangeVar));					
						outcolor.g = 1 - 1 / (1 + exp((outcolor.g - toneRangeMean)/toneRangeVar));					
						outcolor.b = 1 - 1 / (1 + exp((outcolor.b - toneRangeMean)/toneRangeVar));					
						outcolor.w = 1 - 1 / (1 + exp((outcolor.w - toneRangeMean)/toneRangeVar));					
					}																									
					else																								
					{																									
						outcolor.r = (outcolor.r - toneRangeMin) / (toneRangeMax - toneRangeMin);				
						outcolor.g = (outcolor.g - toneRangeMin) / (toneRangeMax - toneRangeMin);				
						outcolor.b = (outcolor.b - toneRangeMin) / (toneRangeMax - toneRangeMin);				
						outcolor.w = (outcolor.w - toneRangeMin) / (toneRangeMax - toneRangeMin);				
					}																									
				}

				if(doGamma)
				{																									
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
			}																										
			)GLSLC0D3";

    temporalFilterPlotVertexShaderSource =
        "   #version 450\n"
        "	void main(void) {		 \n"
        "		float value = temporalWeight(gl_VertexID/2);											\n"
        "		gl_Position	= gl_ModelViewMatrix * vec4(float((gl_VertexID+1)/2), value, 0.5, 1.0);				\n"
        "	}																										\n";
    temporalFilterPlotFragmentShaderSource =
        "   #version 450\n"
        "	void main() {																							\n"
        "		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);															\n"
        "	}																										\n";

    spikeVertexShaderSource =
        "   #version 450\n"
        "	uniform float frameInterval;	\n"
        "	uniform int stride;	\n"
        "	uniform int startOffset;	\n"
        "	void main(void) {		 \n"
        "		gl_Position	= gl_ModelViewMatrix * vec4(float(gl_VertexID/3 * stride + startOffset), ((gl_VertexID+2)%3!=0)?0:1, 0.5, 1.0);   \n"
        "	}																										\n";

    spikeFragmentShaderSource =
        "   #version 450\n"
        "	void main() {																							\n"
        "		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);															\n"
        "	}																										\n";
}

Stimulus::~Stimulus ()
{
    interactives.clear ();
}

float Stimulus::getSpatialPlotMin ()
{
    if (spatialFilter)
        return spatialFilter->minimum;
    return 0;
}


float Stimulus::getSpatialPlotMax ()
{
    if (spatialFilter)
        return spatialFilter->maximum;
    return 1;
}


float Stimulus::getSpatialPlotWidth ()
{
    if (spatialFilter)
        return spatialFilter->width_um;
    return 200.0;
}


float Stimulus::getSpatialPlotHeight ()
{
    if (spatialFilter)
        return spatialFilter->height_um;
    return 200.0;
}


void Stimulus::raiseSignalOnTick (uint32_t iTick, std::string channel)
{
    SignalEvent e;
    e.clear   = false;
    e.channel = channel;
    tickSignals.emplace (iTick, e);
    stimulusChannels.insert (channel);
}


void Stimulus::clearSignalOnTick (uint32_t iTick, std::string channel)
{
    SignalEvent e;
    e.clear   = true;
    e.channel = channel;
    tickSignals.emplace (iTick, e);
    stimulusChannels.insert (channel);
}


void Stimulus::overrideTickSignals ()
{
    tickSignals.clear ();
}


float Stimulus::getDuration_s () const
{
    return duration * sequence->getFrameInterval_s ();
}


void Stimulus::setDuration (unsigned int duration)
{
    this->duration = sequence->useHighFreqRender ? roundForHighFrequency (duration) : duration;
}


unsigned int Stimulus::roundForHighFrequency (unsigned int duration)
{
    return (duration % 3 == 0) ? duration : ((duration % 3 == 1) ? duration - 1 : duration + 1);
}


void Stimulus::setMeasuredDynamics (float  measuredToneRangeMin,
                                    float  measuredToneRangeMax,
                                    float  measuredMean,
                                    float  measuredVariance,
                                    float* histi,
                                    uint32_t   histogramResolution) const
{
    const_cast<Stimulus*> (this)->measuredToneRangeMin = measuredToneRangeMin;
    const_cast<Stimulus*> (this)->measuredToneRangeMax = measuredToneRangeMax;
    const_cast<Stimulus*> (this)->measuredMean         = measuredMean;
    const_cast<Stimulus*> (this)->measuredVariance     = measuredVariance;
    const_cast<Stimulus*> (this)->measuredHistogram.clear ();
    for (unsigned int i = 0; i < histogramResolution * 4; i++)
        const_cast<Stimulus*> (this)->measuredHistogram.push_back (histi[i]);
}

void Stimulus::setToneMappingLinear (float min, float max, bool dynamic) const
{
    const_cast<Stimulus*> (this)->toneMappingMode        = Stimulus::ToneMappingMode::LINEAR;
    const_cast<Stimulus*> (this)->toneRangeMin           = min;
    const_cast<Stimulus*> (this)->toneRangeMax           = max;
    const_cast<Stimulus*> (this)->doesDynamicToneMapping = dynamic;
}

void Stimulus::setToneMappingErf (float mean, float var, bool dynamic) const
{
    const_cast<Stimulus*> (this)->toneMappingMode        = Stimulus::ToneMappingMode::ERF;
    const_cast<Stimulus*> (this)->toneRangeMean          = mean;
    const_cast<Stimulus*> (this)->toneRangeVar           = var;
    const_cast<Stimulus*> (this)->doesDynamicToneMapping = dynamic;
}

void Stimulus::setToneMappingEqualized (bool dynamic) const
{
    const_cast<Stimulus*> (this)->toneMappingMode        = Stimulus::ToneMappingMode::EQUALIZED;
    const_cast<Stimulus*> (this)->doesDynamicToneMapping = dynamic;
}

const std::vector<std::shared_ptr<Pass>>& Stimulus::getPasses () const { return passes; }


void Stimulus::addPass (std::shared_ptr<Pass> pass)
{
    pass->setStimulus (shared_from_this ());
    OnPassAdded (pass);
    mono = mono && pass->mono;
    passes.push_back (pass);
}

void Stimulus::finishLtiSettings ()
{
    GVK_BREAK ();
#if 0
    if (temporalProcessingStateCount == 3) {
        temporalMemoryLength = 64;
        Gears::Math::float4 f (1, 0, 0, 0);

        temporalWeightMax = -FLT_MAX;
        temporalWeightMin = FLT_MAX;
        for (int i = 0; i < 64; i++) {
            f                  = temporalProcessingStateTransitionMatrix[0] * f;
            temporalWeights[i] = f.x;
            f.x                = 0;

            temporalWeightMin = std::min (temporalWeightMin, temporalWeights[i]);
            temporalWeightMax = std::max (temporalWeightMax, temporalWeights[i]);
        }
    } else {
        temporalMemoryLength = 64;
        Gears::Math::float4 f0 (1, 0, 0, 0);
        Gears::Math::float4 f4 (0, 0, 0, 0);

        temporalWeightMax = -FLT_MAX;
        temporalWeightMin = FLT_MAX;
        for (int i = 0; i < 64; i++) {
            Gears::Math::float4 nf0 = temporalProcessingStateTransitionMatrix[0] * f0 + temporalProcessingStateTransitionMatrix[1] * f4;
            Gears::Math::float4 nf4 = temporalProcessingStateTransitionMatrix[2] * f0 + temporalProcessingStateTransitionMatrix[3] * f4;
            f0                      = nf0;
            f4                      = nf4;
            temporalWeights[i]      = f0.x;
            f0.x                    = 0;

            temporalWeightMin = std::min (temporalWeightMin, temporalWeights[i]);
            temporalWeightMax = std::max (temporalWeightMax, temporalWeights[i]);
        }
    }
#endif
}


void Stimulus::onSequenceComplete ()
{
    for (auto p : passes)
        p->onSequenceComplete ();
}

std::string Stimulus::getDynamicToneShaderSource () const
{
    std::string s ("#version 450\n");
    if (toneMappingMode == ToneMappingMode::LINEAR)
        return s +
               "	uniform sampler1D gamma;																	\n" + linearDynamicToneShaderSource;
    else if (toneMappingMode == ToneMappingMode::ERF)
        return s +
               "	uniform sampler1D gamma;																	\n" + erfDynamicToneShaderSource;
    else //if(toneMappingMode == ToneMappingMode::EQUALIZED)
        return s +
               "	uniform sampler1D gamma;																	\n" + equalizedDynamicToneShaderSource;
}


void Stimulus::setSequence (std::shared_ptr<Sequence> sequence)
{
    this->sequence = sequence;
}


unsigned int Stimulus::setStartingFrame (unsigned int offset)
{
    this->startingFrame = offset;
    return duration;
}


uint32_t Stimulus::getDuration () const
{
    return duration;
}


std::string Stimulus::getParticleShaderSource () const
{
    std::string s ("#version 450\n");
    s += "uniform vec2 patternSizeOnRetina;\n";
    s += "uniform int frame;\n";
    s += "uniform float time;\n";

    return s + particleShaderSource;
}

std::string Stimulus::getTemporalFilterShaderSource () const
{
    std::string s ("#version 450\n");
    return s + "	uniform sampler1D gamma;	\n" + temporalFilterFuncSource + temporalFilterShaderSource;
}


std::string Stimulus::getTemporalFilterPlotVertexShaderSource () const
{
    std::string s ("#version 450\n");
    return s + "	uniform sampler1D gamma;	\n" + temporalFilterFuncSource + temporalFilterPlotVertexShaderSource;
}


std::string Stimulus::getTemporalFilterPlotFragmentShaderSource () const
{
    std::string s ("#version 450\n");
    return s + temporalFilterPlotFragmentShaderSource;
}


void Stimulus::setSpatialFilter (std::shared_ptr<SpatialFilter> spatialFilter)
{
    this->spatialFilter                      = spatialFilter;
    this->fullScreenTemporalFiltering        = true;
    this->doesToneMappingInStimulusGenerator = false;
    if (this->temporalMemoryLength == 0 && this->temporalProcessingStateCount == 0) {
        this->temporalMemoryLength = 1;
        this->temporalWeights[63]  = 1;
        for (int i = 0; i < 63; i++)
            temporalWeights[i] = 0;
        temporalWeightMin = 1;
        temporalWeightMax = 1;
    }
}


std::shared_ptr<const SpatialFilter> Stimulus::getSpatialFilter () const
{
    return spatialFilter;
}


uint32_t Stimulus::getStartingFrame () const
{
    return startingFrame;
}


bool Stimulus::hasSpatialFiltering () const
{
    return spatialFilter != nullptr;
}


bool Stimulus::hasTemporalFiltering () const
{
    return temporalMemoryLength > 1 || this->temporalProcessingStateCount > 0;
}


std::shared_ptr<Sequence> Stimulus::getSequence ()
{
    return sequence;
}


bool Stimulus::usesChannel (std::string channel)
{
    return stimulusChannels.count (channel) == 1;
}


uint32_t Stimulus::getChannelCount ()
{
    return stimulusChannels.size ();
}


const Stimulus::SignalMap& Stimulus::getSignals () const
{
    return tickSignals;
}


void Stimulus::enableColorMode ()
{
    mono = false;
}


void Stimulus::setClearColor (float all, float r, float g, float b)
{
    if (all >= -1)
        clearColor = glm::vec3 (all, all, all);
    else
        clearColor = glm::vec3 (r, g, b);
}


void Stimulus::addTag (std::string tag)
{
    interactives.insert (tag);
}


const std::set<std::string>& Stimulus::getTags () const
{
    return interactives;
}


bool Stimulus::doesErfToneMapping () const
{
    return toneMappingMode == ToneMappingMode::ERF;
}


bool Stimulus::IsEquivalent (const Stimulus& other) const
{
    if (passes.size () != other.passes.size ()) {
        return false;
    }

    for (size_t i = 0; i < passes.size (); ++i) {
        const Pass& leftPass  = *passes[i];
        const Pass& rightPass = *other.passes[i];

        if (leftPass.rasterizationMode != rightPass.rasterizationMode) {
            return false;
        }

        const std::string leftVert = leftPass.getStimulusGeneratorVertexShaderSource (leftPass.rasterizationMode);
        const std::string leftGeom = leftPass.getStimulusGeneratorGeometryShaderSource (leftPass.rasterizationMode);
        const std::string leftFrag = leftPass.getStimulusGeneratorShaderSource ();

        const std::string rightVert = rightPass.getStimulusGeneratorVertexShaderSource (rightPass.rasterizationMode);
        const std::string rightGeom = rightPass.getStimulusGeneratorGeometryShaderSource (rightPass.rasterizationMode);
        const std::string rightFrag = rightPass.getStimulusGeneratorShaderSource ();

        if (leftVert != rightVert) {
            return false;
        }
        if (leftGeom != rightGeom) {
            return false;
        }
        if (leftFrag != rightFrag) {
            return false;
        }

        if (leftPass.shaderColors != rightPass.shaderColors) {
            return false;
        }
        if (leftPass.shaderVariables != rightPass.shaderVariables) {
            return false;
        }
        if (leftPass.shaderVectors != rightPass.shaderVectors) {
            return false;
        }
    }

    return requiresClearing == other.requiresClearing &&
           clearColor == other.clearColor &&
           usesForwardRendering == other.usesForwardRendering &&

           toneMappingMode == other.toneMappingMode &&
           toneRangeMin == other.toneRangeMin &&
           toneRangeMax == other.toneRangeMax &&
           toneRangeMean == other.toneRangeMean &&
           toneRangeVar == other.toneRangeVar &&

           rngCompute_shaderSource == other.rngCompute_shaderSource &&
           rngCompute_workGroupSizeX == other.rngCompute_workGroupSizeX &&
           rngCompute_workGroupSizeY == other.rngCompute_workGroupSizeY &&
           rngCompute_seed == other.rngCompute_seed &&
           rngCompute_multiLayer == other.rngCompute_multiLayer && 

           mono == other.mono &&
           sequence->fieldWidth_um == other.sequence->fieldWidth_um &&
           sequence->fieldHeight_um == other.sequence->fieldHeight_um &&
           doesDynamicToneMapping == other.doesDynamicToneMapping &&
           gammaSamplesCount == other.gammaSamplesCount &&
           memcmp (gamma, other.gamma, gammaSamplesCount) == 0 &&
           memcmp (temporalWeights, other.temporalWeights, 64) == 0;
}
