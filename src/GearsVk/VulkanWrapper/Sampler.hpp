#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"
#include "VulkanObject.hpp"

#include <vulkan/vulkan.h>

namespace GVK {

class GVK_RENDERER_API Sampler : public VulkanObject {
private:
    const VkDevice device;
    VkSampler      handle;
    const VkFilter filter;

public:
    Sampler (VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    ~Sampler ();

    operator VkSampler () const { return handle; }
};

} // namespace GVK

#endif