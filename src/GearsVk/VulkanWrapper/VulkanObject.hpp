#ifndef VULKANOBJECT_HPP
#define VULKANOBJECT_HPP

#include "GearsVkAPI.hpp"
#include "Noncopyable.hpp"
#include "Ptr.hpp"
#include "UUID.hpp"

USING_PTR (VulkanObject);
class GEARSVK_API VulkanObject : public Noncopyable {
private:
    GearsVk::UUID uuid;

protected:
    VulkanObject () = default;

public:
    const GearsVk::UUID& GetUUID () const
    {
        return uuid;
    }
};

#endif