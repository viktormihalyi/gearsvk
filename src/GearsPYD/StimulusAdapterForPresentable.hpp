#ifndef STIMULUSADAPTERFORPRESENTABLE_HPP
#define STIMULUSADAPTERFORPRESENTABLE_HPP

#include "Noncopyable.hpp"
#include "Ptr.hpp"

// from RenderGraph
#include "GraphRenderer.hpp"
#include "Operation.hpp"

// from Gears
#include "core/Pass.h"
#include "core/Stimulus.h"


namespace RG {
class RenderGraph;
class UniformReflection;
class Operation;
} // namespace RG

class Presentable;
class VulkanEnvironment;


USING_PTR (StimulusAdapterForPresentable);
class StimulusAdapterForPresentable : public Noncopyable {
    USING_CREATE (StimulusAdapterForPresentable);

private:
    const Stimulus::CP     stimulus;
    const Ptr<Presentable> presentable;

    Ptr<RG::RenderGraph>                    renderGraph;
    Ptr<RG::UniformReflection>              reflection;
    std::map<Pass::P, Ptr<RG::Operation>>   passToOperation;
    RG::SynchronizedSwapchainGraphRendererU renderer;

public:
    StimulusAdapterForPresentable (const VulkanEnvironment& environment, Ptr<Presentable>& presentable, const Stimulus::CP& stimulus);

    void RenderFrameIndex (const uint32_t frameIndex);

private:
    void SetConstantUniforms ();
    void SetUniforms (const GearsVk::UUID& renderOperationId, const uint32_t frameIndex);
};


#endif