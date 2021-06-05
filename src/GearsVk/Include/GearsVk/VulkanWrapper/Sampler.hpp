#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "GearsVkAPI.hpp"
#include "Utils/MovablePtr.hpp"
#include "Utils/Utils.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class GVK_RENDERER_API Sampler : public VulkanObject {
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