#ifndef VULKANOBJECT_HPP
#define VULKANOBJECT_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"
#include "Utils/UUID.hpp"

namespace GVK {

class VULKANWRAPPER_API VulkanObject {
private:
    GVK::UUID   uuid;
    std::string name;

protected:
    VulkanObject ();
    VulkanObject (const VulkanObject&) = default;
    VulkanObject (VulkanObject&&)      = default;

public:
    virtual ~VulkanObject ();

    void SetName (const std::string& value) { name = value; }

    const std::string& GetName () const { return name; }

    const GVK::UUID& GetUUID () const { return uuid; }
};

} // namespace GVK

#endif