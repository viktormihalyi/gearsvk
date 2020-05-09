#ifndef SAMPLER_HPP
#define SAMPLER_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "Utils.hpp"

#include <vulkan/vulkan.h>

class Sampler : public Noncopyable {
private:
    const VkDevice device;
    VkSampler      handle;

public:
    USING_PTR (Sampler);

    Sampler (VkDevice device, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
        : device (device)
    {
        VkSamplerCreateInfo samplerInfo     = {};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = addressMode;
        samplerInfo.addressModeV            = addressMode;
        samplerInfo.addressModeW            = addressMode;
        samplerInfo.anisotropyEnable        = VK_FALSE;
        samplerInfo.maxAnisotropy           = 16;
        samplerInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_NEVER;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 0.0f;

        if (vkCreateSampler (device, &samplerInfo, nullptr, &handle) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create texture sampler!");
        }
    }

    ~Sampler ()
    {
        vkDestroySampler (device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }

    operator VkSampler () const { return handle; }
};

#endif