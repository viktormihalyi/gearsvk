#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#include "GVKUtilsAPI.hpp"

class /* GVK_UTILS_API */ Noncopyable {
public:
    Noncopyable ()          = default;
    virtual ~Noncopyable () = default;

    Noncopyable (const Noncopyable&) = delete;
    Noncopyable& operator= (const Noncopyable&) = delete;

    Noncopyable (Noncopyable&&) = default;
    Noncopyable& operator= (Noncopyable&&) = default;
};

#endif