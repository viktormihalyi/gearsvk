#ifndef GEARSVK_API_HPP
#define GEARSVK_API_HPP

#ifdef _WIN32
#ifdef GEARSVK_EXPORTS
#define GEARSVK_API __declspec(dllexport)
#else
#define GEARSVK_API __declspec(dllimport)
#endif
#else
#define GEARSVK_API
#endif


#endif