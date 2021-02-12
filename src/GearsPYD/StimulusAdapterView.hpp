#ifndef STIMULUSADAPTERVIEW_HPP
#define STIMULUSADAPTERVIEW_HPP

// from GearsVk
#include "Event.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"

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


USING_PTR (StimulusAdapterView);
class StimulusAdapterView : public Noncopyable {
private:
    GVK::VulkanEnvironment& environment;
    const PtrC<Stimulus>    stimulus;

    std::map<Ptr<GVK::Presentable>, Ptr<StimulusAdapterForPresentable>> compiledAdapters;

public:
    StimulusAdapterView (GVK::VulkanEnvironment& environment, const PtrC<Stimulus>& stimulus);

    void CreateForPresentable (Ptr<GVK::Presentable>& presentable);

    void RenderFrameIndex (GVK::RG::Renderer&     renderer,
                           Ptr<GVK::Presentable>& presentable,
                           const PtrC<Stimulus>&  stimulus,
                           const uint32_t         frameIndex,
                           GVK::Event<uint32_t>&  frameIndexPresentedEvent);
};

#endif