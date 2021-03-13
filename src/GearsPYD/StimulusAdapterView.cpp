#include "StimulusAdapterView.hpp"

// from GearsVk
#include "Assert.hpp"
#include "GraphRenderer.hpp"
#include "VulkanEnvironment.hpp"

// from Gears
#include "StimulusAdapterForPresentable.hpp"
#include "core/Stimulus.h"


StimulusAdapterView::StimulusAdapterView (GVK::VulkanEnvironment& environment, const std::shared_ptr<Stimulus const>& stimulus)
    : environment (environment)
    , stimulus (stimulus)
{
}


void StimulusAdapterView::CreateForPresentable (std::shared_ptr<GVK::Presentable>& presentable)
{
    const bool contains = std::find_if (compiledAdapters.begin (), compiledAdapters.end (), [&] (const auto& x) { return x.first == presentable; }) != compiledAdapters.end ();
    if (contains) {
        return;
    }

    compiledAdapters[presentable] = std::make_unique<StimulusAdapterForPresentable> (environment, presentable, stimulus);
}


void StimulusAdapterView::RenderFrameIndex (GVK::RG::Renderer& renderer, std::shared_ptr<GVK::Presentable>& presentable, const std::shared_ptr<Stimulus const>& stimulus, const uint32_t frameIndex, GVK::Event<uint32_t>& frameIndexPresentedEvent)
{
    if (GVK_ERROR (compiledAdapters.find (presentable) == compiledAdapters.end ())) {
        return;
    }

    compiledAdapters[presentable]->RenderFrameIndex (renderer, stimulus, frameIndex, frameIndexPresentedEvent);
}
