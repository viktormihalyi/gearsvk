#ifndef STIMULUSADAPTERVIEW_HPP
#define STIMULUSADAPTERVIEW_HPP

// from RenderGraph
#include "RenderGraph/Utils/Event.hpp"
#include "RenderGraph/Utils/Noncopyable.hpp"

// from Sequence
#include "SequenceAPI.hpp"

// from std
#include <map>
#include <memory>


class StimulusAdapter;
class Stimulus;
class IRandomExporter;

namespace RG {
class VulkanEnvironment;
class Presentable;
class Renderer;
class IFrameDisplayObserver;
} // namespace RG


class SEQUENCE_API StimulusAdapterView : public Noncopyable {
private:
    RG::VulkanEnvironment&               environment;
    const std::shared_ptr<Stimulus const> stimulus;

    std::map<std::shared_ptr<RG::Presentable>, std::shared_ptr<StimulusAdapter>> compiledAdapters;

public:
    StimulusAdapterView (RG::VulkanEnvironment& environment, const std::shared_ptr<Stimulus const>& stimulus);

    void CreateForPresentable (std::shared_ptr<RG::Presentable>& presentable);

    void DestroyForPresentable (const std::shared_ptr<RG::Presentable>& presentable);

    void RenderFrameIndex (RG::Renderer&                     renderer,
                           std::shared_ptr<RG::Presentable>&     presentable,
                           const std::shared_ptr<Stimulus const>& stimulus,
                           const uint32_t                         frameIndex,
                           RG::IFrameDisplayObserver&        frameDisplayObserver,
                           IRandomExporter&                       randomExporter);
};

#endif