#ifndef SHADERPIPELINE_HPP
#define SHADERPIPELINE_HPP

#include "ShaderModule.hpp"

#include <vector>
#include <vulkan/vulkan.h>

struct ShaderPipeline {
    ShaderModule::U vertexShader;
    ShaderModule::U fragmentShader;
    ShaderModule::U geometryShader;
    ShaderModule::U tessellationEvaluationShader;
    ShaderModule::U tessellationControlShader;
    ShaderModule::U computeShader;

    std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages () const
    {
        std::vector<VkPipelineShaderStageCreateInfo> result;

        if (vertexShader != nullptr)
            result.push_back (vertexShader->GetShaderStageCreateInfo ());
        if (fragmentShader != nullptr)
            result.push_back (fragmentShader->GetShaderStageCreateInfo ());
        if (geometryShader != nullptr)
            result.push_back (geometryShader->GetShaderStageCreateInfo ());
        if (tessellationEvaluationShader != nullptr)
            result.push_back (tessellationEvaluationShader->GetShaderStageCreateInfo ());
        if (tessellationControlShader != nullptr)
            result.push_back (tessellationControlShader->GetShaderStageCreateInfo ());
        if (computeShader != nullptr)
            result.push_back (computeShader->GetShaderStageCreateInfo ());

        return result;
    }
};

#endif