#ifndef GEARSVK_API_HPP
#define GEARSVK_API_HPP

#ifdef _WIN32
#ifdef RenderGraph_EXPORTS
#define GVK_RENDERER_API __declspec(dllexport)
#else
#define GVK_RENDERER_API __declspec(dllimport)
#endif
#else
#define GVK_RENDERER_API
#endif


#endif