#ifndef STIMULUSADAPTERFORPRESENTABLE_HPP
#define STIMULUSADAPTERFORPRESENTABLE_HPP

// from RenderGraph
#include "Utils/Event.hpp"
#include "RenderGraph/GraphRenderer.hpp"
#include "Utils/Noncopyable.hpp"
#include "Utils/Time.hpp"

// from Sequence
#include "SequenceAPI.hpp"

// from std
#include <memory>
#include <map>
#include <vector>

#include "glm/glm.hpp"

class Pass;
class Stimulus;

namespace GVK {
class UUID;
}

namespace RG {
class Presentable;
class RenderGraph;
class UniformReflection;
class Operation;
class SynchronizedSwapchainGraphRenderer;
class Renderer;
class GPUBufferResource;
class IFrameDisplayObserver;
class VulkanEnvironment;
} // namespace RG


class SEQUENCE_API IRandomExporter {
public:
    virtual ~IRandomExporter () = default;

    virtual bool IsEnabled () = 0;
    virtual void OnRandomTextureDrawn (RG::GPUBufferResource& randomTexture, uint32_t resourceIndex, uint32_t frameIndex) = 0;
};


class SEQUENCE_API StimulusAdapter : public Noncopyable {
private:
    const RG::VulkanEnvironment& environment;

    const std::shared_ptr<Stimulus const>  stimulus;

    const glm::vec2 patternSizeOnRetina;
    const double    deviceRefreshRate;

    std::shared_ptr<RG::RenderGraph>                                renderGraph;
    std::shared_ptr<RG::UniformReflection>                          reflection;
    std::map<std::shared_ptr<Pass>, std::shared_ptr<RG::Operation>> passToOperation;
    std::shared_ptr<RG::Operation>                                  randomGeneratorOperation;

public:
    StimulusAdapter (const RG::VulkanEnvironment& environment, RG::Presentable& presentable, const std::shared_ptr<Stimulus const>& stimulus);

    void RenderFrameIndex (RG::Renderer&                          renderer,
                           const std::shared_ptr<Stimulus const>& stimulus,
                           const uint32_t                         frameIndex,
                           RG::IFrameDisplayObserver&             frameDisplayObserver,
                           IRandomExporter&                       randomExporter);

    void Wait ();

private:
    void SetUniforms (const GVK::UUID& renderOperationId, const std::shared_ptr<Stimulus const>& stimulus, const uint32_t resourceIndex, const uint32_t frameIndex);
};


#endif