#ifndef INPUTBINDABLE_HPPc
#define INPUTBINDABLE_HPPc

#include <cstdint>
#include <vulkan/vulkan.h>

class InputBufferBindable {
public:
    virtual VkBuffer GetBufferForFrame (uint32_t frameIndex) = 0;
    virtual uint32_t GetBufferSize ()                        = 0;
};

class InputImageBindable {
public:
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t layerIndex) = 0;
    virtual VkSampler   GetSampler ()                                                   = 0;
};

#endif