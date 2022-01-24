#ifndef SEQUENCEADAPTER_HPP
#define SEQUENCEADAPTER_HPP

// from RenderGraph
#include "RenderGraph/Utils/Event.hpp"
#include "RenderGraph/Utils/Time.hpp"
#include "RenderGraph/GraphRenderer.hpp"

// from Sequence
#include "SequenceAPI.hpp"

// from std
#include <map>
#include <optional>
#include <memory>
#include <string>


class StimulusAdapterView;
class Stimulus;
class Sequence;
class IRandomExporter;

namespace RG {
class VulkanEnvironment;
class Presentable;
class SynchronizedSwapchainGraphRenderer;
}


class SEQUENCE_API SequenceAdapter : public RG::IFrameDisplayObserver {
private:
    const std::shared_ptr<Sequence> sequence;

    std::optional<uint32_t> lastRenderedFrameIndex;

    RG::VulkanEnvironment&           environment;
    std::shared_ptr<RG::Presentable> currentPresentable;

    std::map<std::shared_ptr<Stimulus const>, std::shared_ptr<StimulusAdapterView>> views;

    std::unique_ptr<RG::SynchronizedSwapchainGraphRenderer> renderer;

    std::unique_ptr<IRandomExporter> randomExporter;

    std::vector<uint32_t> resourceIndexToRenderedFrameMapping;

    std::string sequenceNameInTitle;

public:
    SequenceAdapter (RG::VulkanEnvironment& environment, const std::shared_ptr<Sequence>& sequence, const std::string& sequenceNameInTitle);

    virtual ~SequenceAdapter () = default;

    void RenderFrameIndex (const uint32_t frameIndex);

    void Wait ();

    void SetCurrentPresentable (std::shared_ptr<RG::Presentable> presentable);

    std::shared_ptr<RG::Presentable> GetCurrentPresentable ();

    void RenderFullOnExternalWindow ();

    std::shared_ptr<Sequence> GetSequence () { return sequence; }

    // implementing RG::IFrameDisplayObserver

    virtual void OnImageAcquisitionFenceSignaled (uint32_t) override;

private:
    void CreateStimulusAdapterViews ();
};


#endif