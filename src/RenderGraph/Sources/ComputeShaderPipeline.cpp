#include "ComputeShaderPipeline.hpp"
#include "ShaderReflectionToDescriptor.hpp"
#include "ShaderReflectionToVertexAttribute.hpp"

#include "VulkanWrapper/DescriptorSetLayout.hpp"
#include "VulkanWrapper/ComputePipeline.hpp"
#include "VulkanWrapper/PipelineLayout.hpp"
#include "VulkanWrapper/RenderPass.hpp"
#include "VulkanWrapper/ShaderModule.hpp"

#include "Utils/Assert.hpp"

#include <stdexcept>


namespace RG {


ComputeShaderPipeline::~ComputeShaderPipeline ()
{
}


void ComputeShaderPipeline::CompileResult::Clear ()
{
    renderPass.reset ();
    pipelineLayout.reset ();
    pipeline.reset ();
}


ComputeShaderPipeline::ComputeShaderPipeline (VkDevice device)
    : device (device)
{
}


ComputeShaderPipeline::ComputeShaderPipeline (VkDevice device, const std::filesystem::path& path)
    : device (device)
{
    computeShader = GVK::ShaderModule::CreateFromGLSLFile (device, path);
}


ComputeShaderPipeline::ComputeShaderPipeline (VkDevice device, const std::string& source)
    : device (device)
{
    computeShader = GVK::ShaderModule::CreateFromGLSLString (device, GVK::ShaderKind::Compute, source);
}


void ComputeShaderPipeline::Compile (CompileSettings&& settings_)
{
    compileSettings = std::move (settings_);
    compileResult.Clear ();

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_COMPUTE;
    subpass.colorAttachmentCount = static_cast<uint32_t> (compileSettings.attachmentReferences.size ());
    subpass.pColorAttachments    = compileSettings.attachmentReferences.data ();
    subpass.inputAttachmentCount = static_cast<uint32_t> (compileSettings.inputAttachmentReferences.size ());
    subpass.pInputAttachments    = compileSettings.inputAttachmentReferences.data ();

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.dependencyFlags     = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.srcAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                               VK_ACCESS_INDEX_READ_BIT |
                               VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                               VK_ACCESS_UNIFORM_READ_BIT |
                               VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                               VK_ACCESS_SHADER_READ_BIT |
                               VK_ACCESS_SHADER_WRITE_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_TRANSFER_READ_BIT |
                               VK_ACCESS_TRANSFER_WRITE_BIT;
    dependency.dstAccessMask = dependency.srcAccessMask;

    VkSubpassDependency dependency2 = dependency;
    dependency.srcSubpass           = 0;
    dependency.dstSubpass           = VK_SUBPASS_EXTERNAL;

    compileResult.renderPass     = std::unique_ptr<GVK::RenderPass> (new GVK::RenderPass (device, {}, { subpass }, { dependency, dependency2 }));
    compileResult.pipelineLayout = std::unique_ptr<GVK::PipelineLayout> (new GVK::PipelineLayout (device, { compileSettings.layout }));

    compileResult.pipeline = std::unique_ptr<GVK::ComputePipeline> (new GVK::ComputePipeline (
        device,
        *compileResult.pipelineLayout,
        *compileResult.renderPass,
        *computeShader));
}


std::unique_ptr<GVK::DescriptorSetLayout> ComputeShaderPipeline::CreateDescriptorSetLayout (VkDevice device) const
{
    return std::make_unique<GVK::DescriptorSetLayout> (device, RG::FromShaderReflection::GetLayout (computeShader->GetReflection (), computeShader->GetShaderKind ()));
}

} // namespace RG
