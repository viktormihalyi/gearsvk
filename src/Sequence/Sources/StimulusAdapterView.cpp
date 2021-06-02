#include "StimulusAdapterView.hpp"

// from GearsVk
#include "Assert.hpp"
#include "GearsVk/RenderGraph/GraphRenderer.hpp"
#include "GearsVk/VulkanEnvironment.hpp"

// from Gears
#include "StimulusAdapter.hpp"
#include "Stimulus.h"


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

    compiledAdapters[presentable] = std::make_unique<StimulusAdapter> (environment, presentable, stimulus);
}


void StimulusAdapterView::DestroyForPresentable (const std::shared_ptr<GVK::Presentable>& presentable)
{
    // TODO use
    auto adapter = std::find_if (compiledAdapters.begin (), compiledAdapters.end (), [&] (const auto& x) { return x.first == presentable; });
    if (GVK_ERROR (adapter != compiledAdapters.end ())) {
        return;
    }

    compiledAdapters.erase (adapter);
}


void StimulusAdapterView::RenderFrameIndex (GVK::RG::Renderer&                     renderer,
                                            std::shared_ptr<GVK::Presentable>&     presentable,
                                            const std::shared_ptr<Stimulus const>& stimulus,
                                            const uint32_t                         frameIndex,
                                            GVK::RG::IFrameDisplayObserver&        frameDisplayObserver,
                                            IRandomExporter&                       randomExporter)
{
    if (GVK_ERROR (compiledAdapters.find (presentable) == compiledAdapters.end ())) {
        return;
    }

    compiledAdapters[presentable]->RenderFrameIndex (renderer, stimulus, frameIndex, frameDisplayObserver, randomExporter);
}
