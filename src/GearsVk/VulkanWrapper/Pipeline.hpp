#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class GVK_RENDERER_API Pipeline : public VulkanObject {
private:
    const VkDevice device;
    VkPipeline     handle;

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

} // namespace GVK

#endif