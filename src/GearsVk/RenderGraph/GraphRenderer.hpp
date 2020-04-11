#ifndef GRAPHRENDERER_HPP
#define GRAPHRENDERER_HPP

#include "RenderGraph.hpp"
#include "Swapchain.hpp"
#include "Window.hpp"


namespace RenderGraphns {


struct GraphRenderer {
public:
    virtual void RenderNextFrame () = 0;

    Window::DrawCallback GetInfiniteDrawCallback ();

    Window::DrawCallback GetInfiniteDrawCallback (const std::function<bool ()>& shouldStop);

    Window::DrawCallback GetCountLimitedDrawCallback (uint64_t limit);
};


struct BlockingGraphRenderer : public GraphRenderer {
    RenderGraph&                              graph;
    Swapchain&                                swapchain;
    std::function<void (uint32_t frameIndex)> preSubmitCallback;
    Semaphore                                 s;

    BlockingGraphRenderer (RenderGraph&                                     graph,
                           Swapchain&                                       swapchain,
                           const std::function<void (uint32_t frameIndex)>& preSubmitCallback = nullptr);

    void RenderNextFrame () override;
};


struct SynchronizedSwapchainGraphRenderer : public GraphRenderer {
    const uint32_t framesInFlight;
    const uint32_t imageCount;
    uint32_t       currentFrameIndex;

    // size is framesInFlight
    std::vector<Semaphore::U> imageAvailableSemaphore; // present signals, submit  waits
    std::vector<Semaphore::U> renderFinishedSemaphore; // submit  signals, present waits
    std::vector<Fence::U>     inFlightFences;          // waited before submit, signaled by submit

    // size is imageCount
    std::vector<uint32_t> imageToFrameMapping;

    RenderGraph&                              graph;
    Swapchain&                                swapchain;
    std::function<void (uint32_t frameIndex)> preSubmitCallback;

    SynchronizedSwapchainGraphRenderer (RenderGraph&                                     graph,
                                        Swapchain&                                       swapchain,
                                        const std::function<void (uint32_t frameIndex)>& preSubmitCallback);

    void RecreateStuff ();

    void RenderNextFrame () override;
};

} // namespace RenderGraphns

#endif