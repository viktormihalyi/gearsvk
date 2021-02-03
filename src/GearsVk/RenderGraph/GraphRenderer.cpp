#include "GraphRenderer.hpp"

#include "Fence.hpp"
#include "Semaphore.hpp"
#include "Swapchain.hpp"

#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"


constexpr bool LOG_RENDERING = false;


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


RecreatableGraphRenderer::RecreatableGraphRenderer (Swapchain& swapchain)
    : swapchain (swapchain)
{
}


BlockingGraphRenderer::BlockingGraphRenderer (const DeviceExtra& device, Swapchain& swapchain)
    : RecreatableGraphRenderer (swapchain)
    , s (Semaphore::Create (device))
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


SynchronizedSwapchainGraphRenderer::SynchronizedSwapchainGraphRenderer (const DeviceExtra& device, Swapchain& swapchain)
    : RecreatableGraphRenderer (swapchain)
    , framesInFlight (swapchain.GetImageCount ())
    , imageCount (swapchain.GetImageCount ())
    , currentFrameIndex (0)
    , swapchain (swapchain)
{
    GVK_ASSERT (imageCount <= framesInFlight);

    for (uint32_t i = 0; i < framesInFlight; ++i) {
        imageAvailableSemaphore.push_back (Semaphore::Create (device));
        renderFinishedSemaphore.push_back (Semaphore::Create (device));
        inFlightFences.push_back (Fence::Create (device));
    }

    for (uint32_t i = 0; i < imageCount; ++i) {
        imageToFrameMapping.push_back (UINT32_MAX);
    }
}


void RecreatableGraphRenderer::Recreate (RenderGraph& graph)
{
    GraphSettings settings = std::move (graph.graphSettings);

    if constexpr (LOG_RENDERING)
        std::cout << "waiting for device... " << std::endl;

    vkDeviceWaitIdle (settings.GetDevice ());
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    if constexpr (LOG_RENDERING)
        std::cout << "recreating swapchain... " << std::endl;

    swapchain.Recreate ();

    if constexpr (LOG_RENDERING)
        std::cout << "recompiling graph swapchain... " << std::endl;

    settings.framesInFlight = swapchain.GetImageCount ();

    graph.CompileResources (settings);

    graph.Compile (std::move (settings));

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
    if constexpr (LOG_RENDERING)
        std::cout << "RenderNextRecreatable called " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    inFlightFences[currentFrameIndex]->Wait ();
    if constexpr (LOG_RENDERING)
        std::cout << "waited for fence " << currentFrameIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    const uint32_t currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentFrameIndex]);
    swapchainImageAcquiredEvent.Notify (currentImageIndex);

    if constexpr (LOG_RENDERING)
        std::cout << "acquired " << currentImageIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
    // the image was last drawn by this frame, wait for its fence
    const uint32_t previousFrameIndex = imageToFrameMapping[currentImageIndex];
    if (previousFrameIndex != UINT32_MAX) {
        inFlightFences[previousFrameIndex]->Wait ();
        if constexpr (LOG_RENDERING)
            std::cout << "waited for fence " << previousFrameIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
    }

    // update mapping
    imageToFrameMapping[currentImageIndex] = currentFrameIndex;

    const std::vector<VkSemaphore> submitWaitSemaphores   = { *imageAvailableSemaphore[currentFrameIndex] };
    const std::vector<VkSemaphore> submitSignalSemaphores = { *renderFinishedSemaphore[currentFrameIndex] };
    const std::vector<VkSemaphore> presentWaitSemaphores  = submitSignalSemaphores;

    inFlightFences[currentFrameIndex]->Reset ();
    if constexpr (LOG_RENDERING)
        std::cout << "fence reset " << currentFrameIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent.Notify (graph, currentFrameIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
        if constexpr (LOG_RENDERING)
            std::cout << "preSubmitEvent " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
    }

    graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentFrameIndex]);
    if constexpr (LOG_RENDERING)
        std::cout << "submitted " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    GVK_ASSERT (swapchain.SupportsPresenting ());

    graph.Present (currentImageIndex, swapchain, presentWaitSemaphores);
    if constexpr (LOG_RENDERING)
        std::cout << "presented " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    currentFrameIndex = (currentFrameIndex + 1) % framesInFlight;
    if constexpr (LOG_RENDERING)
        std::cout << "RenderNextRecreatable ended " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;
}


SynchronizedSwapchainGraphRenderer::~SynchronizedSwapchainGraphRenderer ()
{
}


void SynchronizedSwapchainGraphRenderer::Wait ()
{
    inFlightFences[currentFrameIndex]->Wait ();
}

} // namespace RG
