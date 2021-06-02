// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// reference additional headers your program requires here
#include <iostream>
#include <memory>
#include <pybind11/pybind11.h>


template<typename T>
struct extract {
    T    casted;
    bool ok;

    extract (pybind11::object obj)
    {
        try {
            casted = obj.cast<T> ();
            ok     = true;
        } catch (pybind11::cast_error&) {
            ok = false;
        }
    }

    bool check () { return ok; }

    T operator() () { return casted; }
};


#include "SourceLocation.hpp"

#define THROW_LOC() \
    throw ::std::runtime_error (::Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString ())

#define LOG_LOC() \
    throw ::std::cout << ::Utils::SourceLocation { __FILE__, __LINE__, __func__ }.ToString () << std::endl

using uint = unsigned int;

#define GEARS_SHARED_CREATE(T)                             \
    template<typename... Args>                             \
    inline static std::shared_ptr<T> create (Args... args) \
    {                                                      \
        return std::shared_ptr<T> (new T (args...));       \
    }                                                      \
    using P  = std::shared_ptr<T>;                         \
    using CP = std::shared_ptr<T const>;                   \
    using W  = std::weak_ptr<T>
