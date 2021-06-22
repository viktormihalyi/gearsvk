#ifndef STIMULUSADAPTERVIEW_HPP
#define STIMULUSADAPTERVIEW_HPP

// from RenderGraph
#include "Utils/Event.hpp"
#include "Utils/Noncopyable.hpp"
#include <memory>

// from Sequence
#include "SequenceAPI.hpp"

// from std
#include <map>


class StimulusAdapter;
class Stimulus;
class IRandomExporter;

namespace GVK {
class VulkanEnvironment;
class Presentable;
namespace RG {
class Renderer;
class IFrameDisplayObserver;
} // namespace RG
} // namespace GVK


class SEQUENCE_API StimulusAdapterView : public Noncopyable {
private:
    GVK::VulkanEnvironment&               environment;
    const std::shared_ptr<Stimulus const> stimulus;

    std::map<std::shared_ptr<GVK::Presentable>, std::shared_ptr<StimulusAdapter>> compiledAdapters;

public:
    StimulusAdapterView (GVK::VulkanEnvironment& environment, const std::shared_ptr<Stimulus const>& stimulus);

    void CreateForPresentable (std::shared_ptr<GVK::Presentable>& presentable);

    void DestroyForPresentable (const std::shared_ptr<GVK::Presentable>& presentable);

    void RenderFrameIndex (GVK::RG::Renderer&                     renderer,
                           std::shared_ptr<GVK::Presentable>&     presentable,
                           const std::shared_ptr<Stimulus const>& stimulus,
                           const uint32_t                         frameIndex,
                           GVK::RG::IFrameDisplayObserver&        frameDisplayObserver,
                           IRandomExporter&                       randomExporter);
};

#endif