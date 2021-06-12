#ifndef STIMULUSADAPTERFORPRESENTABLE_HPP
#define STIMULUSADAPTERFORPRESENTABLE_HPP

// from GearsVk
#include "Utils/Event.hpp"
#include "GearsVk/RenderGraph/GraphRenderer.hpp"
#include "Utils/Noncopyable.hpp"
#include "Utils/Time.hpp"

// from std
#include <memory>
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
class ImageResource;
class WritableImageResource;
} // namespace RG
class Presentable;
class IFrameDisplayObserver;
class VulkanEnvironment;
} // namespace GVK


class IRandomExporter {
public:
    virtual ~IRandomExporter () = default;

    virtual bool IsEnabled () = 0;
    virtual void OnRandomTextureDrawn (GVK::RG::ImageResource& randomTexture, uint32_t resourceIndex, uint32_t frameIndex) = 0;
};


class StimulusAdapter : public Noncopyable {
private:
    const GVK::VulkanEnvironment&           environment;

    const std::shared_ptr<Stimulus const>   stimulus;
    const std::shared_ptr<GVK::Presentable> presentable;

    std::shared_ptr<GVK::RG::RenderGraph>                                renderGraph;
    std::shared_ptr<GVK::RG::UniformReflection>                          reflection;
    std::map<std::shared_ptr<Pass>, std::shared_ptr<GVK::RG::Operation>> passToOperation;
    std::shared_ptr<GVK::RG::Operation>                                  randomGeneratorOperation;

    std::shared_ptr<GVK::RG::WritableImageResource> randomTexture;

public:
    StimulusAdapter (const GVK::VulkanEnvironment& environment, std::shared_ptr<GVK::Presentable>& presentable, const std::shared_ptr<Stimulus const>& stimulus);

    void RenderFrameIndex (GVK::RG::Renderer&                     renderer,
                           const std::shared_ptr<Stimulus const>& stimulus,
                           const uint32_t                         frameIndex,
                           GVK::RG::IFrameDisplayObserver&        frameDisplayObserver,
                           IRandomExporter&                       randomExporter);

    void Wait ();

private:
    void SetConstantUniforms ();
    void SetUniforms (const GVK::UUID& renderOperationId, const std::shared_ptr<Stimulus const>& stimulus, const uint32_t frameIndex);
};


#endif