#include "StimulusAdapterView.hpp"

// from RenderGraph
#include "RenderGraph/Utils/Assert.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "RenderGraph/VulkanEnvironment.hpp"

// from Gears
#include "StimulusAdapter.hpp"
#include "Stimulus.h"


StimulusAdapterView::StimulusAdapterView (RG::VulkanEnvironment& environment, const std::shared_ptr<Stimulus const>& stimulus)
    : environment (environment)
    , stimulus (stimulus)
{
}


void StimulusAdapterView::CreateForPresentable (std::shared_ptr<RG::Presentable>& presentable)
{
    const bool contains = std::find_if (compiledAdapters.begin (), compiledAdapters.end (), [&] (const auto& x) { return x.first == presentable; }) != compiledAdapters.end ();
    if (contains) {
        return;
    }

    compiledAdapters[presentable] = std::make_unique<StimulusAdapter> (environment, *presentable, stimulus);
}


void StimulusAdapterView::DestroyForPresentable (const std::shared_ptr<RG::Presentable>& presentable)
{
    // TODO use
    auto adapter = std::find_if (compiledAdapters.begin (), compiledAdapters.end (), [&] (const auto& x) { return x.first == presentable; });
    if (adapter == compiledAdapters.end ()) {
        return;
    }

    compiledAdapters.erase (adapter);
}


void StimulusAdapterView::RenderFrameIndex (RG::Renderer&                          renderer,
                                            std::shared_ptr<RG::Presentable>&      presentable,
                                            const std::shared_ptr<Stimulus const>& stimulus,
                                            const uint32_t                         frameIndex,
                                            RG::IFrameDisplayObserver&             frameDisplayObserver,
                                            IRandomExporter&                       randomExporter)
{
    if (GVK_ERROR (compiledAdapters.find (presentable) == compiledAdapters.end ())) {
        return;
    }

    compiledAdapters[presentable]->RenderFrameIndex (renderer, stimulus, frameIndex, frameDisplayObserver, randomExporter);
}
