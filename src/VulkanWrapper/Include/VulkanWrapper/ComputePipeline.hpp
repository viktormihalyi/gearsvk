#ifndef VULKANWRAPPER_COMPUTEPIPELINE_HPP
#define VULKANWRAPPER_COMPUTEPIPELINE_HPP

#include "VulkanWrapper/VulkanWrapperExport.hpp"
#include "VulkanWrapper/PipelineBase.hpp"

#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class ShaderModule;

class VULKANWRAPPER_DLL_EXPORT ComputePipeline : public PipelineBase {
private:
    VkDevice                    device;
    GVK::MovablePtr<VkPipeline> handle;

public:
    ComputePipeline (VkDevice            device,
                     VkPipelineLayout    pipelineLayout,
                     const ShaderModule& shaderModule);

    ComputePipeline (ComputePipeline&&) = default;
    ComputePipeline& operator= (ComputePipeline&&) = default;

    virtual ~ComputePipeline () override
    {
        vkDestroyPipeline (device, handle, nullptr);
        handle = nullptr;
    }

    virtual void* GetHandleForName () const override { return handle; }

    virtual operator VkPipeline () const override { return handle; }
};

} // namespace GVK

#endif