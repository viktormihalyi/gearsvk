#include "GraphRenderer.hpp"


namespace RG {

Window::DrawCallback GraphRenderer::GetInfiniteDrawCallback ()
{
    return [&] (bool&) -> void {
        RenderNextFrame ();
    };
}


Window::DrawCallback GraphRenderer::GetInfiniteDrawCallback (const std::function<bool ()>& shouldStop)
{
    return [&] (bool& stopFlag) -> void {
        stopFlag = shouldStop ();

        RenderNextFrame ();
    };
}


Window::DrawCallback GraphRenderer::GetCountLimitedDrawCallback (uint64_t limit)
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


BlockingGraphRenderer::BlockingGraphRenderer (RenderGraph&                                     graph,
                                              Swapchain&                                       swapchain,
                                              const std::function<void (uint32_t frameIndex)>& preSubmitCallback)
    : preSubmitCallback (preSubmitCallback)
    , graph (graph)
    , swapchain (swapchain)
    , s (graph.GetGraphSettings ().GetDevice ())
{
}


void BlockingGraphRenderer::RenderNextFrame ()
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


SynchronizedSwapchainGraphRenderer::SynchronizedSwapchainGraphRenderer (RenderGraph&                                     graph,
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


void SynchronizedSwapchainGraphRenderer::RecreateStuff ()
{
    std::cout << "waiting for device... " << std::endl;
    vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());
    vkQueueWaitIdle (graph.GetGraphSettings ().queue);

    std::cout << "recreating swapchain... " << std::endl;
    swapchain.Recreate ();

    std::cout << "recompiling graph swapchain... " << std::endl;
    GraphSettings s  = graph.GetGraphSettings ();
    s.width          = swapchain.GetWidth ();
    s.height         = swapchain.GetHeight ();
    s.framesInFlight = swapchain.GetImageCount ();

    graph.CompileResources (s);
    graph.Compile (s);
}


void SynchronizedSwapchainGraphRenderer::RenderNextFrame ()
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
    }

    currentFrameIndex = (currentFrameIndex + 1) % framesInFlight;
}

} // namespace RG
