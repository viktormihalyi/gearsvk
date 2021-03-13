#ifndef SEQUENCEADAPTER_HPP
#define SEQUENCEADAPTER_HPP

// from GearsVk
#include "Event.hpp"
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

namespace GVK {
namespace RG {
class SynchronizedSwapchainGraphRenderer;
}
} // namespace GVK

class GEARS_API_TEST SequenceAdapter {
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

public:
    SequenceAdapter (GVK::VulkanEnvironment& environment, const std::shared_ptr<Sequence>& sequence);

    void RenderFrameIndex (const uint32_t frameIndex);

    void Wait ();

    void SetCurrentPresentable (std::shared_ptr<GVK::Presentable> presentable);

    std::shared_ptr<GVK::Presentable> GetCurrentPresentable ();

    void RenderFullOnExternalWindow ();
};


#endif