#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

class Noncopyable {
public:
    Noncopyable ()              = default;
    Noncopyable (Noncopyable&)  = delete;
    Noncopyable (Noncopyable&&) = delete;
    Noncopyable& operator= (Noncopyable&) = delete;
    Noncopyable& operator= (Noncopyable&&) = delete;
};

#endif