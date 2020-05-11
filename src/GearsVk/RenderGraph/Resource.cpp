#include "Resource.hpp"


namespace RG {


// sampled formats should always be _SRGB?
const VkFormat WritableImageResource::SingleImageResource::FormatRGBA = VK_FORMAT_R8G8B8A8_SRGB;
const VkFormat WritableImageResource::SingleImageResource::FormatRGB  = VK_FORMAT_R8G8B8_SRGB;


#define ADDVISITOR(type, callback)                   \
    if (auto castedRes = dynamic_cast<type*> (&res)) \
        callback (*castedRes);                       \
    else


void ResourceVisitor::Visit (Resource& res)
{
    ADDVISITOR (WritableImageResource, onWritableImage)
    ADDVISITOR (ReadOnlyImageResource, onReadOnlyImage)
    ADDVISITOR (SwapchainImageResource, onSwapchainImage)
    ADDVISITOR (UniformBlockResource, onUniformBlock)
    ADDVISITOR (UniformReflectionResource, onUniformReflection)
    {
        BREAK ("unexpected resource type");
        throw std::runtime_error ("unexpected resource type");
    }
}

} // namespace RG