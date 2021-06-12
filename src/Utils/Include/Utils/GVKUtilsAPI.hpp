#ifndef GVK_UTILS_API_HPP
#define GVK_UTILS_API_HPP

#ifdef _WIN32
#ifdef Utils_EXPORTS
#define GVK_UTILS_API __declspec(dllexport)
#else
#define GVK_UTILS_API __declspec(dllimport)
#endif
#else
#define GVK_UTILS_API
#endif


#endif