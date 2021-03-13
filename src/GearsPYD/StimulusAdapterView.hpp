#ifndef STIMULUSADAPTERVIEW_HPP
#define STIMULUSADAPTERVIEW_HPP

// from GearsVk
#include "Event.hpp"
#include "Noncopyable.hpp"
#include <memory>

// from std
#include <map>


class StimulusAdapterForPresentable;
class Stimulus;

namespace GVK {
class VulkanEnvironment;
class Presentable;
namespace RG {
class Renderer;
}
} // namespace GVK


class StimulusAdapterView : public Noncopyable {
private:
    GVK::VulkanEnvironment&               environment;
    const std::shared_ptr<Stimulus const> stimulus;

    std::map<std::shared_ptr<GVK::Presentable>, std::shared_ptr<StimulusAdapterForPresentable>> compiledAdapters;

public:
    StimulusAdapterView (GVK::VulkanEnvironment& environment, const std::shared_ptr<Stimulus const>& stimulus);

    void CreateForPresentable (std::shared_ptr<GVK::Presentable>& presentable);

    void RenderFrameIndex (GVK::RG::Renderer&                     renderer,
                           std::shared_ptr<GVK::Presentable>&     presentable,
                           const std::shared_ptr<Stimulus const>& stimulus,
                           const uint32_t                         frameIndex,
                           GVK::Event<uint32_t>&                  frameIndexPresentedEvent);
};

#endif