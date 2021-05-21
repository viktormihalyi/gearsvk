#ifndef PIPELINELAYOUT_HPP
#define PIPELINELAYOUT_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class GVK_RENDERER_API PipelineLayout : public VulkanObject {
private:
    VkDevice                          device;
    GVK::MovablePtr<VkPipelineLayout> handle;

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

    static VkPipelineLayout CreatePipelineLayout (VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPipelineLayout handle;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount             = static_cast<uint32_t> (descriptorSetLayouts.size ());
        pipelineLayoutInfo.pSetLayouts                = descriptorSetLayouts.data ();
        pipelineLayoutInfo.pushConstantRangeCount     = 0;       // Optional
        pipelineLayoutInfo.pPushConstantRanges        = nullptr; // Optional

        if (GVK_ERROR (vkCreatePipelineLayout (device, &pipelineLayoutInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create pipeline layout");
        }

        return handle;
    }

public:
    PipelineLayout (VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
        : device (device)
        , handle (CreatePipelineLayout (device, descriptorSetLayouts))
        , descriptorSetLayouts (descriptorSetLayouts)
    {
    }

    PipelineLayout (PipelineLayout&&) = default;
    PipelineLayout& operator= (PipelineLayout&&) = default;

    virtual ~PipelineLayout () override
    {
        vkDestroyPipelineLayout (device, handle, nullptr);
        handle = nullptr;
    }

    operator VkPipelineLayout () const
    {
        return handle;
    }
};

} // namespace GVK

#endif