#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

#include <vector>

namespace GVK {

class VULKANWRAPPER_API GraphicsPipeline : public VulkanObject {
private:
    VkDevice                    device;
    GVK::MovablePtr<VkPipeline> handle;

public:
    GraphicsPipeline (VkDevice                                              device,
              uint32_t                                              width,
              uint32_t                                              height,
              uint32_t                                              attachmentCount,
              VkPipelineLayout                                      pipelineLayout,
              VkRenderPass                                          renderPass,
              const std::vector<VkPipelineShaderStageCreateInfo>&   shaderStages,
              const std::vector<VkVertexInputBindingDescription>&   vertexBindingDescriptions,
              const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions,
              VkPrimitiveTopology                                   topology,
              bool                                                  blendEnabled = true);

    GraphicsPipeline (GraphicsPipeline&&) = default;
    GraphicsPipeline& operator= (GraphicsPipeline&&) = default;

    virtual ~GraphicsPipeline () override
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