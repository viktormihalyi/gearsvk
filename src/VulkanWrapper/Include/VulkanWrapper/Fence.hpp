#ifndef FENCE_HPP
#define FENCE_HPP

#include <vulkan/vulkan.h>

#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"
#include "VulkanObject.hpp"

namespace GVK {


class VULKANWRAPPER_API Fence : public VulkanObject {
private:
    VkDevice                 device;
    GVK::MovablePtr<VkFence> handle;

public:
    Fence (VkDevice device, bool signaled = true);
    Fence (Fence&&) = default;
    Fence& operator= (Fence&&) = default;

    virtual ~Fence () override;

    operator VkFence () const { return handle; }

    void Wait () const;

    void Reset () const;

private:
    void WaitImpl () const;
};

} // namespace GVK

#endif