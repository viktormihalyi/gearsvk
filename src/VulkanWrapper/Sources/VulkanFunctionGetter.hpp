#ifndef VULKANWRAPPER_VULKANFUNCTIONGETTER_HPP
#define VULKANWRAPPER_VULKANFUNCTIONGETTER_HPP

#include <vulkan/vulkan.h>

#include <stdexcept>

// from Utils
#include "Utils/Assert.hpp"


template<typename FunctionType>
FunctionType GetVulkanFunction (VkInstance instance, const char* functionName)
{
    FunctionType func = reinterpret_cast<FunctionType> (vkGetInstanceProcAddr (instance, functionName));

    if (GVK_ERROR (func == nullptr))
        throw std::runtime_error ("Function not loaded.");

    return func;
}

#endif