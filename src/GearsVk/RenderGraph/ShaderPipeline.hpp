#ifndef SHADERPIPELINE2_HPP
#define SHADERPIPELINE2_HPP

#include "GearsVkAPI.hpp"
#include "VulkanWrapper.hpp"

#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

namespace GVK {

namespace RG {

class GVK_RENDERER_API ShaderPipeline {
private:
    const VkDevice device;

public:
    U<ShaderModule> vertexShader;
    U<ShaderModule> fragmentShader;
    U<ShaderModule> geometryShader;
    U<ShaderModule> tessellationEvaluationShader;
    U<ShaderModule> tessellationControlShader;
    U<ShaderModule> computeShader;


    U<ShaderModule>& GetShaderByIndex (uint32_t index);
    U<ShaderModule>& GetShaderByExtension (const std::string& extension);
    U<ShaderModule>& GetShaderByKind (ShaderKind kind);

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
        U<RenderPass>     renderPass;
        U<PipelineLayout> pipelineLayout;
        U<Pipeline>       pipeline;

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

    U<DescriptorSetLayout> CreateDescriptorSetLayout (VkDevice device) const;
};

} // namespace RG

} // namespace GVK

#endif