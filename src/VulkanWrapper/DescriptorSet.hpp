#ifndef DESCRIPTORSET_HPP
#define DESCRIPTORSET_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"


class DescriptorSet : public Noncopyable {
private:
    const VkDevice         device;
    const VkDescriptorPool descriptorPool;
    VkDescriptorSet        handle;

public:
    USING_PTR (DescriptorSet);

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

        if (vkAllocateDescriptorSets (device, &allocInfo, &handle) != VK_SUCCESS) {
            throw std::runtime_error ("failed to allocate descriptor sets!");
        }
    }

    ~DescriptorSet ()
    {
        // only free for VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
        // vkFreeDescriptorSets (device, descriptorPool, 1, &handle);
    }

    operator VkDescriptorSet () const
    {
        return handle;
    }
};


#endif