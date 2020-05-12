#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#include "GearsVkAPI.hpp"

class GEARSVK_API Noncopyable {
public:
    Noncopyable ()          = default;
    virtual ~Noncopyable () = default;

    Noncopyable (Noncopyable&)  = delete;
    Noncopyable (Noncopyable&&) = delete;
    Noncopyable& operator= (Noncopyable&) = delete;
    Noncopyable& operator= (Noncopyable&&) = delete;
};

#endif