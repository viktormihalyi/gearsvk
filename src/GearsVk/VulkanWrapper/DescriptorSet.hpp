#ifndef DESCRIPTORSET_HPP
#define DESCRIPTORSET_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

USING_PTR (DescriptorSet);

class GEARSVK_API DescriptorSet : public Noncopyable {
private:
    const VkDevice         device;
    const VkDescriptorPool descriptorPool;
    VkDescriptorSet        handle;

public:
    USING_CREATE (DescriptorSet);

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

    DescriptorSet (VkDevice device, const DescriptorPool& descriptorPool, const DescriptorSetLayout& layout)
        : DescriptorSet (device, descriptorPool.operator VkDescriptorPool (), layout)
    {
    }

    ~DescriptorSet ()
    {
        // only free for VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
        // vkFreeDescriptorSets (device, descriptorPool, 1, &handle);
        handle = VK_NULL_HANDLE;
    }


    mutable std::vector<std::tuple<uint32_t, VkDescriptorType, VkWriteDescriptorSet>> writtenInfos;


    void Write (const std::vector<VkWriteDescriptorSet>& writes) const
    {
        vkUpdateDescriptorSets (device, writes.size (), writes.data (), 0, nullptr);
    }


    void WriteOneBufferInfo (uint32_t binding, VkDescriptorType descriptorType, const VkDescriptorBufferInfo& bufferInfo) const
    {
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet               = handle;
        descriptorWrite.dstBinding           = binding;
        descriptorWrite.dstArrayElement      = 0;
        descriptorWrite.descriptorType       = descriptorType;
        descriptorWrite.descriptorCount      = 1;
        descriptorWrite.pBufferInfo          = &bufferInfo;
        descriptorWrite.pImageInfo           = nullptr;
        descriptorWrite.pTexelBufferView     = nullptr;
        vkUpdateDescriptorSets (device, 1, &descriptorWrite, 0, nullptr);

        writtenInfos.emplace_back (binding, descriptorType, descriptorWrite);
    }


    operator VkDescriptorSet () const
    {
        return handle;
    }
};


#endif