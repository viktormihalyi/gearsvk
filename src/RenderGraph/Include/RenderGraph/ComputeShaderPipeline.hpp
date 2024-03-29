#ifndef COMPUTE_SHADERPIPELINE_HPP
#define COMPUTE_SHADERPIPELINE_HPP

#include "RenderGraph/RenderGraphAPI.hpp"

#include "Utils/MovablePtr.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace GVK {
enum class ShaderKind : uint8_t;
class ShaderModule;
class RenderPass;
class ComputePipeline;
class PipelineLayout;
class DescriptorSetLayout;
} // namespace GVK

namespace RG {

class GVK_RENDERER_API ComputeShaderPipeline {
private:
    const VkDevice device;

public:
    std::unique_ptr<GVK::ShaderModule> computeShader;

public:
    struct GVK_RENDERER_API CompileSettings {
        GVK::MovablePtr<VkDescriptorSetLayout> layout;
        std::vector<VkAttachmentReference>     attachmentReferences;
        std::vector<VkAttachmentReference>     inputAttachmentReferences;
        std::vector<VkAttachmentDescription>   attachmentDescriptions;
    };

    struct GVK_RENDERER_API CompileResult {
        std::unique_ptr<GVK::PipelineLayout>  pipelineLayout;
        std::unique_ptr<GVK::ComputePipeline> pipeline;

        void Clear ();
    };

public:
    CompileResult   compileResult;
    CompileSettings compileSettings;

    ComputeShaderPipeline (VkDevice device);
    ComputeShaderPipeline (VkDevice device, const std::filesystem::path& pathes);
    ComputeShaderPipeline (VkDevice device, const std::string& source);

    ~ComputeShaderPipeline ();

    void Compile (CompileSettings&& settings);

    void IterateShaders (const std::function<void(const GVK::ShaderModule&)> iterator) const;

    std::unique_ptr<GVK::DescriptorSetLayout> CreateDescriptorSetLayout (VkDevice device) const;
};

} // namespace RG

#endif