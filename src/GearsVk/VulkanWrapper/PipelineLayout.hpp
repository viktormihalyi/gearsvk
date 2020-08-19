#ifndef PIPELINELAYOUT_HPP
#define PIPELINELAYOUT_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

USING_PTR (PipelineLayout);
class GEARSVK_API PipelineLayout : public Noncopyable {
private:
    const VkDevice   device;
    VkPipelineLayout handle;

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
    USING_CREATE (PipelineLayout);

    PipelineLayout (VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
        : device (device)
        , handle (CreatePipelineLayout (device, descriptorSetLayouts))
    {
    }

    ~PipelineLayout ()
    {
        vkDestroyPipelineLayout (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkPipelineLayout () const
    {
        return handle;
    }
};

#endif