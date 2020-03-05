#ifndef DESCRIPTORSETLAYOUT_HPP
#define DESCRIPTORSETLAYOUT_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"


class DescriptorSetLayout : public Noncopyable {
private:
    const VkDevice              device;
    const VkDescriptorSetLayout handle;

    static VkDescriptorSetLayout CreateDescriptorSetLayout (VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    {
        VkDescriptorSetLayout handle;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount                    = static_cast<uint32_t> (bindings.size ());
        layoutInfo.pBindings                       = bindings.data ();

        if (ERROR (vkCreateDescriptorSetLayout (device, &layoutInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create descriptor set layout!");
        }

        return handle;
    }

public:
    USING_PTR (DescriptorSetLayout);

    DescriptorSetLayout (VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        : device (device)
        , handle (CreateDescriptorSetLayout (device, bindings))
    {
    }

    ~DescriptorSetLayout ()
    {
        vkDestroyDescriptorSetLayout (device, handle, nullptr);
    }

    operator VkDescriptorSetLayout () const
    {
        return handle;
    }
};

#endif