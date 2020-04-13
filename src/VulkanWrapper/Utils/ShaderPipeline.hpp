#ifndef SHADERPIPELINE_HPP
#define SHADERPIPELINE_HPP

#include "Timer.hpp"
#include "VulkanWrapper.hpp"

#include <thread>
#include <vector>
#include <vulkan/vulkan.h>


class ShaderPipeline {
private:
    VkDevice device;

    ShaderModule::U vertexShader;
    ShaderModule::U fragmentShader;
    ShaderModule::U geometryShader;
    ShaderModule::U tessellationEvaluationShader;
    ShaderModule::U tessellationControlShader;
    ShaderModule::U computeShader;

    ShaderModule::U& GetShaderByExtension (const std::string& extension)
    {
        if (extension == ".vert") {
            return vertexShader;
        } else if (extension == ".frag") {
            return fragmentShader;
        } else if (extension == ".tese") {
            return tessellationEvaluationShader;
        } else if (extension == ".tesc") {
            return tessellationControlShader;
        } else if (extension == ".comp") {
            return computeShader;
        }

        ERROR (true);
        throw std::runtime_error ("bad shader extension");
    }

    ShaderModule::U& GetShaderByKind (ShaderModule::ShaderKind kind)
    {
        switch (kind) {
            case ShaderModule::ShaderKind::Vertex: return vertexShader;
            case ShaderModule::ShaderKind::Fragment: return fragmentShader;
            case ShaderModule::ShaderKind::TessellationControl: return tessellationControlShader;
            case ShaderModule::ShaderKind::TessellationEvaluation: return tessellationEvaluationShader;
            case ShaderModule::ShaderKind::Geometry: return geometryShader;
            case ShaderModule::ShaderKind::Compute: return computeShader;
        }

        ERROR (true);
        throw std::runtime_error ("unknown shader kind");
    }

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
    USING_PTR (ShaderPipeline);

    CompileResult   compileResult;
    CompileSettings compileSettings;

    //RenderPass::U     renderPass;
    //PipelineLayout::U pipelineLayout;
    //Pipeline::U       pipeline;

    ShaderPipeline (VkDevice device)
        : device (device)
    {
    }

    ShaderPipeline (VkDevice device, const std::vector<std::filesystem::path>& pathes)
        : ShaderPipeline (device)
    {
        AddShaders (pathes);
    }

    ShaderPipeline (VkDevice device, const std::vector<std::pair<ShaderModule::ShaderKind, std::string>>& sources)
        : ShaderPipeline (device)
    {
        for (auto [kind, source] : sources) {
            GetShaderByKind (kind) = ShaderModule::CreateFromString (device, source, kind);
        }
    }

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


    void SetShader (ShaderModule::ShaderKind shaderKind, const std::string& source)
    {
        ASSERT (GetShaderByKind (shaderKind) == nullptr);
        GetShaderByKind (shaderKind) = ShaderModule::CreateFromString (device, source, shaderKind);
    }

    void SetVertexShader (const std::string& source) { SetShader (ShaderModule::ShaderKind::Vertex, source); }

    void SetFragmentShader (const std::string& source) { SetShader (ShaderModule::ShaderKind::Fragment, source); }

    ShaderPipeline& AddShader (const std::filesystem::path& shaderPath)
    {
        ShaderModule::U& moduleFromExtension = GetShaderByExtension (shaderPath.extension ().u8string ());

        ASSERT (moduleFromExtension == nullptr);

        moduleFromExtension = ShaderModule::CreateFromSource (device, shaderPath);

        return *this;
    }


    ShaderPipeline& AddShaders (const std::vector<std::filesystem::path>& shaderPath)
    {
        std::vector<std::thread> threads;

        for (const std::filesystem::path& p : shaderPath) {
            threads.emplace_back ([=] () {
                AddShader (p);
            });
        }

        for (std::thread& t : threads) {
            t.join ();
        }

        return *this;
    }

    void Compile (const CompileSettings& settings)
    {
        compileSettings = settings;
        compileResult.Clear ();

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = settings.attachmentReferences.size ();
        subpass.pColorAttachments    = settings.attachmentReferences.data ();

        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = 0;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask       = 0;
        dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount        = settings.attachmentDescriptions.size ();
        renderPassInfo.pAttachments           = settings.attachmentDescriptions.data ();
        renderPassInfo.subpassCount           = 1;
        renderPassInfo.pSubpasses             = &subpass;
        renderPassInfo.dependencyCount        = 1;
        renderPassInfo.pDependencies          = &dependency;

        compileResult.renderPass     = std::unique_ptr<RenderPass> (new RenderPass (device, settings.attachmentDescriptions, {subpass}, {dependency}));
        compileResult.pipelineLayout = std::unique_ptr<PipelineLayout> (new PipelineLayout (device, {settings.layout}));
        compileResult.pipeline       = std::unique_ptr<Pipeline> (new Pipeline (device, settings.width, settings.height, settings.attachmentReferences.size (), *compileResult.pipelineLayout, *compileResult.renderPass, GetShaderStages (), settings.inputBindings, settings.inputAttributes));
    }
};

#endif