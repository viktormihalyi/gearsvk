#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include <memory>

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"

namespace GVK {

class Allocator : public Noncopyable {
private:
    VmaAllocator handle;

public:
    Allocator (VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice         = physicalDevice;
        allocatorInfo.device                 = device;
        allocatorInfo.instance               = instance;

        if (GVK_ERROR (vmaCreateAllocator (&allocatorInfo, &handle) != VK_SUCCESS)) {
            throw std::runtime_error ("failed to create vma allocator");
        }
    }

    ~Allocator ()
    {
        vmaDestroyAllocator (handle);
        handle = VK_NULL_HANDLE;
    }

    operator VmaAllocator () const { return handle; }
};

} // namespace GVK

#endif