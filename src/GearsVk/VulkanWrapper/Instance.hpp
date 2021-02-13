#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "GearsVkAPI.hpp"
#include "Noncopyable.hpp"
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
    VkInstance handle;

public:
    Instance (const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers);
    Instance (const InstanceSettings& settings);

    virtual ~Instance () override;

    operator VkInstance () const { return handle; }
};

} // namespace GVK

#endif