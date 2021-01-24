#include "StimulusAdapterView.hpp"

// from GearsVk
#include "Assert.hpp"
#include "VulkanEnvironment.hpp"

// from Gears
#include "StimulusAdapterForPresentable.hpp"
#include "core/Stimulus.h"


StimulusAdapterView::StimulusAdapterView (VulkanEnvironment& environment, const PtrC<Stimulus>& stimulus)
    : environment (environment)
    , stimulus (stimulus)
{
}


void StimulusAdapterView::CreateForPresentable (Ptr<Presentable>& presentable)
{
    compiledAdapters[presentable] = StimulusAdapterForPresentable::Create (environment, presentable, stimulus);
}


void StimulusAdapterView::RenderFrameIndex (Ptr<Presentable>& presentable, const uint32_t frameIndex)
{
    if (GVK_ERROR (compiledAdapters.find (presentable) == compiledAdapters.end ())) {
        return;
    }

    compiledAdapters[presentable]->RenderFrameIndex (frameIndex);
}


uint32_t StimulusAdapterView::GetStartingFrame () const
{
    return stimulus->getStartingFrame ();
}


uint32_t StimulusAdapterView::GetEndingFrame () const
{
    return stimulus->getStartingFrame () + stimulus->getDuration ();
}