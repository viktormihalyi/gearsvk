#ifndef VULKANWRAPPER_VULKANWRAPPEREXPORT_HPP
#define VULKANWRAPPER_VULKANWRAPPEREXPORT_HPP

#ifdef _WIN32
#ifdef VulkanWrapper_EXPORTS
#define VULKANWRAPPER_DLL_EXPORT __declspec(dllexport)
#else
#define VULKANWRAPPER_DLL_EXPORT __declspec(dllimport)
#endif
#else
#ifdef VulkanWrapper_EXPORTS
#define VULKANWRAPPER_DLL_EXPORT __attribute__ ((__visibility__ ("default")))
#else
#define VULKANWRAPPER_DLL_EXPORT
#endif
#endif


#endif