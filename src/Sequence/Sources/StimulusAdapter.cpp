#include "StimulusAdapter.hpp"

// from Gears
#include "Pass.h"
#include "Sequence.h"
#include "Stimulus.h"

// from Utils
#include "Utils/CommandLineFlag.hpp"
#include "Utils/Assert.hpp"
#include "Utils/FileSystemUtils.hpp"
#include "Utils/Utils.hpp"

// from VulkanWrapper
#include "VulkanWrapper/GraphicsPipeline.hpp"
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

constexpr double deviceRefreshRateDefault = 60.0;


StimulusAdapter::StimulusAdapter (const RG::VulkanEnvironment&           environment,
                                  RG::Presentable&                       presentable,
                                  const std::shared_ptr<Stimulus const>& stimulus)
    : environment { environment }
    , patternSizeOnRetina { presentable.GetSwapchain ().GetWidth (), presentable.GetSwapchain ().GetHeight () }
    , deviceRefreshRate { presentable.GetRefreshRate ().value_or (deviceRefreshRateDefault) }
{
    renderGraph = std::make_unique<RG::RenderGraph> ();

    const uint32_t framesInFlight = presentable.GetSwapchain ().GetImageCount ();

    RG::GraphSettings s (*environment.deviceExtra, framesInFlight);

    std::shared_ptr<RG::SwapchainImageResource> presented = std::make_unique<RG::SwapchainImageResource> (presentable);

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

        auto& aTable2 = passOperation->compileSettings.attachmentProvider;
        aTable2->table.push_back ({ "presented", GVK::ShaderKind::Fragment, { presented->GetFormatProvider (), firstPass ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD, presented->GetImageViewForFrameProvider (), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, presented->GetFinalLayout () } });

        s.connectionSet.Add (passOperation, presented);

        passToOperation[pass] = passOperation;
    }

    RG::ImageMap imgMap = RG::CreateEmptyImageResources (s.connectionSet, [&] (const SR::Sampler& sampler) -> std::optional<RG::CreateParams> {
        if (sampler.name == "gamma") {
            return std::make_tuple (glm::uvec3 { 256, 0, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
        }

        return std::nullopt;
    });

    std::shared_ptr<RG::ComputeOperation> rngGen;
    if (!stimulus->rngCompute_shaderSource.empty ()) {

        const std::string preProcessedShaderSource = Utils::ReplaceAll (stimulus->rngCompute_shaderSource, "FRAMESINFLIGHT", [&] () -> std::string {
            return std::to_string (stimulus->rngCompute_multiLayer ? framesInFlight : 1);
        });

        rngGen = std::make_shared<RG::ComputeOperation> (stimulus->rngCompute_workGroupSizeX, stimulus->rngCompute_workGroupSizeY, 1);
        rngGen->SetName ("RNG_Compute");

        rngGen->compileSettings.computeShaderPipeline = std::make_unique<RG::ComputeShaderPipeline> (*environment.device, preProcessedShaderSource);

        auto randomBufferCreator = [&] (const std::shared_ptr<RG::Operation>&, const GVK::ShaderModule&, const std::shared_ptr<SR::BufferObject>& bufferObject, bool& treatAsOutput) -> std::shared_ptr<RG::DescriptorBindableBufferResource> {
            if (bufferObject->name == "OutputBuffer") {
                treatAsOutput = true;
                return std::make_unique<RG::GPUBufferResource> (bufferObject->GetFullSize ());
            }

            return nullptr;
        };

        s.connectionSet.Add (rngGen);
        RG::UniformReflection refl (s.connectionSet, randomBufferCreator);

        auto outputBuffer = s.connectionSet.GetByName<RG::GPUBufferResource> ("OutputBuffer");
        GVK_ASSERT (outputBuffer != nullptr);
        
        s.connectionSet.Add (rngGen, outputBuffer);

        auto rngUserOperation = passToOperation[passes[0]];
        s.connectionSet.Add (outputBuffer, rngUserOperation);
        
        auto& table = std::dynamic_pointer_cast<RG::RenderOperation> (rngUserOperation)->compileSettings.descriptorWriteProvider;
        table->bufferInfos.push_back ({ "RandomBuffer", GVK::ShaderKind::Fragment, outputBuffer->GetBufferForFrameProvider (), 0, outputBuffer->GetBufferSize () });

    
    }

    std::shared_ptr<RG::ReadOnlyImageResource> gammaTexture = imgMap.FindByName ("gamma");

    if (GVK_VERIFY (gammaTexture != nullptr)) {
        // this is a one time compile resource, which doesnt use framesinflight attrib
        gammaTexture->Compile (RG::GraphSettings (*environment.deviceExtra, 0));

        std::vector<float> gammaAndTemporalWeights (256, 0.f);
        for (int i = 0; i < 101; i++)
            gammaAndTemporalWeights[i] = stimulus->gamma[i];
        for (int i = 0; i < 64; i++)
            gammaAndTemporalWeights[128 + i] = stimulus->temporalWeights[i];
        gammaTexture->CopyTransitionTransfer (gammaAndTemporalWeights);
    }

    auto randomBufferSkipper = [&] (const std::shared_ptr<RG::Operation>& op, const GVK::ShaderModule& sm, const std::shared_ptr<SR::BufferObject>& bufferObject, bool& treatAsOutput) -> std::shared_ptr<RG::DescriptorBindableBufferResource> {
        if (bufferObject->name == "RandomBuffer") {
            return nullptr;
        }

        return RG::UniformReflection::DefaultResourceCreator (op, sm, bufferObject, treatAsOutput);
    };

    reflection = std::make_unique<RG::UniformReflection> (s.connectionSet, randomBufferSkipper);

    renderGraph->Compile (std::move (s));

    // set constant uniform values
    {
        for (auto& [pass, op] : passToOperation) {
            for (auto& [name, value] : pass->shaderVariables)
                (*reflection)[op->GetUUID ()][GVK::ShaderKind::Fragment]["commonUniformBlock"][name] = static_cast<float> (value);
            for (auto& [name, value] : pass->shaderVectors)
                (*reflection)[op->GetUUID ()][GVK::ShaderKind::Fragment]["commonUniformBlock"][name] = static_cast<glm::vec2> (value);
            for (auto& [name, value] : pass->shaderColors)
                (*reflection)[op->GetUUID ()][GVK::ShaderKind::Fragment]["commonUniformBlock"][name] = static_cast<glm::vec3> (value);
        }

        auto rngComputeOp = renderGraph->GetConnectionSet ().GetByName<RG::ComputeOperation> ("RNG_Compute");
        if (rngComputeOp != nullptr) {
            (*reflection)[rngComputeOp][GVK::ShaderKind::Compute]["RandomGeneratorConfig"]["seed"] = 7; // TODO RNG
            (*reflection)[rngComputeOp][GVK::ShaderKind::Compute]["RandomGeneratorConfig"]["framesInFlight"] = framesInFlight; // TODO RNG
        }
    }
}


void StimulusAdapter::SetUniforms (const GVK::UUID& renderOperationId, const std::shared_ptr<Stimulus const>& stimulus, const uint32_t resourceIndex, const uint32_t frameIndex)
{
    auto& vertexShaderUniforms   = (*reflection)[renderOperationId][GVK::ShaderKind::Vertex];
    auto& fragmentShaderUniforms = (*reflection)[renderOperationId][GVK::ShaderKind::Fragment];

    const double timeInSeconds = frameIndex / deviceRefreshRate;

    vertexShaderUniforms["PatternSizeOnRetina"] = patternSizeOnRetina;

    fragmentShaderUniforms["commonUniformBlock"]["time"]                = static_cast<float> (timeInSeconds - stimulus->getStartingFrame () / deviceRefreshRate);
    fragmentShaderUniforms["commonUniformBlock"]["patternSizeOnRetina"] = patternSizeOnRetina;
    fragmentShaderUniforms["commonUniformBlock"]["frame"]               = static_cast<int32_t> (frameIndex);

    fragmentShaderUniforms["commonUniformBlock"]["swizzleForFft"] = 0xffffffff;

    if (!stimulus->rngCompute_shaderSource.empty ()) {
        auto& computeRefl = (*reflection)[renderOperationId][GVK::ShaderKind::Fragment];
        if (GVK_VERIFY (computeRefl.Contains ("RandomBufferConfig"))) {
            auto rngComputeOp = renderGraph->GetConnectionSet ().GetByName<RG::ComputeOperation> ("RNG_Compute");
            GVK_ASSERT (rngComputeOp != nullptr);

            computeRefl["RandomBufferConfig"]["randoms_layerIndex"] = stimulus->rngCompute_multiLayer ? resourceIndex : 0;

            computeRefl["RandomBufferConfig"]["randomGridSize"] = glm::ivec2 (rngComputeOp->GetWorkGroupSizeX (), rngComputeOp->GetWorkGroupSizeX ());

            computeRefl["RandomBufferConfig"]["cellSize"] = glm::vec2 (
                stimulus->sequence->fieldWidth_um / rngComputeOp->GetWorkGroupSizeX (),
                stimulus->sequence->fieldHeight_um / rngComputeOp->GetWorkGroupSizeX ());
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


void StimulusAdapter::RenderFrameIndex (RG::Renderer&                          renderer,
                                        const std::shared_ptr<Stimulus const>& stimulus,
                                        const uint32_t                         frameIndex,
                                        RG::IFrameDisplayObserver&             frameDisplayObserver,
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
            SetUniforms (renderOp->GetUUID (), stimulus, renderer.GetNextRenderResourceIndex (), frameIndex);
        }

        auto rngComputeOp = renderGraph->GetConnectionSet ().GetByName<RG::ComputeOperation> ("RNG_Compute");
        if (rngComputeOp != nullptr) {
            auto& uniforms = (*reflection)[rngComputeOp][GVK::ShaderKind::Compute];

            if (GVK_VERIFY (uniforms.Contains ("RandomGeneratorConfig"))) {
                uniforms["RandomGeneratorConfig"]["startFrameIndex"]  = static_cast<uint32_t> (frameIndex);
                uniforms["RandomGeneratorConfig"]["nextElementIndex"] = static_cast<uint32_t> (frameIndex - 1) % renderGraph->graphSettings.framesInFlight;
            }
        }

        if constexpr (LogUniformDebugInfo) {
            reflection->PrintDebugInfo ();
        }

        reflection->Flush (swapchainImageIndex);
    });

    const uint32_t resFrameIndex = renderer.RenderNextFrame (*renderGraph, frameDisplayObserver);

    if (randomExporter.IsEnabled ()) {
        std::shared_ptr<RG::GPUBufferResource> outputBuffer = renderGraph->GetConnectionSet ().GetByName<RG::GPUBufferResource> ("OutputBuffer");
        if (outputBuffer != nullptr) {
            environment.Wait ();
            randomExporter.OnRandomTextureDrawn (*outputBuffer, resFrameIndex, frameIndex);
        }
    }
}
