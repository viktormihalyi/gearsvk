#ifndef GEARS_API_HPP
#define GEARS_API_HPP

#ifdef _WIN32
#ifdef GearsModule_EXPORTS
#define GEARS_API __declspec(dllexport)
#else
#define GEARS_API __declspec(dllimport)
#endif
#else
#ifdef GearsModule_EXPORTS
#define GEARS_API __attribute__ ((__visibility__ ("default")))
#else
#define GEARS_API
#endif
#endif

#ifdef _WIN32
#if defined(GearsModule_EXPORTS) || defined(GearsModuleEmbedded_EXPORTS)
#define GEARS_API_TEST __declspec(dllexport)
#else
#define GEARS_API_TEST __declspec(dllimport)
#endif
#else
#if defined(GearsModule_EXPORTS) || defined(GearsModuleEmbedded_EXPORTS)
#define GEARS_API_TEST __attribute__ ((__visibility__ ("default")))
#else
#define GEARS_API_TEST
#endif
#endif


#endif