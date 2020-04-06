#ifndef CACHE_HPP
#define CACHE_HPP

#include <functional>

template<typename T>
struct Cache {
private:
    T                   value;
    bool                isValid;
    std::function<T ()> calculator;

public:
    Cache (const std::function<T ()>& calculator)
        : value ()
        , isValid (false)
        , calculator (calculator)
    {
    }

    void Calculate ()
    {
        value   = calculator ();
        isValid = true;
    }

    void Invalidate ()
    {
        value   = T ();
        isValid = false;
    }

    const T& Get ()
    {
        if (!isValid) {
            Calculate ();
        }
        return value;
    }

    operator const T& () { return Get (); }
};

#endif