#ifndef DESCRIPTORSETLAYOUT_HPP
#define DESCRIPTORSETLAYOUT_HPP

#include <vulkan/vulkan.h>

#include "Assert.hpp"
#include "MovablePtr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {

class GVK_RENDERER_API DescriptorSetLayout : public VulkanObject {
private:
    VkDevice                               device;
    GVK::MovablePtr<VkDescriptorSetLayout> handle;

    std::vector<VkDescriptorSetLayoutBinding> bindings;

public:
    DescriptorSetLayout (VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        : device (device)
        , handle (VK_NULL_HANDLE)
        , bindings (bindings)
    {
        for (size_t i = 0; i < bindings.size (); ++i) {
            for (size_t j = 0; j < bindings.size (); ++j) {
                if (i != j && bindings[i].binding == bindings[j].binding && (bindings[i].stageFlags ^ bindings[j].stageFlags) == 0) {
                    GVK_BREAK ("duplicate binding");
                }
            }
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount                    = static_cast<uint32_t> (bindings.size ());
        layoutInfo.pBindings                       = bindings.data ();

        if (GVK_ERROR (vkCreateDescriptorSetLayout (device, &layoutInfo, nullptr, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout (DescriptorSetLayout&&) = default;
    DescriptorSetLayout& operator= (DescriptorSetLayout&&) = default;

    virtual ~DescriptorSetLayout () override
    {
        vkDestroyDescriptorSetLayout (device, handle, nullptr);
        handle = nullptr;
    }

    operator VkDescriptorSetLayout () const
    {
        return handle;
    }
};

} // namespace GVK

#endif