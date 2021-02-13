#ifndef VULKANOBJECT_HPP
#define VULKANOBJECT_HPP

#include "GearsVkAPI.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "UUID.hpp"

namespace GVK {

class GVK_RENDERER_API VulkanObject : public Noncopyable {
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