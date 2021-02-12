#include "SequenceAdapter.hpp"

// from GearsVk
#include "Assert.hpp"
#include "GLFWWindow.hpp"
#include "GraphRenderer.hpp"
#include "StimulusAdapterView.hpp"
#include "Surface.hpp"
#include "VulkanEnvironment.hpp"

// from Gears
#include "core/Pass.h"
#include "core/Sequence.h"
#include "core/Stimulus.h"

// from std
#include <algorithm>
#include <map>

static bool IsEquivalentStimulus (const Stimulus& left, const Stimulus& right)
{
    if (left.passes.size () != right.passes.size ()) {
        return false;
    }

    for (size_t i = 0; i < left.passes.size (); ++i) {
        const Pass& leftPass  = *left.passes[i];
        const Pass& rightPass = *right.passes[i];

        if (leftPass.rasterizationMode != rightPass.rasterizationMode) {
            return false;
        }

        const std::string leftVert = leftPass.getStimulusGeneratorVertexShaderSource (leftPass.rasterizationMode);
        const std::string leftGeom = leftPass.getStimulusGeneratorGeometryShaderSource (leftPass.rasterizationMode);
        const std::string leftFrag = leftPass.getStimulusGeneratorShaderSource ();

        const std::string rightVert = rightPass.getStimulusGeneratorVertexShaderSource (rightPass.rasterizationMode);
        const std::string rightGeom = rightPass.getStimulusGeneratorGeometryShaderSource (rightPass.rasterizationMode);
        const std::string rightFrag = rightPass.getStimulusGeneratorShaderSource ();

        if (leftVert != rightVert) {
            return false;
        }
        if (leftGeom != rightGeom) {
            return false;
        }
        if (leftFrag != rightFrag) {
            return false;
        }

        if (leftPass.shaderColors != rightPass.shaderColors) {
            return false;
        }
        if (leftPass.shaderVariables != rightPass.shaderVariables) {
            return false;
        }
        if (leftPass.shaderVectors != rightPass.shaderVectors) {
            return false;
        }
    }

    return left.sequence->maxRandomGridWidth == right.sequence->maxRandomGridWidth &&
           left.sequence->maxRandomGridHeight == right.sequence->maxRandomGridHeight &&
           left.requiresClearing == right.requiresClearing &&
           left.clearColor == right.clearColor &&
           left.usesForwardRendering == right.usesForwardRendering &&
           left.randomGridWidth == right.randomGridWidth &&
           left.randomGridHeight == right.randomGridHeight &&

           left.toneMappingMode == right.toneMappingMode &&
           left.toneRangeMin == right.toneRangeMin &&
           left.toneRangeMax == right.toneRangeMax &&
           left.toneRangeMean == right.toneRangeMean &&
           left.toneRangeVar == right.toneRangeVar &&

           left.mono == right.mono &&
           left.sequence->fieldWidth_um == right.sequence->fieldWidth_um &&
           left.sequence->fieldHeight_um == right.sequence->fieldHeight_um &&
           left.doesDynamicToneMapping == right.doesDynamicToneMapping &&
           left.gammaSamplesCount == right.gammaSamplesCount &&
           memcmp (left.gamma, right.gamma, left.gammaSamplesCount) == 0 &&
           memcmp (left.temporalWeights, right.temporalWeights, 64) == 0 &&
           left.getRandomGeneratorShaderSource () == right.getRandomGeneratorShaderSource ();
}


SequenceAdapter::SequenceAdapter (GVK::VulkanEnvironment& environment, const Sequence::P& sequence)
    : sequence (sequence)
    , environment (environment)
{
    std::map<PtrC<Stimulus>, Ptr<StimulusAdapterView>> created;

    size_t stindex = 0;

    for (auto& [_, stim] : sequence->getStimuli ()) {
        Ptr<StimulusAdapterView> equivalentAdapter;
        for (const auto& [mappedStimulus, adapter] : created) {
            if (IsEquivalentStimulus (*mappedStimulus, *stim)) {
                equivalentAdapter = adapter;
                break;
            }
        }

        if (equivalentAdapter != nullptr) {
            views[stim] = equivalentAdapter;
        } else {
            views[stim]   = Make<StimulusAdapterView> (environment, stim);
            created[stim] = views[stim];
        }

        ++stindex;
    }


    firstFrameMs = 0;
    lastNs       = GVK::TimePoint::SinceApplicationStart ().AsMilliseconds ();
    obs.Observe (presentedFrameIndexEvent, [&] (uint32_t idx) {
        const double frameDisplayRateMs = 1.0 / 60.0 * 1000.0;

        const auto nowT  = GVK::TimePoint::SinceApplicationStart ();
        const auto nowNs = nowT.AsMilliseconds ();
        std::cout << "frame index shown: " << idx << " (ns: " << nowNs << ", delta ns: " << (nowNs - lastNs) << ", delta from expected: " << ((idx * frameDisplayRateMs) - nowNs) << ")" << std::endl;
        lastNs = nowNs;
    });
}


void SequenceAdapter::RenderFrameIndex (const uint32_t frameIndex)
{
    if (GVK_ERROR (renderer == nullptr)) {
        return;
    }

    if (frameIndex == 1) {
        firstFrameMs = GVK::TimePoint::SinceApplicationStart ().AsMilliseconds ();
        std::cout << "firstFrameMs = " << firstFrameMs << std::endl;
    }

    Stimulus::CP stim = sequence->getStimulusAtFrame (frameIndex);
    if (GVK_VERIFY (stim != nullptr)) {
        views[stim]->RenderFrameIndex (*renderer, currentPresentable, stim, frameIndex, presentedFrameIndexEvent);
        lastRenderedFrameIndex = frameIndex;
    }
}


void SequenceAdapter::Wait ()
{
    if (GVK_VERIFY (renderer != nullptr)) {
        renderer->Wait ();
    }
}

void SequenceAdapter::SetCurrentPresentable (Ptr<GVK::Presentable> presentable)
{
    currentPresentable = presentable;

    for (auto& [stim, view] : views) {
        view->CreateForPresentable (currentPresentable);
    }

    renderer = Make<GVK::RG::SynchronizedSwapchainGraphRenderer> (*environment.deviceExtra, presentable->GetSwapchain ());
}


Ptr<GVK::Presentable> SequenceAdapter::GetCurrentPresentable ()
{
    return currentPresentable;
}


void SequenceAdapter::RenderFullOnExternalWindow ()
{
    U<GVK::Window> window = Make<GVK::HiddenGLFWWindow> ();

    SetCurrentPresentable (Make<GVK::Presentable> (environment, *window));

    window->Show ();

    uint32_t frameIndex = 0;

    window->DoEventLoop ([&] (bool& shouldStop) {
        RenderFrameIndex (frameIndex);

        frameIndex++;

        if (frameIndex == sequence->getDuration ()) {
            shouldStop = true;
        }
    });

    window->Close ();
}