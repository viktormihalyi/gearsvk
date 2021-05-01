#ifndef VULKANOBJECT_HPP
#define VULKANOBJECT_HPP

#include "GearsVkAPI.hpp"
#include "UUID.hpp"

namespace GVK {

class GVK_RENDERER_API VulkanObject {
private:
    GVK::UUID   uuid;
    std::string name;

protected:
    VulkanObject ();

public:
    virtual ~VulkanObject ();

    void SetName (const std::string& value) { name = value; }

    const std::string& GetName () const { return name; }

    const GVK::UUID& GetUUID () const { return uuid; }
};

} // namespace GVK

#endif