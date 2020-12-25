#ifndef GEARS_API_HPP
#define GEARS_API_HPP

#ifdef _WIN32
#ifdef GEARS_EXPORTS
#define GEARS_API __declspec(dllexport)
#else
#define GEARS_API __declspec(dllimport)
#endif
#else
#define GEARS_API
#endif


#endif