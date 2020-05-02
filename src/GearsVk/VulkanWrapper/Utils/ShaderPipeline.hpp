#ifndef SHADERPIPELINE_HPP
#define SHADERPIPELINE_HPP

#include "GearsVkAPI.hpp"
#include "VulkanWrapper.hpp"

#include <thread>
#include <vector>
#include <vulkan/vulkan.h>


class GEARSVK_API ShaderPipeline {
public:
    USING_PTR (ShaderPipeline);

private:
    const VkDevice device;

    ShaderModule::U vertexShader;
    ShaderModule::U fragmentShader;
    ShaderModule::U geometryShader;
    ShaderModule::U tessellationEvaluationShader;
    ShaderModule::U tessellationControlShader;
    ShaderModule::U computeShader;

    ShaderModule::U& GetShaderByIndex (uint32_t index);
    ShaderModule::U& GetShaderByExtension (const std::string& extension);
    ShaderModule::U& GetShaderByKind (ShaderModule::ShaderKind kind);

public:
    struct CompileSettings {
        uint32_t                                       width;
        uint32_t                                       height;
        VkDescriptorSetLayout                          layout;
        std::vector<VkAttachmentReference>             attachmentReferences;
        std::vector<VkAttachmentDescription>           attachmentDescriptions;
        std::vector<VkVertexInputBindingDescription>   inputBindings;
        std::vector<VkVertexInputAttributeDescription> inputAttributes;
    };


    struct CompileResult {
        RenderPass::U     renderPass;
        PipelineLayout::U pipelineLayout;
        Pipeline::U       pipeline;

        void Clear ()
        {
            renderPass.reset ();
            pipelineLayout.reset ();
            pipeline.reset ();
        }
    };

public:
    CompileResult   compileResult;
    CompileSettings compileSettings;

    ShaderPipeline (VkDevice device);
    ShaderPipeline (VkDevice device, const std::vector<std::filesystem::path>& pathes);
    ShaderPipeline (VkDevice device, const std::vector<std::pair<ShaderModule::ShaderKind, std::string>>& sources);

    // settings shaders
    void SetShaderFromSourceString (ShaderModule::ShaderKind shaderKind, const std::string& source);
    void SetVertexShaderFromString (const std::string& source);
    void SetFragmentShaderFromString (const std::string& source);

    void SetShaderFromSourceFile (const std::filesystem::path& shaderPath);
    void SetShadersFromSourceFiles (const std::vector<std::filesystem::path>& shaderPath);

    void SetShaderFromBinaryFile (const std::filesystem::path& shaderPath);
    void SetShadersFromBinaryFiles (const std::vector<std::filesystem::path>& shaderPath);

#if 0
    void SetProvidedShader (ShaderModule::ShaderKind shaderKind, std::vector<const ShaderSourceBuilder*> builders, const std::string& source);
#endif

    void Compile (const CompileSettings& settings);

    void Reload ();

    std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages () const;
};

#endif