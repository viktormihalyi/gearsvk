#ifndef DESCRIPTORPOOL_HPP
#define DESCRIPTORPOOL_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Utils.hpp"

class DescriptorPool : public Noncopyable {
private:
    const VkDevice   device;
    VkDescriptorPool handle;

public:
    USING_PTR (DescriptorPool);

    DescriptorPool (VkDevice device, uint32_t descriptorCountUbo, uint32_t descriptorCountSampler, uint32_t maxSets)
        : device (device)
        , handle (VK_NULL_HANDLE)
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {};
        if (descriptorCountUbo > 0)
            poolSizes.push_back ({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCountUbo });
        if (descriptorCountSampler > 0)
            poolSizes.push_back ({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCountSampler });

        if (ERROR (poolSizes.empty ())) {
            throw std::runtime_error ("empty pool");
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount              = static_cast<uint32_t> (poolSizes.size ());
        poolInfo.pPoolSizes                 = poolSizes.data ();
        poolInfo.maxSets                    = maxSets;

        if (ERROR (vkCreateDescriptorPool (device, &poolInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create descriptor pool");
        }
    }

    ~DescriptorPool ()
    {
        vkDestroyDescriptorPool (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkDescriptorPool () const
    {
        return handle;
    }
};

#endif