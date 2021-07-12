#ifndef VW_DEBUGREPORTCALLBACK_HPP
#define VW_DEBUGREPORTCALLBACK_HPP

#include <vulkan/vulkan.h>

#include "VulkanWrapper/VulkanObject.hpp"
#include "Utils/Noncopyable.hpp"
#include "VulkanWrapper/VulkanWrapperAPI.hpp"

namespace GVK {

class VULKANWRAPPER_API DebugReportCallback : public VulkanObject, public Noncopyable {
private:
    VkInstance               instance;
    VkDebugReportCallbackEXT handle;

public:
    DebugReportCallback (VkInstance instance);

    virtual ~DebugReportCallback () override;
};

} // namespace GVK

#endif
