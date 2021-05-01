#include "GraphRenderer.hpp"

#include "Fence.hpp"
#include "Semaphore.hpp"
#include "Swapchain.hpp"

#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"


constexpr bool LOG_RENDERING = false;

namespace GVK {

namespace RG {

IFrameDisplayObserver noOpFrameDisplayObserver;

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
    , s (std::make_unique<Semaphore> (device))
{
}


void BlockingGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph, IFrameDisplayObserver&)
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
    , presentationEngineFence (std::make_unique<Fence> (device))
{
    presentationEngineFence->SetName ("presentationEngineFence");
    
    GVK_ASSERT (imageCount <= framesInFlight);

    for (uint32_t i = 0; i < framesInFlight; ++i) {
        imageAvailableSemaphore.push_back (std::make_unique<Semaphore> (device));
        renderFinishedSemaphore.push_back (std::make_unique<Semaphore> (device));
        inFlightFences.push_back (std::make_unique<Fence> (device));
        inFlightFences.back ()->SetName (std::string ("inFlightFence ") + std::to_string (i));
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


void RecreatableGraphRenderer::RenderNextFrame (RenderGraph& graph, IFrameDisplayObserver& observer)
{
    try {
        RenderNextRecreatableFrame (graph, observer);
    } catch (OutOfDateSwapchain&) {
        throw;
        //Recreate (graph);
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


void SynchronizedSwapchainGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph, IFrameDisplayObserver& frameDisplayObserver)
{
    if constexpr (LOG_RENDERING)
        std::cout << "RenderNextRecreatable called " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    frameDisplayObserver.OnImageFenceWaitStarted (currentFrameIndex);
    //inFlightFences[currentFrameIndex]->Wait ();
    //std::cout << "render ended for " << currentFrameIndex << " at " << std::fixed << GVK::TimePoint::SinceEpoch ().AsMilliseconds () << std::endl;
    frameDisplayObserver.OnImageFenceWaitEnded (currentFrameIndex);
    if constexpr (LOG_RENDERING)
        std::cout << "waited for fence " << currentFrameIndex << " " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    frameDisplayObserver.OnImageAcquisitionStarted ();
    const uint32_t currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentFrameIndex], *presentationEngineFence);
    //std::cout << "got img for      " << currentFrameIndex << " at " << std::fixed << GVK::TimePoint::SinceEpoch ().AsMilliseconds () << std::endl;
    frameDisplayObserver.OnImageAcquisitionReturned (currentFrameIndex);

    presentationEngineFence->Wait ();
    frameDisplayObserver.OnImageAcquisitionFenceSignaled (currentFrameIndex);
    presentationEngineFence->Reset ();

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

    frameDisplayObserver.OnImageAcquisitionEnded (currentFrameIndex);

    // SIGNAL HERE
    presentedEvent.Notify ();


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

    frameDisplayObserver.OnRenderStarted (currentFrameIndex);
    graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentFrameIndex]);
    //graph.Submit (currentFrameIndex, submitWaitSemaphores, submitSignalSemaphores);
    if constexpr (LOG_RENDERING)
        std::cout << "submitted " << t.GetDeltaToLast ().AsMilliseconds () << std::endl;

    GVK_ASSERT (swapchain.SupportsPresenting ());

    frameDisplayObserver.OnPresentStarted (currentFrameIndex);
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
    for (auto& fence : inFlightFences) {
        fence->Wait ();
    }
}

} // namespace RG

} // namespace GVK
