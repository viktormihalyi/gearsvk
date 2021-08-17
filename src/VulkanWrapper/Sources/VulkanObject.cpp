#include "VulkanObject.hpp"
#include "VulkanFunctionGetter.hpp"
#include "DeviceExtra.hpp"
#include "VulkanWrapper.hpp"

#include <set>

namespace GVK {

VulkanObject::VulkanObject () = default;


VulkanObject::~VulkanObject () = default;

void VulkanObject::SetName (const DeviceExtra& device, const std::string& value)
{
    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.pNext = nullptr;
    nameInfo.objectType = GetObjectTypeForName ();
    nameInfo.objectHandle = reinterpret_cast<uint64_t> (GetHandleForName ());
    nameInfo.pObjectName = value.c_str ();

    GetVulkanFunction<PFN_vkSetDebugUtilsObjectNameEXT> (device.instance, "vkSetDebugUtilsObjectNameEXT") (device, &nameInfo);

    name = value;
}

}