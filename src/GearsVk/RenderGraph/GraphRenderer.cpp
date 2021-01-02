#include "GraphRenderer.hpp"

#include "Fence.hpp"
#include "Swapchain.hpp"
#include "Semaphore.hpp"

#include "GraphSettings.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"
#include "Operation.hpp"



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
    , s (Semaphore::Create (settings.GetDevice ()))
{
}


void BlockingGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph)
{
    const uint32_t currentImageIndex = swapchain.GetNextImageIndex (*s);
    swapchainImageAcquiredEvent.Notify (currentImageIndex);

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent.Notify (graph, currentImageIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
    }

    graph.Submit (currentImageIndex);
    vkQueueWaitIdle (graph.device->GetGraphicsQueue ());
    vkDeviceWaitIdle (graph.device->GetDevice ());

    if (swapchain.SupportsPresenting ()) {
        graph.Present (currentImageIndex, swapchain, { *s });
        vkQueueWaitIdle (graph.device->GetGraphicsQueue ());
        vkDeviceWaitIdle (*graph.device);
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
    settings.framesInFlight = swapchain.GetImageCount ();

    graph.CompileResources (settings);
    graph.Compile (settings);

    recreateEvent.Notify ();
}


void RecreatableGraphRenderer::RenderNextFrame (RenderGraph& graph)
{
    try {
        RenderNextRecreatableFrame (graph);
    } catch (OutOfDateSwapchain&) {
        Recreate (graph);
    }
}


class DeltaTimer final {
private:
    TimePoint lastTime;

public:
    DeltaTimer ()
        : lastTime (TimePoint::SinceApplicationStart ())
    {
    }

    TimePoint GetDeltaToLast ()
    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();

        const TimePoint delta = currentTime - lastTime;

        lastTime = currentTime;

        return delta;
    }
};

DeltaTimer t;


void SynchronizedSwapchainGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph)
{
    std::cout << "RenderNextRecreatable called " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    inFlightFences[currentFrameIndex]->Wait ();
    std::cout << "waited for fence " << currentFrameIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    const uint32_t currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentFrameIndex]);
    swapchainImageAcquiredEvent.Notify (currentImageIndex);

    std::cout << "acquired " << currentImageIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
    // the image was last drawn by this frame, wait for its fence
    const uint32_t previousFrameIndex = imageToFrameMapping[currentImageIndex];
    if (previousFrameIndex != UINT32_MAX) {
        inFlightFences[previousFrameIndex]->Wait ();
        std::cout << "waited for fence " << previousFrameIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
    }

    // update mapping
    imageToFrameMapping[currentImageIndex] = currentFrameIndex;

    const std::vector<VkSemaphore> submitWaitSemaphores   = { *imageAvailableSemaphore[currentFrameIndex] };
    const std::vector<VkSemaphore> submitSignalSemaphores = { *renderFinishedSemaphore[currentFrameIndex] };
    const std::vector<VkSemaphore> presentWaitSemaphores  = submitSignalSemaphores;

    inFlightFences[currentFrameIndex]->Reset ();
    std::cout << "fence reset " << currentFrameIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent.Notify (graph, currentFrameIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
        std::cout << "preSubmitEvent " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
    }

    graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentFrameIndex]);
    std::cout << "submitted " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    GVK_ASSERT (swapchain.SupportsPresenting ());

    graph.Present (currentImageIndex, swapchain, presentWaitSemaphores);
    std::cout << "presented " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    currentFrameIndex = (currentFrameIndex + 1) % framesInFlight;
    std::cout << "RenderNextRecreatable ended " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
}


SynchronizedSwapchainGraphRenderer::~SynchronizedSwapchainGraphRenderer ()
{
}


} // namespace RG
