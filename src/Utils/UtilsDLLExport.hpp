#ifndef UTILSDLLEXPORT_HPP
#define UTILSDLLEXPORT_HPP

#ifdef GEARSVK_UTILS_EXPORTS
#define GEARSVK_UTILS_API __declspec(dllexport)
#else
#define GEARSVK_UTILS_API __declspec(dllimport)
#endif

#endif