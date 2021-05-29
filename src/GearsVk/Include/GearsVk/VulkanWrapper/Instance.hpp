#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "GearsVkAPI.hpp"
#include "MovablePtr.hpp"
#include "VulkanObject.hpp"

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace GVK {

struct GVK_RENDERER_API InstanceSettings {
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};


extern GVK_RENDERER_API const InstanceSettings instanceDebugMode;
extern GVK_RENDERER_API const InstanceSettings instanceReleaseMode;


class GVK_RENDERER_API Instance : public VulkanObject {
private:
    GVK::MovablePtr<VkInstance> handle;

public:
    Instance (const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers);
    Instance (const InstanceSettings& settings);

    Instance (Instance&&) = default;
    Instance& operator= (Instance&&) = default;

    virtual ~Instance () override;

    operator VkInstance () const { return handle; }
};

} // namespace GVK

#endif