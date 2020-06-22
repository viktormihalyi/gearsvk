#ifndef DRAWRECORDABLE_HPP
#define DRAWRECORDABLE_HPP

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

#include "Ptr.hpp"


USING_PTR (DrawRecordable);
class DrawRecordable {
public:
    virtual ~DrawRecordable () = default;

    virtual void                                           Record (VkCommandBuffer) const = 0;
    virtual std::vector<VkVertexInputAttributeDescription> GetAttributes () const         = 0;
    virtual std::vector<VkVertexInputBindingDescription>   GetBindings () const           = 0;
};

#endif