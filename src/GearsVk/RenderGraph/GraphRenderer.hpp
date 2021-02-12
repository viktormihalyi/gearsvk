#ifndef GRAPHRENDERER_HPP
#define GRAPHRENDERER_HPP

#include "GearsVkAPI.hpp"

#include "DeviceExtra.hpp"

#include "Event.hpp"
#include "Ptr.hpp"
#include "Time.hpp"
#include "Window.hpp"

namespace GVK {

USING_PTR (Swapchain);
USING_PTR (Fence);
USING_PTR (Semaphore);

namespace RG {

class RenderGraph;
class GraphSettings;


class GVK_RENDERER_API Renderer {
public:
    Event<RenderGraph&, uint32_t, uint64_t> preSubmitEvent;
    Event<>                                 recreateEvent;
    Event<uint32_t>                         swapchainImageAcquiredEvent;
    Event<>                                 presentedEvent;

    virtual ~Renderer () = default;

    virtual void RenderNextFrame (RenderGraph& graph) = 0;

    Window::DrawCallback GetInfiniteDrawCallback (const std::function<RenderGraph&()>& graphProvider);

    Window::DrawCallback GetConditionalDrawCallback (const std::function<RenderGraph&()>& graphProvider, const std::function<bool ()>& shouldStop);

    Window::DrawCallback GetCountLimitedDrawCallback (const std::function<RenderGraph&()>& graphProvider, uint64_t limit);
};


class GVK_RENDERER_API RecreatableGraphRenderer : public Renderer {
protected:
    Swapchain& swapchain;

public:
    RecreatableGraphRenderer (Swapchain& swapchain);
    virtual ~RecreatableGraphRenderer () = default;

    void Recreate (RenderGraph& graph);

    void         RenderNextFrame (RenderGraph& graph) override;
    virtual void RenderNextRecreatableFrame (RenderGraph& graph) = 0;
};


class GVK_RENDERER_API BlockingGraphRenderer final : public RecreatableGraphRenderer {
private:
    SemaphoreU s;
    TimePoint  lastDrawTime;

public:
    BlockingGraphRenderer (const DeviceExtra& device, Swapchain& swapchain);

    void RenderNextRecreatableFrame (RenderGraph& graph) override;
};


USING_PTR (SynchronizedSwapchainGraphRenderer);
class GVK_RENDERER_API SynchronizedSwapchainGraphRenderer final : public RecreatableGraphRenderer {
private:
    // number of render operations able to run simultaneously
    // optimally equal to imageCount, but may be lower.
    // doesnt make sense to be higher than imageCount
    const uint32_t framesInFlight;

    // number of images in Swapchain
    const uint32_t imageCount;

    uint32_t currentFrameIndex;

    // size is framesInFlight
    // synchronization objects for each frame in flight
    std::vector<SemaphoreU> imageAvailableSemaphore; // present signals, submit  waits
    std::vector<SemaphoreU> renderFinishedSemaphore; // submit  signals, present waits
    std::vector<FenceU>     inFlightFences;          // waited before submit, signaled by submit

    // size is imageCount
    // determines what frame is rendering to each swapchain image
    // each value is [0, framesInFlight)
    std::vector<uint32_t> imageToFrameMapping;

    Swapchain& swapchain;
    TimePoint  lastDrawTime;

public:
    SynchronizedSwapchainGraphRenderer (const DeviceExtra& device, Swapchain& swapchain);

    ~SynchronizedSwapchainGraphRenderer ();

    void Wait ();

    void RenderNextRecreatableFrame (RenderGraph& graph) override;
};

} // namespace RG

}

#endif