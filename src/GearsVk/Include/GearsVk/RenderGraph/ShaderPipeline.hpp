#ifndef SHADERPIPELINE2_HPP
#define SHADERPIPELINE2_HPP

#include "GearsVk/GearsVkAPI.hpp"
#include "GearsVk/VulkanWrapper/VulkanWrapper.hpp"

#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

namespace GVK {

namespace RG {

class GVK_RENDERER_API ShaderPipeline {
private:
    const VkDevice device;

public:
    std::unique_ptr<ShaderModule> vertexShader;
    std::unique_ptr<ShaderModule> fragmentShader;
    std::unique_ptr<ShaderModule> geometryShader;
    std::unique_ptr<ShaderModule> tessellationEvaluationShader;
    std::unique_ptr<ShaderModule> tessellationControlShader;
    std::unique_ptr<ShaderModule> computeShader;


    std::unique_ptr<ShaderModule>& GetShaderByIndex (uint32_t index);
    std::unique_ptr<ShaderModule>& GetShaderByExtension (const std::string& extension);
    std::unique_ptr<ShaderModule>& GetShaderByKind (ShaderKind kind);

public:
    struct CompileSettings {
        uint32_t                             width;
        uint32_t                             height;
        VkDescriptorSetLayout                layout;
        std::vector<VkAttachmentReference>   attachmentReferences;
        std::vector<VkAttachmentDescription> attachmentDescriptions;
        VkPrimitiveTopology                  topology;

        std::optional<bool> blendEnabled;
    };


    struct CompileResult {
        std::unique_ptr<RenderPass>     renderPass;
        std::unique_ptr<PipelineLayout> pipelineLayout;
        std::unique_ptr<Pipeline>       pipeline;

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

    std::unique_ptr<DescriptorSetLayout> CreateDescriptorSetLayout (VkDevice device) const;
};

} // namespace RG

} // namespace GVK

#endif