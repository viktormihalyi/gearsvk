#ifndef RENDERGRAPH_API_HPP
#define RENDERGRAPH_API_HPP

#ifdef _WIN32
#ifdef RenderGraph_EXPORTS
#define GVK_RENDERER_API __declspec(dllexport)
#else
#define GVK_RENDERER_API __declspec(dllimport)
#endif
#else
#ifdef RenderGraph_EXPORTS
#define GVK_RENDERER_API __attribute__ ((__visibility__ ("default")))
#else
#define GVK_RENDERER_API
#endif
#endif


#endif