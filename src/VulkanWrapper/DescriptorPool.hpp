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

    static VkDescriptorPool CreateDescriptorPool (VkDevice device, uint32_t descriptorCount, uint32_t maxSets)
    {
        VkDescriptorPool handle;

        VkDescriptorPoolSize poolSize = {};
        poolSize.type                 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount      = descriptorCount;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount              = 1;
        poolInfo.pPoolSizes                 = &poolSize;
        poolInfo.maxSets                    = maxSets;

        VkDescriptorPool descriptorPool;
        if (ERROR (vkCreateDescriptorPool (device, &poolInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create descriptor pool");
        }

        return handle;
    }

public:
    DescriptorPool (VkDevice device, uint32_t descriptorCount, uint32_t maxSets)
        : device (device)
        , handle (CreateDescriptorPool (device, descriptorCount, maxSets))
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