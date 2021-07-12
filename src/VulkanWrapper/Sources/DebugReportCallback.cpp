#include "DebugReportCallback.hpp"

#include "Utils/Assert.hpp"
#include "Utils/StaticInit.hpp"
#include <iostream>

namespace GVK {
    
static VkBool32 cb (VkDebugReportFlagsEXT      flags,
                    VkDebugReportObjectTypeEXT objectType,
                    uint64_t                   object,
                    size_t                     location,
                    int32_t                    messageCode,
                    const char*                pLayerPrefix,
                    const char*                pMessage,
                    void*                      pUserData)
{
    return VK_FALSE;
}


template<typename FunctionType>
FunctionType GetVulkanFunction (VkInstance instance, const char* functionName)
{
    FunctionType func = reinterpret_cast<FunctionType> (vkGetInstanceProcAddr (instance, functionName));

    if (GVK_ERROR (func == nullptr))
        throw std::runtime_error ("Function not loaded.");

    return func;
}

DebugReportCallback::DebugReportCallback (VkInstance instance)
    : instance (instance)
{
    VkDebugReportCallbackCreateInfoEXT createInfo = {};

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = cb;
    createInfo.pUserData = this;

    VkResult result = GetVulkanFunction<PFN_vkCreateDebugReportCallbackEXT> (instance, "vkCreateDebugReportCallbackEXT") (instance, &createInfo, nullptr, &handle);

    if (GVK_ERROR (result != VK_SUCCESS)) {
        throw std::runtime_error ("Could not create VkDebugReportCallbackEXT.");
    }
}


DebugReportCallback::~DebugReportCallback ()
{
    GetVulkanFunction<PFN_vkDestroyDebugReportCallbackEXT> (instance, "vkDestroyDebugReportCallbackEXT") (instance, handle, nullptr);
    handle = nullptr;
}

} // namespace GVK