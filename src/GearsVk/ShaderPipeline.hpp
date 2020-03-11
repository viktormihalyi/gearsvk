#ifndef SHADERPIPELINE_HPP
#define SHADERPIPELINE_HPP

#include "VulkanWrapper.hpp"

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

    RenderPass::U renderPass;
    // (device, outputs.GetAttachmentDescriptions (), {subpass}, {dependency});
    PipelineLayout::U pipelineLayout;
    // (device, {descLayout});
    Pipeline::U pipeline;
    // (device, 512, 512, outputs.textures.size (), pipelineLayout, renderPass, shaders.GetShaderStages (), {}, {});

    void Compile (VkDevice device, uint32_t width, uint32_t height, VkDescriptorSetLayout layout, const std::vector<VkAttachmentReference>& attachmentReferences, const std::vector<VkAttachmentDescription>& attachmentDescriptions)
    {
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = attachmentReferences.size ();
        subpass.pColorAttachments    = attachmentReferences.data ();

        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = 0;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask       = 0;
        dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount        = attachmentDescriptions.size ();
        renderPassInfo.pAttachments           = attachmentDescriptions.data ();
        renderPassInfo.subpassCount           = 1;
        renderPassInfo.pSubpasses             = &subpass;
        renderPassInfo.dependencyCount        = 1;
        renderPassInfo.pDependencies          = &dependency;

        renderPass     = std::unique_ptr<RenderPass> (new RenderPass (device, attachmentDescriptions, {subpass}, {dependency}));
        pipelineLayout = std::unique_ptr<PipelineLayout> (new PipelineLayout (device, {layout}));
        pipeline       = std::unique_ptr<Pipeline> (new Pipeline (device, width, height, attachmentReferences.size (), *pipelineLayout, *renderPass, GetShaderStages (), {}, {}));
    }
};

#endif