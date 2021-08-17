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


class /* GVK_UTILS_API */ Nonmovable {
public:
    Nonmovable ()          = default;
    virtual ~Nonmovable () = default;

    Nonmovable (const Nonmovable&) = default;
    Nonmovable& operator= (const Nonmovable&) = default;

    Nonmovable (Nonmovable&&) = delete;
    Nonmovable& operator= (Nonmovable&&) = delete;
};

#endif