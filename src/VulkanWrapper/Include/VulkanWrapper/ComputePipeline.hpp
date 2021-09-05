#ifndef VULKANWRAPPER_COMPUTEPIPELINE_HPP
#define VULKANWRAPPER_COMPUTEPIPELINE_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"
#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class ShaderModule;

class VULKANWRAPPER_API ComputePipeline : public VulkanObject {
private:
    VkDevice                    device;
    GVK::MovablePtr<VkPipeline> handle;

public:
    ComputePipeline (VkDevice            device,
                     VkPipelineLayout    pipelineLayout,
                     VkRenderPass        renderPass,
                     const ShaderModule& shaderModule);

    ComputePipeline (ComputePipeline&&) = default;
    ComputePipeline& operator= (ComputePipeline&&) = default;

    virtual ~ComputePipeline () override
    {
        vkDestroyPipeline (device, handle, nullptr);
        handle = nullptr;
    }

    virtual void* GetHandleForName () const override { return handle; }

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_PIPELINE; }

    operator VkPipeline () const
    {
        return handle;
    }
};

} // namespace GVK

#endif