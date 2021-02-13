#ifndef STIMULUSADAPTERFORPRESENTABLE_HPP
#define STIMULUSADAPTERFORPRESENTABLE_HPP

// from GearsVk
#include "Event.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

// from std
#include <map>
#include <vector>

class Pass;
class Stimulus;

namespace GVK {
class UUID;
}

namespace GVK {
namespace RG {
class RenderGraph;
class UniformReflection;
class Operation;
class SynchronizedSwapchainGraphRenderer;
class Renderer;
} // namespace RG
class Presentable;
class VulkanEnvironment;
} // namespace GVK


class StimulusAdapterForPresentable : public Noncopyable {
private:
    const PtrC<Stimulus>        stimulus;
    const Ptr<GVK::Presentable> presentable;

    Ptr<GVK::RG::RenderGraph>                    renderGraph;
    Ptr<GVK::RG::UniformReflection>              reflection;
    std::map<Ptr<Pass>, Ptr<GVK::RG::Operation>> passToOperation;
    Ptr<GVK::RG::Operation>                      randomGeneratorOperation;

    std::unordered_map<uint32_t, U<GVK::SingleEventObserver>> presentObservers;
    std::vector<uint32_t>                                     presentedEventDeleteQueue;

public:
    StimulusAdapterForPresentable (const GVK::VulkanEnvironment& environment, Ptr<GVK::Presentable>& presentable, const PtrC<Stimulus>& stimulus);

    void RenderFrameIndex (GVK::RG::Renderer& renderer, const PtrC<Stimulus>& stimulus, const uint32_t frameIndex, GVK::Event<uint32_t>& frameIndexPresentedEvent);

    void Wait ();

private:
    void SetConstantUniforms ();
    void SetUniforms (const GVK::UUID& renderOperationId, const PtrC<Stimulus>& stimulus, const uint32_t frameIndex);
};


#endif