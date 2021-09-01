#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "VulkanWrapper/VulkanWrapperAPI.hpp"
#include "Utils/MovablePtr.hpp"
#include "VulkanObject.hpp"

#include <vector>
#include <vulkan/vulkan.h>

namespace GVK {

struct VULKANWRAPPER_API InstanceSettings {
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};


extern VULKANWRAPPER_API const InstanceSettings instanceDebugMode;
extern VULKANWRAPPER_API const InstanceSettings instanceReleaseMode;


class VULKANWRAPPER_API Instance : public VulkanObject, public Nonmovable {
private:
    GVK::MovablePtr<VkInstance> handle;

public:
    Instance (const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers);
    Instance (const InstanceSettings& settings);

    virtual ~Instance () override;

    virtual void* GetHandleForName () const override { return handle; }

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_INSTANCE; }

    operator VkInstance () const { return handle; }
};

} // namespace GVK

#endif