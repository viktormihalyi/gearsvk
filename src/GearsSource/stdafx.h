// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// reference additional headers your program requires here
#include <memory>
#include <pybind11/pybind11.h>


template<typename T>
struct extract {
    pybind11::object obj;
    bool             ok;

    extract (pybind11::object obj)
        : ok (pybind11::isinstance<T> (obj))
    {
    }

    bool check () { return ok; }

    T operator() () { return obj.cast<T> (); }
};


#include "SourceLocation.hpp"

#define THROW_LOC() \
    throw ::std::runtime_error (::Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString ())

#define LOG_LOC() \
    throw ::std::cout << ::Utils::SourceLocation {__FILE__, __LINE__, __func__}.ToString () << std::endl

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

#define GEARS_SHARED_CREATE_WITH_GETSHAREDPTR_SUB(T)                                     \
private:                                                                                 \
    std::weak_ptr<T> weakPtrForGetSharedPtr;                                             \
                                                                                         \
protected:                                                                               \
    void setWeakPtrForGetSharedPtr (std::weak_ptr<T> w)                                  \
    {                                                                                    \
        weakPtrForGetSharedPtr = w;                                                      \
        __super::setWeakPtrForGetSharedPtr (w);                                          \
    }                                                                                    \
                                                                                         \
public:                                                                                  \
    template<typename... Args>                                                           \
    inline static std::shared_ptr<T> create (Args... args)                               \
    {                                                                                    \
        std::shared_ptr<T> p (new T (args...));                                          \
        p->setWeakPtrForGetSharedPtr (p);                                                \
        return p;                                                                        \
    }                                                                                    \
    inline std::shared_ptr<T> getSharedPtr () { return weakPtrForGetSharedPtr.lock (); } \
    using P  = std::shared_ptr<T>;                                                       \
    using CP = std::shared_ptr<T const>;                                                 \
    using W  = std::weak_ptr<T>

#define GEARS_SHARED_CREATE_WITH_GETSHAREDPTR(T)                                         \
private:                                                                                 \
    std::weak_ptr<T> weakPtrForGetSharedPtr;                                             \
                                                                                         \
protected:                                                                               \
    void setWeakPtrForGetSharedPtr (std::weak_ptr<T> w) { weakPtrForGetSharedPtr = w; }  \
                                                                                         \
public:                                                                                  \
    template<typename... Args>                                                           \
    inline static std::shared_ptr<T> create (Args... args)                               \
    {                                                                                    \
        std::shared_ptr<T> p (new T (args...));                                          \
        p->setWeakPtrForGetSharedPtr (p);                                                \
        return p;                                                                        \
    }                                                                                    \
    inline std::shared_ptr<T> getSharedPtr () { return weakPtrForGetSharedPtr.lock (); } \
    using P  = std::shared_ptr<T>;                                                       \
    using CP = std::shared_ptr<T const>;                                                 \
    using W  = std::weak_ptr<T>;

template<class _Ty, class... _Types>
inline std::unique_ptr<_Ty> make_unique (_Types&&... _Args)
{
    return (std::unique_ptr<_Ty> (new _Ty (std::forward<_Types> (_Args)...)));
}
