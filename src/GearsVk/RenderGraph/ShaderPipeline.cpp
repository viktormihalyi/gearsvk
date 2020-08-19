#include "ShaderPipeline.hpp"

#include "MultithreadedFunction.hpp"
#include "Timer.hpp"


ShaderPipeline::ShaderObject& ShaderPipeline::GetShaderByIndex (uint32_t index)
{
    switch (index) {
        case 0: return vertexShader;
        case 1: return fragmentShader;
        case 2: return geometryShader;
        case 3: return tessellationEvaluationShader;
        case 4: return tessellationControlShader;
        case 5: return computeShader;
        default:
            GVK_ASSERT (false);
            throw std::runtime_error ("no");
    }
}


ShaderPipeline::ShaderObject& ShaderPipeline::GetShaderByExtension (const std::string& extension)
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

    GVK_ERROR (true);
    throw std::runtime_error ("bad shader extension");
}


ShaderPipeline::ShaderObject& ShaderPipeline::GetShaderByKind (ShaderModule::ShaderKind kind)
{
    switch (kind) {
        case ShaderModule::ShaderKind::Vertex: return vertexShader;
        case ShaderModule::ShaderKind::Fragment: return fragmentShader;
        case ShaderModule::ShaderKind::TessellationControl: return tessellationControlShader;
        case ShaderModule::ShaderKind::TessellationEvaluation: return tessellationEvaluationShader;
        case ShaderModule::ShaderKind::Geometry: return geometryShader;
        case ShaderModule::ShaderKind::Compute: return computeShader;
    }

    GVK_ERROR (true);
    throw std::runtime_error ("unknown shader kind");
}


ShaderPipeline::ShaderPipeline (VkDevice device)
    : device (device)
{
}


ShaderPipeline::ShaderPipeline (VkDevice device, const std::vector<std::filesystem::path>& pathes)
    : ShaderPipeline (device)
{
    SetShadersFromSourceFiles (pathes);
}


ShaderPipeline::ShaderPipeline (VkDevice device, const std::vector<std::pair<ShaderModule::ShaderKind, std::string>>& sources)
    : ShaderPipeline (device)
{
    for (auto [kind, source] : sources) {
        GetShaderByKind (kind).Set (ShaderModule::CreateFromGLSLString (device, kind, source));
    }
}

std::vector<VkPipelineShaderStageCreateInfo> ShaderPipeline::GetShaderStages () const
{
    std::vector<VkPipelineShaderStageCreateInfo> result;

    if (vertexShader.shader != nullptr)
        result.push_back (vertexShader.shader->GetShaderStageCreateInfo ());
    if (fragmentShader.shader != nullptr)
        result.push_back (fragmentShader.shader->GetShaderStageCreateInfo ());
    if (geometryShader.shader != nullptr)
        result.push_back (geometryShader.shader->GetShaderStageCreateInfo ());
    if (tessellationEvaluationShader.shader != nullptr)
        result.push_back (tessellationEvaluationShader.shader->GetShaderStageCreateInfo ());
    if (tessellationControlShader.shader != nullptr)
        result.push_back (tessellationControlShader.shader->GetShaderStageCreateInfo ());
    if (computeShader.shader != nullptr)
        result.push_back (computeShader.shader->GetShaderStageCreateInfo ());

    return result;
}


void ShaderPipeline::SetShaderFromSourceString (ShaderModule::ShaderKind shaderKind, const std::string& source)
{
    GVK_ASSERT (GetShaderByKind (shaderKind).shader == nullptr);
    GetShaderByKind (shaderKind).Set (ShaderModule::CreateFromGLSLString (device, shaderKind, source));
}


void ShaderPipeline::SetVertexShaderFromString (const std::string& source)
{
    SetShaderFromSourceString (ShaderModule::ShaderKind::Vertex, source);
}


void ShaderPipeline::SetFragmentShaderFromString (const std::string& source)
{
    SetShaderFromSourceString (ShaderModule::ShaderKind::Fragment, source);
}


void ShaderPipeline::SetShaderFromSourceFile (const std::filesystem::path& shaderPath)
{
    // assert on overwriting shader
    GVK_ASSERT (GetShaderByExtension (shaderPath.extension ().u8string ()).shader == nullptr);

    GetShaderByExtension (shaderPath.extension ().u8string ()).Set (ShaderModule::CreateFromGLSLFile (device, shaderPath));
}


void ShaderPipeline::SetShadersFromSourceFiles (const std::vector<std::filesystem::path>& shaderPath)
{
    MultithreadedFunction d (shaderPath.size (), [&] (uint32_t threadCount, uint32_t threadIndex) {
        SetShaderFromSourceFile (shaderPath[threadIndex]);
    });
}


void ShaderPipeline::Compile (const CompileSettings& settings)
{
    compileSettings = settings;
    compileResult.Clear ();

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t> (settings.attachmentReferences.size ());
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
    renderPassInfo.attachmentCount        = static_cast<uint32_t> (settings.attachmentDescriptions.size ());
    renderPassInfo.pAttachments           = settings.attachmentDescriptions.data ();
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;
    renderPassInfo.dependencyCount        = 1;
    renderPassInfo.pDependencies          = &dependency;

    compileResult.renderPass     = std::unique_ptr<RenderPass> (new RenderPass (device, settings.attachmentDescriptions, {subpass}, {dependency}));
    compileResult.pipelineLayout = std::unique_ptr<PipelineLayout> (new PipelineLayout (device, {settings.layout}));
    compileResult.pipeline       = std::unique_ptr<Pipeline> (new Pipeline (device, settings.width, settings.height, static_cast<uint32_t> (settings.attachmentReferences.size ()), *compileResult.pipelineLayout, *compileResult.renderPass, GetShaderStages (), settings.inputBindings, settings.inputAttributes, settings.topology));
}


void ShaderPipeline::Reload ()
{
    Utils::DebugTimerLogger tl ("reloading shaders");
    Utils::TimerScope       ts (tl);

    MultithreadedFunction reloader (5, [&] (uint32_t, uint32_t threadIndex) {
        ShaderObject& currentShader = GetShaderByIndex (threadIndex);
        ShaderObject  newShader;

        if (currentShader.shader != nullptr) {
            switch (currentShader.shader->GetReadMode ()) {
                case ShaderModule::ReadMode::GLSLFilePath:
                    try {
                        newShader.Set (ShaderModule::CreateFromGLSLFile (device, currentShader.shader->GetLocation ()));
                    } catch (ShaderCompileException&) {
                    }
                    break;

                case ShaderModule::ReadMode::SPVFilePath:
                    try {
                        newShader.Set (ShaderModule::CreateFromSPVFile (device, currentShader.shader->GetShaderKind (), currentShader.shader->GetLocation ()));
                    } catch (ShaderCompileException&) {
                    }
                    break;

                case ShaderModule::ReadMode::GLSLString:
                    // cannot reload string shaders
                    break;

                default:
                    GVK_ASSERT ("unknown shader read mode");
                    break;
            }

            if (newShader.shader != nullptr) {
                currentShader = std::move (newShader);
            }
        }
    });
}


void ShaderPipeline::IterateShaders (const std::function<void (ShaderModule&)>& func) const
{
    if (vertexShader.shader) {
        func (*vertexShader.shader);
    }
    if (fragmentShader.shader) {
        func (*fragmentShader.shader);
    }
    if (geometryShader.shader) {
        func (*geometryShader.shader);
    }
    if (tessellationEvaluationShader.shader) {
        func (*tessellationEvaluationShader.shader);
    }
    if (tessellationControlShader.shader) {
        func (*tessellationControlShader.shader);
    }
    if (computeShader.shader) {
        func (*computeShader.shader);
    }
}