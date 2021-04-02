#ifndef VULKANOBJECT_HPP
#define VULKANOBJECT_HPP

#include "GearsVkAPI.hpp"
#include "UUID.hpp"

namespace GVK {

class GVK_RENDERER_API VulkanObject {
private:
    GVK::UUID uuid;

protected:
    VulkanObject ();

public:
    virtual ~VulkanObject ();

    const GVK::UUID& GetUUID () const
    {
        return uuid;
    }
};

} // namespace GVK

#endif