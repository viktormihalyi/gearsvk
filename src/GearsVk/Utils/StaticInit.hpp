#ifndef STATICINIT_HPP
#define STATICINIT_HPP

#include "BuildType.hpp"
#include "Noncopyable.hpp"

#include <functional>


class StaticInit : public Noncopyable {
private:
    const bool             hasDtor;
    std::function<void ()> dtorCallback;

public:
    StaticInit (const std::function<void ()>& callback)
        : hasDtor (false)
    {
        callback ();
    }

    ~StaticInit ()
    {
        if (hasDtor) {
            dtorCallback ();
        }
    }
};


class DebugOnlyStaticInit : public Noncopyable {
public:
    DebugOnlyStaticInit (const std::function<void ()>& callback)
    {
        if constexpr (IsDebugBuild) {
            callback ();
        }
    }
};

#endif