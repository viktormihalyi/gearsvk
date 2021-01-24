#include "SequenceAdapter.hpp"

// from GearsVk
#include "Assert.hpp"
#include "GLFWWindow.hpp"
#include "StimulusAdapterView.hpp"
#include "Surface.hpp"
#include "VulkanEnvironment.hpp"

// from Gears
#include "core/Sequence.h"
#include "core/Stimulus.h"


SequenceAdapter::SequenceAdapter (VulkanEnvironment& environment, const Sequence::P& sequence)
    : sequence (sequence)
    , environment (environment)
{
    for (auto& [startFrame, stim] : sequence->getStimuli ()) {
        views[stim] = StimulusAdapterView::Create (environment, stim);
    }
}


void SequenceAdapter::RenderFrameIndex (const uint32_t frameIndex)
{
    Stimulus::CP stim = sequence->getStimulusAtFrame (frameIndex);
    if (GVK_VERIFY (stim != nullptr)) {
        views[stim]->RenderFrameIndex (currentPresentable, frameIndex);
        lastRenderedFrameIndex = frameIndex;
    }
}


void SequenceAdapter::Wait ()
{
    if (GVK_ERROR (!lastRenderedFrameIndex.has_value ())) {
        return;
    }

    Stimulus::CP stim = sequence->getStimulusAtFrame (*lastRenderedFrameIndex);
    if (GVK_VERIFY (stim != nullptr)) {
        views[stim]->RenderFrameIndex (currentPresentable, *lastRenderedFrameIndex);
    }
}

void SequenceAdapter::SetCurrentPresentable (Ptr<Presentable> presentable)
{
    currentPresentable = presentable;

    for (auto& [stim, view] : views) {
        view->CreateForPresentable (currentPresentable);
    }
}


Ptr<Presentable> SequenceAdapter::GetCurrentPresentable ()
{
    return currentPresentable;
}


void SequenceAdapter::RenderFullOnExternalWindow ()
{
    WindowU window = HiddenGLFWWindow::Create ();

    SetCurrentPresentable (Presentable::Create (environment, *window));

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