#include "StimulusAdapterForPresentable.hpp"

// from Gears
#include "core/Sequence.h"

// from GearsVkUtils
#include "Assert.hpp"

// from GearsVk
#include "DrawRecordableInfo.hpp"
#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "UniformReflection.hpp"
#include "VulkanEnvironment.hpp"

// from std
#include <random>
#include <string>


StimulusAdapterForPresentable::StimulusAdapterForPresentable (const VulkanEnvironment& environment, Ptr<Presentable>& presentable, const Stimulus::CP& stimulus)
    : stimulus (stimulus)
    , presentable (presentable)
{
    renderGraph = RG::RenderGraph::Create ();

    RG::GraphSettings s (*environment.deviceExtra, presentable->GetSwapchain ().GetImageCount ());

    Ptr<RG::SwapchainImageResource> presented = RG::SwapchainImageResource::Create (*presentable);

    renderer = RG::SynchronizedSwapchainGraphRenderer::Create (*environment.deviceExtra, presentable->GetSwapchain ());

    const std::vector<Pass::P> passes = stimulus->getPasses ();

    GVK_ASSERT (passes.size () == 1);

    for (const Pass::P& pass : passes) {
        GVK_ASSERT (pass->rasterizationMode == Pass::RasterizationMode::fullscreen);
        const std::string vert = pass->getStimulusGeneratorVertexShaderSource (Pass::RasterizationMode::fullscreen);
        const std::string geom = pass->getStimulusGeneratorGeometryShaderSource (Pass::RasterizationMode::fullscreen);
        const std::string frag = pass->getStimulusGeneratorShaderSource ();

        /*
        ShaderPipelineP randomPip = ShaderPipeline::Create (*environment.device);
        randomPip->SetVertexShaderFromString (*Utils::ReadTextFile (PROJECT_ROOT / "src" / "UserInterface" / "Project" / "Shaders" / "quad.vert"));
        randomPip->SetFragmentShaderFromString (stim->getRandomGeneratorShaderSource ());

        {
            RG::RenderGraph randomGenerator;

            auto randomResourceHistory = randomGenerator.CreateResource<SingleWritableImageResource> (VK_FILTER_LINEAR, 256, 256, 5, VK_FORMAT_R8G8B8A8_UINT);
            auto randomResourceOUT     = randomGenerator.CreateResource<SingleWritableImageResource> (VK_FILTER_LINEAR, 256, 256, 1, VK_FORMAT_R8G8B8A8_UINT);
            auto randomRender          = randomGenerator.CreateOperation<RenderOperation> (DrawRecordableInfo::Create (1, 4), randomPip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
            auto backTransfer1         = randomGenerator.CreateOperation<TransferOperation> ();
            auto backTransfer2         = randomGenerator.CreateOperation<TransferOperation> ();
            auto backTransfer3         = randomGenerator.CreateOperation<TransferOperation> ();
            auto backTransfer4         = randomGenerator.CreateOperation<TransferOperation> ();

            randomGenerator.CreateInputConnection (*randomRender, *randomResourceHistory, ImageInputBinding::Create (0, *randomResourceHistory));
            randomGenerator.CreateInputConnection (*randomRender, *randomResourceHistory, ImageInputBinding::Create (0, *randomResourceHistory));
            randomGenerator.CreateOutputConnection (*randomRender, 0, *randomResourceOUT);
        }
        */

        ShaderPipelineU sequencePip = ShaderPipeline::Create (*environment.device);

        sequencePip->SetVertexShaderFromString (vert);
        if (!geom.empty ()) {
            sequencePip->SetShaderFromSourceString (ShaderKind::Geometry, geom);
        }
        sequencePip->SetFragmentShaderFromString (frag);

        Ptr<RG::RenderOperation> passOperation = RG::RenderOperation::Create (
            DrawRecordableInfo::Create (1, 4), std::move (sequencePip), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

        if (stimulus->requiresClearing) {
            passOperation->compileSettings.clearColor = { stimulus->clearColor, 1.0 };
        }
        passOperation->compileSettings.blendEnabled = passes.size () > 1;

        s.connectionSet.Add (passOperation, presented,
                             RG::OutputBinding::Create (0,
                                                        presented->GetFormatProvider (),
                                                        presented->GetFinalLayout (),
                                                        VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                        VK_ATTACHMENT_STORE_OP_STORE));

        passToOperation[pass] = passOperation;
        break;
    }

    reflection = RG::UniformReflection::Create (s.connectionSet);

    RG::ImageMap imgMap = RG::CreateEmptyImageResources (s.connectionSet, [&] (const SR::Sampler& sampler) -> std::optional<RG::CreateParams> {
        if (sampler.name == "gamma") {
            return std::make_tuple (glm::uvec3 { 256, 0, 0 }, VK_FORMAT_R32_SFLOAT, VK_FILTER_NEAREST);
        }

        if (sampler.name == "randoms") {
            GVK_ASSERT (stimulus->sequence->maxRandomGridWidth > 0 && stimulus->sequence->maxRandomGridHeight > 0);
            return std::make_tuple (glm::uvec3 { stimulus->sequence->maxRandomGridWidth, stimulus->sequence->maxRandomGridHeight, 0 }, VK_FORMAT_R32G32B32A32_UINT, VK_FILTER_NEAREST);
        }

        return std::nullopt;
    });

    Ptr<RG::ReadOnlyImageResource> randomTexture = imgMap.FindByName ("randoms");
    if (randomTexture != nullptr) {
        randomTexture->Compile (RG::GraphSettings (*environment.deviceExtra, 0));

        std::random_device rd;
        std::mt19937_64    gen (rd ());

        std::uniform_int_distribution<uint32_t> dis;

        const size_t          randomsSize = randomTexture->GetImages ()[0]->GetWidth () * randomTexture->GetImages ()[0]->GetHeight () * 4;
        std::vector<uint32_t> randomValues (randomsSize);
        for (uint32_t i = 0; i < randomsSize; i++)
            randomValues[i] = dis (gen);
        for (uint32_t i = 0; i < randomTexture->GetImages ()[0]->GetArrayLayers (); i++)
            randomTexture->CopyLayer (randomValues, i);
    }

    Ptr<RG::ReadOnlyImageResource> gammaTexture = imgMap.FindByName ("gamma");
    GVK_ASSERT (gammaTexture != nullptr);

    // this is a one time compile resource, which doesnt use framesinflight attrib
    gammaTexture->Compile (RG::GraphSettings (*environment.deviceExtra, 0));

    std::vector<float> gammaAndTemporalWeights (256, 0.f);
    for (int i = 0; i < 101; i++)
        gammaAndTemporalWeights[i] = stimulus->gamma[i];
    for (int i = 0; i < 64; i++)
        gammaAndTemporalWeights[128 + i] = stimulus->temporalWeights[i];
    gammaTexture->CopyTransitionTransfer (gammaAndTemporalWeights);


    renderGraph->Compile (std::move (s));

    SetConstantUniforms ();
}


void StimulusAdapterForPresentable::SetConstantUniforms ()
{
    for (auto& [pass, op] : passToOperation) {
        for (auto& [name, value] : pass->shaderVariables)
            (*reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<float> (value);
        for (auto& [name, value] : pass->shaderVectors)
            (*reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec2> (value);
        for (auto& [name, value] : pass->shaderColors)
            (*reflection)[op->GetUUID ()][ShaderKind::Fragment][std::string ("ubo_" + name)] = static_cast<glm::vec3> (value);
    }
}

void StimulusAdapterForPresentable::SetUniforms (const GearsVk::UUID& renderOperationId, const uint32_t frameIndex)
{
    RG::UniformReflection::ShaderUniforms& vertexShaderUniforms   = (*reflection)[renderOperationId][ShaderKind::Vertex];
    RG::UniformReflection::ShaderUniforms& fragmentShaderUniforms = (*reflection)[renderOperationId][ShaderKind::Fragment];

    const double timeInSeconds = frameIndex / 60.0;

    const glm::vec2 patternSizeOnRetina (1920, 1080);

    vertexShaderUniforms["PatternSizeOnRetina"] = patternSizeOnRetina;

    fragmentShaderUniforms["ubo_time"]                = static_cast<float> (timeInSeconds - stimulus->getStartingFrame () / 60.f);
    fragmentShaderUniforms["ubo_patternSizeOnRetina"] = patternSizeOnRetina;
    fragmentShaderUniforms["ubo_frame"]               = static_cast<int32_t> (frameIndex);

    fragmentShaderUniforms["ubo_swizzleForFft"] = 0xffffffff;

    if (stimulus->sequence->maxRandomGridWidth > 0 && stimulus->sequence->maxRandomGridHeight > 0) {
        fragmentShaderUniforms["ubo_cellSize"] = static_cast<glm::vec2> (
            stimulus->sequence->fieldWidth_um / stimulus->randomGridWidth,
            stimulus->sequence->fieldHeight_um / stimulus->randomGridHeight);
        fragmentShaderUniforms["ubo_randomGridSize"] = static_cast<glm::ivec2> (
            stimulus->sequence->maxRandomGridWidth,
            stimulus->sequence->maxRandomGridHeight);
    }

    if (stimulus->doesToneMappingInStimulusGenerator) {
        fragmentShaderUniforms["ubo_toneRangeMin"] = stimulus->toneRangeMin;
        fragmentShaderUniforms["ubo_toneRangeMax"] = stimulus->toneRangeMax;

        if (stimulus->toneMappingMode == Stimulus::ToneMappingMode::ERF) {
            fragmentShaderUniforms["ubo_toneRangeMean"] = stimulus->toneRangeMean;
            fragmentShaderUniforms["ubo_toneRangeVar"]  = stimulus->toneRangeVar;
        } else {
            fragmentShaderUniforms["ubo_toneRangeMean"] = 0.f;
            fragmentShaderUniforms["ubo_toneRangeVar"]  = -1.f;
        }

        fragmentShaderUniforms["ubo_doTone"]           = static_cast<int32_t> (!stimulus->doesDynamicToneMapping);
        fragmentShaderUniforms["ubo_doGamma"]          = static_cast<int32_t> (!stimulus->doesDynamicToneMapping);
        fragmentShaderUniforms["ubo_gammaSampleCount"] = static_cast<int32_t> (stimulus->gammaSamplesCount);
    }
}

void StimulusAdapterForPresentable::RenderFrameIndex (const uint32_t frameIndex)
{
    const uint32_t stimulusStartingFrame = stimulus->getStartingFrame ();
    const uint32_t stimulusEndingFrame   = stimulus->getStartingFrame () + stimulus->getDuration ();

    if (GVK_ERROR (frameIndex < stimulusStartingFrame || frameIndex >= stimulusEndingFrame)) {
        return;
    }

    SingleEventObserver obs;
    obs.Observe (renderer->preSubmitEvent, [&] (RG::RenderGraph& graph, uint32_t swapchainImageIndex, uint64_t timeNs) {
        for (auto& [pass, renderOp] : passToOperation) {
            SetUniforms (renderOp->GetUUID (), frameIndex);
        }

        reflection->PrintDebugInfo ();

        reflection->Flush (swapchainImageIndex);
    });

    renderer->RenderNextFrame (*renderGraph);
}


void StimulusAdapterForPresentable::Wait ()
{
    renderer->Wait ();
}