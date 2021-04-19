#ifndef STIMULUSADAPTERFORPRESENTABLE_HPP
#define STIMULUSADAPTERFORPRESENTABLE_HPP

// from GearsVk
#include "Event.hpp"
#include "GraphRenderer.hpp"
#include "Noncopyable.hpp"
#include "Time.hpp"
#include <memory>

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
class IFrameDisplayObserver;
class VulkanEnvironment;
} // namespace GVK


class StimulusAdapterForPresentable : public Noncopyable {
private:
    const std::shared_ptr<Stimulus const>   stimulus;
    const std::shared_ptr<GVK::Presentable> presentable;

    std::shared_ptr<GVK::RG::RenderGraph>                                renderGraph;
    std::shared_ptr<GVK::RG::UniformReflection>                          reflection;
    std::map<std::shared_ptr<Pass>, std::shared_ptr<GVK::RG::Operation>> passToOperation;
    std::shared_ptr<GVK::RG::Operation>                                  randomGeneratorOperation;

    std::unordered_map<uint32_t, std::unique_ptr<GVK::SingleEventObserver>> presentObservers;
    std::vector<uint32_t>                                                   presentedEventDeleteQueue;

public:
    StimulusAdapterForPresentable (const GVK::VulkanEnvironment& environment, std::shared_ptr<GVK::Presentable>& presentable, const std::shared_ptr<Stimulus const>& stimulus);

    void RenderFrameIndex (GVK::RG::Renderer&                     renderer,
                           const std::shared_ptr<Stimulus const>& stimulus,
                           const uint32_t                         frameIndex,
                           GVK::RG::IFrameDisplayObserver&        frameDisplayObserver);

    void Wait ();

private:
    void SetConstantUniforms ();
    void SetUniforms (const GVK::UUID& renderOperationId, const std::shared_ptr<Stimulus const>& stimulus, const uint32_t frameIndex);
};


#endif