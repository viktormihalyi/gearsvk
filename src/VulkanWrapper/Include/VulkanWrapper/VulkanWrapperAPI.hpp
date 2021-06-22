#ifndef VULKANWRAPPER_API_HPP
#define VULKANWRAPPER_API_HPP

#ifdef _WIN32
#ifdef VulkanWrapper_EXPORTS
#define VULKANWRAPPER_API __declspec(dllexport)
#else
#define VULKANWRAPPER_API __declspec(dllimport)
#endif
#else
#ifdef RenderGraph_EXPORTS
#define VULKANWRAPPER_API __attribute__ ((__visibility__ ("default")))
#else
#define VULKANWRAPPER_API
#endif
#endif


#endif