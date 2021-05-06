#ifndef SEQUENCEADAPTER_HPP
#define SEQUENCEADAPTER_HPP

// from GearsVk
#include "Event.hpp"
#include "GraphRenderer.hpp"
#include "Time.hpp"
#include <memory>

// from Gears
#include "GearsAPI.hpp"

// from std
#include <map>
#include <optional>


namespace GVK {
class VulkanEnvironment;
class Presentable;
} // namespace GVK

class StimulusAdapterView;
class Stimulus;
class Sequence;
class IRandomExporter;

namespace GVK {
namespace RG {
class SynchronizedSwapchainGraphRenderer;
}
} // namespace GVK


class SequenceFrameData : public GVK::RG::IFrameDisplayObserver {
public:
    GVK::TimePoint imageFenceWaitStarted;
    GVK::TimePoint imageFenceWaitEnded_delta;
    GVK::TimePoint imageAcquisitionStarted_delta;
    GVK::TimePoint imageAcquisitionReturned_delta;
    GVK::TimePoint imageAcquisitionEnded_delta;
    GVK::TimePoint renderStarted_delta;
    GVK::TimePoint presentStarted_delta;

public:
    virtual ~SequenceFrameData () = default;

    virtual void OnImageFenceWaitStarted (uint32_t) override { imageFenceWaitStarted = GVK::TimePoint::SinceEpoch (); }

    // all submitted command buffers have completed execution
    virtual void OnImageFenceWaitEnded (uint32_t) override { imageFenceWaitEnded_delta = GVK::TimePoint::SinceEpoch () - imageFenceWaitStarted; }

    virtual void OnImageAcquisitionStarted () override { imageAcquisitionStarted_delta = GVK::TimePoint::SinceEpoch () - imageFenceWaitStarted; }
    virtual void OnImageAcquisitionReturned (uint32_t) override { imageAcquisitionReturned_delta = GVK::TimePoint::SinceEpoch () - imageFenceWaitStarted; }
    virtual void OnImageAcquisitionEnded (uint32_t) override { imageAcquisitionEnded_delta = GVK::TimePoint::SinceEpoch () - imageFenceWaitStarted; }
    virtual void OnRenderStarted (uint32_t) override { renderStarted_delta = GVK::TimePoint::SinceEpoch () - imageFenceWaitStarted; }
    virtual void OnPresentStarted (uint32_t) override { presentStarted_delta = GVK::TimePoint::SinceEpoch () - imageFenceWaitStarted; }
};


class SequenceTiming : public GVK::RG::IFrameDisplayObserver {
public:
    uint64_t currentFrameIndex;

    std::unordered_map<uint32_t, GVK::TimePoint> frameRenderStarted;
    std::unordered_map<uint32_t, GVK::TimePoint> frameRenderFinished;
    std::unordered_map<uint32_t, GVK::TimePoint> framePresentFinished;

    struct TimeData {
        GVK::TimePoint beg;
        GVK::TimePoint end;
    };

    std::vector<TimeData> timeData;

    size_t lastDisplayedFrameIndex;

public:
    SequenceTiming (uint64_t currentFrameIndex)
        : currentFrameIndex (currentFrameIndex)
        , lastDisplayedFrameIndex { 0 }
    {
    }

    virtual ~SequenceTiming () = default;

    virtual void OnImageFenceWaitStarted (uint32_t) override {}

    virtual void OnImageFenceWaitEnded (uint32_t frameIndex) override;

    virtual void OnImageAcquisitionStarted () override {}
    virtual void OnImageAcquisitionFenceSignaled (uint32_t) override {}

    virtual void OnImageAcquisitionEnded (uint32_t frameIndex) override;

    virtual void OnRenderStarted (uint32_t frameIndex) override { frameRenderStarted[frameIndex] = GVK::TimePoint::SinceEpoch (); }
    virtual void OnPresentStarted (uint32_t) override {}
};


class GEARS_API_TEST SequenceAdapter : public GVK::RG::IFrameDisplayObserver {
private:
    const std::shared_ptr<Sequence> sequence;

    std::optional<uint32_t> lastRenderedFrameIndex;

    GVK::VulkanEnvironment&           environment;
    std::shared_ptr<GVK::Presentable> currentPresentable;

    std::map<std::shared_ptr<Stimulus const>, std::shared_ptr<StimulusAdapterView>> views;

    std::unique_ptr<GVK::RG::SynchronizedSwapchainGraphRenderer> renderer;


    GVK::Event<uint32_t>     presentedFrameIndexEvent;
    GVK::SingleEventObserver obs;
    uint64_t                 lastNs;

    uint64_t firstFrameMs;

    SequenceTiming timings;

    size_t lastDisplayedFrame;

    std::unique_ptr<IRandomExporter> randomExporter;

    std::vector<size_t> f;

public:
    SequenceAdapter (GVK::VulkanEnvironment& environment, const std::shared_ptr<Sequence>& sequence);

    virtual ~SequenceAdapter () = default;

    void RenderFrameIndex (const uint32_t frameIndex);

    void Wait ();

    void SetCurrentPresentable (std::shared_ptr<GVK::Presentable> presentable);

    std::shared_ptr<GVK::Presentable> GetCurrentPresentable ();

    void RenderFullOnExternalWindow ();

    // GVK::RG::IFrameDisplayObserver
    virtual void OnImageAcquisitionFenceSignaled (uint32_t) override;

private:
    void CreateStimulusAdapterViews ();
};


#endif