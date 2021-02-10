#include "StimulusAdapterView.hpp"

// from GearsVk
#include "Assert.hpp"
#include "GraphRenderer.hpp"
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
    const bool contains = std::find_if (compiledAdapters.begin (), compiledAdapters.end (), [&] (const auto& x) { return x.first == presentable; }) != compiledAdapters.end ();
    if (contains) {
        return;
    }

    compiledAdapters[presentable] = StimulusAdapterForPresentable::Create (environment, presentable, stimulus);
}


void StimulusAdapterView::RenderFrameIndex (RG::Renderer& renderer, Ptr<Presentable>& presentable, const PtrC<Stimulus>& stimulus, const uint32_t frameIndex)
{
    if (GVK_ERROR (compiledAdapters.find (presentable) == compiledAdapters.end ())) {
        return;
    }

    compiledAdapters[presentable]->RenderFrameIndex (renderer, stimulus, frameIndex);
}
