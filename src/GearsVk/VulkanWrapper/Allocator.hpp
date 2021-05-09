#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include "MovablePtr.hpp"
#include "VulkanObject.hpp"

#include "vk_mem_alloc.h"

namespace GVK {

class Allocator : public VulkanObject {
private:
    GVK::MovablePtr<VmaAllocator> handle;

public:
    Allocator (VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    Allocator (Allocator&&) = default;
    Allocator& operator= (Allocator&&) = default;

    virtual ~Allocator () override;

    operator VmaAllocator () const { return handle; }
};

} // namespace GVK

#endif