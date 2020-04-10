#ifndef GRAPHRENDERER_HPP
#define GRAPHRENDERER_HPP

#include "RenderGraph.hpp"
#include "Swapchain.hpp"
#include "Window.hpp"


namespace RenderGraphns {


struct GraphRenderer {
public:
    virtual void RenderNextFrame () = 0;

    Window::DrawCallback GetInfiniteDrawCallback ()
    {
        return [&] (bool&) -> void {
            RenderNextFrame ();
        };
    }

    Window::DrawCallback GetInfiniteDrawCallback (const std::function<bool ()>& shouldStop)
    {
        return [&] (bool& stopFlag) -> void {
            stopFlag = shouldStop ();
            RenderNextFrame ();
        };
    }

    Window::DrawCallback GetCountLimitedDrawCallback (uint64_t limit)
    {
        uint64_t drawCount = 0;
        return [&] (bool& stopFlag) -> void {
            RenderNextFrame ();

            ++drawCount;
            if (drawCount >= limit) {
                stopFlag = true;
            }
        };
    }
};


struct BlockingGraphRenderer : public GraphRenderer {
    RenderGraph&                              graph;
    Swapchain&                                swapchain;
    std::function<void (uint32_t frameIndex)> preSubmitCallback;
    Semaphore                                 s;

    BlockingGraphRenderer (RenderGraph&                                     graph,
                           Swapchain&                                       swapchain,
                           const std::function<void (uint32_t frameIndex)>& preSubmitCallback = nullptr)
        : preSubmitCallback (preSubmitCallback)
        , graph (graph)
        , swapchain (swapchain)
        , s (graph.GetGraphSettings ().GetDevice ())
    {
    }

    void RenderNextFrame () override
    {
        const uint32_t currentImageIndex = swapchain.GetNextImageIndex (s);

        if (preSubmitCallback != nullptr) {
            preSubmitCallback (currentImageIndex);
        }

        graph.Submit (currentImageIndex);
        vkQueueWaitIdle (graph.GetGraphSettings ().queue);
        vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());

        if (swapchain.SupportsPresenting ()) {
            graph.Present (currentImageIndex, swapchain, {s});
            vkQueueWaitIdle (graph.GetGraphSettings ().queue);
            vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());
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

    RenderGraph&                              graph;
    Swapchain&                                swapchain;
    std::function<void (uint32_t frameIndex)> preSubmitCallback;

    SynchronizedSwapchainGraphRenderer (RenderGraph&                                     graph,
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
            imageAvailableSemaphore.push_back (Semaphore::Create (graph.GetGraphSettings ().GetDevice ()));
            renderFinishedSemaphore.push_back (Semaphore::Create (graph.GetGraphSettings ().GetDevice ()));
            inFlightFences.push_back (Fence::Create (graph.GetGraphSettings ().GetDevice ()));
        }

        for (uint32_t i = 0; i < imageCount; ++i) {
            imageToFrameMapping.push_back (UINT32_MAX);
        }
    }

    void RecreateStuff ()
    {
        vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());
        vkQueueWaitIdle (graph.GetGraphSettings ().queue);

        swapchain.Recreate ();
        GraphSettings s = graph.GetGraphSettings ();
        s.width = swapchain.GetWidth ();
        s.height = swapchain.GetHeight ();
        s.framesInFlight = swapchain.GetImageCount ();

        graph.SetGraphSettings (s);
        graph.Compile ();
    }


    void RenderNextFrame () override
    {
        inFlightFences[currentFrameIndex]->Wait ();

        uint32_t currentImageIndex;
        try {
            currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentFrameIndex]);
        } catch (OutOfDateSwapchain&) {
            RecreateStuff ();
            return;
        }

        if (imageToFrameMapping[currentImageIndex] != UINT32_MAX) {
            inFlightFences[imageToFrameMapping[currentImageIndex]]->Wait ();
        }
        imageToFrameMapping[currentImageIndex] = currentFrameIndex;

        const std::vector<VkSemaphore> submitWaitSemaphores   = {*imageAvailableSemaphore[currentFrameIndex]};
        const std::vector<VkSemaphore> submitSignalSemaphores = {*renderFinishedSemaphore[currentFrameIndex]};
        const std::vector<VkSemaphore> presentWaitSemaphores  = submitSignalSemaphores;

        inFlightFences[currentFrameIndex]->Reset ();

        if (preSubmitCallback != nullptr) {
            preSubmitCallback (currentFrameIndex);
        }

        graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentFrameIndex]);

        ASSERT (swapchain.SupportsPresenting ());


        try {
            graph.Present (currentImageIndex, swapchain, presentWaitSemaphores);
        } catch (OutOfDateSwapchain&) {
            RecreateStuff ();
            return;
        }

        currentFrameIndex = (currentFrameIndex + 1) % framesInFlight;
    }
};

} // namespace RenderGraphns

#endif