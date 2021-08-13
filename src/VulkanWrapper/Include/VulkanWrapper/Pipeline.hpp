#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

#include <vector>

namespace GVK {

class VULKANWRAPPER_API Pipeline : public VulkanObject {
private:
    VkDevice                    device;
    GVK::MovablePtr<VkPipeline> handle;

public:
    Pipeline (VkDevice                                              device,
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

    Pipeline (Pipeline&&) = default;
    Pipeline& operator= (Pipeline&&) = default;

    ~Pipeline ()
    {
        vkDestroyPipeline (device, handle, nullptr);
        handle = nullptr;
    }

    operator VkPipeline () const
    {
        return handle;
    }
};

} // namespace GVK

#endif