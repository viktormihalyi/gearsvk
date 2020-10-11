#ifndef RG_IMAGERESOURCE_HPP
#define RG_IMAGERESOURCE_HPP

#include "GearsVkAPI.hpp"

#include "Ptr.hpp"

#include "Resource.hpp"

#include "CommandBuffer.hpp"
#include "Image.hpp"

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace RG {


USING_PTR (ImageResource);
class GEARSVK_API ImageResource : public Resource {
public:
    virtual ~ImageResource () = default;

    virtual void                    BindRead (uint32_t imageIndex, CommandBuffer& commandBuffer)  = 0;
    virtual void                    BindWrite (uint32_t imageIndex, CommandBuffer& commandBuffer) = 0;
    virtual VkImageLayout           GetFinalLayout () const                                       = 0;
    virtual VkFormat                GetFormat () const                                            = 0;
    virtual uint32_t                GetDescriptorCount () const                                   = 0;
    virtual std::vector<ImageBase*> GetImages () const                                            = 0;
};

} // namespace RG

#endif
