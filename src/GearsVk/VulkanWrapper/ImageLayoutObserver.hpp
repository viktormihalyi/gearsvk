#ifndef IMAGELAYOUTOBSERVER_HPP
#define IMAGELAYOUTOBSERVER_HPP

#include "GearsVkAPI.hpp"
#include "Ptr.hpp"

#include <vulkan/vulkan.h>

class ImageBase;

USING_PTR (ImageLayoutObserver);
class GEARSVK_API ImageLayoutObserver {
public:
    ~ImageLayoutObserver () = default;

    virtual void ImageLayoutChanged (const ImageBase& image, VkImageLayout from, VkImageLayout to) = 0;
};


#endif