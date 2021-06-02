#pragma once

#include "stdafx.h"
#include "SequenceAPI.hpp"

#include <algorithm>
#include <glm/glm.hpp>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class Sequence;
class Stimulus;

//! A structure that specifies a shape in a stimulus.
class SEQUENCE_API Pass : public std::enable_shared_from_this<Pass> {
public:
    std::string name;  //< Unique name.
    std::string brief; //< A short discription of the stimulus.

    std::shared_ptr<Stimulus> stimulus; //< Part of this stimulus.

    unsigned int duration;      //< Pass duration [frames].
    unsigned int startingFrame; //< Stimulus starts at this time in stimulus [frames].

    std::string stimulusGeneratorShaderSource; //< Maps random numbers to stimulus values, writes texture in the stimulus queue.
    std::string stimulusGeneratorGeometryShaderMotionTransformFunction;

    std::string timelineVertexShaderSource; //< Renders timeline representation.
    std::string timelineFragmentShaderSource;

    using ShaderVariableMap = std::map<std::string, float>;
    ShaderVariableMap shaderVariables;
    using ShaderColorMap = std::map<std::string, glm::vec3>;
    ShaderColorMap shaderColors;
    using ShaderVectorMap = std::map<std::string, glm::vec2>;
    ShaderVectorMap shaderVectors;
    using ShaderFunctionMap = std::map<std::string, std::string>;
    ShaderFunctionMap shaderFunctions;
    ShaderFunctionMap geomShaderFunctions;
    using ShaderFunctionOrder = std::list<std::string>;
    ShaderFunctionOrder shaderFunctionOrder;
    ShaderFunctionMap   temporalShaderFunctions;
    using ShaderImageMap = std::map<std::string, std::string>;
    ShaderImageMap shaderImages;

    std::vector<glm::vec2> polygonMask;
    struct QuadData {
        float x;
        float y;
        float halfwidth;
        float halfheight;
        float pif;
        float motion;
    };
    std::vector<QuadData> quads;
    enum class RasterizationMode {
        fullscreen,
        triangles,
        quads
    } rasterizationMode;

    std::string videoFileName;
    bool        loop;
    bool        mono;
    bool        transparent;

    //! Constructor.
    Pass ();

public:
    GEARS_SHARED_CREATE_WITH_GETSHAREDPTR (Pass);

    //! Destructor. Releases dynamically allocated memory.
    virtual ~Pass ();

    void setStimulus (std::shared_ptr<Stimulus> stimulus);

    void setShaderImage (std::string varName, std::string file)
    {
        shaderImages[varName] = file;
    }

    void setShaderVariable (std::string varName, float value)
    {
        shaderVariables[varName] = value;
    }

    void setShaderColor (std::string varName, float all, float r, float g, float b)
    {
        if (all >= -1)
            shaderColors[varName] = glm::vec3 (all, all, all);
        else
            shaderColors[varName] = glm::vec3 (r, g, b);
    }

    void setShaderVector (std::string varName, float x, float y)
    {
        shaderVectors[varName] = glm::vec2 (x, y);
    }

    void setMotionTransformFunction (std::string src)
    {
        stimulusGeneratorGeometryShaderMotionTransformFunction = src;
    }

    void setShaderFunction (std::string name, std::string src)
    {
        //		std::stringstream ss;
        //		ss << std::setfill('0') << std::setw(5) << shaderFunctions.size() << "_" << name;
        auto i = shaderFunctions.find (name);
        if (i == shaderFunctions.end ()) {
            shaderFunctionOrder.push_back (name);
        }
        shaderFunctions[name] = src;
    }

    void setGeomShaderFunction (std::string name, std::string src)
    {
        geomShaderFunctions[name] = src;
    }

    unsigned int setStartingFrame (unsigned int offset)
    {
        this->startingFrame = offset;
        return duration;
    }

    void saveConfig (const std::string& expName);

    void setStimulusGeneratorShaderSource (const std::string& src);

    void setDuration (unsigned int duration) { this->duration = duration; }
    uint getDuration () const { return duration; }

    void setTimelineVertexShaderSource (const std::string& src)
    {
        timelineVertexShaderSource = src;
    }

    void setTimelineFragmentShaderSource (const std::string& src)
    {
        timelineFragmentShaderSource = src;
    }

    std::string getTimelineVertexShaderSource () const
    {
        std::string s ("#version 150 compatibility\n");
        s += "uniform vec2 patternSizeOnRetina;\n";
        s += "uniform int frame;\n";
        s += "uniform float time;\n";

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
        for (std::string sfunc : shaderFunctionOrder) {
            s += shaderFunctions.find (sfunc)->second;
            s += "\n";
        }
        if (temporalShaderFunctions.empty ())
            s += "vec3 plottedIntensity(float time){ return vec3(1.0, 1.0, 1.0);}\n";
        else
            s += "vec3 plottedIntensity(float time){ return " + temporalShaderFunctions.begin ()->second + "(time);}\n";
        return s + timelineVertexShaderSource;
    }

    std::string getTimelineFragmentShaderSource () const
    {
        std::string s ("#version 150\n");
        s += "uniform vec2 patternSizeOnRetina;\n";
        s += "uniform int frame;\n";
        s += "uniform float time;\n";

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
        for (std::string sfunc : shaderFunctionOrder) {
            s += shaderFunctions.find (sfunc)->second;
            s += "\n";
        }
        return s + timelineFragmentShaderSource;
    }

    std::string getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode mode) const;
    std::string getStimulusGeneratorGeometryShaderSource (Pass::RasterizationMode mode) const;
    std::string getStimulusGeneratorShaderSource () const;

    uint getStartingFrame () const
    {
        return startingFrame;
    }

    std::shared_ptr<Stimulus> getStimulus () const;
    std::shared_ptr<Sequence> getSequence () const;

    float getDuration_s () const;

    void setVideo (std::string videoFileName);

    bool        hasVideo () const { return videoFileName != ""; }
    std::string getVideo () const { return videoFileName; }

    void registerTemporalFunction (std::string functionName, std::string displayName);

    void onSequenceComplete ();

    void enableColorMode () { mono = false; }

    std::string ToDebugString () const;
};
