#ifndef GRAPHRENDERER_HPP
#define GRAPHRENDERER_HPP

#include "GearsVkAPI.hpp"

#include "Event.hpp"
#include "RenderGraph.hpp"
#include "Swapchain.hpp"
#include "Time.hpp"
#include "Window.hpp"


namespace RG {


class GEARSVK_API Renderer {
public:
    Event<uint32_t, uint64_t> preSubmitEvent;
    Event<>                   recreateEvent;

    virtual ~Renderer () = default;

    virtual void RenderNextFrame () = 0;

    Window::DrawCallback GetInfiniteDrawCallback ();

    Window::DrawCallback GetConditionalDrawCallback (const std::function<bool ()>& shouldStop);

    Window::DrawCallback GetCountLimitedDrawCallback (uint64_t limit);
};


class GEARSVK_API RecreatableGraphRenderer : public Renderer {
private:
    RenderGraph& graph;
    Swapchain&   swapchain;

public:
    RecreatableGraphRenderer (RenderGraph& graph, Swapchain& swapchain);
    virtual ~RecreatableGraphRenderer () = default;

    void Recreate ();

    void         RenderNextFrame () override;
    virtual void RenderNextRecreatableFrame () = 0;
};


class GEARSVK_API BlockingGraphRenderer final : public RecreatableGraphRenderer {
private:
    RenderGraph& graph;
    Swapchain&   swapchain;
    Semaphore    s;
    TimePoint    lastDrawTime;

public:
    BlockingGraphRenderer (RenderGraph& graph, Swapchain& swapchain);

    void RenderNextRecreatableFrame () override;
};


class GEARSVK_API SynchronizedSwapchainGraphRenderer final : public RecreatableGraphRenderer {
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
    std::vector<Semaphore::U> imageAvailableSemaphore; // present signals, submit  waits
    std::vector<Semaphore::U> renderFinishedSemaphore; // submit  signals, present waits
    std::vector<Fence::U>     inFlightFences;          // waited before submit, signaled by submit

    // size is imageCount
    // determines what frame is rendering to each swapchain image
    // each value is [0, framesInFlight)
    std::vector<uint32_t> imageToFrameMapping;

    RenderGraph& graph;
    Swapchain&   swapchain;
    TimePoint    lastDrawTime;

public:
    SynchronizedSwapchainGraphRenderer (RenderGraph& graph, Swapchain& swapchain);

    ~SynchronizedSwapchainGraphRenderer ();

    void RenderNextRecreatableFrame () override;
};

} // namespace RG

#endif