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

    inline void Invalidate ()
    {
#ifdef NDEBUG
        value = T ();
#endif
        isValid = false;
    }

    inline const T& Get ()
    {
        if (!isValid) {
            Calculate ();
        }
        return value;
    }

    inline const T& operator* () { return Get (); }

    inline operator const T& () { return Get (); }
};

#endif