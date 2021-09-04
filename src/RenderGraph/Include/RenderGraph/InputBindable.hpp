#ifndef INPUTBINDABLE_HPPc
#define INPUTBINDABLE_HPPc

#include <cstdint>
#include <functional>
#include <vulkan/vulkan.h>

#include "Node.hpp"

namespace RG {

class InputBufferBindable {
public:
    virtual VkBuffer GetBufferForFrame (uint32_t frameIndex) = 0;
    
    std::function<VkBuffer (uint32_t)> GetBufferForFrameProvider ()
    {
        return [=] (uint32_t frameIndex) -> VkBuffer {
            return GetBufferForFrame (frameIndex);
        };
    }

    virtual uint32_t GetBufferSize () = 0;
};

class InputImageBindable {
public:
    virtual VkImageView GetImageViewForFrame (uint32_t frameIndex, uint32_t layerIndex) = 0;
    virtual VkSampler   GetSampler ()                                                   = 0;

    std::function<VkImageView (uint32_t, uint32_t)> GetImageViewForFrameProvider ()
    {
        return [=] (uint32_t frameIndex, uint32_t layerIndex) -> VkImageView {
            return GetImageViewForFrame (frameIndex, layerIndex);
        };
    }

    std::function<VkSampler ()> GetSamplerProvider ()
    {
        return [=] () -> VkSampler {
            return GetSampler ();
        };
    }
};

} // namespace RG

#endif
