#ifndef GRAPHRENDERER_HPP
#define GRAPHRENDERER_HPP

#include "GearsVk/GearsVkAPI.hpp"
#include "GearsVk/VulkanWrapper/DeviceExtra.hpp"
#include "GearsVk/VulkanWrapper/Semaphore.hpp"
#include "GearsVk/Window/Window.hpp"

#include "Utils/Event.hpp"
#include "Utils/Time.hpp"

#include <memory>

namespace GVK {

class Swapchain;
class Fence;

namespace RG {

class RenderGraph;
class GraphSettings;


class GVK_RENDERER_API IFrameDisplayObserver {
public:
    virtual ~IFrameDisplayObserver () = default;

    virtual void OnImageFenceWaitStarted (uint32_t) {}
    virtual void OnImageFenceWaitEnded (uint32_t) {}
    virtual void OnImageAcquisitionStarted () {}
    virtual void OnImageAcquisitionReturned (uint32_t) {}
    virtual void OnImageAcquisitionFenceSignaled (uint32_t) {}
    virtual void OnImageAcquisitionEnded (uint32_t) {}
    virtual void OnRenderStarted (uint32_t) {}
    virtual void OnPresentStarted (uint32_t) {}
};

extern GVK_RENDERER_API IFrameDisplayObserver noOpFrameDisplayObserver;


class GVK_RENDERER_API Renderer {
public:
    Event<RenderGraph&, uint32_t, uint64_t> preSubmitEvent;
    Event<>                                 recreateEvent;
    Event<uint32_t>                         swapchainImageAcquiredEvent;
    Event<>                                 presentedEvent;

    virtual ~Renderer () = default;

    virtual uint32_t GetNextRenderResourceIndex () = 0;
    virtual uint32_t RenderNextFrame (RenderGraph& graph, IFrameDisplayObserver& observer = noOpFrameDisplayObserver) = 0;

    Window::DrawCallback GetInfiniteDrawCallback (const std::function<RenderGraph&()>& graphProvider);

    Window::DrawCallback GetConditionalDrawCallback (const std::function<RenderGraph&()>& graphProvider, const std::function<bool ()>& shouldStop);

    Window::DrawCallback GetCountLimitedDrawCallback (const std::function<RenderGraph&()>& graphProvider, uint64_t limit);
};


class GVK_RENDERER_API RecreatableGraphRenderer : public Renderer {
protected:
    Swapchain& swapchain;

public:
    RecreatableGraphRenderer (Swapchain& swapchain);
    virtual ~RecreatableGraphRenderer () = default;

    void Recreate (RenderGraph& graph);

    uint32_t         RenderNextFrame (RenderGraph& graph, IFrameDisplayObserver& observer = noOpFrameDisplayObserver) override;
    virtual uint32_t RenderNextRecreatableFrame (RenderGraph& graph, IFrameDisplayObserver& observer = noOpFrameDisplayObserver) = 0;
};


class GVK_RENDERER_API BlockingGraphRenderer final : public RecreatableGraphRenderer {
private:
    std::unique_ptr<Semaphore> s;
    TimePoint                  lastDrawTime;

public:
    BlockingGraphRenderer (const DeviceExtra& device, Swapchain& swapchain);

    virtual uint32_t GetNextRenderResourceIndex () override { return 0; }
    uint32_t         RenderNextRecreatableFrame (RenderGraph& graph, IFrameDisplayObserver& observer = noOpFrameDisplayObserver) override;
};


class GVK_RENDERER_API SynchronizedSwapchainGraphRenderer final : public RecreatableGraphRenderer {
private:
    // number of render operations able to run simultaneously
    // optimally equal to imageCount, but may be lower.
    // doesnt make sense to be higher than imageCount
    const uint32_t framesInFlight;

    // number of images in Swapchain
    const uint32_t imageCount;

    uint32_t currentResourceIndex;

    // size is framesInFlight
    // synchronization objects for each frame in flight
    std::vector<std::unique_ptr<Semaphore>> imageAvailableSemaphore; // present signals, submit  waits
    std::vector<std::unique_ptr<Semaphore>> renderFinishedSemaphore; // submit  signals, present waits
    std::vector<std::unique_ptr<Fence>>     inFlightFences;          // waited before submit, signaled by submit

    std::unique_ptr<Fence> presentationEngineFence;

    // size is imageCount
    // determines what frame is rendering to each swapchain image
    // each value is [0, framesInFlight)
    std::vector<uint32_t> imageToFrameMapping;

    Swapchain& swapchain;
    TimePoint  lastDrawTime;

public:
    SynchronizedSwapchainGraphRenderer (const DeviceExtra& device, Swapchain& swapchain);

    ~SynchronizedSwapchainGraphRenderer ();

    void Wait ();

    virtual uint32_t GetNextRenderResourceIndex () override { return currentResourceIndex; }
    uint32_t         GetFramesInFlight () { return framesInFlight; }

    uint32_t         RenderNextRecreatableFrame (RenderGraph& graph, IFrameDisplayObserver& observer = noOpFrameDisplayObserver) override;
};

} // namespace RG

} // namespace GVK

#endif