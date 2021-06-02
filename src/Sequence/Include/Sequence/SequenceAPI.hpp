#ifndef SEQUENCE_API_HPP
#define SEQUENCE_API_HPP

#ifdef _WIN32
#ifdef Sequence_EXPORTS
#define SEQUENCE_API __declspec(dllexport)
#else
#define SEQUENCE_API __declspec(dllimport)
#endif
#else
#define SEQUENCE_API
#endif

#endif