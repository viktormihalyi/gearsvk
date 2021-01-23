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


struct GVK_RENDERER_API InstanceSettings {
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
};


extern GVK_RENDERER_API const InstanceSettings instanceDebugMode;
extern GVK_RENDERER_API const InstanceSettings instanceReleaseMode;


USING_PTR (Instance);
class GVK_RENDERER_API Instance : public VulkanObject {
    USING_CREATE (Instance);

private:
    VkInstance handle;

public:
    Instance (const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& instanceLayers);
    Instance (const InstanceSettings& settings);

    virtual ~Instance () override;

    operator VkInstance () const { return handle; }
};

#endif