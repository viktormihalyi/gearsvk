#ifndef VULKANWRAPPER_EVENT_HPP
#define VULKANWRAPPER_EVENT_HPP

#include "VulkanWrapper/VulkanWrapperExport.hpp"
#include "VulkanWrapper/VulkanObject.hpp"

#include "Utils/MovablePtr.hpp"

namespace VW {

class VULKANWRAPPER_DLL_EXPORT Event : public GVK::VulkanObject {
private:
    VkDevice                 device;
    GVK::MovablePtr<VkEvent> handle;

public:
    Event (VkDevice device);

    virtual ~Event () override;

    virtual void* GetHandleForName () const override { return handle; }

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_EVENT; }

    operator VkEvent () const { return handle; }
};

} // namespace VW

#endif