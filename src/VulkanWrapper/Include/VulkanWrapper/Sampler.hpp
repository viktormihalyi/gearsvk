#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class VULKANWRAPPER_API Sampler : public VulkanObject {
private:
    VkDevice                   device;
    GVK::MovablePtr<VkSampler> handle;
    VkFilter                   filter;

public:
    Sampler (VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    Sampler (Sampler&&) = default;
    Sampler& operator= (Sampler&&) = default;

    ~Sampler ();

    operator VkSampler () const { return handle; }
};

} // namespace GVK

#endif