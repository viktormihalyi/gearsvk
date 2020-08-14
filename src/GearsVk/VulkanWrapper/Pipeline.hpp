#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

USING_PTR (Pipeline);
class GEARSVK_API Pipeline : public VulkanObject {
private:
    const VkDevice device;
    VkPipeline     handle;

public:
    USING_CREATE (Pipeline);

    Pipeline (VkDevice                                              device,
              uint32_t                                              width,
              uint32_t                                              height,
              uint32_t                                              attachmentCount,
              VkPipelineLayout                                      pipelineLayout,
              VkRenderPass                                          renderPass,
              const std::vector<VkPipelineShaderStageCreateInfo>&   shaderStages,
              const std::vector<VkVertexInputBindingDescription>&   vertexBindingDescriptions,
              const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions,
              VkPrimitiveTopology                                   topology);

    ~Pipeline ()
    {
        vkDestroyPipeline (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkPipeline () const
    {
        return handle;
    }
};

#endif