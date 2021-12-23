#ifndef VW_DEBUGREPORTCALLBACK_HPP
#define VW_DEBUGREPORTCALLBACK_HPP

#include <vulkan/vulkan.h>

#include "VulkanWrapper/VulkanObject.hpp"
#include "Utils/Noncopyable.hpp"
#include "VulkanWrapper/VulkanWrapperExport.hpp"

namespace GVK {

class VULKANWRAPPER_DLL_EXPORT DebugReportCallback : public VulkanObject {
private:
    VkInstance               instance;
    VkDebugReportCallbackEXT handle;

public:
    DebugReportCallback (VkInstance instance);

    virtual ~DebugReportCallback () override;

    virtual void* GetHandleForName () const override { return handle; }

    virtual VkObjectType GetObjectTypeForName () const override { return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; }

};

} // namespace GVK

#endif
