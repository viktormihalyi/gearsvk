#ifndef PYSEQUENCE_API_HPP
#define PYSEQUENCE_API_HPP

#ifdef _WIN32
#ifdef PySequence_EXPORTS
#define PYSEQUENCE_API __declspec(dllexport)
#else
#define PYSEQUENCE_API __declspec(dllimport)
#endif
#else
#ifdef PySequence_EXPORTS
#define PYSEQUENCE_API __attribute__ ((__visibility__ ("default")))
#else
#define PYSEQUENCE_API
#endif
#endif


#endif