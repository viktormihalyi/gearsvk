#ifndef DESCRIPTORSET_HPP
#define DESCRIPTORSET_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"

#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

namespace GVK {

class GVK_RENDERER_API DescriptorSet : public VulkanObject {
private:
    VkDevice                         device;
    VkDescriptorPool                 descriptorPool;
    GVK::MovablePtr<VkDescriptorSet> handle;

public:
    DescriptorSet (VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout layout)
        : device (device)
        , descriptorPool (descriptorPool)
        , handle (VK_NULL_HANDLE)
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool              = descriptorPool;
        allocInfo.descriptorSetCount          = 1;
        allocInfo.pSetLayouts                 = &layout;

        if (GVK_ERROR (vkAllocateDescriptorSets (device, &allocInfo, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to allocate descriptor sets!");
        }
    }

    DescriptorSet (DescriptorSet&&) = default;
    DescriptorSet& operator= (DescriptorSet&&) = default;

    DescriptorSet (VkDevice device, const DescriptorPool& descriptorPool, const DescriptorSetLayout& layout)
        : DescriptorSet (device, descriptorPool.operator VkDescriptorPool (), layout)
    {
    }

    virtual ~DescriptorSet () override
    {
        // only free for VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
        // vkFreeDescriptorSets (device, descriptorPool, 1, &handle);
        handle = nullptr;
    }


    mutable std::vector<std::tuple<uint32_t, VkDescriptorType, VkWriteDescriptorSet>> writtenInfos;


    operator VkDescriptorSet () const
    {
        return handle;
    }
};

} // namespace GVK

#endif