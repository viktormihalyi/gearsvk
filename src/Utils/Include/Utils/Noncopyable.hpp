#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#include "GVKUtilsAPI.hpp"

class /* GVK_UTILS_API */ Noncopyable {
public:
    Noncopyable ()          = default;
    virtual ~Noncopyable () = default;

    Noncopyable (Noncopyable&)  = delete;
    Noncopyable& operator= (Noncopyable&) = delete;
};

#endif