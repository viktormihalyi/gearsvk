#pragma once

#include <glm/glm.hpp>
#include "SequenceAPI.hpp"
#include <memory>

#include <list>
#include <map>
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

class Sequence;

//! Structure describing a spatial filter logic.
class SEQUENCE_API SpatialFilter : public std::enable_shared_from_this<SpatialFilter> {
public:
    SpatialFilter ();

    std::string kernelFuncSource;
    std::string kernelProfileVertexSource;
    std::string kernelProfileFragmentSource;
    std::string spatialDomainConvolutionShaderSource;

    uint32_t uniqueId;

    float width_um;
    float height_um;
    float minimum;
    float maximum;

    bool useFft;
    bool separable;
    bool kernelGivenInFrequencyDomain;
    bool showFft;
    bool stimulusGivenInFrequencyDomain;
    uint32_t fftSwizzleMask;

    uint32_t horizontalSampleCount;
    uint32_t verticalSampleCount;

    using ShaderVariableMap = std::map<std::string, float>;
    ShaderVariableMap shaderVariables;
    using ShaderColorMap = std::map<std::string, glm::vec3>;
    ShaderColorMap shaderColors;
    using ShaderVectorMap = std::map<std::string, glm::vec2>;
    ShaderVectorMap shaderVectors;
    using ShaderFunctionMap = std::map<std::string, std::string>;
    ShaderFunctionMap shaderFunctions;
    using ShaderFunctionOrder = std::list<std::string>;
    ShaderFunctionOrder shaderFunctionOrder;

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

    void setShaderFunction (std::string funcName, std::string source)
    {
        auto i = shaderFunctions.find (funcName);
        if (i == shaderFunctions.end ()) {
            shaderFunctionOrder.push_back (funcName);
        }
        shaderFunctions[funcName] = source;
    }

    std::string getKernelGeneratorShaderSource () const
    {
        std::string s ("#version 150\n");
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
            s += ";\n";
        }
        return s + kernelFuncSource;
    }

    std::string getProfileVertexShaderSource () const
    {
        std::string s ("#version 150 compatibility\n");
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
        return s + kernelProfileVertexSource;
    }

    std::string getProfileFragmentShaderSource () const
    {
        std::string s ("#version 150 compatibility\n");
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
        return s + kernelProfileFragmentSource;
    }

    std::string getKernelGeneratorShaderSourceWithParameters () const
    {
        std::string s ("#version 150\n");
        if (uniqueId == 0) {
            for (auto& svar : shaderColors) {
                s += "uniform vec3 ";
                s += svar.first;
                s += " = ";
                s += std::to_string (svar.second.x);
                s += " , ";
                s += std::to_string (svar.second.y);
                s += " , ";
                s += std::to_string (svar.second.z);
                s += ";\n";
            }
            for (auto& svar : shaderVectors) {
                s += "uniform vec2 ";
                s += svar.first;
                s += " = ";
                s += std::to_string (svar.second.x);
                s += " , ";
                s += std::to_string (svar.second.y);
                s += ";\n";
            }
            for (auto& svar : shaderVariables) {
                s += "uniform float ";
                s += svar.first;
                s += " = ";
                s += std::to_string (svar.second);
                s += ";\n";
            }
            for (std::string sfunc : shaderFunctionOrder) {
                s += shaderFunctions.find (sfunc)->second;
                s += ";\n";
            }
        } else
            s += std::to_string (uniqueId);
        return s + kernelFuncSource;
    }

    void setSpatialDomainShader (std::string s)
    {
        spatialDomainConvolutionShaderSource = s;
    }

    void makeUnique ();

    
    template <typename Archive>
    void serialize (Archive& ar)
    {
        ar (CEREAL_NVP (kernelFuncSource));
        ar (CEREAL_NVP (kernelProfileVertexSource));
        ar (CEREAL_NVP (kernelProfileFragmentSource));
        ar (CEREAL_NVP (spatialDomainConvolutionShaderSource));

        ar (CEREAL_NVP (uniqueId));

        ar (CEREAL_NVP (width_um));
        ar (CEREAL_NVP (height_um));
        ar (CEREAL_NVP (minimum));
        ar (CEREAL_NVP (maximum));

        ar (CEREAL_NVP (useFft));
        ar (CEREAL_NVP (separable));
        ar (CEREAL_NVP (kernelGivenInFrequencyDomain));
        ar (CEREAL_NVP (showFft));
        ar (CEREAL_NVP (stimulusGivenInFrequencyDomain));
        ar (CEREAL_NVP (fftSwizzleMask));

        ar (CEREAL_NVP (horizontalSampleCount));
        ar (CEREAL_NVP (verticalSampleCount));

        ar (CEREAL_NVP (shaderVariables));
        ar (CEREAL_NVP (shaderColors));
        ar (CEREAL_NVP (shaderVectors));
        ar (CEREAL_NVP (shaderFunctions));
        ar (CEREAL_NVP (shaderFunctionOrder));
    }
};
