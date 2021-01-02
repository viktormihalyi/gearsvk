#ifndef SHADERPIPELINE2_HPP
#define SHADERPIPELINE2_HPP

#include "GearsVkAPI.hpp"
#include "VulkanWrapper.hpp"

#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

USING_PTR (ShaderPipeline);
class GEARSVK_API ShaderPipeline {
    USING_CREATE (ShaderPipeline);

private:
    const VkDevice device;

public:
    ShaderModuleU vertexShader;
    ShaderModuleU fragmentShader;
    ShaderModuleU geometryShader;
    ShaderModuleU tessellationEvaluationShader;
    ShaderModuleU tessellationControlShader;
    ShaderModuleU computeShader;


    ShaderModuleU& GetShaderByIndex (uint32_t index);
    ShaderModuleU& GetShaderByExtension (const std::string& extension);
    ShaderModuleU& GetShaderByKind (ShaderKind kind);

public:
    struct CompileSettings {
        uint32_t                             width;
        uint32_t                             height;
        VkDescriptorSetLayout                layout;
        std::vector<VkAttachmentReference>   attachmentReferences;
        std::vector<VkAttachmentDescription> attachmentDescriptions;
        VkPrimitiveTopology                  topology;
    };


    struct CompileResult {
        RenderPassU     renderPass;
        PipelineLayoutU pipelineLayout;
        PipelineU       pipeline;

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
    ShaderPipeline (VkDevice device, const std::vector<std::pair<ShaderKind, std::string>>& sources);

    // settings shaders
    void SetShaderFromSourceString (ShaderKind shaderKind, const std::string& source, ShaderPreprocessor& preprocessor = emptyPreprocessor);
    void SetVertexShaderFromString (const std::string& source, ShaderPreprocessor& preprocessor = emptyPreprocessor);
    void SetFragmentShaderFromString (const std::string& source, ShaderPreprocessor& preprocessor = emptyPreprocessor);

    void SetShaderFromSourceFile (const std::filesystem::path& shaderPath);
    void SetShadersFromSourceFiles (const std::vector<std::filesystem::path>& shaderPath);

    void Compile (const CompileSettings& settings);

    void Reload ();

    void IterateShaders (const std::function<void (ShaderModule&)>& func) const;

    std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages () const;

    DescriptorSetLayoutU CreateDescriptorSetLayout (VkDevice device) const;
};

#endif