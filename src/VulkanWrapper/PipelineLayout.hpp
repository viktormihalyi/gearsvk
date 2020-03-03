#ifndef PIPELINELAYOUT_HPP
#define PIPELINELAYOUT_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class PipelineLayout : public Noncopyable {
private:
    const VkDevice         device;
    const VkPipelineLayout handle;

    static VkPipelineLayout CreatePipelineLayout (VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPipelineLayout handle;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount             = static_cast<uint32_t> (descriptorSetLayouts.size ());
        pipelineLayoutInfo.pSetLayouts                = descriptorSetLayouts.data ();
        pipelineLayoutInfo.pushConstantRangeCount     = 0;       // Optional
        pipelineLayoutInfo.pPushConstantRanges        = nullptr; // Optional

        if (ERROR (vkCreatePipelineLayout (device, &pipelineLayoutInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create pipeline layout");
        }

        return handle;
    }

public:
    PipelineLayout (VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
        : device (device)
        , handle (CreatePipelineLayout (device, descriptorSetLayouts))
    {
    }

    ~PipelineLayout ()
    {
        vkDestroyPipelineLayout (device, handle, nullptr);
    }

    operator VkPipelineLayout () const
    {
        return handle;
    }
};

#endif