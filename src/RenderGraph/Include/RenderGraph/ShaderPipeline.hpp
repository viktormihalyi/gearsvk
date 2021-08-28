#ifndef SHADERPIPELINE2_HPP
#define SHADERPIPELINE2_HPP

#include "RenderGraph/RenderGraphAPI.hpp"
#include "VulkanWrapper/VulkanWrapper.hpp"

#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

namespace RG {

class GVK_RENDERER_API ShaderPipeline {
private:
    const VkDevice device;

public:
    std::unique_ptr<GVK::ShaderModule> vertexShader;
    std::unique_ptr<GVK::ShaderModule> fragmentShader;
    std::unique_ptr<GVK::ShaderModule> geometryShader;
    std::unique_ptr<GVK::ShaderModule> tessellationEvaluationShader;
    std::unique_ptr<GVK::ShaderModule> tessellationControlShader;
    std::unique_ptr<GVK::ShaderModule> computeShader;


    std::unique_ptr<GVK::ShaderModule>& GetShaderByIndex (uint32_t index);
    std::unique_ptr<GVK::ShaderModule>& GetShaderByExtension (const std::string& extension);
    std::unique_ptr<GVK::ShaderModule>& GetShaderByKind (GVK::ShaderKind kind);

public:
    struct CompileSettings {
        uint32_t                               width;
        uint32_t                               height;
        GVK::MovablePtr<VkDescriptorSetLayout> layout;
        std::vector<VkAttachmentReference>     attachmentReferences;
        std::vector<VkAttachmentDescription>   attachmentDescriptions;
        VkPrimitiveTopology                    topology;

        std::optional<bool> blendEnabled;
    };


    struct CompileResult {
        std::unique_ptr<GVK::RenderPass>     renderPass;
        std::unique_ptr<GVK::PipelineLayout> pipelineLayout;
        std::unique_ptr<GVK::Pipeline>       pipeline;

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
    ShaderPipeline (VkDevice device, const std::vector<std::pair<GVK::ShaderKind, std::string>>& sources);

    // settings shaders
    void SetShaderFromSourceString (GVK::ShaderKind shaderKind, const std::string& source);
    void SetVertexShaderFromString (const std::string& source);
    void SetFragmentShaderFromString (const std::string& source);

    void SetShaderFromSourceFile (const std::filesystem::path& shaderPath);
    void SetShadersFromSourceFiles (const std::vector<std::filesystem::path>& shaderPath);

    void Compile (CompileSettings&& settings);

    void Reload ();

    void IterateShaders (const std::function<void (GVK::ShaderModule&)>& func) const;

    std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages () const;

    std::unique_ptr<GVK::DescriptorSetLayout> CreateDescriptorSetLayout (VkDevice device) const;
};

} // namespace RG

#endif