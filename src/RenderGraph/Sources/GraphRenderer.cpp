#include "GraphRenderer.hpp"

#include "VulkanWrapper/Fence.hpp"
#include "VulkanWrapper/Semaphore.hpp"
#include "VulkanWrapper/Swapchain.hpp"

#include "GraphSettings.hpp"
#include "Operation.hpp"
#include "DrawRecordable.hpp"
#include "RenderGraph.hpp"
#include "Resource.hpp"



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


uint32_t BlockingGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph, IFrameDisplayObserver&)
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

    return currentImageIndex;
}


SynchronizedSwapchainGraphRenderer::SynchronizedSwapchainGraphRenderer (const DeviceExtra& device, Swapchain& swapchain)
    : RecreatableGraphRenderer { swapchain }
    , framesInFlight { swapchain.GetImageCount () }
    , imageCount { swapchain.GetImageCount () }
    , currentResourceIndex { 0 }
    , swapchain { swapchain }
    , presentationEngineFence { std::make_unique<Fence> (device, false) }
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

    vkDeviceWaitIdle (settings.GetDevice ());
    vkQueueWaitIdle (settings.GetDevice ().GetGraphicsQueue ());

    swapchain.Recreate ();

    settings.framesInFlight = swapchain.GetImageCount ();

    graph.CompileResources (settings);

    graph.Compile (std::move (settings));

    recreateEvent.Notify ();
}


uint32_t RecreatableGraphRenderer::RenderNextFrame (RenderGraph& graph, IFrameDisplayObserver& observer)
{
    try {
        return RenderNextRecreatableFrame (graph, observer);
    } catch (OutOfDateSwapchain&) {
        throw;
        //Recreate (graph);
    }
}


uint32_t SynchronizedSwapchainGraphRenderer::RenderNextRecreatableFrame (RenderGraph& graph, IFrameDisplayObserver& frameDisplayObserver)
{
    frameDisplayObserver.OnImageFenceWaitStarted (currentResourceIndex);
    //inFlightFences[currentResourceIndex]->Wait ();
    //std::cout << "render ended for " << currentResourceIndex<< " at " << std::fixed << GVK::TimePoint::SinceEpoch ().AsMilliseconds () << std::endl;
    frameDisplayObserver.OnImageFenceWaitEnded (currentResourceIndex);

    frameDisplayObserver.OnImageAcquisitionStarted ();
    const uint32_t currentImageIndex = swapchain.GetNextImageIndex (*imageAvailableSemaphore[currentResourceIndex], *presentationEngineFence);
    //std::cout << "got img for      " << currentResourceIndex<< " at " << std::fixed << GVK::TimePoint::SinceEpoch ().AsMilliseconds () << std::endl;
    frameDisplayObserver.OnImageAcquisitionReturned (currentResourceIndex);

    presentationEngineFence->Wait ();
    frameDisplayObserver.OnImageAcquisitionFenceSignaled (currentResourceIndex);
    presentationEngineFence->Reset ();

    swapchainImageAcquiredEvent.Notify (currentImageIndex);

    // the image was last drawn by this frame, wait for its fence
    const uint32_t previousFrameIndex = imageToFrameMapping[currentImageIndex];
    if (previousFrameIndex != UINT32_MAX) {
        inFlightFences[previousFrameIndex]->Wait ();
    }

    frameDisplayObserver.OnImageAcquisitionEnded (currentResourceIndex);

    // SIGNAL HERE
    presentedEvent.Notify ();


    // update mapping
    imageToFrameMapping[currentImageIndex] = currentResourceIndex;

    const std::vector<VkSemaphore> submitWaitSemaphores   = { *imageAvailableSemaphore[currentResourceIndex] };
    const std::vector<VkSemaphore> submitSignalSemaphores = { *renderFinishedSemaphore[currentResourceIndex] };
    const std::vector<VkSemaphore> presentWaitSemaphores  = submitSignalSemaphores;

    inFlightFences[currentResourceIndex]->Reset ();

    {
        const TimePoint currentTime = TimePoint::SinceApplicationStart ();
        preSubmitEvent.Notify (graph, currentResourceIndex, currentTime - lastDrawTime);
        lastDrawTime = currentTime;
    }

    frameDisplayObserver.OnRenderStarted (currentResourceIndex);
    graph.Submit (currentResourceIndex, submitWaitSemaphores, submitSignalSemaphores, *inFlightFences[currentResourceIndex]);
    //graph.Submit (currentResourceIndex, submitWaitSemaphores, submitSignalSemaphores);

    GVK_ASSERT (swapchain.SupportsPresenting ());

    frameDisplayObserver.OnPresentStarted (currentResourceIndex);
    graph.Present (currentImageIndex, swapchain, presentWaitSemaphores);

    const uint32_t usedFrameIndex = currentResourceIndex;

    currentResourceIndex= (currentResourceIndex+ 1) % framesInFlight;

    return usedFrameIndex;
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
