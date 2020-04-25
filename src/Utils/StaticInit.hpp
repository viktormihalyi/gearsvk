#ifndef STATICINIT_HPP
#define STATICINIT_HPP

#include "Noncopyable.hpp"

#include <functional>

// used for global variables

class StaticInit : public Noncopyable {
public:
    StaticInit (const std::function<void ()>& callback)
    {
        callback ();
    }
};

#endif