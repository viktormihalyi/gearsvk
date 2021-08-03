#ifndef VULKANOBJECT_HPP
#define VULKANOBJECT_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"
#include "Utils/UUID.hpp"
#include "Utils/Noncopyable.hpp"

namespace GVK {

class VULKANWRAPPER_API VulkanObject : public Noncopyable {
private:
    GVK::UUID   uuid;
    std::string name;

protected:
    VulkanObject ();

    VulkanObject (const VulkanObject&) = delete;
    VulkanObject& operator= (const VulkanObject&) = delete;

    VulkanObject (VulkanObject&&) = default;
    VulkanObject& operator= (VulkanObject&&) = default;

public:
    virtual ~VulkanObject () override;

    void SetName (const std::string& value) { name = value; }

    const std::string& GetName () const { return name; }

    const GVK::UUID& GetUUID () const { return uuid; }
};

} // namespace GVK

#endif