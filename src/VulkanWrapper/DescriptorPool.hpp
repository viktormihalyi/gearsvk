#ifndef DESCRIPTORPOOL_HPP
#define DESCRIPTORPOOL_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class DescriptorPool : public Noncopyable {
private:
    const VkDevice         device;
    const VkDescriptorPool handle;

    static VkDescriptorPool CreateDescriptorPool (VkDevice device, uint32_t descriptorCountUbo, uint32_t descriptorCountSampler, uint32_t maxSets)
    {
        VkDescriptorPool handle;

        std::vector<VkDescriptorPoolSize> poolSizes = {};
        if (descriptorCountUbo > 0)
            poolSizes.push_back ({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCountUbo });
        if (descriptorCountSampler > 0)
            poolSizes.push_back ({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCountSampler });

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount              = static_cast<uint32_t> (poolSizes.size ());
        poolInfo.pPoolSizes                 = poolSizes.data ();
        poolInfo.maxSets                    = maxSets;

        VkDescriptorPool descriptorPool;
        if (ERROR (vkCreateDescriptorPool (device, &poolInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create descriptor pool");
        }

        return handle;
    }

public:
    USING_PTR (DescriptorPool);

    DescriptorPool (VkDevice device, uint32_t descriptorCountUbo, uint32_t descriptorCountSampler, uint32_t maxSets)
        : device (device)
        , handle (CreateDescriptorPool (device, descriptorCountUbo, descriptorCountSampler, maxSets))
    {
    }

    ~DescriptorPool ()
    {
        vkDestroyDescriptorPool (device, handle, nullptr);
    }

    operator VkDescriptorPool () const
    {
        return handle;
    }
};

#endif