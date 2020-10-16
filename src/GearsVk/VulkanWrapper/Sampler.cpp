#include "Sampler.hpp"


Sampler::Sampler (VkDevice device, VkSamplerAddressMode addressMode)
    : device (device)
{
    VkSamplerCreateInfo samplerInfo     = {};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = VK_FILTER_LINEAR; // TODO MAKE IT A CHOICE
    samplerInfo.minFilter               = VK_FILTER_LINEAR; // TODO MAKE IT A CHOICE
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


Sampler::~Sampler ()
{
    vkDestroySampler (device, handle, nullptr);
    handle = VK_NULL_HANDLE;
}
