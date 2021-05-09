#include "Allocator.hpp"
#include "Assert.hpp"

#include "spdlog/spdlog.h"

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "vk_mem_alloc.h"


namespace GVK {

Allocator::Allocator (VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
    : handle { VK_NULL_HANDLE }
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice         = physicalDevice;
    allocatorInfo.device                 = device;
    allocatorInfo.instance               = instance;

    if (GVK_ERROR (vmaCreateAllocator (&allocatorInfo, &handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to create vma allocator");
    }

    spdlog::debug ("VmaAllocator created: {}, uuid: {}.", handle, GetUUID ().GetValue ());
}


Allocator::~Allocator ()
{
    vmaDestroyAllocator (handle);
    handle = nullptr;
}

} // namespace GVK
