#include "SequenceAdapter.hpp"

// from GearsVk
#include "GLFWWindow.hpp"
#include "StimulusAdapterView.hpp"

// from Gears
#include "VulkanEnvironment.hpp"


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
    auto stim = sequence->getStimulusAtFrame (frameIndex);
    if (stim) {
        views[stim]->RenderFrameIndex (currentPresentable, frameIndex);
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