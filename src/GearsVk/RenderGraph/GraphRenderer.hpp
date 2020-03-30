#ifndef GRAPHRENDERER_HPP
#define GRAPHRENDERER_HPP

#include "RenderGraph.hpp"
#include "Swapchain.hpp"


namespace RenderGraph {


struct GraphRenderer {
public:
    virtual void RenderNextFrame () = 0;
};


struct BlockingGraphRenderer : public GraphRenderer {
    Graph&                                    graph;
    Swapchain&                                swapchain;
    std::function<void (uint32_t frameIndex)> preSubmitCallback;
    Semaphore                                 s;

    BlockingGraphRenderer (Graph&                                           graph,
                           Swapchain&                                       swapchain,
                           const std::function<void (uint32_t frameIndex)>& preSubmitCallback)
        : preSubmitCallback (preSubmitCallback)
        , graph (graph)
        , swapchain (swapchain)
        , s (graph.GetGraphSettings ().device)
    {
    }

    void RenderNextFrame () override
    {
        const uint32_t currentImageIndex = swapchain.GetNextImageIndex (s);

        preSubmitCallback (currentImageIndex);

        graph.Submit (currentImageIndex);
        vkQueueWaitIdle (graph.GetGraphSettings ().queue);
        vkDeviceWaitIdle (graph.GetGraphSettings ().device);

        if (swapchain.SupportsPresenting ()) {
            graph.Present (currentImageIndex, swapchain, {s});
            vkQueueWaitIdle (graph.GetGraphSettings ().queue);
            vkDeviceWaitIdle (graph.GetGraphSettings ().device);
        }
    }
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

    Graph&                                    graph;
    Swapchain&                                swapchain;
    std::function<void (uint32_t frameIndex)> preSubmitCallback;

    SynchronizedSwapchainGraphRenderer (Graph&                                           graph,
                                        Swapchain&                                       swapchain,
                                        const std::function<void (uint32_t frameIndex)>& preSubmitCallback)
        : framesInFlight (graph.GetGraphSettings ().framesInFlight)
        , imageCount (swapchain.GetImageCount ())
        , currentFrameIndex (0)
        , preSubmitCallback (preSubmitCallback)
        , graph (graph)
        , swapchain (swapchain)
    {
        ASSERT (imageCount == framesInFlight);

        for (uint32_t i = 0; i < framesInFlight; ++i) {
            imageAvailableSemaphore.push_back (Semaphore::Create (graph.GetGraphSettings ().device));
            renderFinishedSemaphore.push_back (Semaphore::Create (graph.GetGraphSettings ().device));
            inFlightFences.push_back (Fence::Create (graph.GetGraphSettings ().device));
        }

        for (uint32_t i = 0; i < imageCount; ++i) {
            imageToFrameMapping.push_back (UINT32_MAX);
        }
    }


    void RenderNextFrame () override
    {
        inFlightFences[currentFrameIndex]->Wait ();

        const uint32_t currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentFrameIndex]);

        if (imageToFrameMapping[currentImageIndex] != UINT32_MAX) {
            inFlightFences[imageToFrameMapping[currentImageIndex]]->Wait ();
        }
        imageToFrameMapping[currentImageIndex] = currentFrameIndex;

        const std::vector<VkSemaphore> submitWaitSemaphores   = {*imageAvailableSemaphore[currentFrameIndex]};
        const std::vector<VkSemaphore> submitSignalSemaphores = {*renderFinishedSemaphore[currentFrameIndex]};
        const std::vector<VkSemaphore> presentWaitSemaphores  = submitSignalSemaphores;

        inFlightFences[currentFrameIndex]->Reset ();

        preSubmitCallback (currentFrameIndex);

        graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentFrameIndex]);

        ASSERT (swapchain.SupportsPresenting ());
        graph.Present (currentImageIndex, swapchain, presentWaitSemaphores);

        currentFrameIndex = (currentFrameIndex + 1) % framesInFlight;
    }
};

} // namespace RenderGraph

#endif