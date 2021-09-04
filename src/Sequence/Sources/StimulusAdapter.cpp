#include "StimulusAdapter.hpp"

// from Gears
#include "Pass.h"
#include "Sequence.h"
#include "Stimulus.h"

// from Utils
#include "Utils/CommandLineFlag.hpp"
#include "Utils/Assert.hpp"
#include "Utils/FileSystemUtils.hpp"

// from VulkanWrapper
#include "VulkanWrapper/Pipeline.hpp"
#include "VulkanWrapper/PipelineLayout.hpp"
#include "VulkanWrapper/RenderPass.hpp"
#include "VulkanWrapper/DescriptorSet.hpp"
#include "VulkanWrapper/DescriptorPool.hpp"
#include "VulkanWrapper/DescriptorSetLayout.hpp"

// from RenderGraph
#include "RenderGraph/DrawRecordable/DrawRecordable.hpp"
#include "RenderGraph/DrawRecordable/DrawRecordableInfo.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "RenderGraph/GraphSettings.hpp"
#include "RenderGraph/Operation.hpp"
#include "RenderGraph/RenderGraph.hpp"
#include "RenderGraph/Resource.hpp"
#include "RenderGraph/UniformReflection.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"
#include "RenderGraph/ShaderPipeline.hpp"

// from std
#include <random>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>


constexpr bool LogDebugInfo = false;
constexpr bool LogUniformDebugInfo = false;


StimulusAdapter::StimulusAdapter (const RG::VulkanEnvironment&          environment,
                                  std::shared_ptr<RG::Presentable>&     presentable,
                                  const std::shared_ptr<Stimulus const>& stimulus)
    : environment { environment }
    , presentable { presentable }
{
    renderGraph = std::make_unique<RG::RenderGraph> ();

    RG::GraphSettings s (*environment.deviceExtra, presentable->GetSwapchain ().GetImageCount ());

    std::shared_ptr<RG::SwapchainImageResource> presented = std::make_unique<RG::SwapchainImageResource> (*presentable);

    presented->SetName ("Swapchain");
    presented->SetDebugInfo ("Made by StimulusAdapter.");

    std::vector<std::shared_ptr<Pass>> passes = stimulus->getPasses ();

    for (size_t i = 0; i < passes.size (); ++i) {
        const std::shared_ptr<Pass>& pass = passes[i];
        
        const bool firstPass = i == 0;
        const bool lastPass = i == passes.size () - 1;

        if constexpr (LogDebugInfo) {
            std::cout << pass->ToDebugString () << std::endl;
        }

        GVK_ASSERT (pass->rasterizationMode == Pass::RasterizationMode::fullscreen);

        const std::string vert = pass->getStimulusGeneratorVertexShaderSource (pass->rasterizationMode);
        const std::string geom = pass->getStimulusGeneratorGeometryShaderSource (pass->rasterizationMode);
        const std::string frag = pass->getStimulusGeneratorShaderSource ();

        std::unique_ptr<RG::ShaderPipeline> sequencePip = std::make_unique<RG::ShaderPipeline> (*environment.device);

        sequencePip->SetVertexShaderFromString (vert);
        if (!geom.empty ()) {
            sequencePip->SetShaderFromSourceString (GVK::ShaderKind::Geometry, geom);
        }
        sequencePip->SetFragmentShaderFromString (frag);

        std::shared_ptr<RG::RenderOperation> passOperation = std::make_unique<RG::RenderOperation> (
            std::make_unique<RG::DrawRecordableInfo> (1, 6), std::move (sequencePip), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        passOperation->SetName (pass->name);
        passOperation->SetDebugInfo (pass->brief);

        //if (stimulus->requiresClearing) {
        //    passOperation->compileSettings.clearColor = { stimulus->clearColor, 1.0 };
        //}
        //
        //if (!firstPass) {
        //    passOperation->compileSettings.clearColor = std::nullopt;
        //}

        passOperation->compileSettings.blendEnabled = true;

        GVK_ASSERT (!stimulus->usesForwardRendering);
        //GVK_ASSERT (stimulus->mono);

        auto aTable2 = passOperation->compileSettings.GetAttachmentProvider<RG::RenderOperation::AttachmentDataTable> ();
        aTable2->table.push_back ({ "presented", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), firstPass ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD, presented->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, presented->GetFinalLayout () } });

        s.connectionSet.Add (passOperation, presented);

        passToOperation[pass] = passOperation;
    }

    std::optional<uint32_t> randomBinding;

    RG::ImageMap imgMap = RG::CreateEmptyImageResources (s.connectionSet, [&] (const SR::Sampler& sampler) -> std::optional<RG::CreateParams> {
        if (sampler.name == "gamma") {
            return std::make_tuple (glm::uvec3 { 256, 0, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
        }

        if (sampler.name == "randoms") {
            randomBinding = sampler.binding;
            return std::nullopt;
        }

        return std::nullopt;
    });

    // TODO 0 means viewport size
    if (stimulus->sequence->maxRandomGridWidth > 0 && stimulus->sequence->maxRandomGridHeight > 0 && randomBinding.has_value ()) {
        randomTexture = std::make_unique<RG::WritableImageResource> (
            VK_FILTER_NEAREST,
            stimulus->sequence->maxRandomGridWidth,
            stimulus->sequence->maxRandomGridHeight,
            1,
            VK_FORMAT_R32G32B32A32_UINT);

        randomTexture->SetName ("RandomTexture");
        randomTexture->SetDebugInfo ("Made by StimulusAdapter.");

        std::unique_ptr<RG::ShaderPipeline> randoSeqPipeline = std::make_unique<RG::ShaderPipeline> (*environment.device);
        randoSeqPipeline->SetVertexShaderFromString (*Utils::ReadTextFile (std::filesystem::current_path () / "Project" / "Shaders" / "quad.vert"));
        randoSeqPipeline->SetFragmentShaderFromString (stimulus->getRandomGeneratorShaderSource ());

        randomGeneratorOperation = std::make_unique<RG::RenderOperation> (std::make_unique<RG::DrawRecordableInfo> (1, 4), std::move (randoSeqPipeline), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

        std::static_pointer_cast<RG::RenderOperation> (randomGeneratorOperation)->compileSettings.blendEnabled = false;

        auto aTable2 = std::dynamic_pointer_cast<RG::RenderOperation> (randomGeneratorOperation)->compileSettings.GetAttachmentProvider<RG::RenderOperation::AttachmentDataTable> ();
        aTable2->table.push_back ({ "nextElement", GVK::ShaderKind::Fragment, { randomTexture->GetFormatProvider (), VK_ATTACHMENT_LOAD_OP_CLEAR, randomTexture->GetImageViewForFrameProvider (), randomTexture->GetInitialLayout (), randomTexture->GetFinalLayout () } });

        s.connectionSet.Add (randomGeneratorOperation, randomTexture);

        GVK_ASSERT (randomBinding.has_value ());
        s.connectionSet.Add (randomTexture, passToOperation[passes[0]]);
        auto table = std::dynamic_pointer_cast<RG::RenderOperation> (passToOperation[passes[0]])->compileSettings.GetDescriptorWriteInfoProvider<GVK::ShaderModule::Reflection::DescriptorWriteInfoTable> ();
        table->imageInfos.push_back ({ "randoms", GVK::ShaderKind::Fragment, randomTexture->GetSamplerProvider (), randomTexture->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
    }


    std::shared_ptr<RG::ReadOnlyImageResource> gammaTexture = imgMap.FindByName ("gamma");
    GVK_ASSERT (gammaTexture != nullptr);

    // this is a one time compile resource, which doesnt use framesinflight attrib
    gammaTexture->Compile (RG::GraphSettings (*environment.deviceExtra, 0));

    std::vector<float> gammaAndTemporalWeights (256, 0.f);
    for (int i = 0; i < 101; i++)
        gammaAndTemporalWeights[i] = stimulus->gamma[i];
    for (int i = 0; i < 64; i++)
        gammaAndTemporalWeights[128 + i] = stimulus->temporalWeights[i];
    gammaTexture->CopyTransitionTransfer (gammaAndTemporalWeights);

    reflection = std::make_unique<RG::UniformReflection> (s.connectionSet);

    renderGraph->Compile (std::move (s));

    SetConstantUniforms ();
}


void StimulusAdapter::SetConstantUniforms ()
{
    for (auto& [pass, op] : passToOperation) {
        for (auto& [name, value] : pass->shaderVariables)
            (*reflection)[op->GetUUID ()][GVK::ShaderKind::Fragment]["commonUniformBlock"][name] = static_cast<float> (value);
        for (auto& [name, value] : pass->shaderVectors)
            (*reflection)[op->GetUUID ()][GVK::ShaderKind::Fragment]["commonUniformBlock"][name] = static_cast<glm::vec2> (value);
        for (auto& [name, value] : pass->shaderColors)
            (*reflection)[op->GetUUID ()][GVK::ShaderKind::Fragment]["commonUniformBlock"][name] = static_cast<glm::vec3> (value);
    }
}


void StimulusAdapter::SetUniforms (const GVK::UUID& renderOperationId, const std::shared_ptr<Stimulus const>& stimulus, const uint32_t frameIndex)
{
    RG::UniformReflection::ShaderUniforms& vertexShaderUniforms   = (*reflection)[renderOperationId][GVK::ShaderKind::Vertex];
    RG::UniformReflection::ShaderUniforms& fragmentShaderUniforms = (*reflection)[renderOperationId][GVK::ShaderKind::Fragment];

    const double deviceRefreshRateDefault = 60.0;
    const double deviceRefreshRate        = presentable->GetRefreshRate ().value_or (deviceRefreshRateDefault);

    const double timeInSeconds = frameIndex / deviceRefreshRate;

    const glm::vec2 patternSizeOnRetina (presentable->GetSwapchain ().GetWidth (), presentable->GetSwapchain ().GetHeight ());

    vertexShaderUniforms["PatternSizeOnRetina"] = patternSizeOnRetina;

    fragmentShaderUniforms["commonUniformBlock"]["time"]                = static_cast<float> (timeInSeconds - stimulus->getStartingFrame () / deviceRefreshRate);
    fragmentShaderUniforms["commonUniformBlock"]["patternSizeOnRetina"] = patternSizeOnRetina;
    fragmentShaderUniforms["commonUniformBlock"]["frame"]               = static_cast<int32_t> (frameIndex);

    fragmentShaderUniforms["commonUniformBlock"]["swizzleForFft"] = 0xffffffff;

    if (stimulus->sequence->maxRandomGridWidth > 0 && stimulus->sequence->maxRandomGridHeight > 0) {
        if (fragmentShaderUniforms.Contains ("randomUniformBlock")) {
            fragmentShaderUniforms["randomUniformBlock"]["cellSize"] = glm::vec2 (
                stimulus->sequence->fieldWidth_um / stimulus->randomGridWidth,
                stimulus->sequence->fieldHeight_um / stimulus->randomGridHeight);
            fragmentShaderUniforms["randomUniformBlock"]["randomGridSize"] = glm::ivec2 (
                stimulus->sequence->maxRandomGridWidth,
                stimulus->sequence->maxRandomGridHeight);
        }
    }

    if (stimulus->doesToneMappingInStimulusGenerator) {
        fragmentShaderUniforms["toneMapping"]["toneRangeMin"] = stimulus->toneRangeMin;
        fragmentShaderUniforms["toneMapping"]["toneRangeMax"] = stimulus->toneRangeMax;

        if (stimulus->toneMappingMode == Stimulus::ToneMappingMode::ERF) {
            fragmentShaderUniforms["toneMapping"]["toneRangeMean"] = stimulus->toneRangeMean;
            fragmentShaderUniforms["toneMapping"]["toneRangeVar"]  = stimulus->toneRangeVar;
        } else {
            fragmentShaderUniforms["toneMapping"]["toneRangeMean"] = 0.f;
            fragmentShaderUniforms["toneMapping"]["toneRangeVar"]  = -1.f;
        }

        fragmentShaderUniforms["toneMapping"]["doTone"]           = static_cast<int32_t> (!stimulus->doesDynamicToneMapping);
        fragmentShaderUniforms["toneMapping"]["doGamma"]          = static_cast<int32_t> (!stimulus->doesDynamicToneMapping);
        fragmentShaderUniforms["toneMapping"]["gammaSampleCount"] = static_cast<int32_t> (stimulus->gammaSamplesCount);
    }
}


void StimulusAdapter::RenderFrameIndex (RG::Renderer&                     renderer,
                                        const std::shared_ptr<Stimulus const>& stimulus,
                                        const uint32_t                         frameIndex,
                                        RG::IFrameDisplayObserver&        frameDisplayObserver,
                                        IRandomExporter&                       randomExporter)
{
    const uint32_t stimulusStartingFrame = stimulus->getStartingFrame ();
    const uint32_t stimulusEndingFrame   = stimulus->getStartingFrame () + stimulus->getDuration ();

    if (GVK_ERROR (frameIndex < stimulusStartingFrame || frameIndex >= stimulusEndingFrame)) {
        return;
    }

    GVK::EventObserver obs;
    obs.Observe (renderer.preSubmitEvent, [&] (RG::RenderGraph& graph, uint32_t swapchainImageIndex, uint64_t timeNs) {
        for (auto& [pass, renderOp] : passToOperation) {
            SetUniforms (renderOp->GetUUID (), stimulus, frameIndex);
        }

        if (randomGeneratorOperation != nullptr) {
            auto& fragmentShaderUniforms = (*reflection)[randomGeneratorOperation->GetUUID ()][GVK::ShaderKind::Fragment];
            if (fragmentShaderUniforms.Contains ("ubo_seed")) {
                fragmentShaderUniforms["ubo_seed"] = static_cast<uint32_t> (frameIndex);
            }
            if (fragmentShaderUniforms.Contains ("ubo_lcg")) {
                fragmentShaderUniforms["ubo_lcg"]["frameIndex"] = static_cast<uint32_t> (frameIndex);
                fragmentShaderUniforms["ubo_lcg"]["seed"]       = static_cast<uint32_t> (89236738);
                fragmentShaderUniforms["ubo_lcg"]["gridSizeX"]  = static_cast<uint32_t> (stimulus->sequence->maxRandomGridWidth);
                fragmentShaderUniforms["ubo_lcg"]["gridSizeY"]  = static_cast<uint32_t> (stimulus->sequence->maxRandomGridHeight);
            }
        }

        if constexpr (LogUniformDebugInfo) {
            reflection->PrintDebugInfo ();
        }

        reflection->Flush (swapchainImageIndex);
    });

    const uint32_t resFrameIndex = renderer.RenderNextFrame (*renderGraph, frameDisplayObserver);

    if (randomExporter.IsEnabled ()) {
        if (randomTexture != nullptr) {
            environment.Wait ();
            randomExporter.OnRandomTextureDrawn (*randomTexture, resFrameIndex, frameIndex);
        }
    }
}
