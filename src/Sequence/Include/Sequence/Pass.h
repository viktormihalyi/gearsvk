#pragma once

#include "SequenceAPI.hpp"

#include <glm/glm.hpp>

#include <list>
#include <map>
#include <string>
#include <vector>
#include <memory>

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>


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
        
        template <class Archive>
        void serialize (Archive& ar)
        {
            ar (CEREAL_NVP (x));
            ar (CEREAL_NVP (y));
            ar (CEREAL_NVP (halfwidth));
            ar (CEREAL_NVP (halfheight));
            ar (CEREAL_NVP (pif));
            ar (CEREAL_NVP (motion));
        }
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


public:
    Pass ();

    virtual ~Pass ();

    void setStimulus (std::shared_ptr<Stimulus> stimulus);

    void setShaderImage (std::string varName, std::string file);

    void setShaderVariable (std::string varName, float value);

    void setShaderColor (std::string varName, float all, float r, float g, float b);

    void setShaderVector (std::string varName, float x, float y);

    void setMotionTransformFunction (std::string src);

    void setShaderFunction (std::string name, std::string src);

    void setGeomShaderFunction (std::string name, std::string src);

    unsigned int setStartingFrame (unsigned int offset);

    void saveConfig (const std::string& expName);

    void setStimulusGeneratorShaderSource (const std::string& src);

    void setDuration (unsigned int duration) { this->duration = duration; }
    uint32_t getDuration () const { return duration; }

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

    uint32_t getStartingFrame () const
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

    template <class Archive>
    void serialize (Archive& ar)
    {
        ar (CEREAL_NVP (name));
        ar (CEREAL_NVP (brief));
        ar (CEREAL_NVP (stimulus));
        ar (CEREAL_NVP (duration));
        ar (CEREAL_NVP (startingFrame));
        ar (CEREAL_NVP (stimulusGeneratorShaderSource));
        ar (CEREAL_NVP (stimulusGeneratorGeometryShaderMotionTransformFunction));
        ar (CEREAL_NVP (timelineVertexShaderSource));
        ar (CEREAL_NVP (timelineFragmentShaderSource));
        ar (CEREAL_NVP (shaderVariables));
        ar (CEREAL_NVP (shaderColors));
        ar (CEREAL_NVP (shaderVectors));
        ar (CEREAL_NVP (shaderFunctions));
        ar (CEREAL_NVP (geomShaderFunctions));
        ar (CEREAL_NVP (shaderFunctionOrder));
        ar (CEREAL_NVP (temporalShaderFunctions));
        ar (CEREAL_NVP (shaderImages));
        ar (CEREAL_NVP (polygonMask));
        ar (CEREAL_NVP (quads));
        ar (CEREAL_NVP (rasterizationMode));
        ar (CEREAL_NVP (videoFileName));
        ar (CEREAL_NVP (loop));
        ar (CEREAL_NVP (mono));
        ar (CEREAL_NVP (transparent));
    }
};


CEREAL_REGISTER_TYPE (Pass)