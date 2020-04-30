#include "GraphRenderer.hpp"


namespace RG {

Window::DrawCallback GraphRenderer::GetInfiniteDrawCallback ()
{
    return [&] (bool&) -> void {
        RenderNextFrame ();
    };
}


Window::DrawCallback GraphRenderer::GetConditionalDrawCallback (const std::function<bool ()>& shouldStop)
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


RecreatableGraphRenderer::RecreatableGraphRenderer (RenderGraph& graph, Swapchain& swapchain)
    : graph (graph)
    , swapchain (swapchain)
{
}


BlockingGraphRenderer::BlockingGraphRenderer (RenderGraph& graph, Swapchain& swapchain)
    : RecreatableGraphRenderer (graph, swapchain)
    , graph (graph)
    , swapchain (swapchain)
    , s (graph.GetGraphSettings ().GetDevice ())
{
}


void BlockingGraphRenderer::RenderNextRecreatableFrame ()
{
    const uint32_t currentImageIndex = swapchain.GetNextImageIndex (s);

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent (currentImageIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
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


SynchronizedSwapchainGraphRenderer::SynchronizedSwapchainGraphRenderer (RenderGraph& graph, Swapchain& swapchain)
    : RecreatableGraphRenderer (graph, swapchain)
    , framesInFlight (graph.GetGraphSettings ().framesInFlight)
    , imageCount (swapchain.GetImageCount ())
    , currentFrameIndex (0)
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


void RecreatableGraphRenderer::Recreate ()
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

    recreateEvent ();
}


void RecreatableGraphRenderer::RenderNextFrame ()
{
    try {
        RenderNextRecreatableFrame ();
    } catch (OutOfDateSwapchain&) {
        Recreate ();
    }
}


void SynchronizedSwapchainGraphRenderer::RenderNextRecreatableFrame ()
{
    inFlightFences[currentFrameIndex]->Wait ();

    uint32_t currentImageIndex;
    currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentFrameIndex]);

    if (imageToFrameMapping[currentImageIndex] != UINT32_MAX) {
        inFlightFences[imageToFrameMapping[currentImageIndex]]->Wait ();
    }
    imageToFrameMapping[currentImageIndex] = currentFrameIndex;

    const std::vector<VkSemaphore> submitWaitSemaphores   = {*imageAvailableSemaphore[currentFrameIndex]};
    const std::vector<VkSemaphore> submitSignalSemaphores = {*renderFinishedSemaphore[currentFrameIndex]};
    const std::vector<VkSemaphore> presentWaitSemaphores  = submitSignalSemaphores;

    inFlightFences[currentFrameIndex]->Reset ();

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent (currentFrameIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
    }

    graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentFrameIndex]);

    ASSERT (swapchain.SupportsPresenting ());

    graph.Present (currentImageIndex, swapchain, presentWaitSemaphores);

    currentFrameIndex = (currentFrameIndex + 1) % framesInFlight;
}


SynchronizedSwapchainGraphRenderer::~SynchronizedSwapchainGraphRenderer ()
{
    vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());
    vkQueueWaitIdle (graph.GetGraphSettings ().queue);
}


} // namespace RG
