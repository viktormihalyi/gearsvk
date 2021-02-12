#ifndef SEQUENCEADAPTER_HPP
#define SEQUENCEADAPTER_HPP

// from GearsVk
#include "Event.hpp"
#include "Ptr.hpp"

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

USING_PTR (SequenceAdapter);
class GEARS_API_TEST SequenceAdapter {
private:
    const Ptr<Sequence> sequence;

    std::optional<uint32_t> lastRenderedFrameIndex;

    GVK::VulkanEnvironment& environment;
    Ptr<GVK::Presentable>   currentPresentable;

    std::map<PtrC<Stimulus>, Ptr<StimulusAdapterView>> views;

    U<GVK::RG::SynchronizedSwapchainGraphRenderer> renderer;

    GVK::Event<uint32_t>     presentedFrameIndexEvent;
    GVK::SingleEventObserver obs;
    uint64_t                 lastNs;

    uint64_t firstFrameMs;

public:
    SequenceAdapter (GVK::VulkanEnvironment& environment, const Ptr<Sequence>& sequence);

    void RenderFrameIndex (const uint32_t frameIndex);

    void Wait ();

    void SetCurrentPresentable (Ptr<GVK::Presentable> presentable);

    Ptr<GVK::Presentable> GetCurrentPresentable ();

    void RenderFullOnExternalWindow ();
};


#endif