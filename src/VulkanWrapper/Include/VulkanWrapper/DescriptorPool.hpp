#ifndef DESCRIPTORPOOL_HPP
#define DESCRIPTORPOOL_HPP

#include <vulkan/vulkan.h>

#include "Utils/Assert.hpp"
#include "Utils/Noncopyable.hpp"
#include "Utils/Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class /* VULKANWRAPPER_API */ DescriptorPool : public VulkanObject {
private:
    const VkDevice   device;
    VkDescriptorPool handle;

public:
    DescriptorPool (VkDevice device, uint32_t descriptorCountUbo, uint32_t descriptorCountSampler, uint32_t maxSets)
        : device (device)
        , handle (VK_NULL_HANDLE)
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {};
        if (descriptorCountUbo > 0)
            poolSizes.push_back ({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCountUbo });
        if (descriptorCountSampler > 0)
            poolSizes.push_back ({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCountSampler });

        if (GVK_ERROR (poolSizes.empty ())) {
            throw std::runtime_error ("empty pool");
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount              = static_cast<uint32_t> (poolSizes.size ());
        poolInfo.pPoolSizes                 = poolSizes.data ();
        poolInfo.maxSets                    = maxSets;

        if (GVK_ERROR (vkCreateDescriptorPool (device, &poolInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create descriptor pool");
        }
    }

    DescriptorPool (DescriptorPool&&) = default;
    DescriptorPool& operator= (DescriptorPool&&) = default;

    ~DescriptorPool ()
    {
        vkDestroyDescriptorPool (device, handle, nullptr);
        handle = nullptr;
    }

    operator VkDescriptorPool () const
    {
        return handle;
    }
};

} // namespace GVK

#endif