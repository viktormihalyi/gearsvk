#ifndef COMPILER_DEFINITIONS_HPP
#define COMPILER_DEFINITIONS_HPP

#ifdef _MSC_VER
#define COMPILER_MSVC
#endif

#ifdef __GNUC__
#define COMPILER_GCC
#endif

#ifdef __clang__
#define COMPILER_CLANG
#endif

#endif