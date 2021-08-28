#include "Resource.hpp"

namespace RG {


// sampled formats should always be _SRGB?
const VkFormat WritableImageResource::SingleImageResource::FormatRGBA = VK_FORMAT_R8G8B8A8_SRGB;
const VkFormat WritableImageResource::SingleImageResource::FormatRGB  = VK_FORMAT_R8G8B8_SRGB;


} // namespace RG
