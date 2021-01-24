#ifndef STIMULUSADAPTERFORPRESENTABLE_HPP
#define STIMULUSADAPTERFORPRESENTABLE_HPP

#include "Noncopyable.hpp"
#include "Ptr.hpp"

// from std
#include <map>

class Pass;
class Stimulus;

namespace GearsVk {
class UUID;
}

namespace RG {
class RenderGraph;
class UniformReflection;
class Operation;
class SynchronizedSwapchainGraphRenderer;
} // namespace RG

class Presentable;
class VulkanEnvironment;


USING_PTR (StimulusAdapterForPresentable);
class StimulusAdapterForPresentable : public Noncopyable {
    USING_CREATE (StimulusAdapterForPresentable);

private:
    const PtrC<Stimulus>   stimulus;
    const Ptr<Presentable> presentable;

    Ptr<RG::RenderGraph>                      renderGraph;
    Ptr<RG::UniformReflection>                reflection;
    std::map<Ptr<Pass>, Ptr<RG::Operation>>   passToOperation;
    U<RG::SynchronizedSwapchainGraphRenderer> renderer;

public:
    StimulusAdapterForPresentable (const VulkanEnvironment& environment, Ptr<Presentable>& presentable, const PtrC<Stimulus>& stimulus);

    void RenderFrameIndex (const uint32_t frameIndex);

    void Wait ();

private:
    void SetConstantUniforms ();
    void SetUniforms (const GearsVk::UUID& renderOperationId, const uint32_t frameIndex);
};


#endif