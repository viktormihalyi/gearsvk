#ifndef SHADERPIPELINE2_HPP
#define SHADERPIPELINE2_HPP

#include "GearsVkAPI.hpp"
#include "UniformBlock.hpp"
#include "VulkanWrapper.hpp"

#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

USING_PTR (ShaderPipeline);

class GEARSVK_API ShaderPipeline {
public:
    USING_CREATE (ShaderPipeline);

private:
    const VkDevice device;

public:
    struct ShaderObject {
        ShaderModuleU shader;

        void Set (ShaderModuleU&& _shader)
        {
            shader = std::move (_shader);
            ubos   = ShaderBlocks::Create ();

            for (const auto& s : shader->GetReflection ().ubos) {
                ShaderStruct autoStruct (*s);
                auto         autoBlock = UniformBlock::Create (s->binding, s->name, autoStruct);
                ubos->AddBlock (std::move (autoBlock));
            }
        }

    private:
        ShaderBlocksU ubos; // TODO remove
    };

    ShaderObject vertexShader;
    ShaderObject fragmentShader;
    ShaderObject geometryShader;
    ShaderObject tessellationEvaluationShader;
    ShaderObject tessellationControlShader;
    ShaderObject computeShader;

    ShaderObject& GetShaderByIndex (uint32_t index);
    ShaderObject& GetShaderByExtension (const std::string& extension);
    ShaderObject& GetShaderByKind (ShaderModule::ShaderKind kind);

public:
    struct CompileSettings {
        uint32_t                                       width;
        uint32_t                                       height;
        VkDescriptorSetLayout                          layout;
        std::vector<VkAttachmentReference>             attachmentReferences;
        std::vector<VkAttachmentDescription>           attachmentDescriptions;
        std::vector<VkVertexInputBindingDescription>   inputBindings;
        std::vector<VkVertexInputAttributeDescription> inputAttributes;
        VkPrimitiveTopology                            topology;
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
    ShaderPipeline (VkDevice device, const std::vector<std::pair<ShaderModule::ShaderKind, std::string>>& sources);

    // settings shaders
    void SetShaderFromSourceString (ShaderModule::ShaderKind shaderKind, const std::string& source);
    void SetVertexShaderFromString (const std::string& source);
    void SetFragmentShaderFromString (const std::string& source);

    void SetShaderFromSourceFile (const std::filesystem::path& shaderPath);
    void SetShadersFromSourceFiles (const std::vector<std::filesystem::path>& shaderPath);

    void Compile (const CompileSettings& settings);

    void Reload ();

    void IterateShaders (const std::function<void (ShaderModule&)>& func) const;

    std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages () const;
};

#endif