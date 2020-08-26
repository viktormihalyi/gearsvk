#include "DeviceMemory.hpp"

#include <sstream>

static void LogMemoryOperation (const std::string& operation, const size_t allocationSize, const uint32_t memoryTypeIndex)
{
    static const uint32_t ONE_GB = 1024 * 1024 * 1024;
    static const uint32_t ONE_MB = 1024 * 1024;
    static const uint32_t ONE_KB = 1024;

    std::stringstream allocString;
    allocString << "[DeviceMemory] " << operation << " ";
    if (allocationSize >= ONE_GB) {
        allocString << allocationSize / static_cast<double> (ONE_GB) << " gigabyte(s)";
    } else if (allocationSize >= ONE_MB) {
        allocString << allocationSize / static_cast<double> (ONE_MB) << " megabyte(s)";
    } else if (allocationSize >= ONE_KB) {
        allocString << allocationSize / static_cast<double> (ONE_KB) << " kilobyte(s)";
    } else {
        allocString << allocationSize << " byte(s)";
    }
    allocString << " (idx: " << memoryTypeIndex << ")";
    std::cout << allocString.str () << std::endl;
}


static void LogFree (const size_t allocationSize, const uint32_t memoryTypeIndex)
{
    LogMemoryOperation ("deallocated", allocationSize, memoryTypeIndex);
}


static void LogAllocation (const size_t allocationSize, const uint32_t memoryTypeIndex)
{
    LogMemoryOperation ("allocated", allocationSize, memoryTypeIndex);
}


DeviceMemory::DeviceMemory (VkDevice device, const size_t allocationSize, const uint32_t memoryTypeIndex)
    : device (device)
    , allocationSize (allocationSize)
    , memoryTypeIndex (memoryTypeIndex)
    , handle (VK_NULL_HANDLE)
{
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize       = allocationSize;
    allocInfo.memoryTypeIndex      = memoryTypeIndex;

    if (GVK_ERROR (vkAllocateMemory (device, &allocInfo, nullptr, &handle) != VK_SUCCESS)) {
        throw std::runtime_error ("failed to allocate memory");
    }

#if 0
    LogAllocation (allocationSize, memoryTypeIndex);
#endif
}


DeviceMemory::DeviceMemory (VkDevice device, const Device::AllocateInfo allocateInfo)
    : DeviceMemory (device, allocateInfo.size, allocateInfo.memoryTypeIndex)
{
}


DeviceMemory::~DeviceMemory ()
{
    vkFreeMemory (device, handle, nullptr);
    handle = VK_NULL_HANDLE;

#if 0
    LogFree (allocationSize, memoryTypeIndex);
#endif
}
