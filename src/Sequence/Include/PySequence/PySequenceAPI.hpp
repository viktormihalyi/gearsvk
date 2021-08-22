#ifndef SEQUENCE_API_HPP
#define SEQUENCE_API_HPP

#ifdef _WIN32
#ifdef PySequence_EXPORTS
#define SEQUENCE_API __declspec(dllexport)
#else
#define SEQUENCE_API __declspec(dllimport)
#endif
#else
#ifdef PySequence_EXPORTS
#define SEQUENCE_API __attribute__ ((__visibility__ ("default")))
#else
#define SEQUENCE_API
#endif
#endif


#endif