#include "GraphRenderer.hpp"


namespace RG {

Window::DrawCallback Renderer::GetInfiniteDrawCallback (const std::function<RenderGraph&()>& graphProvider)
{
    return [&] (bool&) -> void {
        RenderNextFrame (graphProvider ());
    };
}


Window::DrawCallback Renderer::GetConditionalDrawCallback (const std::function<RenderGraph&()>& graphProvider, const std::function<bool ()>& shouldStop)
{
    return [&] (bool& stopFlag) -> void {
        stopFlag = shouldStop ();

        RenderNextFrame (graphProvider ());
    };
}


Window::DrawCallback Renderer::GetCountLimitedDrawCallback (const std::function<RenderGraph&()>& graphProvider, uint64_t limit)
{
    uint64_t drawCount = 0;
    return [&] (bool& stopFlag) -> void {
        RenderNextFrame (graphProvider ());

        ++drawCount;
        if (drawCount >= limit) {
            stopFlag = true;
        }
    };
}


RecreatableGraphRenderer::RecreatableGraphRenderer (GraphSettings& settings, Swapchain& swapchain)
    : settings (settings)
    , swapchain (swapchain)
{
}


BlockingGraphRenderer::BlockingGraphRenderer (GraphSettings& settings, Swapchain& swapchain)
    : RecreatableGraphRenderer (settings, swapchain)
    , s (settings.GetDevice ())
{
}


void BlockingGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph)
{
    const uint32_t currentImageIndex = swapchain.GetNextImageIndex (s);

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent (graph, currentImageIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
    }

    graph.Submit (currentImageIndex);
    vkQueueWaitIdle (graph.GetGraphSettings ().GetDevice ().GetGraphicsQueue ());
    vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());

    if (swapchain.SupportsPresenting ()) {
        graph.Present (currentImageIndex, swapchain, { s });
        vkQueueWaitIdle (graph.GetGraphSettings ().GetDevice ().GetGraphicsQueue ());
        vkDeviceWaitIdle (graph.GetGraphSettings ().GetDevice ());
    }
}


SynchronizedSwapchainGraphRenderer::SynchronizedSwapchainGraphRenderer (GraphSettings& settings, Swapchain& swapchain)
    : RecreatableGraphRenderer (settings, swapchain)
    , framesInFlight (swapchain.GetImageCount ())
    , imageCount (settings.framesInFlight)
    , currentFrameIndex (0)
    , swapchain (swapchain)
{
    GVK_ASSERT (imageCount <= framesInFlight);

    for (uint32_t i = 0; i < framesInFlight; ++i) {
        imageAvailableSemaphore.push_back (Semaphore::Create (settings.GetDevice ()));
        renderFinishedSemaphore.push_back (Semaphore::Create (settings.GetDevice ()));
        inFlightFences.push_back (Fence::Create (settings.GetDevice ()));
    }

    for (uint32_t i = 0; i < imageCount; ++i) {
        imageToFrameMapping.push_back (UINT32_MAX);
    }
}


void RecreatableGraphRenderer::Recreate (RenderGraph& graph)
{
    std::cout << "waiting for device... " << std::endl;
    vkDeviceWaitIdle (settings.GetDevice ());
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    std::cout << "recreating swapchain... " << std::endl;
    swapchain.Recreate ();

    std::cout << "recompiling graph swapchain... " << std::endl;
    settings.width          = swapchain.GetWidth ();
    settings.height         = swapchain.GetHeight ();
    settings.framesInFlight = swapchain.GetImageCount ();

    graph.CompileResources (settings);
    graph.Compile (settings);

    recreateEvent ();
}


void RecreatableGraphRenderer::RenderNextFrame (RenderGraph& graph)
{
    try {
        RenderNextRecreatableFrame (graph);
    } catch (OutOfDateSwapchain&) {
        Recreate (graph);
    }
}


void SynchronizedSwapchainGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph)
{
    inFlightFences[currentFrameIndex]->Wait ();

    uint32_t currentImageIndex;
    currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentFrameIndex]);

    // the image was last drawn by this frame, wait for its fence
    const uint32_t previousFrameIndex = imageToFrameMapping[currentImageIndex];
    if (previousFrameIndex != UINT32_MAX) {
        inFlightFences[previousFrameIndex]->Wait ();
    }

    // update mapping
    imageToFrameMapping[currentImageIndex] = currentFrameIndex;

    const std::vector<VkSemaphore> submitWaitSemaphores   = { *imageAvailableSemaphore[currentFrameIndex] };
    const std::vector<VkSemaphore> submitSignalSemaphores = { *renderFinishedSemaphore[currentFrameIndex] };
    const std::vector<VkSemaphore> presentWaitSemaphores  = submitSignalSemaphores;

    inFlightFences[currentFrameIndex]->Reset ();

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent (graph, currentFrameIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
    }

    graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentFrameIndex]);

    GVK_ASSERT (swapchain.SupportsPresenting ());

    graph.Present (currentImageIndex, swapchain, presentWaitSemaphores);

    currentFrameIndex = (currentFrameIndex + 1) % framesInFlight;
}


SynchronizedSwapchainGraphRenderer::~SynchronizedSwapchainGraphRenderer ()
{
    vkDeviceWaitIdle (graph->GetGraphSettings ().GetDevice ());
    vkQueueWaitIdle (graph->GetGraphSettings ().GetGrahpicsQueue ());
}


} // namespace RG
