#include "ShaderPipeline.hpp"

#include "Utils/MultithreadedFunction.hpp"
#include "Utils/Timer.hpp"
#include "Utils/BuildType.hpp"

namespace GVK {

namespace RG {

std::unique_ptr<ShaderModule>& ShaderPipeline::GetShaderByIndex (uint32_t index)
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


std::unique_ptr<ShaderModule>& ShaderPipeline::GetShaderByExtension (const std::string& extension)
{
    if (extension == ".vert") {
        return vertexShader;
    } else if (extension == ".frag") {
        return fragmentShader;
    } else if (extension == ".geom") {
        return geometryShader;
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


std::unique_ptr<ShaderModule>& ShaderPipeline::GetShaderByKind (ShaderKind kind)
{
    switch (kind) {
        case ShaderKind::Vertex: return vertexShader;
        case ShaderKind::Fragment: return fragmentShader;
        case ShaderKind::TessellationControl: return tessellationControlShader;
        case ShaderKind::TessellationEvaluation: return tessellationEvaluationShader;
        case ShaderKind::Geometry: return geometryShader;
        case ShaderKind::Compute: return computeShader;
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


ShaderPipeline::ShaderPipeline (VkDevice device, const std::vector<std::pair<ShaderKind, std::string>>& sources)
    : ShaderPipeline (device)
{
    for (auto [kind, source] : sources) {
        GetShaderByKind (kind) = ShaderModule::CreateFromGLSLString (device, kind, source);
    }
}


std::vector<VkPipelineShaderStageCreateInfo> ShaderPipeline::GetShaderStages () const
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


void ShaderPipeline::SetShaderFromSourceString (ShaderKind shaderKind, const std::string& source, ShaderPreprocessor& preprocessor)
{
    GVK_ASSERT (GetShaderByKind (shaderKind) == nullptr);
    GetShaderByKind (shaderKind) = ShaderModule::CreateFromGLSLString (device, shaderKind, source, preprocessor);
}


void ShaderPipeline::SetVertexShaderFromString (const std::string& source, ShaderPreprocessor& preprocessor)
{
    SetShaderFromSourceString (ShaderKind::Vertex, source, preprocessor);
}


void ShaderPipeline::SetFragmentShaderFromString (const std::string& source, ShaderPreprocessor& preprocessor)
{
    SetShaderFromSourceString (ShaderKind::Fragment, source, preprocessor);
}


void ShaderPipeline::SetShaderFromSourceFile (const std::filesystem::path& shaderPath)
{
    // assert on overwriting shader
    GVK_ASSERT (GetShaderByExtension (shaderPath.extension ().string ()) == nullptr);

    GetShaderByExtension (shaderPath.extension ().string ()) = ShaderModule::CreateFromGLSLFile (device, shaderPath);
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

    const auto instancedVertexProvider = [] (const std::string&) { return false; };

    const std::vector<VkVertexInputAttributeDescription> attribs  = vertexShader->GetReflection ().GetVertexAttributes (instancedVertexProvider);
    const std::vector<VkVertexInputBindingDescription>   bindings = vertexShader->GetReflection ().GetVertexBindings (instancedVertexProvider);

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t> (settings.attachmentReferences.size ());
    subpass.pColorAttachments    = settings.attachmentReferences.data ();

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.dependencyFlags     = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = 0;

    compileResult.renderPass     = std::unique_ptr<RenderPass> (new RenderPass (device, settings.attachmentDescriptions, { subpass }, {  dependency  }));
    compileResult.pipelineLayout = std::unique_ptr<PipelineLayout> (new PipelineLayout (device, { settings.layout }));
    compileResult.pipeline       = std::unique_ptr<Pipeline> (new Pipeline (
        device,
        settings.width,
        settings.height,
        static_cast<uint32_t> (settings.attachmentReferences.size ()),
        *compileResult.pipelineLayout,
        *compileResult.renderPass,
        GetShaderStages (),
        bindings,
        attribs,
        settings.topology,
        compileSettings.blendEnabled.has_value () ? *compileSettings.blendEnabled : true));
}


void ShaderPipeline::Reload ()
{
    Utils::DebugTimerLogger tl ("reloading shaders");
    Utils::TimerScope       ts (tl);

    MultithreadedFunction reloader (5, [&] (uint32_t, uint32_t threadIndex) {
        std::unique_ptr<ShaderModule>& currentShader = GetShaderByIndex (threadIndex);
        std::unique_ptr<ShaderModule>  newShader;

        if (currentShader != nullptr) {
            switch (currentShader->GetReadMode ()) {
                case ShaderModule::ReadMode::GLSLFilePath:
                    try {
                        newShader = ShaderModule::CreateFromGLSLFile (device, currentShader->GetLocation ());
                    } catch (ShaderCompileException&) {
                    }
                    break;

                case ShaderModule::ReadMode::SPVFilePath:
                    try {
                        newShader = ShaderModule::CreateFromSPVFile (device, currentShader->GetShaderKind (), currentShader->GetLocation ());
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

            if (newShader != nullptr) {
                currentShader = std::move (newShader);
            }
        }
    });
}


void ShaderPipeline::IterateShaders (const std::function<void (ShaderModule&)>& func) const
{
    if (vertexShader) {
        func (*vertexShader);
    }
    if (fragmentShader) {
        func (*fragmentShader);
    }
    if (geometryShader) {
        func (*geometryShader);
    }
    if (tessellationEvaluationShader) {
        func (*tessellationEvaluationShader);
    }
    if (tessellationControlShader) {
        func (*tessellationControlShader);
    }
    if (computeShader) {
        func (*computeShader);
    }
}


std::unique_ptr<DescriptorSetLayout> ShaderPipeline::CreateDescriptorSetLayout (VkDevice device) const
{
    std::vector<VkDescriptorSetLayoutBinding> layout;

    std::vector<std::string> shaderSources;

    if constexpr (IsDebugBuild) {
        IterateShaders ([&] (ShaderModule& shaderModule) {
            shaderSources.push_back (shaderModule.GetSourceCode ());
        });
    }

    IterateShaders ([&] (ShaderModule& shaderModule) {
        auto layoutPart = shaderModule.GetReflection ().GetLayout ();
        layout.insert (layout.end (), layoutPart.begin (), layoutPart.end ());
    });

    return std::make_unique<DescriptorSetLayout> (device, layout);
}

} // namespace RG

} // namespace GVK
